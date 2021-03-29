#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "Quantities.hpp"

using namespace quantitiesDG;

short				nLayers;		// ���̾� ����
GS::Array<short>	layerInds;		// ���̴� ���̾� �ε���
GS::Array<string>	layerNames;		// ���̴� ���̾� �̸�

short				selectedCombobox;	// ������ �޺��ڽ� ����

// ����(��,������,��,���)���� �����簡 ���� �� �ִ� �鿡 ���������� �ڵ����� ������
GSErrCode	placeQuantityPlywood (void)
{
	GSErrCode	err = NoError;

	API_Element				elem;
	API_ElementMemo			memo;
	API_ElemInfo3D			info3D;

	short	xx, yy;
	long	nElems;		// ��� ����
	qElem*	elems;		// ��� �迭
	short	result;		// ���̾�α� ���� ��
	
	API_Attribute	attrib;

	GS::Array<API_Guid>	elemList_All;		// ��ҵ��� GUID (��ü ���� ���)
	GS::Array<API_Guid>	elemList_Wall;		// ��ҵ��� GUID (��)
	GS::Array<API_Guid>	elemList_Slab;		// ��ҵ��� GUID (������)
	GS::Array<API_Guid>	elemList_Beam;		// ��ҵ��� GUID (��)
	GS::Array<API_Guid>	elemList_Column;	// ��ҵ��� GUID (���)


	// ������Ʈ �� ���̾� ������ �˾Ƴ�
	BNZeroMemory (&attrib, sizeof (API_Attribute));
	attrib.header.typeID = API_LayerID;
	err = ACAPI_Attribute_GetNum (API_LayerID, &nLayers);

	// ���̴� ���̾��� �ε���, �̸� ����
	for (xx = 1; xx <= nLayers ; ++xx) {
		attrib.header.index = xx;
		err = ACAPI_Attribute_Get (&attrib);

		if (err == NoError) {
			if ((attrib.layer.head.flags & APILay_Hidden) == false) {
				layerInds.Push (attrib.layer.head.index);
				layerNames.Push (attrib.layer.head.name);
			}
		}
	}

	// ���̾� ���� ������Ʈ
	nLayers = layerInds.GetSize ();

	// ���̴� ���̾� ���� ��, ������, ��, ��� ��ü�� ������
	ACAPI_Element_GetElemList (API_WallID, &elemList_Wall, APIFilt_OnVisLayer);
	ACAPI_Element_GetElemList (API_SlabID, &elemList_Slab, APIFilt_OnVisLayer);
	ACAPI_Element_GetElemList (API_BeamID, &elemList_Beam, APIFilt_OnVisLayer);
	ACAPI_Element_GetElemList (API_ColumnID, &elemList_Column, APIFilt_OnVisLayer);
	while (!elemList_Wall.IsEmpty ())
		elemList_All.Push (elemList_Wall.Pop ());
	while (!elemList_Slab.IsEmpty ())
		elemList_All.Push (elemList_Slab.Pop ());
	while (!elemList_Beam.IsEmpty ())
		elemList_All.Push (elemList_Beam.Pop ());
	while (!elemList_Column.IsEmpty ())
		elemList_All.Push (elemList_Column.Pop ());

	// ��� ����Ʈ ���� ���� ����ü �迭�� ������ ��
	nElems = elemList_All.GetSize ();
	elems = new qElem [nElems];
	for (xx = 0 ; xx < nElems ; ++xx) {
		// ��� �迭 ������Ʈ
		elems [xx].guid = elemList_All.Pop ();

		// ��Ҹ� ������ �� �ʵ尪 ä���
		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		elem.header.guid = elems [xx].guid;
		err = ACAPI_Element_Get (&elem);						// elem.wall.poly.nCoords : ������ ���� ������ �� ����
		err = ACAPI_Element_GetMemo (elem.header.guid, &memo);	// memo.coords : ������ ��ǥ�� ������ �� ����
		err = ACAPI_Element_Get3DInfo (elem.header, &info3D);

		// ��� Ÿ�� ����
		if (elem.header.typeID == API_WallID)
			elems [xx].typeOfElem = ELEM_WALL;
		else if (elem.header.typeID == API_SlabID)
			elems [xx].typeOfElem = ELEM_SLAB;
		else if (elem.header.typeID == API_BeamID)
			elems [xx].typeOfElem = ELEM_BEAM;
		else if (elem.header.typeID == API_ColumnID)
			elems [xx].typeOfElem = ELEM_COLUMN;
		else
			elems [xx].typeOfElem = ELEM_UNKNOWN;

		// �� �ε��� ����
		elems [xx].floorInd = elem.header.floorInd;

		// ���̾� �ε��� ����
		elems [xx].layerInd = elem.header.layer;

		// x, y, z�� �ּ�, �ִ밪 ����
		elems [xx].bottomPoint.x = info3D.bounds.xMin;
		elems [xx].bottomPoint.y = info3D.bounds.yMin;
		elems [xx].bottomPoint.z = info3D.bounds.zMin;
		elems [xx].topPoint.x = info3D.bounds.xMax;
		elems [xx].topPoint.y = info3D.bounds.yMax;
		elems [xx].topPoint.z = info3D.bounds.zMax;

		// memo ��ü ����
		ACAPI_DisposeElemMemoHdls (&memo);
	}

	// [���̾�α�] �������� ���̾� ����
	result = DGBlankModalDialog (600, 100, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, quantityPlywoodUIHandler, 0);

	//for (xx = 0 ; xx < nElems ; ++xx) {
	//	for (yy = 0 ; yy < nElems ; ++yy) {
	// ��, �ڱ� �ڽŰ��� �񱳴� ���� �ʴ´�. (���� xx��° guid�� yy��° guid�� �����ϸ� continue)

	// *** (�߿�) ������ ��Ҵ� ���� �Ǵ� �ظ鿡 ���� X, Y, Z�� �� ���� ������ ���� �־�� ��

	// �˰���
	// 1. xx�� ���� ���
		// yy�� ���� ���
			// ���� 1
			// ���� 2
				// yy�� ���� xx�� X-Y ������ ����
		// yy�� �������� ���
			// ���� 1
			// ���� 2
				// yy�� ���� xx�� Z ������ ����
		// yy�� ���� ���
			// ���� 1
			// ���� 2
				// yy�� ���� xx�� Z ������ ����
		// yy�� ����� ���
			// ���� 1
			// ���� 2
				// yy�� ���� xx�� X-Y ������ ����
	// 2. xx�� �������� ���
		// yy�� ���� ���
			// �ظ�
				// yy�� ���� xx�� X-Y ������ ����
		// yy�� �������� ���
			// �ظ�
				// yy�� ���� xx�� X-Y ������ ����
		// yy�� ���� ���
			// �ظ�
				// yy�� ���� xx�� X-Y ������ ����
		// yy�� ����� ���
			// �ظ�
				// yy�� ���� xx�� X-Y ������ ����
	// 3. xx�� ���� ���
		// yy�� ���� ���
			// �ظ�
				// yy�� ���� xx�� X-Y ������ ����
			// ���� 1
			// ���� 2
				// yy�� ���� xx�� Z ������ ����
		// yy�� �������� ���
			// �ظ�
				// yy�� ���� xx�� X-Y ������ ����
			// ���� 1
			// ���� 2
				// yy�� ���� xx�� Z ������ ����
		// yy�� ���� ���
			// �ظ�
				// yy�� ���� xx�� X-Y ������ ����
			// ���� 1
			// ���� 2
				// yy�� ���� xx�� Z ������ ����
		// yy�� ����� ���
			// �ظ�
				// yy�� ���� xx�� X-Y ������ ����
			// ���� 1
			// ���� 2
				// yy�� ���� xx�� Z ������ ����
	// 4. xx�� ����� ���
		// yy�� ���� ���
			// ���� 1
			// ���� 2
			// ���� 3
			// ���� 4
				// yy�� ���� xx�� Z ������ ����
		// yy�� �������� ���
			// ���� 1
			// ���� 2
			// ���� 3
			// ���� 4
				// yy�� ���� xx�� Z ������ ����
		// yy�� ���� ���
			// ���� 1
			// ���� 2
			// ���� 3
			// ���� 4
				// yy�� ���� xx�� Z ������ ����
		// yy�� ����� ���
			// ���� 1
			// ���� 2
			// ���� 3
			// ���� 4
				// yy�� ���� xx�� Z ������ ����

	// ȭ�� ���ΰ�ħ
	//ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	//bool	regenerate = true;
	//ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

	delete []	elems;

	return	err;
}

