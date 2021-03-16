#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "WallTableformPlacer.hpp"

using namespace wallTableformPlacerDG;

static WallTableformPlacingZone		placingZone;		// �⺻ ���� ���� ����
static InfoWallForWallTableform		infoWall;			// �� ��ü ����

static short	layerInd_Euroform;		// ���̾� ��ȣ: ������
static short	layerInd_RectPipe;		// ���̾� ��ȣ: ��� ������
static short	layerInd_PinBolt;		// ���̾� ��ȣ: �ɺ�Ʈ ��Ʈ
static short	layerInd_WallTie;		// ���̾� ��ȣ: ��ü Ÿ��
static short	layerInd_Clamp;			// ���̾� ��ȣ: ���� Ŭ����
static short	layerInd_HeadPiece;		// ���̾� ��ȣ: ����ǽ�

// ���̾�α� ���� ��� �ε��� ��ȣ ����
static short	EDITCONTROL_REMAIN_WIDTH;
static short	POPUP_WIDTH [50];


// ���� ���̺����� ��ġ�ϴ� ���� ��ƾ
GSErrCode	placeTableformOnWall (void)
{
	GSErrCode	err = NoError;
	short		result;
	long		nSel;
	short		xx, yy;
	double		dx, dy, ang1, ang2;
	//double		xPosLB, yPosLB, zPosLB;
	//double		xPosRT, yPosRT, zPosRT;
	double		width;

	// Selection Manager ���� ����
	API_SelectionInfo		selectionInfo;
	API_Element				tElem;
	API_Neig				**selNeigs;
	GS::Array<API_Guid>&	walls = GS::Array<API_Guid> ();
	GS::Array<API_Guid>&	morphs = GS::Array<API_Guid> ();
	long					nWalls = 0;
	long					nMorphs = 0;

	// ��ü ���� ��������
	API_Element				elem;
	API_ElementMemo			memo;
	API_ElemInfo3D			info3D;

	// ���� ��ü ����
	InfoMorphForWallTableform	infoMorph;

	// �۾� �� ����
	API_StoryInfo	storyInfo;
	double			workLevel_wall;		// ���� �۾� �� ����


	// ������ ��� ��������
	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err == APIERR_NOPLAN) {
		ACAPI_WriteReport ("���� ������Ʈ â�� �����ϴ�.", true);
	}
	if (err == APIERR_NOSEL) {
		ACAPI_WriteReport ("�ƹ� �͵� �������� �ʾҽ��ϴ�.\n�ʼ� ����: �� (1��), ���� ���� ���� (1��)", true);
	}
	if (err != NoError) {
		BMKillHandle ((GSHandle *) &selNeigs);
		return err;
	}

	// �� 1��, ���� 1�� �����ؾ� ��
	if (selectionInfo.typeID != API_SelEmpty) {
		nSel = BMGetHandleSize ((GSHandle) selNeigs) / sizeof (API_Neig);
		for (xx = 0 ; xx < nSel && err == NoError ; ++xx) {
			tElem.header.typeID = Neig_To_ElemID ((*selNeigs)[xx].neigID);

			tElem.header.guid = (*selNeigs)[xx].guid;
			if (ACAPI_Element_Get (&tElem) != NoError)	// ������ �� �ִ� ����ΰ�?
				continue;

			if (tElem.header.typeID == API_WallID)		// ���ΰ�?
				walls.Push (tElem.header.guid);

			if (tElem.header.typeID == API_MorphID)		// �����ΰ�?
				morphs.Push (tElem.header.guid);
		}
	}
	BMKillHandle ((GSHandle *) &selNeigs);
	nWalls = walls.GetSize ();
	nMorphs = morphs.GetSize ();

	// ���� 1���ΰ�?
	if (nWalls != 1) {
		ACAPI_WriteReport ("���� 1�� �����ؾ� �մϴ�.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// ������ 1���ΰ�?
	if (nMorphs != 1) {
		ACAPI_WriteReport ("���� ���� ������ 1�� �����ϼž� �մϴ�.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// (1) �� ������ ������
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	elem.header.guid = walls.Pop ();
	err = ACAPI_Element_Get (&elem);						// elem.wall.poly.nCoords : ������ ���� ������ �� ����
	err = ACAPI_Element_GetMemo (elem.header.guid, &memo);	// memo.coords : ������ ��ǥ�� ������ �� ����
	
	if (elem.wall.thickness != elem.wall.thickness1) {
		ACAPI_WriteReport ("���� �β��� �����ؾ� �մϴ�.", true);
		err = APIERR_GENERAL;
		return err;
	}
	infoWall.wallThk		= elem.wall.thickness;
	infoWall.floorInd		= elem.header.floorInd;
	infoWall.bottomOffset	= elem.wall.bottomOffset;
	infoWall.begX			= elem.wall.begC.x;
	infoWall.begY			= elem.wall.begC.y;
	infoWall.endX			= elem.wall.endC.x;
	infoWall.endY			= elem.wall.endC.y;

	ACAPI_DisposeElemMemoHdls (&memo);

	// (2) ���� ������ ������
	BNZeroMemory (&elem, sizeof (API_Element));
	elem.header.guid = morphs.Pop ();
	err = ACAPI_Element_Get (&elem);
	err = ACAPI_Element_Get3DInfo (elem.header, &info3D);

	// ���� ������ ���� ������(������ ���� ������) �ߴ�
	if (abs (info3D.bounds.zMax - info3D.bounds.zMin) < EPS) {
		ACAPI_WriteReport ("������ ������ ���� �ʽ��ϴ�.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// ������ GUID ����
	infoMorph.guid = elem.header.guid;

	// ������ ���ϴ�, ���� �� ����
	if (abs (elem.morph.tranmat.tmx [11] - info3D.bounds.zMin) < EPS) {
		// ���ϴ� ��ǥ ����
		infoMorph.leftBottomX = elem.morph.tranmat.tmx [3];
		infoMorph.leftBottomY = elem.morph.tranmat.tmx [7];
		infoMorph.leftBottomZ = elem.morph.tranmat.tmx [11];

		// ���� ��ǥ��?
		if (abs (infoMorph.leftBottomX - info3D.bounds.xMin) < EPS)
			infoMorph.rightTopX = info3D.bounds.xMax;
		else
			infoMorph.rightTopX = info3D.bounds.xMin;
		if (abs (infoMorph.leftBottomY - info3D.bounds.yMin) < EPS)
			infoMorph.rightTopY = info3D.bounds.yMax;
		else
			infoMorph.rightTopY = info3D.bounds.yMin;
		if (abs (infoMorph.leftBottomZ - info3D.bounds.zMin) < EPS)
			infoMorph.rightTopZ = info3D.bounds.zMax;
		else
			infoMorph.rightTopZ = info3D.bounds.zMin;
	} else {
		// ���� ��ǥ ����
		infoMorph.rightTopX = elem.morph.tranmat.tmx [3];
		infoMorph.rightTopY = elem.morph.tranmat.tmx [7];
		infoMorph.rightTopZ = elem.morph.tranmat.tmx [11];

		// ���ϴ� ��ǥ��?
		if (abs (infoMorph.rightTopX - info3D.bounds.xMin) < EPS)
			infoMorph.leftBottomX = info3D.bounds.xMax;
		else
			infoMorph.leftBottomX = info3D.bounds.xMin;
		if (abs (infoMorph.rightTopY - info3D.bounds.yMin) < EPS)
			infoMorph.leftBottomY = info3D.bounds.yMax;
		else
			infoMorph.leftBottomY = info3D.bounds.yMin;
		if (abs (infoMorph.rightTopZ - info3D.bounds.zMin) < EPS)
			infoMorph.leftBottomZ = info3D.bounds.zMax;
		else
			infoMorph.leftBottomZ = info3D.bounds.zMin;
	}

	// ������ Z�� ȸ�� ���� (���� ��ġ ����)
	dx = infoMorph.rightTopX - infoMorph.leftBottomX;
	dy = infoMorph.rightTopY - infoMorph.leftBottomY;
	infoMorph.ang = RadToDegree (atan2 (dy, dx));

	// ������ ���� ����
	infoMorph.horLen = GetDistance (info3D.bounds.xMin, info3D.bounds.yMin, info3D.bounds.xMax, info3D.bounds.yMax);

	// ������ ���� ����
	infoMorph.verLen = abs (info3D.bounds.zMax - info3D.bounds.zMin);

	// ���� ������ ���� ���� ���� ������Ʈ
	placingZone.leftBottomX		= infoMorph.leftBottomX;
	placingZone.leftBottomY		= infoMorph.leftBottomY;
	placingZone.leftBottomZ		= infoMorph.leftBottomZ;
	placingZone.horLen			= infoMorph.horLen;
	placingZone.verLen			= infoMorph.verLen;
	placingZone.ang				= DegreeToRad (infoMorph.ang);
	
	// �۾� �� ���� �ݿ� -- ����
	BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	workLevel_wall = 0.0;
	ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
	for (xx = 0 ; xx < (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
		if (storyInfo.data [0][xx].index == infoWall.floorInd) {
			workLevel_wall = storyInfo.data [0][xx].level;
			break;
		}
	}
	BMKillHandle ((GSHandle *) &storyInfo.data);

	// ���� ������ �� ������ ����
	placingZone.leftBottomZ = infoWall.bottomOffset;

	// ���� ���� ����
	API_Elem_Head* headList = new API_Elem_Head [1];
	headList [0] = elem.header;
	err = ACAPI_Element_Delete (&headList, 1);
	delete headList;

	// ���̺��� ���� �ʱ�ȭ
	placingZone.n1800w = 0;
	placingZone.n1850w = 0;
	placingZone.n1900w = 0;
	placingZone.n1950w = 0;
	placingZone.n2000w = 0;
	placingZone.n2050w = 0;
	placingZone.n2100w = 0;
	placingZone.n2150w = 0;
	placingZone.n2200w = 0;
	placingZone.n2250w = 0;
	placingZone.n2300w = 0;

	// ���̺��� ���� ���
	width = placingZone.horLen;
	while (width > EPS) {
		if (width > 2.300) {
			width -= 2.300;		placingZone.n2300w ++;
		} else if (width > 2.250) {
			width -= 2.250;		placingZone.n2250w ++;
		} else if (width > 2.200) {
			width -= 2.200;		placingZone.n2200w ++;
		} else if (width > 2.150) {
			width -= 2.150;		placingZone.n2150w ++;
		} else if (width > 2.100) {
			width -= 2.100;		placingZone.n2100w ++;
		} else if (width > 2.050) {
			width -= 2.050;		placingZone.n2050w ++;
		} else if (width > 2.000) {
			width -= 2.000;		placingZone.n2000w ++;
		} else if (width > 1.950) {
			width -= 1.950;		placingZone.n1950w ++;
		} else if (width > 1.900) {
			width -= 1.900;		placingZone.n1900w ++;
		} else if (width > 1.850) {
			width -= 1.850;		placingZone.n1850w ++;
		} else if (width > 1.800) {
			width -= 1.800;		placingZone.n1800w ++;
		} else {
			break;
		}
	}

	// [DIALOG] 1��° ���̾�α׿��� �� �ʺ� ������ ���̺��� ������ ������
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32517, ACAPI_GetOwnResModule (), wallTableformPlacerHandler, 0);

	// ������ �������� ���� ���� ������Ʈ
	infoWall.wallThk		+= (placingZone.gap * 2);

	if (result != DG_OK)
		return err;

	// ... �� ����, �� ������ �о�� �� ���̺��� ��ġ�ϱ�

	return	err;
}

// ���̺��� ��ġ�� ���� ���Ǹ� ��û�ϴ� ���̾�α�
short DGCALLBACK wallTableformPlacerHandler (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	API_UCCallbackType	ucb;

	short	xx;
	short	tableColumn;	// ���̺��� �ʺ� ���� ����
	short	buttonPosX;		// ������ ��ư ��ġ
	short	itmIdx;
	double	width;

	char	labelStr [32];

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, "���̺��� ���� ��ġ");

			//////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			// ���� ��ư
			DGSetItemText (dialogID, DG_OK, "Ȯ ��");

			// ���� ��ư
			DGSetItemText (dialogID, DG_CANCEL, "�� ��");

			//////////////////////////////////////////////////////////// ������ ��ġ (������)
			DGSetItemText (dialogID, LABEL_HEIGHT, "����");
			DGSetItemText (dialogID, LABEL_WIDTH, "�ʺ�");
			DGSetItemText (dialogID, LABEL_ERR_MESSAGE, "���̴� ���� ġ���� ������\n1500, 1800, 2100, 2400, 2700, 3000, 3300, 3600, 3900\n4200, 4500, 4800, 5100, 5400, 5700, 6000");
			DGSetItemText (dialogID, LABEL_GAP_LENGTH, "������ ����");

			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, "���纰 ���̾� ����");
			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM, "������");
			DGSetItemText (dialogID, LABEL_LAYER_RECTPIPE, "��� ������");
			DGSetItemText (dialogID, LABEL_LAYER_PINBOLT, "�ɺ�Ʈ ��Ʈ");
			DGSetItemText (dialogID, LABEL_LAYER_WALLTIE, "��ü Ÿ��");
			DGSetItemText (dialogID, LABEL_LAYER_CLAMP, "���� Ŭ����");
			DGSetItemText (dialogID, LABEL_LAYER_HEADPIECE, "����ǽ�");

			// ���� ��Ʈ�� �ʱ�ȭ
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_LAYER_EUROFORM;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, 1);

			ucb.itemID	 = USERCONTROL_LAYER_RECTPIPE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, 1);

			ucb.itemID	 = USERCONTROL_LAYER_PINBOLT;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, 1);

			ucb.itemID	 = USERCONTROL_LAYER_WALLTIE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, 1);

			ucb.itemID	 = USERCONTROL_LAYER_CLAMP;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_CLAMP, 1);

			ucb.itemID	 = USERCONTROL_LAYER_HEADPIECE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, 1);

			// ������ ��� �׸���
			tableColumn = 0;
			if (placingZone.n2300w > 0)		tableColumn += placingZone.n2300w;
			if (placingZone.n2250w > 0)		tableColumn += placingZone.n2250w;
			if (placingZone.n2200w > 0)		tableColumn += placingZone.n2200w;
			if (placingZone.n2150w > 0)		tableColumn += placingZone.n2150w;
			if (placingZone.n2100w > 0)		tableColumn += placingZone.n2100w;
			if (placingZone.n2050w > 0)		tableColumn += placingZone.n2050w;
			if (placingZone.n2000w > 0)		tableColumn += placingZone.n2000w;
			if (placingZone.n1950w > 0)		tableColumn += placingZone.n1950w;
			if (placingZone.n1900w > 0)		tableColumn += placingZone.n1900w;
			if (placingZone.n1850w > 0)		tableColumn += placingZone.n1850w;
			if (placingZone.n1800w > 0)		tableColumn += placingZone.n1800w;

			// ���̺��� ���� ���
			width = placingZone.horLen;
			while (width > EPS) {
				if (width > 2.300) {
					width -= 2.300;
				} else if (width > 2.250) {
					width -= 2.250;
				} else if (width > 2.200) {
					width -= 2.200;
				} else if (width > 2.150) {
					width -= 2.150;
				} else if (width > 2.100) {
					width -= 2.100;
				} else if (width > 2.050) {
					width -= 2.050;
				} else if (width > 2.000) {
					width -= 2.000;
				} else if (width > 1.950) {
					width -= 1.950;
				} else if (width > 1.900) {
					width -= 1.900;
				} else if (width > 1.850) {
					width -= 1.850;
				} else if (width > 1.800) {
					width -= 1.800;
				} else {
					break;
				}
			}

			// ���� �ʺ�
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 280, 20, 70, 23);
			DGSetItemText (dialogID, itmIdx, "���� �ʺ�");
			DGShowItem (dialogID, itmIdx);

			EDITCONTROL_REMAIN_WIDTH = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 350, 13, 50, 25);
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_WIDTH, width);
			DGDisableItem (dialogID, EDITCONTROL_REMAIN_WIDTH);
			DGShowItem (dialogID, EDITCONTROL_REMAIN_WIDTH);

			// ���̾�α� �ʺ� ����
			DGSetDialogSize (dialogID, DG_CLIENT, 350 + (tableColumn * 100), 500, DG_TOPLEFT, true);
			
			// ������
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 170, 45, tableColumn * 100 + 50, 110);
			DGShowItem (dialogID, itmIdx);

			width = placingZone.horLen;
			buttonPosX = 195;
			for (xx = 0 ; xx < tableColumn ; ++xx) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, buttonPosX, 50, 99, 100);
				DGShowItem (dialogID, itmIdx);

				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, buttonPosX + 20, 60, 60, 23);
				sprintf (labelStr, "�ʺ� (%d)", xx + 1);
				DGSetItemText (dialogID, itmIdx, labelStr);
				DGShowItem (dialogID, itmIdx);

				// �ʺ� �ش��ϴ� �޺��ڽ�
				POPUP_WIDTH [xx] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, buttonPosX + 20, 100, 60, 25);
				DGSetItemFont (dialogID, POPUP_WIDTH [xx], DG_IS_LARGE | DG_IS_PLAIN);
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "2300");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "2250");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "2200");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "2150");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "2100");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "2050");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "2000");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "1950");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "1900");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "1850");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "1800");
				DGShowItem (dialogID, POPUP_WIDTH [xx]);

				// �޺��ڽ��� �� ����
				if (width > EPS) {
					if (width > 2.300) {
						width -= 2.300;
						DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 1);
					} else if (width > 2.250) {
						width -= 2.250;
						DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 2);
					} else if (width > 2.200) {
						width -= 2.200;
						DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 3);
					} else if (width > 2.150) {
						width -= 2.150;
						DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 4);
					} else if (width > 2.100) {
						width -= 2.100;
						DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 5);
					} else if (width > 2.050) {
						width -= 2.050;
						DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 6);
					} else if (width > 2.000) {
						width -= 2.000;
						DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 7);
					} else if (width > 1.950) {
						width -= 1.950;
						DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 8);
					} else if (width > 1.900) {
						width -= 1.900;
						DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 9);
					} else if (width > 1.850) {
						width -= 1.850;
						DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 10);
					} else if (width > 1.800) {
						width -= 1.800;
						DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 11);
					}
				}
				buttonPosX += 100;
			}

			// ���� ���
			DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT, placingZone.verLen);
			DGDisableItem (dialogID, EDITCONTROL_HEIGHT);

			// ���� ���� �޾Ƶ��� �� �ִ� ���ΰ�?
			if ( (abs (placingZone.verLen - 1.500) < EPS) || (abs (placingZone.verLen - 1.800) < EPS) || (abs (placingZone.verLen - 2.100) < EPS) || (abs (placingZone.verLen - 2.400) < EPS) ||
				(abs (placingZone.verLen - 2.700) < EPS) || (abs (placingZone.verLen - 3.000) < EPS) || (abs (placingZone.verLen - 3.300) < EPS) || (abs (placingZone.verLen - 3.600) < EPS) ||
				(abs (placingZone.verLen - 3.900) < EPS) || (abs (placingZone.verLen - 4.200) < EPS) || (abs (placingZone.verLen - 4.500) < EPS) || (abs (placingZone.verLen - 4.800) < EPS) ||
				(abs (placingZone.verLen - 5.100) < EPS) || (abs (placingZone.verLen - 5.400) < EPS) || (abs (placingZone.verLen - 5.700) < EPS) || (abs (placingZone.verLen - 6.000) < EPS) ) {

				DGHideItem (dialogID, LABEL_ERR_MESSAGE);
			} else {
				DGShowItem (dialogID, LABEL_ERR_MESSAGE);
			}

			// �ʺ� ���
			DGSetItemValDouble (dialogID, EDITCONTROL_WIDTH, placingZone.horLen);
			DGDisableItem (dialogID, EDITCONTROL_WIDTH);

			break;

		case DG_MSG_CHANGE:

			// ... POPUP_WIDTH [50] : �ʺ� �޺��ڽ��� ������ ������ ���� �ʺ� ���

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					//////////////////////////////////////// ���̾�α� â ������ �Է� ����
					// ... POPUP_WIDTH [50] : �� ������ �� ���� �Է�

					// ������ ����
					placingZone.gap = DGGetItemValDouble (dialogID, EDITCONTROL_GAP_LENGTH);

					// ���̾� ��ȣ ����
					layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM);
					layerInd_RectPipe		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE);
					layerInd_PinBolt		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT);
					layerInd_WallTie		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE);
					layerInd_Clamp			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_CLAMP);
					layerInd_HeadPiece		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE);
	
					break;
				case DG_CANCEL:
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