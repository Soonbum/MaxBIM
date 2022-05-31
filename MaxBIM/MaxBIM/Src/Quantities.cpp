#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "Quantities.hpp"

using namespace quantitiesDG;

qElem	qElemInfo;
insulElem	insulElemInfo;
static GS::Array<API_Guid>	elemList;	// �׷�ȭ�� ���� ������ ��������� GUID�� ���� ������

// ���������� ������ �� �ִ� �ȷ�Ʈ�� ���
GSErrCode	placeQuantityPlywood (void)
{
	GSErrCode	err = NoError;

	qElemInfo.dialogID = 0;

	if ((qElemInfo.dialogID == 0) || !DGIsDialogOpen (qElemInfo.dialogID)) {
		qElemInfo.dialogID = DGModelessInit (ACAPI_GetOwnResModule (), 32504, ACAPI_GetOwnResModule (), qElemDlgCallBack, (DGUserData) &qElemInfo, 1);
	}

	return	err;
}

// �ȷ�Ʈ�� ���� �ݹ� �Լ� 1
static short DGCALLBACK qElemDlgCallBack (short message, short dialID, short item, DGUserData userData, DGMessageData /*msgData*/)
{
	qElem		*dlgData = (qElem *) userData;
	API_UCCallbackType	ucb;
	GSErrCode	err = NoError;
	GSErrCode	inputErr;
	bool	regenerate = true;
	short	xx;

	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = L("��������(�ֽ������).gsm");
	double				aParam;
	double				bParam;
	Int32				addParNum;

	char				tempStr [256];
	double				horLen, verLen;
	bool				bValid;

	// ���� �Է� ����
	API_GetPointType	pointInfo;
	double				dx, dy, dz;

	API_Coord3D		p1, p2, p3, p4, p5;
	double			angle1, angle2;

	API_StoryInfo	storyInfo;
	double			storyLevel;
	char			floorName [256];

	
	err = ACAPI_CallUndoableCommand ("�������� �����ϱ�", [&] () -> GSErrCode {
		switch (message) {
			case DG_MSG_INIT:
				// ��
				DGSetItemText (dialID, LABEL_EXPLANATION_QELEM, "3�� Ŭ��: ���簢��, 5�� Ŭ��: â����");

				// ���̾�
				BNZeroMemory (&ucb, sizeof (ucb));
				ucb.dialogID = dialID;
				ucb.type	 = APIUserControlType_Layer;
				ucb.itemID	 = USERCONTROL_QPLYWOOD_LAYER;
				ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
				DGSetItemValLong (dialID, USERCONTROL_QPLYWOOD_LAYER, 1);

				// �˾� ��Ʈ�� (�������� Ÿ��)
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, "��");
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, "���(����)");
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, "���(��ü)");
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, "��ü(����)");
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, "��ü(�ܺ�)");
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, "��ü(�պ�)");
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, "��ü(�Ķ���)");
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, "��ü(�����)");
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, "������(����)");
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, "������(RC)");
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, "������(��ũ)");
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, "������(����)");
				DGPopUpInsertItem (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialID, POPUP_QPLYWOOD_TYPE, DG_POPUP_BOTTOM, "���");

				// �� ���� ����
				BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
				ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
				for (xx = 0 ; xx <= (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
					sprintf (floorName, "%d. %s", storyInfo.data [0][xx].index, storyInfo.data [0][xx].name);
					DGPopUpInsertItem (dialID, POPUP_FLOOR_QELEM, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialID, POPUP_FLOOR_QELEM, DG_POPUP_BOTTOM, floorName);
				}
				for (xx = 0 ; xx <= (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
					if (storyInfo.data [0][xx].index == 0) {
						DGPopUpSelectItem (dialID, POPUP_FLOOR_QELEM, xx+1);
					}
				}
				qElemInfo.floorInd = storyInfo.data [0][DGPopUpGetSelected (dialID, POPUP_FLOOR_QELEM) - 1].index;
				BMKillHandle ((GSHandle *) &storyInfo.data);

				// ��ư
				DGSetItemText (dialID, BUTTON_DRAW_RECT_QELEM, "�������� �׸���\n(���簢��)");
				DGSetItemText (dialID, BUTTON_DRAW_WINDOW_QELEM, "�������� �׸���\n(â����)");

				// ���̾� ���� ����
				qElemInfo.layerInd = (short)DGGetItemValLong (dialID, USERCONTROL_QPLYWOOD_LAYER);

				// �з� ���� ����
				if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_QPLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_QPLYWOOD_TYPE))).ToCStr ().Get (), "��") == 0) {
					strcpy (qElemInfo.m_type, "��");
					qElemInfo.panel_mat = 78;
				} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_QPLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_QPLYWOOD_TYPE))).ToCStr ().Get (), "���(����)") == 0) {
					strcpy (qElemInfo.m_type, "���(����)");
					qElemInfo.panel_mat = 20;
				} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_QPLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_QPLYWOOD_TYPE))).ToCStr ().Get (), "���(��ü)") == 0) {
					strcpy (qElemInfo.m_type, "���(��ü)");
					qElemInfo.panel_mat = 77;
				} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_QPLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_QPLYWOOD_TYPE))).ToCStr ().Get (), "��ü(����)") == 0) {
					strcpy (qElemInfo.m_type, "��ü(����)");
					qElemInfo.panel_mat = 75;
				} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_QPLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_QPLYWOOD_TYPE))).ToCStr ().Get (), "��ü(�ܺ�)") == 0) {
					strcpy (qElemInfo.m_type, "��ü(�ܺ�)");
					qElemInfo.panel_mat = 76;
				} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_QPLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_QPLYWOOD_TYPE))).ToCStr ().Get (), "��ü(�պ�)") == 0) {
					strcpy (qElemInfo.m_type, "��ü(�պ�)");
					qElemInfo.panel_mat = 72;
				} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_QPLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_QPLYWOOD_TYPE))).ToCStr ().Get (), "��ü(�Ķ���)") == 0) {
					strcpy (qElemInfo.m_type, "��ü(�Ķ���)");
					qElemInfo.panel_mat = 32;
				} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_QPLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_QPLYWOOD_TYPE))).ToCStr ().Get (), "��ü(�����)") == 0) {
					strcpy (qElemInfo.m_type, "��ü(�����)");
					qElemInfo.panel_mat = 12;
				} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_QPLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_QPLYWOOD_TYPE))).ToCStr ().Get (), "������(����)") == 0) {
					strcpy (qElemInfo.m_type, "�����(����)");
					qElemInfo.panel_mat = 66;
				} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_QPLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_QPLYWOOD_TYPE))).ToCStr ().Get (), "������(RC)") == 0) {
					strcpy (qElemInfo.m_type, "�����(RC)");
					qElemInfo.panel_mat = 100;
				} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_QPLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_QPLYWOOD_TYPE))).ToCStr ().Get (), "������(��ũ)") == 0) {
					strcpy (qElemInfo.m_type, "�����(��ũ)");
					qElemInfo.panel_mat = 99;
				} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_QPLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_QPLYWOOD_TYPE))).ToCStr ().Get (), "������(����)") == 0) {
					strcpy (qElemInfo.m_type, "�����(����)");
					qElemInfo.panel_mat = 3;
				} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_QPLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_QPLYWOOD_TYPE))).ToCStr ().Get (), "���") == 0) {
					strcpy (qElemInfo.m_type, "���");
					qElemInfo.panel_mat = 73;
				}
 
				// �ȷ�Ʈ â ���
				if (ACAPI_RegisterModelessWindow (dialID, qElemPaletteAPIControlCallBack,
							API_PalEnabled_FloorPlan + API_PalEnabled_Section + API_PalEnabled_Elevation +
							API_PalEnabled_InteriorElevation + API_PalEnabled_Detail + API_PalEnabled_Worksheet + API_PalEnabled_3D + API_PalEnabled_Layout) != NoError)
					DBPrintf ("Test:: ACAPI_RegisterModelessWindow failed\n");

				break;

			case DG_MSG_CHANGE:

				switch (item) {
					case USERCONTROL_QPLYWOOD_LAYER:
						qElemInfo.layerInd = (short)DGGetItemValLong (dialID, USERCONTROL_QPLYWOOD_LAYER);
						break;

					case POPUP_QPLYWOOD_TYPE:
						if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_QPLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_QPLYWOOD_TYPE))).ToCStr ().Get (), "��") == 0) {
							strcpy (qElemInfo.m_type, "��");
							qElemInfo.panel_mat = 78;
						} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_QPLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_QPLYWOOD_TYPE))).ToCStr ().Get (), "���(����)") == 0) {
							strcpy (qElemInfo.m_type, "���(����)");
							qElemInfo.panel_mat = 20;
						} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_QPLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_QPLYWOOD_TYPE))).ToCStr ().Get (), "���(��ü)") == 0) {
							strcpy (qElemInfo.m_type, "���(��ü)");
							qElemInfo.panel_mat = 77;
						} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_QPLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_QPLYWOOD_TYPE))).ToCStr ().Get (), "��ü(����)") == 0) {
							strcpy (qElemInfo.m_type, "��ü(����)");
							qElemInfo.panel_mat = 75;
						} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_QPLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_QPLYWOOD_TYPE))).ToCStr ().Get (), "��ü(�ܺ�)") == 0) {
							strcpy (qElemInfo.m_type, "��ü(�ܺ�)");
							qElemInfo.panel_mat = 76;
						} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_QPLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_QPLYWOOD_TYPE))).ToCStr ().Get (), "��ü(�պ�)") == 0) {
							strcpy (qElemInfo.m_type, "��ü(�պ�)");
							qElemInfo.panel_mat = 72;
						} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_QPLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_QPLYWOOD_TYPE))).ToCStr ().Get (), "��ü(�Ķ���)") == 0) {
							strcpy (qElemInfo.m_type, "��ü(�Ķ���)");
							qElemInfo.panel_mat = 32;
						} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_QPLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_QPLYWOOD_TYPE))).ToCStr ().Get (), "��ü(�����)") == 0) {
							strcpy (qElemInfo.m_type, "��ü(�����)");
							qElemInfo.panel_mat = 12;
						} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_QPLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_QPLYWOOD_TYPE))).ToCStr ().Get (), "������(����)") == 0) {
							strcpy (qElemInfo.m_type, "�����(����)");
							qElemInfo.panel_mat = 66;
						} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_QPLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_QPLYWOOD_TYPE))).ToCStr ().Get (), "������(RC)") == 0) {
							strcpy (qElemInfo.m_type, "�����(RC)");
							qElemInfo.panel_mat = 100;
						} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_QPLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_QPLYWOOD_TYPE))).ToCStr ().Get (), "������(��ũ)") == 0) {
							strcpy (qElemInfo.m_type, "�����(��ũ)");
							qElemInfo.panel_mat = 99;
						} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_QPLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_QPLYWOOD_TYPE))).ToCStr ().Get (), "������(����)") == 0) {
							strcpy (qElemInfo.m_type, "�����(����)");
							qElemInfo.panel_mat = 3;
						} else if (my_strcmp (DGPopUpGetItemText (dialID, POPUP_QPLYWOOD_TYPE, static_cast<short>(DGGetItemValLong (dialID, POPUP_QPLYWOOD_TYPE))).ToCStr ().Get (), "���") == 0) {
							strcpy (qElemInfo.m_type, "���");
							qElemInfo.panel_mat = 73;
						}

						break;

					case POPUP_FLOOR_QELEM:
						// �� ���� ����
						BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
						ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
						qElemInfo.floorInd = storyInfo.data [0][DGPopUpGetSelected (dialID, POPUP_FLOOR_QELEM) - 1].index;
						BMKillHandle ((GSHandle *) &storyInfo.data);

						break;
				}

				break;

			case DG_MSG_CLICK:
				switch (item) {
					case BUTTON_DRAW_RECT_QELEM:
						do {
							BNZeroMemory (&pointInfo, sizeof (API_GetPointType));

							CHCopyC ("�ٰ����� 1��° ���(���ϴ�)�� Ŭ���Ͻʽÿ�.", pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p1.x = pointInfo.pos.x;
							p1.y = pointInfo.pos.y;
							p1.z = pointInfo.pos.z;

							CHCopyC ("�ٰ����� 2��° ���(���ϴ�)�� Ŭ���Ͻʽÿ�.", pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p2.x = pointInfo.pos.x;
							p2.y = pointInfo.pos.y;
							p2.z = pointInfo.pos.z;

							CHCopyC ("�ٰ����� 3��° ���(����)�� Ŭ���Ͻʽÿ�.", pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p3.x = pointInfo.pos.x;
							p3.y = pointInfo.pos.y;
							p3.z = pointInfo.pos.z;

							// ��ü �ε�
							BNZeroMemory (&elem, sizeof (API_Element));
							BNZeroMemory (&memo, sizeof (API_ElementMemo));
							BNZeroMemory (&libPart, sizeof (libPart));
							GS::ucscpy (libPart.file_UName, gsmName);
							err = ACAPI_LibPart_Search (&libPart, false);
							if (libPart.location != NULL)
								delete libPart.location;
							if (err != NoError)
								break;

							ACAPI_LibPart_Get (&libPart);

							elem.header.typeID = API_ObjectID;
							elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

							ACAPI_Element_GetDefaults (&elem, &memo);
							ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

							// �� ����
							BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
							ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
							for (xx = 0 ; xx <= (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
								if (storyInfo.data [0][xx].index == qElemInfo.floorInd) {
									storyLevel = storyInfo.data [0][xx].level;
									break;
								}
							}
							BMKillHandle ((GSHandle *) &storyInfo.data);

							// ���̺귯���� �Ķ���� �� �Է� (����)
							elem.header.floorInd = qElemInfo.floorInd;
							elem.object.reflected = false;
							elem.object.libInd = libPart.index;
							elem.object.xRatio = aParam;
							elem.object.yRatio = bParam;
							elem.header.layer = qElemInfo.layerInd;
							elem.object.level = storyLevel;

							setParameterByName (&memo, "m_type", qElemInfo.m_type);
							setParameterByName (&memo, "PANEL_MAT", qElemInfo.panel_mat);

							// ���ǵβ� (���ڿ�)
							strcpy (tempStr, "12");		// 12mm
							setParameterByName (&memo, "m_size", tempStr);

							// ǰ��
							strcpy (tempStr, "���Ǹ���");
							setParameterByName (&memo, "m_size2", tempStr);

							dx = p2.x - p1.x;
							dy = p2.y - p1.y;

							elem.object.angle = atan2 (dy, dx);
							elem.object.pos.x = p1.x;
							elem.object.pos.y = p1.y;
							elem.object.level = p1.z;
							elem.object.level -= storyLevel;

							// ���α���
							horLen = GetDistance (p1, p2);
							setParameterByName (&memo, "NO_WD", horLen);
							elem.object.xRatio = horLen;

							// ���α���
							verLen = GetDistance (p2, p3);
							setParameterByName (&memo, "no_lg1", verLen);
							elem.object.yRatio = verLen;

							bValid = false;
							strcpy (tempStr, "���� �����");	// �⺻��

							// ��ġ��ġ
							strcpy (tempStr, "��԰�");
							setParameterByName (&memo, "m_size1", tempStr);

							// p1, p2, p3 ���� ���� ��� ���� ��
							if ((abs (p1.z - p2.z) < EPS) && (abs (p2.z - p3.z) < EPS)) {
								dx = p2.x - p1.x;
								dy = p2.y - p1.y;
								angle1 = RadToDegree (atan2 (dy, dx));

								dx = p3.x - p2.x;
								dy = p3.y - p2.y;
								angle2 = RadToDegree (atan2 (dy, dx));

								// p2-p3 ���� ������ p2-p1 ���� �������� 90�� ŭ
								if (abs (angle2 - angle1 - 90) < EPS) {
									strcpy (tempStr, "�ٴڵ���");
									bValid = true;
								}

								// p2-p3 ���� ������ p2-p1 ���� �������� 90�� ����
								if (abs (angle1 - angle2 - 90) < EPS) {
									strcpy (tempStr, "�ٴڱ��");
									moveIn3D ('y', elem.object.angle, -verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
									bValid = true;
								}
							// p1, p2 ���� ���� ���� p3 ���� ���� �ٸ� ��
							} else {
								// p2�� p3�� x, y ��ǥ�� ����
								if ((abs (p2.x - p3.x) < EPS) && (abs (p2.y - p3.y) < EPS)) {
									// ???
									strcpy (tempStr, "���� �����");
									setParameterByName (&memo, "ZZYZX", verLen);
									bValid = true;
								
								// p2�� p3�� x, y ��ǥ�� �ٸ�
								} else {
									strcpy (tempStr, "��缳ġ");
									bValid = true;

									// ��ġ����: asin ((p3.z - p2.z) / (p3�� p2 ���� �Ÿ�))
									// ��ġ����: acos ((p3�� p2 ���� ��� ���� �Ÿ�) / (p3�� p2 ���� �Ÿ�))
									// ��ġ����: atan2 ((p3.z - p2.z) / (p3�� p2 ���� ��� ���� �Ÿ�))
									dx = GetDistance (p2.x, p2.y, p3.x, p3.y);
									dy = abs (p3.z - p2.z);
									dz = verLen;
									setParameterByName (&memo, "cons_ang", DegreeToRad (180.0) - acos (dx/dz));
								}
							}
							setParameterByName (&memo, "CONS_DR", tempStr);

							// ��ü ��ġ
							if ((horLen > EPS) && (verLen > EPS) && (bValid == true)) {
								ACAPI_Element_Create (&elem, &memo);
							}

							ACAPI_DisposeElemMemoHdls (&memo);

							// ȭ�� ���ΰ�ħ
							ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
							ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

						} while (inputErr != APIERR_CANCEL);

						break;

					case BUTTON_DRAW_WINDOW_QELEM:
						do {
							BNZeroMemory (&pointInfo, sizeof (API_GetPointType));

							CHCopyC ("�ٰ����� 1��° ���(���ϴ�)�� Ŭ���Ͻʽÿ�.", pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p1.x = pointInfo.pos.x;
							p1.y = pointInfo.pos.y;
							p1.z = pointInfo.pos.z;

							CHCopyC ("�ٰ����� 2��° ���(���ϴ�)�� Ŭ���Ͻʽÿ�.", pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p2.x = pointInfo.pos.x;
							p2.y = pointInfo.pos.y;
							p2.z = pointInfo.pos.z;

							CHCopyC ("�ٰ����� 3��° ���(����)�� Ŭ���Ͻʽÿ�.", pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p3.x = pointInfo.pos.x;
							p3.y = pointInfo.pos.y;
							p3.z = pointInfo.pos.z;

							CHCopyC ("�ٰ����� 4��° ���(������ ���ϴ�)�� Ŭ���Ͻʽÿ�.", pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p4.x = pointInfo.pos.x;
							p4.y = pointInfo.pos.y;
							p4.z = pointInfo.pos.z;

							CHCopyC ("�ٰ����� 5��° ���(������ ����)�� Ŭ���Ͻʽÿ�.", pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p5.x = pointInfo.pos.x;
							p5.y = pointInfo.pos.y;
							p5.z = pointInfo.pos.z;

							// ��ü �ε�
							BNZeroMemory (&elem, sizeof (API_Element));
							BNZeroMemory (&memo, sizeof (API_ElementMemo));
							BNZeroMemory (&libPart, sizeof (libPart));
							GS::ucscpy (libPart.file_UName, gsmName);
							err = ACAPI_LibPart_Search (&libPart, false);
							if (libPart.location != NULL)
								delete libPart.location;
							if (err != NoError)
								break;

							ACAPI_LibPart_Get (&libPart);

							elem.header.typeID = API_ObjectID;
							elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

							ACAPI_Element_GetDefaults (&elem, &memo);
							ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

							// ���̺귯���� �Ķ���� �� �Է� (����)
							elem.header.floorInd = qElemInfo.floorInd;
							elem.object.reflected = false;
							elem.object.libInd = libPart.index;
							elem.object.xRatio = aParam;
							elem.object.yRatio = bParam;
							elem.header.layer = qElemInfo.layerInd;

							setParameterByName (&memo, "m_type", qElemInfo.m_type);
							setParameterByName (&memo, "PANEL_MAT", qElemInfo.panel_mat);

							// ���ǵβ� (���ڿ�)
							strcpy (tempStr, "12");		// 12mm
							setParameterByName (&memo, "m_size", tempStr);

							// ǰ��
							strcpy (tempStr, "���Ǹ���");
							setParameterByName (&memo, "m_size2", tempStr);

							dx = p2.x - p1.x;
							dy = p2.y - p1.y;

							elem.object.angle = atan2 (dy, dx);
							elem.object.pos.x = p1.x;
							elem.object.pos.y = p1.y;
							elem.object.level = p1.z;
							elem.object.level -= storyLevel;

							// ���α���
							horLen = GetDistance (p1, p2);
							setParameterByName (&memo, "NO_WD", horLen);
							elem.object.xRatio = horLen;

							// ���α���
							verLen = GetDistance (p2, p3);
							setParameterByName (&memo, "no_lg1", verLen);
							elem.object.yRatio = verLen;

							bValid = false;
							strcpy (tempStr, "���� �����");	// �⺻��

							// ��ġ��ġ
							strcpy (tempStr, "â����");
							setParameterByName (&memo, "m_size1", tempStr);

							// ��ġ����
							API_Coord3D		origin;
							API_Coord3D		target_before, target_after;
								
							// p1, p2, p3 ���� ���� ��� ���� ��
							if ((abs (p1.z - p2.z) < EPS) && (abs (p2.z - p3.z) < EPS)) {
								dx = p2.x - p1.x;
								dy = p2.y - p1.y;
								angle1 = RadToDegree (atan2 (dy, dx));

								dx = p3.x - p2.x;
								dy = p3.y - p2.y;
								angle2 = RadToDegree (atan2 (dy, dx));

								// p2-p3 ���� ������ p2-p1 ���� �������� 90�� ŭ
								if (abs (angle2 - angle1 - 90) < EPS) {
									origin = p1;
									target_before = p4;
									target_after = getUnrotatedPoint (target_before, origin, -RadToDegree (elem.object.angle));

									setParameterByName (&memo, "vw1", abs (origin.x - target_after.x));				// ��1: vw1
									setParameterByName (&memo, "vh1", abs (origin.y - target_after.y));				// ����1: vh1

									target_before = p5;
									target_after = getUnrotatedPoint (target_before, origin, -RadToDegree (elem.object.angle));

									setParameterByName (&memo, "vw2", abs (origin.x + horLen - target_after.x));	// ��2: vw2
									setParameterByName (&memo, "vh2", abs (origin.y + verLen - target_after.y));	// ����2: vh2

									strcpy (tempStr, "�ٴڵ���");
									bValid = true;
								}

								// p2-p3 ���� ������ p2-p1 ���� �������� 90�� ����
								if (abs (angle1 - angle2 - 90) < EPS) {
									origin = p1;
									target_before = p4;
									target_after = getUnrotatedPoint (target_before, origin, -RadToDegree (elem.object.angle));

									setParameterByName (&memo, "vw1", abs (origin.x - target_after.x));				// ��1: vw1
									setParameterByName (&memo, "vh2", abs (origin.y - target_after.y));				// ����2: vh2

									target_before = p5;
									target_after = getUnrotatedPoint (target_before, origin, -RadToDegree (elem.object.angle));

									setParameterByName (&memo, "vw2", abs (origin.x + horLen - target_after.x));	// ��2: vw2
									setParameterByName (&memo, "vh1", verLen - abs (origin.y - target_after.y));	// ����1: vh1

									strcpy (tempStr, "�ٴڱ��");
									moveIn3D ('y', elem.object.angle, -verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
									bValid = true;
								}
								
							// p1, p2 ���� ���� ���� p3 ���� ���� �ٸ� ��
							} else {
								if ((abs (p2.x - p3.x) < EPS) && (abs (p2.y - p3.y) < EPS)) {
									// p2�� p3�� x, y ��ǥ�� ����
									strcpy (tempStr, "���� �����");
									setParameterByName (&memo, "ZZYZX", verLen);
									bValid = true;

									origin = p1;
									target_before = p4;
									target_after = getUnrotatedPoint (target_before, origin, -RadToDegree (elem.object.angle));

									setParameterByName (&memo, "vw1", target_after.x - origin.x);						// ��1: vw1
									setParameterByName (&memo, "vh1", target_after.z - origin.z);						// ����1: vh1

									target_before = p5;
									target_after = getUnrotatedPoint (target_before, origin, -RadToDegree (elem.object.angle));

									setParameterByName (&memo, "vw2", origin.x + horLen - target_after.x);				// ��2: vw2
									setParameterByName (&memo, "vh2", origin.z + verLen - target_after.z);				// ����2: vh2
								}
							}
							setParameterByName (&memo, "CONS_DR", tempStr);

							// ��ü ��ġ
							if ((horLen > EPS) && (verLen > EPS) && (bValid == true)) {
								ACAPI_Element_Create (&elem, &memo);
							}

							ACAPI_DisposeElemMemoHdls (&memo);

							// ȭ�� ���ΰ�ħ
							ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
							ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

						} while (inputErr != APIERR_CANCEL);

						break;

					case DG_CLOSEBOX:
						return item;	// �̰��� DG_MSG_CLOSE �޽����� ����
				}
				break;

			case DG_MSG_DOUBLECLICK:
				break;

			case DG_MSG_CLOSE:
				ACAPI_UnregisterModelessWindow (dlgData->dialogID);
				dlgData->dialogID = 0;
				break;
		}

		return err;
	});

	return 0;
}

// �ȷ�Ʈ�� ���� �ݹ� �Լ� 2
static GSErrCode __ACENV_CALL	qElemPaletteAPIControlCallBack (Int32 referenceID, API_PaletteMessageID messageID, GS::IntPtr /*param*/)
{
	if (referenceID == qElemInfo.dialogID) {
		switch (messageID) {
			case APIPalMsg_ClosePalette:		DGModelessClose (qElemInfo.dialogID);
												break;

			case APIPalMsg_HidePalette_Begin:	break;
			case APIPalMsg_HidePalette_End:		break;

			case APIPalMsg_DisableItems_Begin:	break;
			case APIPalMsg_DisableItems_End:	break;
			case APIPalMsg_OpenPalette:			break;
			default:							break;
		}
	}

	return NoError;
}


// �ܿ��縦 ������ �� �ִ� �ȷ�Ʈ�� ���
GSErrCode	placeInsulation (void)
{
	GSErrCode	err = NoError;

	insulElemInfo.dialogID = 0;

	if ((insulElemInfo.dialogID == 0) || !DGIsDialogOpen (insulElemInfo.dialogID)) {
		insulElemInfo.dialogID = DGModelessInit (ACAPI_GetOwnResModule (), 32507, ACAPI_GetOwnResModule (), insulElemDlgCallBack, (DGUserData) &insulElemInfo, 1);
	}

	return	err;
}

// �ȷ�Ʈ�� ���� �ݹ� �Լ� 1
static short DGCALLBACK insulElemDlgCallBack (short message, short dialID, short item, DGUserData userData, DGMessageData /*msgData*/)
{
	insulElem		*dlgData = (insulElem *) userData;
	API_UCCallbackType	ucb;
	GSErrCode	err = NoError;
	GSErrCode	inputErr;
	bool	regenerate = true;

	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = L("�ܿ���v1.0.gsm");
	double				aParam;
	double				bParam;
	Int32				addParNum;

	short				xx, yy;
	short				totalXX, totalYY;
	double				horLen, verLen;
	double				remainHorLen, remainVerLen;

	// ���� �Է� ����
	API_GetPointType	pointInfo;
	double				dx, dy, dz;

	// �� 3���� ��ġ�� ������ ����
	API_Coord3D		p1, p2, p3;
	double			angle1, angle2;

	// �� ����
	API_StoryInfo	storyInfo;
	double			storyLevel;
	char			floorName [256];

	
	err = ACAPI_CallUndoableCommand ("�ܿ��� �����ϱ�", [&] () -> GSErrCode {
		switch (message) {
			case DG_MSG_INIT:
				// ��
				DGSetItemText (dialID, LABEL_EXPLANATION_INS, "3�� Ŭ��: ���簢��");
				DGSetItemText (dialID, LABEL_INSULATION_THK, "�β�");
				DGSetItemText (dialID, LABEL_INS_HORLEN, "����");
				DGSetItemText (dialID, LABEL_INS_VERLEN, "����");
				DGSetItemText (dialID, LABEL_FLOOR_INS, "�� ����");

				// üũ�ڽ�
				DGSetItemText (dialID, CHECKBOX_INS_LIMIT_SIZE, "����/���� ũ�� ����");
				DGSetItemValLong (dialID, CHECKBOX_INS_LIMIT_SIZE, TRUE);

				// Edit ��Ʈ��
				DGSetItemValDouble (dialID, EDITCONTROL_INSULATION_THK, 0.120);
				DGSetItemValDouble (dialID, EDITCONTROL_INS_HORLEN, 0.900);
				DGSetItemValDouble (dialID, EDITCONTROL_INS_VERLEN, 1.800);

				// ���̾�
				BNZeroMemory (&ucb, sizeof (ucb));
				ucb.dialogID = dialID;
				ucb.type	 = APIUserControlType_Layer;
				ucb.itemID	 = USERCONTROL_INSULATION_LAYER;
				ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
				DGSetItemValLong (dialID, USERCONTROL_INSULATION_LAYER, 1);

				// ��ư
				DGSetItemText (dialID, BUTTON_DRAW_INSULATION, "�ܿ��� �׸���");

				// �� ���� ����
				BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
				ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
				for (xx = 0 ; xx <= (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
					sprintf (floorName, "%d. %s", storyInfo.data [0][xx].index, storyInfo.data [0][xx].name);
					DGPopUpInsertItem (dialID, POPUP_FLOOR_INS, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialID, POPUP_FLOOR_INS, DG_POPUP_BOTTOM, floorName);
				}
				for (xx = 0 ; xx <= (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
					if (storyInfo.data [0][xx].index == 0) {
						DGPopUpSelectItem (dialID, POPUP_FLOOR_INS, xx+1);
					}
				}
				insulElemInfo.floorInd = storyInfo.data [0][DGPopUpGetSelected (dialID, POPUP_FLOOR_INS) - 1].index;
				BMKillHandle ((GSHandle *) &storyInfo.data);

				// ���̾� ���� ����
				insulElemInfo.layerInd = (short)DGGetItemValLong (dialID, USERCONTROL_INSULATION_LAYER);

				// �β�, ����, ���� ����
				insulElemInfo.thk = DGGetItemValDouble (dialID, EDITCONTROL_INSULATION_THK);
				insulElemInfo.maxHorLen = DGGetItemValDouble (dialID, EDITCONTROL_INS_HORLEN);
				insulElemInfo.maxVerLen = DGGetItemValDouble (dialID, EDITCONTROL_INS_VERLEN);
				if (DGGetItemValLong (dialID, CHECKBOX_INS_LIMIT_SIZE) == TRUE)
					insulElemInfo.bLimitSize = true;
				else
					insulElemInfo.bLimitSize = false;
 
				// �ȷ�Ʈ â ���
				if (ACAPI_RegisterModelessWindow (dialID, insulElemPaletteAPIControlCallBack,
							API_PalEnabled_FloorPlan + API_PalEnabled_Section + API_PalEnabled_Elevation +
							API_PalEnabled_InteriorElevation + API_PalEnabled_Detail + API_PalEnabled_Worksheet + API_PalEnabled_3D + API_PalEnabled_Layout) != NoError)
					DBPrintf ("Test:: ACAPI_RegisterModelessWindow failed\n");

				break;

			case DG_MSG_CHANGE:
				switch (item) {
					case CHECKBOX_INS_LIMIT_SIZE:
						if (DGGetItemValLong (dialID, CHECKBOX_INS_LIMIT_SIZE) == TRUE) {
							DGEnableItem (dialID, EDITCONTROL_INS_HORLEN);
							DGEnableItem (dialID, EDITCONTROL_INS_VERLEN);
						} else {
							DGDisableItem (dialID, EDITCONTROL_INS_HORLEN);
							DGDisableItem (dialID, EDITCONTROL_INS_VERLEN);
						}
						break;
				}

				// �� ���� ����
				BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
				ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
				insulElemInfo.floorInd = storyInfo.data [0][DGPopUpGetSelected (dialID, POPUP_FLOOR_INS) - 1].index;
				BMKillHandle ((GSHandle *) &storyInfo.data);

				// ���̾� ���� ����
				insulElemInfo.layerInd = (short)DGGetItemValLong (dialID, USERCONTROL_INSULATION_LAYER);

				// �β�, ����, ���� ����
				insulElemInfo.thk = DGGetItemValDouble (dialID, EDITCONTROL_INSULATION_THK);
				insulElemInfo.maxHorLen = DGGetItemValDouble (dialID, EDITCONTROL_INS_HORLEN);
				insulElemInfo.maxVerLen = DGGetItemValDouble (dialID, EDITCONTROL_INS_VERLEN);
				if (DGGetItemValLong (dialID, CHECKBOX_INS_LIMIT_SIZE) == TRUE)
					insulElemInfo.bLimitSize = true;
				else
					insulElemInfo.bLimitSize = false;
 
				break;

			case DG_MSG_CLICK:
				switch (item) {
					case BUTTON_DRAW_INSULATION:
						do {
							BNZeroMemory (&pointInfo, sizeof (API_GetPointType));

							CHCopyC ("�ٰ����� 1��° ���(���ϴ�)�� Ŭ���Ͻʽÿ�.", pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p1.x = pointInfo.pos.x;
							p1.y = pointInfo.pos.y;
							p1.z = pointInfo.pos.z;

							CHCopyC ("�ٰ����� 2��° ���(���ϴ�)�� Ŭ���Ͻʽÿ�.", pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p2.x = pointInfo.pos.x;
							p2.y = pointInfo.pos.y;
							p2.z = pointInfo.pos.z;

							CHCopyC ("�ٰ����� 3��° ���(����)�� Ŭ���Ͻʽÿ�.", pointInfo.prompt);
							inputErr = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
							if (inputErr != NoError)	break;
							p3.x = pointInfo.pos.x;
							p3.y = pointInfo.pos.y;
							p3.z = pointInfo.pos.z;

							// ���� ���
							dx = p2.x - p1.x;
							dy = p2.y - p1.y;
							angle1 = RadToDegree (atan2 (dy, dx));

							dx = p3.x - p2.x;
							dy = p3.y - p2.y;
							angle2 = RadToDegree (atan2 (dy, dx));

							// ��ü �ε�
							BNZeroMemory (&elem, sizeof (API_Element));
							BNZeroMemory (&memo, sizeof (API_ElementMemo));
							BNZeroMemory (&libPart, sizeof (libPart));
							GS::ucscpy (libPart.file_UName, gsmName);
							err = ACAPI_LibPart_Search (&libPart, false);
							if (libPart.location != NULL)
								delete libPart.location;
							if (err != NoError)
								break;

							ACAPI_LibPart_Get (&libPart);

							elem.header.typeID = API_ObjectID;
							elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

							ACAPI_Element_GetDefaults (&elem, &memo);
							ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

							// �� ����
							BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
							ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
							for (xx = 0 ; xx <= (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
								if (storyInfo.data [0][xx].index == insulElemInfo.floorInd) {
									storyLevel = storyInfo.data [0][xx].level;
									break;
								}
							}
							BMKillHandle ((GSHandle *) &storyInfo.data);

							// ���̺귯���� �Ķ���� �� �Է� (����)
							elem.header.floorInd = insulElemInfo.floorInd;
							elem.object.libInd = libPart.index;
							elem.object.reflected = false;
							elem.object.xRatio = aParam;
							elem.object.yRatio = bParam;
							elem.header.layer = insulElemInfo.layerInd;

							setParameterByName (&memo, "bRestrictSize", insulElemInfo.bLimitSize);	// ũ�� ���� ����
							setParameterByName (&memo, "maxHorLen", insulElemInfo.maxHorLen);		// �ܿ��� �ִ� ���� ����
							setParameterByName (&memo, "maxVerLen", insulElemInfo.maxVerLen);		// �ܿ��� �ִ� ���� ����
							setParameterByName (&memo, "ZZYZX", insulElemInfo.thk);					// �ܿ��� �β�
							setParameterByName (&memo, "bLShape", 0.0);								// �ܿ���� ���簢�� ���·� ����ϹǷ� L������ Off

							// ����/���� ũ�� ������ true�� ��
							if (insulElemInfo.bLimitSize == true) {
								// p1-p2�� p2-p3���� ��ų� ���� ��� (���� ����)
								if (GetDistance (p1, p2) >= GetDistance (p2, p3)) {
									// ���� ũ�� >= ���� ũ�� (�״�� ��ġ)
									if (insulElemInfo.maxHorLen >= insulElemInfo.maxVerLen) {
										// p1, p2, p3 ���� ���� ��� ���� ��
										if ((abs (p1.z - p2.z) < EPS) && (abs (p2.z - p3.z) < EPS)) {
											// p2-p3 ���� ������ p2-p1 ���� �������� 90�� ŭ (�ٴڸ� ����)
											if (abs (angle2 - angle1 - 90) < EPS) {
												// ��ü�� �ʱ� ��ġ �� ����
												elem.object.pos.x = p1.x;
												elem.object.pos.y = p1.y;
												elem.object.level = p1.z;
												elem.object.level -= storyLevel;
												elem.object.angle = DegreeToRad (angle1);

												remainHorLen = GetDistance (p1, p2);
												remainVerLen = GetDistance (p2, p3);

												totalXX = (short)floor (remainHorLen / insulElemInfo.maxHorLen);
												totalYY = (short)floor (remainVerLen / insulElemInfo.maxVerLen);

												for (xx = 0 ; xx < totalXX+1 ; ++xx) {
													for (yy = 0 ; yy < totalYY+1 ; ++yy) {
														(remainHorLen > insulElemInfo.maxHorLen) ? horLen = insulElemInfo.maxHorLen : horLen = remainHorLen;
														(remainVerLen > insulElemInfo.maxVerLen) ? verLen = insulElemInfo.maxVerLen : verLen = remainVerLen;
														
														setParameterByName (&memo, "angX", DegreeToRad (0.0));
														setParameterByName (&memo, "angY", DegreeToRad (0.0));
														setParameterByName (&memo, "bVerticalCut", 0.0);
														setParameterByName (&memo, "A", horLen);
														setParameterByName (&memo, "B", verLen);
														elem.object.xRatio = horLen;
														elem.object.yRatio = verLen;

														// ��ü ��ġ
														if ((horLen > EPS) && (verLen > EPS)) {
															ACAPI_Element_Create (&elem, &memo);
															elemList.Push (elem.header.guid);
														}

														remainVerLen -= insulElemInfo.maxVerLen;
														moveIn3D ('y', elem.object.angle, verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													}
													remainHorLen -= insulElemInfo.maxHorLen;
													remainVerLen = GetDistance (p2, p3);
													moveIn3D ('y', elem.object.angle, -GetDistance (p2, p3), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('x', elem.object.angle, horLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
												}
												ACAPI_DisposeElemMemoHdls (&memo);
											}
											// p2-p3 ���� ������ p2-p1 ���� �������� 90�� ���� (õ��� ����)
											if (abs (angle1 - angle2 - 90) < EPS) {
												// ��ü�� �ʱ� ��ġ �� ����
												elem.object.pos.x = p1.x;
												elem.object.pos.y = p1.y;
												elem.object.level = p1.z;
												elem.object.level -= storyLevel;
												elem.object.angle = DegreeToRad (angle1);

												remainHorLen = GetDistance (p1, p2);
												remainVerLen = GetDistance (p2, p3);

												totalXX = (short)floor (remainHorLen / insulElemInfo.maxHorLen);
												totalYY = (short)floor (remainVerLen / insulElemInfo.maxVerLen);

												for (xx = 0 ; xx < totalXX+1 ; ++xx) {
													for (yy = 0 ; yy < totalYY+1 ; ++yy) {
														(remainHorLen > insulElemInfo.maxHorLen) ? horLen = insulElemInfo.maxHorLen : horLen = remainHorLen;
														(remainVerLen > insulElemInfo.maxVerLen) ? verLen = insulElemInfo.maxVerLen : verLen = remainVerLen;

														setParameterByName (&memo, "angX", DegreeToRad (180.0));
														setParameterByName (&memo, "angY", DegreeToRad (0.0));
														setParameterByName (&memo, "bVerticalCut", 0.0);
														setParameterByName (&memo, "A", horLen);
														setParameterByName (&memo, "B", verLen);
														elem.object.xRatio = horLen;
														elem.object.yRatio = verLen;

														// ��ü ��ġ
														if ((horLen > EPS) && (verLen > EPS)) {
															ACAPI_Element_Create (&elem, &memo);
															elemList.Push (elem.header.guid);
														}

														remainVerLen -= insulElemInfo.maxVerLen;
														moveIn3D ('y', elem.object.angle, -verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													}
													remainHorLen -= insulElemInfo.maxHorLen;
													remainVerLen = GetDistance (p2, p3);
													moveIn3D ('y', elem.object.angle, GetDistance (p2, p3), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('x', elem.object.angle, horLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
												}
												ACAPI_DisposeElemMemoHdls (&memo);
											}
										
										// p1, p2 ���� ���� ���� p3 ���� ���� �ٸ� ��
										} else {
											// p2�� p3�� x, y ��ǥ�� ���� (����)
											if ((abs (p2.x - p3.x) < EPS) && (abs (p2.y - p3.y) < EPS)) {
												// ��ü�� �ʱ� ��ġ �� ����
												elem.object.pos.x = p1.x;
												elem.object.pos.y = p1.y;
												elem.object.level = p1.z;
												elem.object.level -= storyLevel;
												elem.object.angle = DegreeToRad (angle1);

												remainHorLen = GetDistance (p1, p2);
												remainVerLen = GetDistance (p2, p3);

												totalXX = (short)floor (remainHorLen / insulElemInfo.maxHorLen);
												totalYY = (short)floor (remainVerLen / insulElemInfo.maxVerLen);

												for (xx = 0 ; xx < totalXX+1 ; ++xx) {
													for (yy = 0 ; yy < totalYY+1 ; ++yy) {
														(remainHorLen > insulElemInfo.maxHorLen) ? horLen = insulElemInfo.maxHorLen : horLen = remainHorLen;
														(remainVerLen > insulElemInfo.maxVerLen) ? verLen = insulElemInfo.maxVerLen : verLen = remainVerLen;
														
														setParameterByName (&memo, "angX", DegreeToRad (90.0));
														setParameterByName (&memo, "angY", DegreeToRad (0.0));
														setParameterByName (&memo, "bVerticalCut", 0.0);
														setParameterByName (&memo, "A", horLen);
														setParameterByName (&memo, "B", verLen);
														elem.object.xRatio = horLen;
														elem.object.yRatio = verLen;

														// ��ü ��ġ
														if ((horLen > EPS) && (verLen > EPS)) {
															ACAPI_Element_Create (&elem, &memo);
															elemList.Push (elem.header.guid);
														}

														remainVerLen -= insulElemInfo.maxVerLen;
														moveIn3D ('z', elem.object.angle, verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													}
													remainHorLen -= insulElemInfo.maxHorLen;
													remainVerLen = GetDistance (p2, p3);
													moveIn3D ('z', elem.object.angle, -GetDistance (p2, p3), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('x', elem.object.angle, horLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
												}
												ACAPI_DisposeElemMemoHdls (&memo);
											
											// p2�� p3�� x, y ��ǥ�� �ٸ� (���)
											} else {
												// ��ü�� �ʱ� ��ġ �� ����
												elem.object.pos.x = p1.x;
												elem.object.pos.y = p1.y;
												elem.object.level = p1.z - insulElemInfo.thk;
												elem.object.level -= storyLevel;
												elem.object.angle = DegreeToRad (angle1);

												remainHorLen = GetDistance (p1, p2);
												remainVerLen = GetDistance (p2, p3);

												totalXX = (short)floor (remainHorLen / insulElemInfo.maxHorLen);
												totalYY = (short)floor (remainVerLen / insulElemInfo.maxVerLen);

												dx = GetDistance (p2.x, p2.y, p3.x, p3.y);
												dy = abs (p3.z - p2.z);
												dz = GetDistance (p2, p3);

												for (xx = 0 ; xx < totalXX+1 ; ++xx) {
													for (yy = 0 ; yy < totalYY+1 ; ++yy) {
														(remainHorLen > insulElemInfo.maxHorLen) ? horLen = insulElemInfo.maxHorLen : horLen = remainHorLen;
														(remainVerLen > insulElemInfo.maxVerLen) ? verLen = insulElemInfo.maxVerLen : verLen = remainVerLen;
														
														setParameterByName (&memo, "angX", DegreeToRad (180.0) - acos (dx/dz));
														setParameterByName (&memo, "angY", DegreeToRad (0.0));
														setParameterByName (&memo, "bVerticalCut", 1.0);
														setParameterByName (&memo, "A", horLen);
														setParameterByName (&memo, "B", verLen);
														elem.object.xRatio = horLen;
														elem.object.yRatio = verLen;

														// ��ü ��ġ
														if ((horLen > EPS) && (verLen > EPS)) {
															ACAPI_Element_Create (&elem, &memo);
															elemList.Push (elem.header.guid);
														}

														remainVerLen -= insulElemInfo.maxVerLen;
														moveIn3D ('y', elem.object.angle, -verLen * cos (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
														moveIn3D ('z', elem.object.angle, verLen * sin (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													}
													remainHorLen -= insulElemInfo.maxHorLen;
													remainVerLen = GetDistance (p2, p3);
													moveIn3D ('y', elem.object.angle, GetDistance (p2, p3) * cos (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('z', elem.object.angle, -GetDistance (p2, p3) * sin (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('x', elem.object.angle, horLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
												}
												ACAPI_DisposeElemMemoHdls (&memo);
											}
										}
									
									// ���� ũ�� < ���� ũ�� (������ ��ġ)
									} else {
										// p1, p2, p3 ���� ���� ��� ���� ��
										if ((abs (p1.z - p2.z) < EPS) && (abs (p2.z - p3.z) < EPS)) {
											// p2-p3 ���� ������ p2-p1 ���� �������� 90�� ŭ (�ٴڸ� ����)
											if (abs (angle2 - angle1 - 90) < EPS) {
												// ��ü�� �ʱ� ��ġ �� ����
												elem.object.pos.x = p1.x;
												elem.object.pos.y = p1.y;
												elem.object.level = p1.z;
												elem.object.level -= storyLevel;
												elem.object.angle = DegreeToRad (angle1);

												remainHorLen = GetDistance (p1, p2);
												remainVerLen = GetDistance (p2, p3);

												totalXX = (short)floor (remainHorLen / insulElemInfo.maxVerLen);
												totalYY = (short)floor (remainVerLen / insulElemInfo.maxHorLen);

												for (xx = 0 ; xx < totalXX+1 ; ++xx) {
													for (yy = 0 ; yy < totalYY+1 ; ++yy) {
														(remainHorLen > insulElemInfo.maxVerLen) ? verLen = insulElemInfo.maxVerLen : verLen = remainHorLen;
														(remainVerLen > insulElemInfo.maxHorLen) ? horLen = insulElemInfo.maxHorLen : horLen = remainVerLen;

														setParameterByName (&memo, "angX", DegreeToRad (0.0));
														setParameterByName (&memo, "angY", DegreeToRad (0.0));
														setParameterByName (&memo, "bVerticalCut", 0.0);
														setParameterByName (&memo, "A", horLen);
														setParameterByName (&memo, "B", verLen);
														elem.object.xRatio = horLen;
														elem.object.yRatio = verLen;

														// ��ü ��ġ
														if ((horLen > EPS) && (verLen > EPS)) {
															moveIn3D ('y', elem.object.angle, horLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
															elem.object.angle += DegreeToRad (-90.0);
															ACAPI_Element_Create (&elem, &memo);
															elemList.Push (elem.header.guid);
															elem.object.angle -= DegreeToRad (-90.0);
															moveIn3D ('y', elem.object.angle, -horLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
														}

														remainVerLen -= insulElemInfo.maxHorLen;
														moveIn3D ('y', elem.object.angle, horLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													}
													remainHorLen -= insulElemInfo.maxVerLen;
													remainVerLen = GetDistance (p2, p3);
													moveIn3D ('y', elem.object.angle, -GetDistance (p2, p3), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('x', elem.object.angle, verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
												}
												ACAPI_DisposeElemMemoHdls (&memo);
											}
											// p2-p3 ���� ������ p2-p1 ���� �������� 90�� ���� (õ��� ����)
											if (abs (angle1 - angle2 - 90) < EPS) {
												// ��ü�� �ʱ� ��ġ �� ����
												elem.object.pos.x = p1.x;
												elem.object.pos.y = p1.y;
												elem.object.level = p1.z;
												elem.object.level -= storyLevel;
												elem.object.angle = DegreeToRad (angle1);

												remainHorLen = GetDistance (p1, p2);
												remainVerLen = GetDistance (p2, p3);

												totalXX = (short)floor (remainHorLen / insulElemInfo.maxVerLen);
												totalYY = (short)floor (remainVerLen / insulElemInfo.maxHorLen);

												for (xx = 0 ; xx < totalXX+1 ; ++xx) {
													for (yy = 0 ; yy < totalYY+1 ; ++yy) {
														(remainHorLen > insulElemInfo.maxVerLen) ? verLen = insulElemInfo.maxVerLen : verLen = remainHorLen;
														(remainVerLen > insulElemInfo.maxHorLen) ? horLen = insulElemInfo.maxHorLen : horLen = remainVerLen;

														setParameterByName (&memo, "angX", DegreeToRad (180.0));
														setParameterByName (&memo, "angY", DegreeToRad (0.0));
														setParameterByName (&memo, "bVerticalCut", 0.0);
														setParameterByName (&memo, "A", horLen);
														setParameterByName (&memo, "B", verLen);
														elem.object.xRatio = horLen;
														elem.object.yRatio = verLen;

														// ��ü ��ġ
														if ((horLen > EPS) && (verLen > EPS)) {
															moveIn3D ('x', elem.object.angle, verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
															elem.object.angle += DegreeToRad (-90.0);
															ACAPI_Element_Create (&elem, &memo);
															elemList.Push (elem.header.guid);
															elem.object.angle -= DegreeToRad (-90.0);
															moveIn3D ('x', elem.object.angle, -verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
														}

														remainVerLen -= insulElemInfo.maxHorLen;
														moveIn3D ('y', elem.object.angle, -horLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													}
													remainHorLen -= insulElemInfo.maxVerLen;
													remainVerLen = GetDistance (p2, p3);
													moveIn3D ('y', elem.object.angle, GetDistance (p2, p3), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('x', elem.object.angle, verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
												}
												ACAPI_DisposeElemMemoHdls (&memo);
											}
										
										// p1, p2 ���� ���� ���� p3 ���� ���� �ٸ� ��
										} else {
											// p2�� p3�� x, y ��ǥ�� ���� (����)
											if ((abs (p2.x - p3.x) < EPS) && (abs (p2.y - p3.y) < EPS)) {
												// ��ü�� �ʱ� ��ġ �� ����
												elem.object.pos.x = p1.x;
												elem.object.pos.y = p1.y;
												elem.object.level = p1.z;
												elem.object.level -= storyLevel;
												elem.object.angle = DegreeToRad (angle1);

												remainHorLen = GetDistance (p1, p2);
												remainVerLen = GetDistance (p2, p3);

												totalXX = (short)floor (remainHorLen / insulElemInfo.maxVerLen);
												totalYY = (short)floor (remainVerLen / insulElemInfo.maxHorLen);

												for (xx = 0 ; xx < totalXX+1 ; ++xx) {
													for (yy = 0 ; yy < totalYY+1 ; ++yy) {
														(remainHorLen > insulElemInfo.maxVerLen) ? verLen = insulElemInfo.maxVerLen : verLen = remainHorLen;
														(remainVerLen > insulElemInfo.maxHorLen) ? horLen = insulElemInfo.maxHorLen : horLen = remainVerLen;
														
														setParameterByName (&memo, "angX", DegreeToRad (0.0));
														setParameterByName (&memo, "angY", DegreeToRad (90.0));
														setParameterByName (&memo, "bVerticalCut", 0.0);
														setParameterByName (&memo, "A", horLen);
														setParameterByName (&memo, "B", verLen);
														elem.object.xRatio = horLen;
														elem.object.yRatio = verLen;

														// ��ü ��ġ
														if ((horLen > EPS) && (verLen > EPS)) {
															moveIn3D ('z', elem.object.angle, horLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
															elem.object.angle += DegreeToRad (-90.0);
															ACAPI_Element_Create (&elem, &memo);
															elemList.Push (elem.header.guid);
															elem.object.angle -= DegreeToRad (-90.0);
															moveIn3D ('z', elem.object.angle, -horLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
														}

														remainVerLen -= insulElemInfo.maxHorLen;
														moveIn3D ('z', elem.object.angle, horLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													}
													remainHorLen -= insulElemInfo.maxVerLen;
													remainVerLen = GetDistance (p2, p3);
													moveIn3D ('z', elem.object.angle, -GetDistance (p2, p3), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('x', elem.object.angle, verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
												}
												ACAPI_DisposeElemMemoHdls (&memo);
											
											// p2�� p3�� x, y ��ǥ�� �ٸ� (���)
											} else {
												// ��ü�� �ʱ� ��ġ �� ����
												elem.object.pos.x = p1.x;
												elem.object.pos.y = p1.y;
												elem.object.level = p1.z - insulElemInfo.thk;
												elem.object.level -= storyLevel;
												elem.object.angle = DegreeToRad (angle1);

												remainHorLen = GetDistance (p1, p2);
												remainVerLen = GetDistance (p2, p3);

												totalXX = (short)floor (remainHorLen / insulElemInfo.maxVerLen);
												totalYY = (short)floor (remainVerLen / insulElemInfo.maxHorLen);

												dx = GetDistance (p2.x, p2.y, p3.x, p3.y);
												dy = abs (p3.z - p2.z);
												dz = GetDistance (p2, p3);

												for (xx = 0 ; xx < totalXX+1 ; ++xx) {
													for (yy = 0 ; yy < totalYY+1 ; ++yy) {
														(remainHorLen > insulElemInfo.maxVerLen) ? verLen = insulElemInfo.maxVerLen : verLen = remainHorLen;
														(remainVerLen > insulElemInfo.maxHorLen) ? horLen = insulElemInfo.maxHorLen : horLen = remainVerLen;
														
														setParameterByName (&memo, "angX", DegreeToRad (0.0));
														setParameterByName (&memo, "angY", DegreeToRad (180.0) - acos (dx/dz));
														setParameterByName (&memo, "bVerticalCut", 1.0);
														setParameterByName (&memo, "A", horLen);
														setParameterByName (&memo, "B", verLen);
														elem.object.xRatio = horLen;
														elem.object.yRatio = verLen;

														// ��ü ��ġ
														if ((horLen > EPS) && (verLen > EPS)) {
															moveIn3D ('z', elem.object.angle, horLen * sin (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
															moveIn3D ('y', elem.object.angle, -horLen * cos (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
															elem.object.angle += DegreeToRad (-90.0);
															ACAPI_Element_Create (&elem, &memo);
															elemList.Push (elem.header.guid);
															elem.object.angle -= DegreeToRad (-90.0);
															moveIn3D ('z', elem.object.angle, -horLen * sin (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
															moveIn3D ('y', elem.object.angle, horLen * cos (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
														}

														remainVerLen -= insulElemInfo.maxHorLen;
														moveIn3D ('y', elem.object.angle, -horLen * cos (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
														moveIn3D ('z', elem.object.angle, horLen * sin (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													}
													remainHorLen -= insulElemInfo.maxVerLen;
													remainVerLen = GetDistance (p2, p3);
													moveIn3D ('y', elem.object.angle, GetDistance (p2, p3) * cos (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('z', elem.object.angle, -GetDistance (p2, p3) * sin (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('x', elem.object.angle, verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
												}
												ACAPI_DisposeElemMemoHdls (&memo);
											}
										}
									}
								
								// p1-p2�� p2-p3���� ª�� ��� (���� ����)
								} else {
									// ���� ũ�� >= ���� ũ�� (������ ��ġ)
									if (insulElemInfo.maxHorLen >= insulElemInfo.maxVerLen) {
										// p1, p2, p3 ���� ���� ��� ���� ��
										if ((abs (p1.z - p2.z) < EPS) && (abs (p2.z - p3.z) < EPS)) {
											// p2-p3 ���� ������ p2-p1 ���� �������� 90�� ŭ (�ٴڸ� ����)
											if (abs (angle2 - angle1 - 90) < EPS) {
												// ��ü�� �ʱ� ��ġ �� ����
												elem.object.pos.x = p1.x;
												elem.object.pos.y = p1.y;
												elem.object.level = p1.z;
												elem.object.level -= storyLevel;
												elem.object.angle = DegreeToRad (angle1);

												remainHorLen = GetDistance (p1, p2);
												remainVerLen = GetDistance (p2, p3);

												totalXX = (short)floor (remainHorLen / insulElemInfo.maxVerLen);
												totalYY = (short)floor (remainVerLen / insulElemInfo.maxHorLen);

												for (xx = 0 ; xx < totalXX+1 ; ++xx) {
													for (yy = 0 ; yy < totalYY+1 ; ++yy) {
														(remainHorLen > insulElemInfo.maxVerLen) ? verLen = insulElemInfo.maxVerLen : verLen = remainHorLen;
														(remainVerLen > insulElemInfo.maxHorLen) ? horLen = insulElemInfo.maxHorLen : horLen = remainVerLen;

														setParameterByName (&memo, "angX", DegreeToRad (0.0));
														setParameterByName (&memo, "angY", DegreeToRad (0.0));
														setParameterByName (&memo, "bVerticalCut", 0.0);
														setParameterByName (&memo, "A", horLen);
														setParameterByName (&memo, "B", verLen);
														elem.object.xRatio = horLen;
														elem.object.yRatio = verLen;

														// ��ü ��ġ
														if ((horLen > EPS) && (verLen > EPS)) {
															moveIn3D ('x', elem.object.angle, verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
															elem.object.angle += DegreeToRad (90.0);
															ACAPI_Element_Create (&elem, &memo);
															elemList.Push (elem.header.guid);
															elem.object.angle -= DegreeToRad (90.0);
															moveIn3D ('x', elem.object.angle, -verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
														}

														remainVerLen -= insulElemInfo.maxHorLen;
														moveIn3D ('y', elem.object.angle, horLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													}
													remainHorLen -= insulElemInfo.maxVerLen;
													remainVerLen = GetDistance (p2, p3);
													moveIn3D ('y', elem.object.angle, -GetDistance (p2, p3), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('x', elem.object.angle, verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
												}
												ACAPI_DisposeElemMemoHdls (&memo);
											}
											// p2-p3 ���� ������ p2-p1 ���� �������� 90�� ���� (õ��� ����)
											if (abs (angle1 - angle2 - 90) < EPS) {
												// ��ü�� �ʱ� ��ġ �� ����
												elem.object.pos.x = p1.x;
												elem.object.pos.y = p1.y;
												elem.object.level = p1.z;
												elem.object.level -= storyLevel;
												elem.object.angle = DegreeToRad (angle1);

												remainHorLen = GetDistance (p1, p2);
												remainVerLen = GetDistance (p2, p3);

												totalXX = (short)floor (remainHorLen / insulElemInfo.maxVerLen);
												totalYY = (short)floor (remainVerLen / insulElemInfo.maxHorLen);

												for (xx = 0 ; xx < totalXX+1 ; ++xx) {
													for (yy = 0 ; yy < totalYY+1 ; ++yy) {
														(remainHorLen > insulElemInfo.maxVerLen) ? verLen = insulElemInfo.maxVerLen : verLen = remainHorLen;
														(remainVerLen > insulElemInfo.maxHorLen) ? horLen = insulElemInfo.maxHorLen : horLen = remainVerLen;

														setParameterByName (&memo, "angX", DegreeToRad (180.0));
														setParameterByName (&memo, "angY", DegreeToRad (0.0));
														setParameterByName (&memo, "bVerticalCut", 0.0);
														setParameterByName (&memo, "A", horLen);
														setParameterByName (&memo, "B", verLen);
														elem.object.xRatio = horLen;
														elem.object.yRatio = verLen;

														// ��ü ��ġ
														if ((horLen > EPS) && (verLen > EPS)) {
															moveIn3D ('x', elem.object.angle, verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
															elem.object.angle += DegreeToRad (-90.0);
															ACAPI_Element_Create (&elem, &memo);
															elemList.Push (elem.header.guid);
															elem.object.angle -= DegreeToRad (-90.0);
															moveIn3D ('x', elem.object.angle, -verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
														}

														remainVerLen -= insulElemInfo.maxHorLen;
														moveIn3D ('y', elem.object.angle, -horLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													}
													remainHorLen -= insulElemInfo.maxVerLen;
													remainVerLen = GetDistance (p2, p3);
													moveIn3D ('y', elem.object.angle, GetDistance (p2, p3), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('x', elem.object.angle, verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
												}
												ACAPI_DisposeElemMemoHdls (&memo);
											}
										
										// p1, p2 ���� ���� ���� p3 ���� ���� �ٸ� ��
										} else {
											// p2�� p3�� x, y ��ǥ�� ���� (����)
											if ((abs (p2.x - p3.x) < EPS) && (abs (p2.y - p3.y) < EPS)) {
												// ��ü�� �ʱ� ��ġ �� ����
												elem.object.pos.x = p1.x;
												elem.object.pos.y = p1.y;
												elem.object.level = p1.z;
												elem.object.level -= storyLevel;
												elem.object.angle = DegreeToRad (angle1);

												remainHorLen = GetDistance (p1, p2);
												remainVerLen = GetDistance (p2, p3);

												totalXX = (short)floor (remainHorLen / insulElemInfo.maxVerLen);
												totalYY = (short)floor (remainVerLen / insulElemInfo.maxHorLen);

												for (xx = 0 ; xx < totalXX+1 ; ++xx) {
													for (yy = 0 ; yy < totalYY+1 ; ++yy) {
														(remainHorLen > insulElemInfo.maxVerLen) ? verLen = insulElemInfo.maxVerLen : verLen = remainHorLen;
														(remainVerLen > insulElemInfo.maxHorLen) ? horLen = insulElemInfo.maxHorLen : horLen = remainVerLen;
														
														setParameterByName (&memo, "angX", DegreeToRad (90.0));
														setParameterByName (&memo, "angY", DegreeToRad (-90.0));
														setParameterByName (&memo, "bVerticalCut", 0.0);
														setParameterByName (&memo, "A", horLen);
														setParameterByName (&memo, "B", verLen);
														elem.object.xRatio = horLen;
														elem.object.yRatio = verLen;

														// ��ü ��ġ
														if ((horLen > EPS) && (verLen > EPS)) {
															moveIn3D ('x', elem.object.angle, verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
															ACAPI_Element_Create (&elem, &memo);
															elemList.Push (elem.header.guid);
															moveIn3D ('x', elem.object.angle, -verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
														}

														remainVerLen -= insulElemInfo.maxHorLen;
														moveIn3D ('z', elem.object.angle, horLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													}
													remainHorLen -= insulElemInfo.maxVerLen;
													remainVerLen = GetDistance (p2, p3);
													moveIn3D ('z', elem.object.angle, -GetDistance (p2, p3), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('x', elem.object.angle, verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
												}
												ACAPI_DisposeElemMemoHdls (&memo);
											
											// p2�� p3�� x, y ��ǥ�� �ٸ� (���)
											} else {
												// ��ü�� �ʱ� ��ġ �� ����
												elem.object.pos.x = p1.x;
												elem.object.pos.y = p1.y;
												elem.object.level = p1.z - insulElemInfo.thk;
												elem.object.level -= storyLevel;
												elem.object.angle = DegreeToRad (angle1);

												remainHorLen = GetDistance (p1, p2);
												remainVerLen = GetDistance (p2, p3);

												totalXX = (short)floor (remainHorLen / insulElemInfo.maxVerLen);
												totalYY = (short)floor (remainVerLen / insulElemInfo.maxHorLen);

												dx = GetDistance (p2.x, p2.y, p3.x, p3.y);
												dy = abs (p3.z - p2.z);
												dz = GetDistance (p2, p3);

												for (xx = 0 ; xx < totalXX+1 ; ++xx) {
													for (yy = 0 ; yy < totalYY+1 ; ++yy) {
														(remainHorLen > insulElemInfo.maxVerLen) ? verLen = insulElemInfo.maxVerLen : verLen = remainHorLen;
														(remainVerLen > insulElemInfo.maxHorLen) ? horLen = insulElemInfo.maxHorLen : horLen = remainVerLen;
														
														setParameterByName (&memo, "angX", DegreeToRad (0.0));
														setParameterByName (&memo, "angY", acos (dx/dz));
														setParameterByName (&memo, "bVerticalCut", 1.0);
														setParameterByName (&memo, "A", horLen);
														setParameterByName (&memo, "B", verLen);
														elem.object.xRatio = horLen;
														elem.object.yRatio = verLen;

														// ��ü ��ġ
														if ((horLen > EPS) && (verLen > EPS)) {
															moveIn3D ('z', elem.object.angle, horLen * sin (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
															moveIn3D ('y', elem.object.angle, -horLen * cos (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
															moveIn3D ('x', elem.object.angle, verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
															elem.object.angle += DegreeToRad (90.0);
															ACAPI_Element_Create (&elem, &memo);
															elemList.Push (elem.header.guid);
															elem.object.angle -= DegreeToRad (90.0);
															moveIn3D ('z', elem.object.angle, -horLen * sin (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
															moveIn3D ('y', elem.object.angle, horLen * cos (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
															moveIn3D ('x', elem.object.angle, -verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
														}

														remainVerLen -= insulElemInfo.maxHorLen;
														moveIn3D ('y', elem.object.angle, -horLen * cos (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
														moveIn3D ('z', elem.object.angle, horLen * sin (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													}
													remainHorLen -= insulElemInfo.maxVerLen;
													remainVerLen = GetDistance (p2, p3);
													moveIn3D ('y', elem.object.angle, GetDistance (p2, p3) * cos (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('z', elem.object.angle, -GetDistance (p2, p3) * sin (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('x', elem.object.angle, verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
												}
												ACAPI_DisposeElemMemoHdls (&memo);
											}
										}
									
									// ���� ũ�� < ���� ũ�� (�״�� ��ġ)
									} else {
										// p1, p2, p3 ���� ���� ��� ���� ��
										if ((abs (p1.z - p2.z) < EPS) && (abs (p2.z - p3.z) < EPS)) {
											// p2-p3 ���� ������ p2-p1 ���� �������� 90�� ŭ (�ٴڸ� ����)
											if (abs (angle2 - angle1 - 90) < EPS) {
												// ��ü�� �ʱ� ��ġ �� ����
												elem.object.pos.x = p1.x;
												elem.object.pos.y = p1.y;
												elem.object.level = p1.z;
												elem.object.level -= storyLevel;
												elem.object.angle = DegreeToRad (angle1);

												remainHorLen = GetDistance (p1, p2);
												remainVerLen = GetDistance (p2, p3);

												totalXX = (short)floor (remainHorLen / insulElemInfo.maxHorLen);
												totalYY = (short)floor (remainVerLen / insulElemInfo.maxVerLen);

												for (xx = 0 ; xx < totalXX+1 ; ++xx) {
													for (yy = 0 ; yy < totalYY+1 ; ++yy) {
														(remainHorLen > insulElemInfo.maxHorLen) ? horLen = insulElemInfo.maxHorLen : horLen = remainHorLen;
														(remainVerLen > insulElemInfo.maxVerLen) ? verLen = insulElemInfo.maxVerLen : verLen = remainVerLen;
														
														setParameterByName (&memo, "angX", DegreeToRad (0.0));
														setParameterByName (&memo, "angY", DegreeToRad (0.0));
														setParameterByName (&memo, "bVerticalCut", 0.0);
														setParameterByName (&memo, "A", horLen);
														setParameterByName (&memo, "B", verLen);
														elem.object.xRatio = horLen;
														elem.object.yRatio = verLen;

														// ��ü ��ġ
														if ((horLen > EPS) && (verLen > EPS)) {
															ACAPI_Element_Create (&elem, &memo);
															elemList.Push (elem.header.guid);
														}

														remainVerLen -= insulElemInfo.maxVerLen;
														moveIn3D ('y', elem.object.angle, verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													}
													remainHorLen -= insulElemInfo.maxHorLen;
													remainVerLen = GetDistance (p2, p3);
													moveIn3D ('y', elem.object.angle, -GetDistance (p2, p3), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('x', elem.object.angle, horLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
												}
												ACAPI_DisposeElemMemoHdls (&memo);
											}
											// p2-p3 ���� ������ p2-p1 ���� �������� 90�� ���� (õ��� ����)
											if (abs (angle1 - angle2 - 90) < EPS) {
												// ��ü�� �ʱ� ��ġ �� ����
												elem.object.pos.x = p1.x;
												elem.object.pos.y = p1.y;
												elem.object.level = p1.z;
												elem.object.level -= storyLevel;
												elem.object.angle = DegreeToRad (angle1);

												remainHorLen = GetDistance (p1, p2);
												remainVerLen = GetDistance (p2, p3);

												totalXX = (short)floor (remainHorLen / insulElemInfo.maxHorLen);
												totalYY = (short)floor (remainVerLen / insulElemInfo.maxVerLen);

												for (xx = 0 ; xx < totalXX+1 ; ++xx) {
													for (yy = 0 ; yy < totalYY+1 ; ++yy) {
														(remainHorLen > insulElemInfo.maxHorLen) ? horLen = insulElemInfo.maxHorLen : horLen = remainHorLen;
														(remainVerLen > insulElemInfo.maxVerLen) ? verLen = insulElemInfo.maxVerLen : verLen = remainVerLen;

														setParameterByName (&memo, "angX", DegreeToRad (180.0));
														setParameterByName (&memo, "angY", DegreeToRad (0.0));
														setParameterByName (&memo, "bVerticalCut", 0.0);
														setParameterByName (&memo, "A", horLen);
														setParameterByName (&memo, "B", verLen);
														elem.object.xRatio = horLen;
														elem.object.yRatio = verLen;

														// ��ü ��ġ
														if ((horLen > EPS) && (verLen > EPS)) {
															ACAPI_Element_Create (&elem, &memo);
															elemList.Push (elem.header.guid);
														}

														remainVerLen -= insulElemInfo.maxVerLen;
														moveIn3D ('y', elem.object.angle, -verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													}
													remainHorLen -= insulElemInfo.maxHorLen;
													remainVerLen = GetDistance (p2, p3);
													moveIn3D ('y', elem.object.angle, GetDistance (p2, p3), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('x', elem.object.angle, horLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
												}
												ACAPI_DisposeElemMemoHdls (&memo);
											}
										
										// p1, p2 ���� ���� ���� p3 ���� ���� �ٸ� ��
										} else {
											// p2�� p3�� x, y ��ǥ�� ���� (����)
											if ((abs (p2.x - p3.x) < EPS) && (abs (p2.y - p3.y) < EPS)) {
												// ��ü�� �ʱ� ��ġ �� ����
												elem.object.pos.x = p1.x;
												elem.object.pos.y = p1.y;
												elem.object.level = p1.z;
												elem.object.level -= storyLevel;
												elem.object.angle = DegreeToRad (angle1);

												remainHorLen = GetDistance (p1, p2);
												remainVerLen = GetDistance (p2, p3);

												totalXX = (short)floor (remainHorLen / insulElemInfo.maxHorLen);
												totalYY = (short)floor (remainVerLen / insulElemInfo.maxVerLen);

												for (xx = 0 ; xx < totalXX+1 ; ++xx) {
													for (yy = 0 ; yy < totalYY+1 ; ++yy) {
														(remainHorLen > insulElemInfo.maxHorLen) ? horLen = insulElemInfo.maxHorLen : horLen = remainHorLen;
														(remainVerLen > insulElemInfo.maxVerLen) ? verLen = insulElemInfo.maxVerLen : verLen = remainVerLen;
														
														setParameterByName (&memo, "angX", DegreeToRad (90.0));
														setParameterByName (&memo, "angY", DegreeToRad (0.0));
														setParameterByName (&memo, "bVerticalCut", 0.0);
														setParameterByName (&memo, "A", horLen);
														setParameterByName (&memo, "B", verLen);
														elem.object.xRatio = horLen;
														elem.object.yRatio = verLen;

														// ��ü ��ġ
														if ((horLen > EPS) && (verLen > EPS)) {
															ACAPI_Element_Create (&elem, &memo);
															elemList.Push (elem.header.guid);
														}

														remainVerLen -= insulElemInfo.maxVerLen;
														moveIn3D ('z', elem.object.angle, verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													}
													remainHorLen -= insulElemInfo.maxHorLen;
													remainVerLen = GetDistance (p2, p3);
													moveIn3D ('z', elem.object.angle, -GetDistance (p2, p3), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('x', elem.object.angle, horLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
												}
												ACAPI_DisposeElemMemoHdls (&memo);
											
											// p2�� p3�� x, y ��ǥ�� �ٸ� (���)
											} else {
												// ��ü�� �ʱ� ��ġ �� ����
												elem.object.pos.x = p1.x;
												elem.object.pos.y = p1.y;
												elem.object.level = p1.z - insulElemInfo.thk;
												elem.object.level -= storyLevel;
												elem.object.angle = DegreeToRad (angle1);

												remainHorLen = GetDistance (p1, p2);
												remainVerLen = GetDistance (p2, p3);

												totalXX = (short)floor (remainHorLen / insulElemInfo.maxHorLen);
												totalYY = (short)floor (remainVerLen / insulElemInfo.maxVerLen);

												dx = GetDistance (p2.x, p2.y, p3.x, p3.y);
												dy = abs (p3.z - p2.z);
												dz = GetDistance (p2, p3);

												for (xx = 0 ; xx < totalXX+1 ; ++xx) {
													for (yy = 0 ; yy < totalYY+1 ; ++yy) {
														(remainHorLen > insulElemInfo.maxHorLen) ? horLen = insulElemInfo.maxHorLen : horLen = remainHorLen;
														(remainVerLen > insulElemInfo.maxVerLen) ? verLen = insulElemInfo.maxVerLen : verLen = remainVerLen;
														
														setParameterByName (&memo, "angX", DegreeToRad (180.0) - acos (dx/dz));
														setParameterByName (&memo, "angY", DegreeToRad (0.0));
														setParameterByName (&memo, "bVerticalCut", 1.0);
														setParameterByName (&memo, "A", horLen);
														setParameterByName (&memo, "B", verLen);
														elem.object.xRatio = horLen;
														elem.object.yRatio = verLen;

														// ��ü ��ġ
														if ((horLen > EPS) && (verLen > EPS)) {
															ACAPI_Element_Create (&elem, &memo);
															elemList.Push (elem.header.guid);
														}

														remainVerLen -= insulElemInfo.maxVerLen;
														moveIn3D ('y', elem.object.angle, -verLen * cos (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
														moveIn3D ('z', elem.object.angle, verLen * sin (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													}
													remainHorLen -= insulElemInfo.maxHorLen;
													remainVerLen = GetDistance (p2, p3);
													moveIn3D ('y', elem.object.angle, GetDistance (p2, p3) * cos (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('z', elem.object.angle, -GetDistance (p2, p3) * sin (acos (dx/dz)), &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
													moveIn3D ('x', elem.object.angle, horLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
												}
												ACAPI_DisposeElemMemoHdls (&memo);
											}
										}
									}
								}
							
							// ����/���� ũ�� ������ false�� ��
							} else {
								// p1, p2, p3 ���� ���� ��� ���� ��
								if ((abs (p1.z - p2.z) < EPS) && (abs (p2.z - p3.z) < EPS)) {
									// p2-p3 ���� ������ p2-p1 ���� �������� 90�� ŭ (�ٴڸ� ����)
									if (abs (angle2 - angle1 - 90) < EPS) {
										// ��ü�� �ʱ� ��ġ �� ����
										elem.object.pos.x = p1.x;
										elem.object.pos.y = p1.y;
										elem.object.level = p1.z;
										elem.object.level -= storyLevel;
										elem.object.angle = DegreeToRad (angle1);

										horLen = GetDistance (p1, p2);
										verLen = GetDistance (p2, p3);

										setParameterByName (&memo, "angX", DegreeToRad (0.0));
										setParameterByName (&memo, "angY", DegreeToRad (0.0));
										setParameterByName (&memo, "bVerticalCut", 0.0);
										setParameterByName (&memo, "A", horLen);
										setParameterByName (&memo, "B", verLen);
										elem.object.xRatio = horLen;
										elem.object.yRatio = verLen;

										// ��ü ��ġ
										if ((horLen > EPS) && (verLen > EPS)) {
											ACAPI_Element_Create (&elem, &memo);
											elemList.Push (elem.header.guid);
										}

										ACAPI_DisposeElemMemoHdls (&memo);
									}
									// p2-p3 ���� ������ p2-p1 ���� �������� 90�� ���� (õ��� ����)
									if (abs (angle1 - angle2 - 90) < EPS) {
										// ��ü�� �ʱ� ��ġ �� ����
										elem.object.pos.x = p1.x;
										elem.object.pos.y = p1.y;
										elem.object.level = p1.z;
										elem.object.level -= storyLevel;
										elem.object.angle = DegreeToRad (angle1);

										horLen = GetDistance (p1, p2);
										verLen = GetDistance (p2, p3);

										setParameterByName (&memo, "angX", DegreeToRad (180.0));
										setParameterByName (&memo, "angY", DegreeToRad (0.0));
										setParameterByName (&memo, "bVerticalCut", 0.0);
										setParameterByName (&memo, "A", horLen);
										setParameterByName (&memo, "B", verLen);
										elem.object.xRatio = horLen;
										elem.object.yRatio = verLen;

										// ��ü ��ġ
										if ((horLen > EPS) && (verLen > EPS)) {
											ACAPI_Element_Create (&elem, &memo);
											elemList.Push (elem.header.guid);
										}

										ACAPI_DisposeElemMemoHdls (&memo);
									}
										
								// p1, p2 ���� ���� ���� p3 ���� ���� �ٸ� ��
								} else {
									// p2�� p3�� x, y ��ǥ�� ���� (����)
									if ((abs (p2.x - p3.x) < EPS) && (abs (p2.y - p3.y) < EPS)) {
										// ��ü�� �ʱ� ��ġ �� ����
										elem.object.pos.x = p1.x;
										elem.object.pos.y = p1.y;
										elem.object.level = p1.z;
										elem.object.level -= storyLevel;
										elem.object.angle = DegreeToRad (angle1);

										horLen = GetDistance (p1, p2);
										verLen = GetDistance (p2, p3);

										setParameterByName (&memo, "angX", DegreeToRad (90.0));
										setParameterByName (&memo, "angY", DegreeToRad (0.0));
										setParameterByName (&memo, "bVerticalCut", 0.0);
										setParameterByName (&memo, "A", horLen);
										setParameterByName (&memo, "B", verLen);
										elem.object.xRatio = horLen;
										elem.object.yRatio = verLen;

										// ��ü ��ġ
										if ((horLen > EPS) && (verLen > EPS)) {
											ACAPI_Element_Create (&elem, &memo);
											elemList.Push (elem.header.guid);
										}

										ACAPI_DisposeElemMemoHdls (&memo);
											
									// p2�� p3�� x, y ��ǥ�� �ٸ� (���)
									} else {
										// ��ü�� �ʱ� ��ġ �� ����
										elem.object.pos.x = p1.x;
										elem.object.pos.y = p1.y;
										elem.object.level = p1.z - insulElemInfo.thk;
										elem.object.level -= storyLevel;
										elem.object.angle = DegreeToRad (angle1);

										horLen = GetDistance (p1, p2);
										verLen = GetDistance (p2, p3);

										dx = GetDistance (p2.x, p2.y, p3.x, p3.y);
										dy = abs (p3.z - p2.z);
										dz = GetDistance (p2, p3);

										setParameterByName (&memo, "angX", DegreeToRad (180.0) - acos (dx/dz));
										setParameterByName (&memo, "angY", DegreeToRad (0.0));
										setParameterByName (&memo, "bVerticalCut", 1.0);
										setParameterByName (&memo, "A", horLen);
										setParameterByName (&memo, "B", verLen);
										elem.object.xRatio = horLen;
										elem.object.yRatio = verLen;

										// ��ü ��ġ
										if ((horLen > EPS) && (verLen > EPS)) {
											ACAPI_Element_Create (&elem, &memo);
											elemList.Push (elem.header.guid);
										}

										ACAPI_DisposeElemMemoHdls (&memo);
									}
								}
							}

							// �׷�ȭ�ϱ�
							groupElements (elemList);
							elemList.Clear (false);

							// ȭ�� ���ΰ�ħ
							ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
							ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

						} while (inputErr != APIERR_CANCEL);

						break;

					case DG_CLOSEBOX:
						return item;	// �̰��� DG_MSG_CLOSE �޽����� ����
				}
				break;

			case DG_MSG_DOUBLECLICK:
				break;

			case DG_MSG_CLOSE:
				ACAPI_UnregisterModelessWindow (dlgData->dialogID);
				dlgData->dialogID = 0;
				break;
		}

		return err;
	});

	return 0;
}

// �ȷ�Ʈ�� ���� �ݹ� �Լ� 2
static GSErrCode __ACENV_CALL	insulElemPaletteAPIControlCallBack (Int32 referenceID, API_PaletteMessageID messageID, GS::IntPtr /*param*/)
{
	if (referenceID == insulElemInfo.dialogID) {
		switch (messageID) {
			case APIPalMsg_ClosePalette:		DGModelessClose (insulElemInfo.dialogID);
												break;

			case APIPalMsg_HidePalette_Begin:	break;
			case APIPalMsg_HidePalette_End:		break;

			case APIPalMsg_DisableItems_Begin:	break;
			case APIPalMsg_DisableItems_End:	break;
			case APIPalMsg_OpenPalette:			break;
			default:							break;
		}
	}

	return NoError;
}