// [���̾�α�] �������� ���� ���̾� ����
short DGCALLBACK quantityPlywoodUIHandler (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;
	short	itmPosX, itmPosY;
	short	xx;
	//char	tempStr [20];
	short	dialogSizeX, dialogSizeY;

	GSErrCode err = NoError;
	//API_ModulData  info;

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, "�������� �����ϱ�");

			// Ȯ�� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 20, 60, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "Ȯ��");
			DGShowItem (dialogID, DG_OK);

			// ��� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 55, 60, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "���");
			DGShowItem (dialogID, DG_CANCEL);
			
			// ��: ���� ���̾�
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 120, 25, 150, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "���� ���̾�");
			DGShowItem (dialogID, itmIdx);

			// ��: �������� ���̾�
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 280, 25, 150, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "�������� ���̾�");
			DGShowItem (dialogID, itmIdx);

			itmPosX = 120;
			itmPosY = 50;

			// ���� ���̾�: ó������ 1�ุ ǥ���� ��
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 500, 25, 120, itmPosY, 150, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "����");
			for (xx = 0 ; xx < nLayers ; ++xx) {
				DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, layerNames.Get (xx).c_str ());
			}
			DGShowItem (dialogID, itmIdx);

			break;
		
		case DG_MSG_CHANGE:

			// ...
			// selectedCombobox : ������ �޺��ڽ� ����?

			// itmPosX, itmPosY
			// dialogSizeX, dialogSizeY
			// DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);

			// ���̾� ������ŭ for�� �ݺ�
			// 1�� (���̾� �̸�)		2�� (�������� ���̾�)
									//	�˾�: ���� ���̾� / EditControl: ���̾� �̸�

			// UI (��� ���̾� - �������� ���̾�� 1:1�� ��Ī���Ѿ� ��)
			// ���̾� ���� ����� ���� �ֵ�� - ���̾� ��� ����
			// *** ��� ���̾�� ���� ���̴� ���̾ �� �о� ���� �� ��� ������ ��
				// ���� ���̾�		�������� ���̾�				�������� Ÿ��
				//					���� | ���� | Ŀ���� �Է�	��, ������, ��, ���

			// �Ʒ��� ���� ���� (��ȿ���� ����)
			// ��� ���̾� ����(����)	-->	�������� ���̾� ����(����)		(�������� ������)
				// �� (����)					�� (����)						����(��)
				// �� (�ܺ�)					�� (�ܺ�)						...
				// �� (�պ�)					�� (�պ�)						...
				// �� (�Ķ���)					�� (�Ķ���)						...
				// �� (�����)					�� (�����)						...
				// ������ (����)				������ (����)					�Ϻ�
				// ������ (RC)					������ (RC)						...
				// ������ (��ũ)				������ (��ũ)					...
				// ������ (����)				������ (����)					...
				// ��							��								����(��), �Ϻ�
				// ��� (����)					��� (����)						����(��)
				// ��� (��ü)					��� (��ü)						...
				// ��� (����)					��� (����)						...

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:

					// elems�� layerInd�� ��Ī�ϴ� qLayerInd ������ ��

					break;

				case DG_CANCEL:
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
	}

	result = item;

	return	result;
}
