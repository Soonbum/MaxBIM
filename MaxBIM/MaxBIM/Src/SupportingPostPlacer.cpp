#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "SupportingPostPlacer.hpp"

using namespace SupportingPostPlacerDG;

static short	layerInd_vPost;		// ���̾� ��ȣ: ������
static short	layerInd_hPost;		// ���̾� ��ȣ: ������

static GS::Array<API_Guid>	elemList;	// �׷�ȭ�� ���� ������ ��������� GUID�� ���� ������

// ������ ������ü ������ ������� PERI ���ٸ��� ��ġ��
GSErrCode	placePERIPost (void)
{
	GSErrCode	err = NoError;
	short		result;
	long		nSel;
	short		xx, yy;

	// Selection Manager ���� ����
	API_SelectionInfo		selectionInfo;
	API_Element				tElem;
	API_Neig				**selNeigs;
	GS::Array<API_Guid>&	morphs = GS::Array<API_Guid> ();
	long					nMorphs = 0;

	// ��ü ���� ��������
	API_Element				elem;
	API_ElementMemo			memo;
	API_ElemInfo3D			info3D;

	// ���� 3D ������� ��������
	API_Component3D			component;
	API_Tranmat				tm;
	Int32					nVert, nEdge, nPgon;
	Int32					elemIdx, bodyIdx;
	API_Coord3D				trCoord;
	GS::Array<API_Coord3D>&	coords = GS::Array<API_Coord3D> ();
	long					nNodes;
	API_Coord3D				point3D;

	// ���� ��ü ����
	InfoMorphForSupportingPost	infoMorph;

	// �۾� �� ����
	API_StoryInfo	storyInfo;
	double			workLevel_morph;	// ������ �۾� �� ����


	// ������ ��� ��������
	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err == APIERR_NOPLAN) {
		ACAPI_WriteReport ("���� ������Ʈ â�� �����ϴ�.", true);
	}
	if (err == APIERR_NOSEL) {
		ACAPI_WriteReport ("�ƹ� �͵� �������� �ʾҽ��ϴ�.\n�ʼ� ����: ���� ������ü (1��)", true);
		//ACAPI_WriteReport ("�ƹ� �͵� �������� �ʾҽ��ϴ�.\n�ʼ� ����: ���ٸ� �Ϻο� ��ġ�ϴ� ������ �Ǵ� �� (1��), ���� ������ü (1��)", true);
	}
	if (err != NoError) {
		BMKillHandle ((GSHandle *) &selNeigs);
		return err;
	}

	// ���� 1�� �����ؾ� ��
	if (selectionInfo.typeID != API_SelEmpty) {
		nSel = BMGetHandleSize ((GSHandle) selNeigs) / sizeof (API_Neig);
		for (xx = 0 ; xx < nSel && err == NoError ; ++xx) {
			tElem.header.typeID = Neig_To_ElemID ((*selNeigs)[xx].neigID);

			tElem.header.guid = (*selNeigs)[xx].guid;
			if (ACAPI_Element_Get (&tElem) != NoError)	// ������ �� �ִ� ����ΰ�?
				continue;

			if (tElem.header.typeID == API_MorphID)		// �����ΰ�?
				morphs.Push (tElem.header.guid);
		}
	}
	BMKillHandle ((GSHandle *) &selNeigs);
	nMorphs = morphs.GetSize ();

	// ������ 1���ΰ�?
	if (nMorphs != 1) {
		ACAPI_WriteReport ("������ü ������ 1�� �����ϼž� �մϴ�.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// ���� ������ ������
	BNZeroMemory (&elem, sizeof (API_Element));
	elem.header.guid = morphs.Pop ();
	err = ACAPI_Element_Get (&elem);
	err = ACAPI_Element_Get3DInfo (elem.header, &info3D);

	// ���� ������ ���� ���̰� 0�̸� �ߴ�
	if (abs (info3D.bounds.zMax - info3D.bounds.zMin) < EPS) {
		ACAPI_WriteReport ("������ü ������ ���� ���̰� �����ϴ�.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// ������ GUID ����
	infoMorph.guid = elem.header.guid;

	// ������ 3D �ٵ� ������
	BNZeroMemory (&component, sizeof (API_Component3D));
	component.header.typeID = API_BodyID;
	component.header.index = info3D.fbody;
	err = ACAPI_3D_GetComponent (&component);

	nVert = component.body.nVert;
	nEdge = component.body.nEdge;
	nPgon = component.body.nPgon;
	tm = component.body.tranmat;
	elemIdx = component.body.head.elemIndex - 1;
	bodyIdx = component.body.head.bodyIndex - 1;
	
	yy = 0;

	// ������ 8�� ������ ���ϱ�
	for (xx = 1 ; xx <= nVert ; ++xx) {
		component.header.typeID	= API_VertID;
		component.header.index	= xx;
		err = ACAPI_3D_GetComponent (&component);
		if (err == NoError) {
			trCoord.x = tm.tmx[0]*component.vert.x + tm.tmx[1]*component.vert.y + tm.tmx[2]*component.vert.z + tm.tmx[3];
			trCoord.y = tm.tmx[4]*component.vert.x + tm.tmx[5]*component.vert.y + tm.tmx[6]*component.vert.z + tm.tmx[7];
			trCoord.z = tm.tmx[8]*component.vert.x + tm.tmx[9]*component.vert.y + tm.tmx[10]*component.vert.z + tm.tmx[11];
			// ���⼭ ���� ������ ����Ʈ�� �߰����� �ʴ´�.
			if ((abs (trCoord.x) < EPS) && (abs (trCoord.y) < EPS) && (abs (trCoord.z - 1.0) < EPS))		// 1�� (0, 0, 1)
				; // pass
			else if ((abs (trCoord.x) < EPS) && (abs (trCoord.y - 1.0) < EPS) && (abs (trCoord.z) < EPS))	// 2�� (0, 1, 0)
				; // pass
			else if ((abs (trCoord.x - 1.0) < EPS) && (abs (trCoord.y) < EPS) && (abs (trCoord.z) < EPS))	// 3�� (1, 0, 0)
				; // pass
			else if ((abs (trCoord.x) < EPS) && (abs (trCoord.y) < EPS) && (abs (trCoord.z) < EPS))			// 4�� (0, 0, 0)
				; // pass
			else {
				//placeCoordinateLabel (trCoord.x, trCoord.y, trCoord.z);
				coords.Push (trCoord);
				if (yy < 8) infoMorph.points [yy++] = trCoord;
			}
		}
	}
	nNodes = coords.GetSize ();

	// ������ ���� ���� ���ϱ�
	infoMorph.leftBottomX = infoMorph.points [0].x;
	infoMorph.leftBottomY = infoMorph.points [0].y;
	infoMorph.leftBottomZ = infoMorph.points [0].z;
	infoMorph.rightTopX = infoMorph.points [0].x;
	infoMorph.rightTopY = infoMorph.points [0].y;
	infoMorph.rightTopZ = infoMorph.points [0].z;

	for (xx = 0 ; xx < 8 ; ++xx) {
		if (infoMorph.leftBottomX > infoMorph.points [xx].x)	infoMorph.leftBottomX = infoMorph.points [xx].x;
		if (infoMorph.leftBottomY > infoMorph.points [xx].y)	infoMorph.leftBottomY = infoMorph.points [xx].y;
		if (infoMorph.leftBottomZ > infoMorph.points [xx].z)	infoMorph.leftBottomZ = infoMorph.points [xx].z;

		if (infoMorph.rightTopX < infoMorph.points [xx].x)		infoMorph.rightTopX = infoMorph.points [xx].x;
		if (infoMorph.rightTopY < infoMorph.points [xx].y)		infoMorph.rightTopY = infoMorph.points [xx].y;
		if (infoMorph.rightTopZ < infoMorph.points [xx].z)		infoMorph.rightTopZ = infoMorph.points [xx].z;
	}
	infoMorph.ang = 0.0;
	infoMorph.width = GetDistance (infoMorph.leftBottomX, infoMorph.leftBottomY, infoMorph.rightTopX, infoMorph.leftBottomY);
	infoMorph.depth = GetDistance (infoMorph.leftBottomX, infoMorph.leftBottomY, infoMorph.leftBottomX, infoMorph.rightTopY);
	infoMorph.height = abs (info3D.bounds.zMax - info3D.bounds.zMin);

	// [���̾�α�] ������ �ܼ� (1/2��, ���̰� 6���� �ʰ��Ǹ� 2�� ������ ��), �������� �԰�/����, ������ ����(��, ���̰� 3500 �̻��̸� �߰��� ���� ������ ��), ������ �ʺ�, ũ�ν���� ����, ������/������ ���̾�
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32521, ACAPI_GetOwnResModule (), PERISupportingPostPlacerHandler1, (DGUserData) &infoMorph);

	// ���� ���� ����
	API_Elem_Head* headList = new API_Elem_Head [1];
	headList [0] = elem.header;
	err = ACAPI_Element_Delete (&headList, 1);
	delete headList;

	// �Է��� �����͸� ������� ������, ������ ��ġ
	if (result == DG_OK) {
		// ...
	}

	return	err;
}

// ������ �ܼ� (1/2��, ���̰� 6���� �ʰ��Ǹ� 2�� ������ ��), �������� �԰�/����, ������ ����(��, ���̰� 3500 �̻��̸� �߰��� ���� ������ ��), ������ �ʺ�, ũ�ν���� ����, ������/������ ���̾ ����
short DGCALLBACK PERISupportingPostPlacerHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData /* msgData */)
{
	InfoMorphForSupportingPost *dlgData = (InfoMorphForSupportingPost *) userData;
	short	result;
	API_UCCallbackType	ucb;

	short	itmIdx;

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, "PERI ���ٸ� �ڵ� ��ġ");

			//////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			// ���� ��ư
			DGSetItemText (dialogID, DG_OK, "Ȯ ��");

			// ���� ��ư
			DGSetItemText (dialogID, DG_CANCEL, "�� ��");

			//////////////////////////////////////////////////////////// ������ ��ġ (������)
			DGSetItemText (dialogID, LABEL_VPOST, "������");				// ��: ������
			DGSetItemText (dialogID, LABEL_HPOST, "������");				// ��: ������

			DGSetItemText (dialogID, LABEL_TOTAL_HEIGHT, "�� ����");		// ��: �� ����
			DGSetItemText (dialogID, LABEL_REMAIN_HEIGHT, "���� ����");		// ��: ���� ����

			DGSetItemText (dialogID, CHECKBOX_CROSSHEAD, "ũ�ν����");		// üũ�ڽ�: ũ�ν����

			DGSetItemText (dialogID, CHECKBOX_VPOST1, "1��");				// üũ�ڽ�: 1��
			DGSetItemText (dialogID, LABEL_VPOST1_NOMINAL, "�԰�");			// ��: �԰�
			DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "����");			// ��: ����
			DGSetItemText (dialogID, CHECKBOX_VPOST2, "2��");				// üũ�ڽ�: 2��
			DGSetItemText (dialogID, LABEL_VPOST2_NOMINAL, "�԰�");			// ��: �԰�
			DGSetItemText (dialogID, LABEL_VPOST2_HEIGHT, "����");			// ��: ����

			DGSetItemText (dialogID, CHECKBOX_HPOST, "������");				// üũ�ڽ�: ������

			DGSetItemText (dialogID, LABEL_PLAN_WIDTH, "����");				// ��: ����
			DGSetItemText (dialogID, LABEL_PLAN_DEPTH, "����");				// ��: ����

			DGSetItemText (dialogID, LABEL_WIDTH_NORTH, "�ʺ�(��)");		// ��: �ʺ�(��)
			DGSetItemText (dialogID, LABEL_WIDTH_WEST, "�ʺ�(��)");			// ��: �ʺ�(��)
			DGSetItemText (dialogID, LABEL_WIDTH_EAST, "�ʺ�(��)");			// ��: �ʺ�(��)
			DGSetItemText (dialogID, LABEL_WIDTH_SOUTH, "�ʺ�(��)");		// ��: �ʺ�(��)

			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, "���纰 ���̾� ����");
			DGSetItemText (dialogID, LABEL_LAYER_VPOST, "������");
			DGSetItemText (dialogID, LABEL_LAYER_HPOST, "������");

			// üũ�ڽ�: ���̾� ����
			DGSetItemText (dialogID, CHECKBOX_LAYER_COUPLING, "���̾� ����");
			DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, TRUE);

			// ���� ��Ʈ�� �ʱ�ȭ
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_LAYER_VPOST;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_VPOST, 1);

			ucb.itemID	 = USERCONTROL_LAYER_HPOST;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_HPOST, 1);

			// �ʱ� ����
			// 1. ������ �԰� �˾� �߰� - MP 120, MP 250, MP 350, MP 480, MP 625
			DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "MP 120");
			DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "MP 250");
			DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "MP 350");
			DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "MP 480");
			DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "MP 625");

			DGPopUpInsertItem (dialogID, POPUP_VPOST2_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST2_NOMINAL, DG_POPUP_BOTTOM, "MP 120");
			DGPopUpInsertItem (dialogID, POPUP_VPOST2_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST2_NOMINAL, DG_POPUP_BOTTOM, "MP 250");
			DGPopUpInsertItem (dialogID, POPUP_VPOST2_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST2_NOMINAL, DG_POPUP_BOTTOM, "MP 350");
			DGPopUpInsertItem (dialogID, POPUP_VPOST2_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST2_NOMINAL, DG_POPUP_BOTTOM, "MP 480");
			DGPopUpInsertItem (dialogID, POPUP_VPOST2_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST2_NOMINAL, DG_POPUP_BOTTOM, "MP 625");
			
			// 2. ������ ���� �� ���� ���� ǥ�� - MP 120 (800~1200), MP 250 (1450~2500), MP 350 (1950~3500), MP 480 (2600~4800), MP 625 (4300~6250)
			DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 800~1200");
			DGSetItemText (dialogID, LABEL_VPOST2_HEIGHT, "H. 800~1200");

			// 3. ������ 1�� On, 2�� Off
			DGSetItemValLong (dialogID, CHECKBOX_VPOST1, TRUE);
			DGSetItemValLong (dialogID, CHECKBOX_VPOST2, FALSE);

			// 4. �������� �ʺ� 4�� ��Ȱ��ȭ
			DGSetItemValLong (dialogID, CHECKBOX_HPOST, FALSE);
			DGDisableItem (dialogID, LABEL_WIDTH_NORTH);
			DGDisableItem (dialogID, LABEL_WIDTH_SOUTH);
			DGDisableItem (dialogID, LABEL_WIDTH_WEST);
			DGDisableItem (dialogID, LABEL_WIDTH_EAST);
			DGDisableItem (dialogID, POPUP_WIDTH_NORTH);
			DGDisableItem (dialogID, POPUP_WIDTH_SOUTH);
			DGDisableItem (dialogID, POPUP_WIDTH_WEST);
			DGDisableItem (dialogID, POPUP_WIDTH_EAST);

			// 5. �������� �ʺ� �˾� �߰� - ����, 625, 750, 900, 1200, 1375, 1500, 2015, 2250, 2300, 2370, 2660, 2960
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "����");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "625");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "750");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "900");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "1200");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "1375");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "1500");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "2015");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "2250");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "2300");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "2370");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "2660");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "2960");

			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "����");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "625");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "750");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "900");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "1200");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "1375");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "1500");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "2015");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "2250");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "2300");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "2370");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "2660");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "2960");

			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "����");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "625");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "750");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "900");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "1200");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "1375");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "1500");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "2015");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "2250");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "2300");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "2370");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "2660");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "2960");

			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "����");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "625");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "750");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "900");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "1200");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "1375");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "1500");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "2015");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "2250");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "2300");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "2370");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "2660");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "2960");

			// 6. ������ ���� �Է� ���� ����
			DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 0.800);
			DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 1.200);
			DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 0.800);
			DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 1.200);

			// 7. �� ����, ���� ���� ��Ȱ��ȭ �� �� ���
			DGDisableItem (dialogID, EDITCONTROL_TOTAL_HEIGHT);
			DGDisableItem (dialogID, EDITCONTROL_REMAIN_HEIGHT);
			DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_HEIGHT, dlgData->height);
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HEIGHT, dlgData->height - (DGGetItemValDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT) * DGGetItemValLong (dialogID, CHECKBOX_VPOST1) + DGGetItemValDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT) * DGGetItemValLong (dialogID, CHECKBOX_VPOST2)) - 0.003 * DGGetItemValLong (dialogID, CHECKBOX_CROSSHEAD));

			// 8. ������ ����, ���� �� ��Ȱ��ȭ �� �� ���
			DGDisableItem (dialogID, EDITCONTROL_PLAN_WIDTH);
			DGDisableItem (dialogID, EDITCONTROL_PLAN_DEPTH);
			DGSetItemValDouble (dialogID, EDITCONTROL_PLAN_WIDTH, dlgData->width);
			DGSetItemValDouble (dialogID, EDITCONTROL_PLAN_DEPTH, dlgData->depth);

			break;

		case DG_MSG_CHANGE:
			// 1. ���̾� ���� üũ�ڽ� On/Off�� ���� �̺�Ʈ ó��
			if (DGGetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING) == 1) {
				switch (item) {
					case USERCONTROL_LAYER_VPOST:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HPOST, DGGetItemValLong (dialogID, USERCONTROL_LAYER_VPOST));
						break;
					case USERCONTROL_LAYER_HPOST:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_VPOST, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HPOST));
						break;
				}
			}

			// 2. ������ �԰��� �ٲ� ������ ���� ���� ���ڿ��� ����ǰ�, ������ ���� ���� �ּ�/�ִ밪 ����� - MP 120 (800~1200), MP 250 (1450~2500), MP 350 (1950~3500), MP 480 (2600~4800), MP 625 (4300~6250)
			if (DGPopUpGetSelected (dialogID, POPUP_VPOST1_NOMINAL) == 1) {
				DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 800~1200");
				DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 0.800);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 1.200);
			} else if (DGPopUpGetSelected (dialogID, POPUP_VPOST1_NOMINAL) == 2) {
				DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 1450~2500");
				DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 1.450);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 2.500);
			} else if (DGPopUpGetSelected (dialogID, POPUP_VPOST1_NOMINAL) == 3) {
				DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 1950~3500");
				DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 1.950);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 3.500);
			} else if (DGPopUpGetSelected (dialogID, POPUP_VPOST1_NOMINAL) == 4) {
				DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 2600~4800");
				DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 2.600);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 4.800);
			} else if (DGPopUpGetSelected (dialogID, POPUP_VPOST1_NOMINAL) == 5) {
				DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 4300~6250");
				DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 4.300);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 6.250);
			}

			if (DGPopUpGetSelected (dialogID, POPUP_VPOST2_NOMINAL) == 1) {
				DGSetItemText (dialogID, LABEL_VPOST2_HEIGHT, "H. 800~1200");
				DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 0.800);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 1.200);
			} else if (DGPopUpGetSelected (dialogID, POPUP_VPOST2_NOMINAL) == 2) {
				DGSetItemText (dialogID, LABEL_VPOST2_HEIGHT, "H. 1450~2500");
				DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 1.450);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 2.500);
			} else if (DGPopUpGetSelected (dialogID, POPUP_VPOST2_NOMINAL) == 3) {
				DGSetItemText (dialogID, LABEL_VPOST2_HEIGHT, "H. 1950~3500");
				DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 1.950);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 3.500);
			} else if (DGPopUpGetSelected (dialogID, POPUP_VPOST2_NOMINAL) == 4) {
				DGSetItemText (dialogID, LABEL_VPOST2_HEIGHT, "H. 2600~4800");
				DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 2.600);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 4.800);
			} else if (DGPopUpGetSelected (dialogID, POPUP_VPOST2_NOMINAL) == 5) {
				DGSetItemText (dialogID, LABEL_VPOST2_HEIGHT, "H. 4300~6250");
				DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 4.300);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 6.250);
			}

			// 3. ������ �԰��� �ٲ�ų�, ������ 1��/2�� üũ�ڽ� ���°� �ٲ�ų�, ������ ���̰� �ٲ�ų�, ũ�ν���� üũ ���¿� ���� ���� ���� �����
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HEIGHT, dlgData->height - (DGGetItemValDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT) * DGGetItemValLong (dialogID, CHECKBOX_VPOST1) + DGGetItemValDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT) * DGGetItemValLong (dialogID, CHECKBOX_VPOST2)) - 0.003 * DGGetItemValLong (dialogID, CHECKBOX_CROSSHEAD));

			// 4. ������ üũ�ڽ��� ���� ������ UI�� �ʺ� ���� ���� (����, ����) Ȱ��ȭ/��Ȱ��ȭ
			if (DGGetItemValLong (dialogID, CHECKBOX_HPOST) == TRUE) {
				DGEnableItem (dialogID, LABEL_WIDTH_WEST);
				DGEnableItem (dialogID, POPUP_WIDTH_WEST);
				DGEnableItem (dialogID, LABEL_WIDTH_SOUTH);
				DGEnableItem (dialogID, POPUP_WIDTH_SOUTH);
			} else {
				DGDisableItem (dialogID, LABEL_WIDTH_WEST);
				DGDisableItem (dialogID, POPUP_WIDTH_WEST);
				DGDisableItem (dialogID, LABEL_WIDTH_SOUTH);
				DGDisableItem (dialogID, POPUP_WIDTH_SOUTH);
			}

			// 5. ������ ����/���� �ʺ� �ٲ�� ����/���� �ʺ� �����ϰ� �����
			DGPopUpSelectItem (dialogID, POPUP_WIDTH_EAST, DGPopUpGetSelected (dialogID, POPUP_WIDTH_WEST));
			DGPopUpSelectItem (dialogID, POPUP_WIDTH_NORTH, DGPopUpGetSelected (dialogID, POPUP_WIDTH_SOUTH));

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// ...
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