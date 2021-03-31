#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "Quantities.hpp"

using namespace quantitiesDG;

long	nElems;		// ��� ����
qElem*	elems;		// ��� �迭

short				nLayers;		// ���̾� ����
short				nObjects;		// �������� ��ü Ÿ�� ����
GS::Array<short>	layerInds;		// ���̴� ���̾� �ε���
GS::Array<string>	layerNames;		// ���̴� ���̾� �̸�
GS::Array<string>	objType;		// ��ü Ÿ��

short	itmPosX, itmPosY;

short	nPopupControl;				// �˾� ��Ʈ�� ��
short	existingLayerPopup [50];	// �˾� ��Ʈ��: ���� ���̾�
short	qLayerPopup [50];			// �˾� ��Ʈ��: �������� ���̾�
short	objTypePopup [50];			// �˾� ��Ʈ��: ��ü Ÿ��

// ����(��,������,��,���)���� �����簡 ���� �� �ִ� �鿡 ���������� �ڵ����� ������
GSErrCode	placeQuantityPlywood (void)
{
	GSErrCode	err = NoError;

	API_Element				elem;
	API_ElementMemo			memo;
	API_ElemInfo3D			info3D;

	short	xx, yy;
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
	nLayers = static_cast<short> (layerInds.GetSize ());

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

		// ���� ����
		elems [xx].NorthLeftBottom.x = elems [xx].topPoint.x;
		elems [xx].NorthLeftBottom.y = elems [xx].topPoint.y;
		elems [xx].NorthLeftBottom.z = elems [xx].bottomPoint.z;
		elems [xx].NorthRightTop.x = elems [xx].bottomPoint.x;
		elems [xx].NorthRightTop.y = elems [xx].topPoint.y;
		elems [xx].NorthRightTop.z = elems [xx].topPoint.z;

		// ���� ����
		elems [xx].SouthLeftBottom.x = elems [xx].bottomPoint.x;
		elems [xx].SouthLeftBottom.y = elems [xx].bottomPoint.y;
		elems [xx].SouthLeftBottom.z = elems [xx].bottomPoint.z;
		elems [xx].SouthRightTop.x = elems [xx].topPoint.x;
		elems [xx].SouthRightTop.y = elems [xx].bottomPoint.y;
		elems [xx].SouthRightTop.z = elems [xx].topPoint.z;

		// ���� ����
		elems [xx].EastLeftBottom.x = elems [xx].topPoint.x;
		elems [xx].EastLeftBottom.y = elems [xx].bottomPoint.y;
		elems [xx].EastLeftBottom.z = elems [xx].bottomPoint.z;
		elems [xx].EastRightTop.x = elems [xx].topPoint.x;
		elems [xx].EastRightTop.y = elems [xx].topPoint.y;
		elems [xx].EastRightTop.z = elems [xx].topPoint.z;

		// ���� ����
		elems [xx].WestLeftBottom.x = elems [xx].bottomPoint.x;
		elems [xx].WestLeftBottom.y = elems [xx].topPoint.y;
		elems [xx].WestLeftBottom.z = elems [xx].bottomPoint.z;
		elems [xx].WestRightTop.x = elems [xx].bottomPoint.x;
		elems [xx].WestRightTop.y = elems [xx].bottomPoint.y;
		elems [xx].WestRightTop.z = elems [xx].topPoint.z;

		// �ظ�
		elems [xx].BaseLeftBottom.x = elems [xx].bottomPoint.x;
		elems [xx].BaseLeftBottom.y = elems [xx].bottomPoint.y;
		elems [xx].BaseLeftBottom.z = elems [xx].bottomPoint.z;
		elems [xx].BaseRightTop.x = elems [xx].topPoint.x;
		elems [xx].BaseRightTop.y = elems [xx].topPoint.y;
		elems [xx].BaseRightTop.z = elems [xx].bottomPoint.z;

		// ��ȿ�� ����
		elems [xx].validNorth = true;
		elems [xx].validSouth = true;
		elems [xx].validEast = true;
		elems [xx].validWest = true;
		elems [xx].validBase = true;

		// ��� Ÿ�� ����
		if (elem.header.typeID == API_WallID) {
			elems [xx].typeOfElem = ELEM_WALL;

			// ��: ª�� ��� �ظ��� ��ȿ���� ����
			invalidateShortTwoSide (&elems [xx]);
			invalidateBase (&elems [xx]);
		} else if (elem.header.typeID == API_SlabID) {
			elems [xx].typeOfElem = ELEM_SLAB;

			// ������: ��� ������ ��ȿ���� ����
			invalidateAllSide (&elems [xx]);
		} else if (elem.header.typeID == API_BeamID) {
			elems [xx].typeOfElem = ELEM_BEAM;

			// ��: ª�� ���� ��ȿ���� ����
			invalidateShortTwoSide (&elems [xx]);
		} else if (elem.header.typeID == API_ColumnID) {
			elems [xx].typeOfElem = ELEM_COLUMN;

			// ���: �ظ��� ��ȿ���� ����
			invalidateBase (&elems [xx]);
		} else {
			elems [xx].typeOfElem = ELEM_UNKNOWN;

			// �ٸ� ��ü���� ��� ����� �ظ��� ��ȿ���� ����
			invalidateAllSide (&elems [xx]);
			invalidateBase (&elems [xx]);
		}

		// memo ��ü ����
		ACAPI_DisposeElemMemoHdls (&memo);
	}

	// ��ü Ÿ�� ����
	objType.Push ("�� (����)");
	objType.Push ("�� (�ܺ�)");
	objType.Push ("�� (�պ�)");
	objType.Push ("�� (�Ķ���)");
	objType.Push ("�� (�����)");
	objType.Push ("������ (����)");
	objType.Push ("������ (RC)");
	objType.Push ("������ (��ũ)");
	objType.Push ("������ (����)");
	objType.Push ("��");
	objType.Push ("��� (����)");
	objType.Push ("��� (��ü)");

	nObjects = static_cast<short> (objType.GetSize ());

	// [���̾�α�] �������� ���̾� ����
	result = DGBlankModalDialog (750, 100, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, quantityPlywoodUIHandler, 0);

	if (result != DG_OK)
		return err;

	for (xx = 0 ; xx < nElems ; ++xx) {
		for (yy = 0 ; yy < nElems ; ++yy) {
			// ������ ��ҳ����� ������ ����
			if (xx == yy)	continue;

			// ���� ���� ���ϱ�
			subtractArea (&elems [xx], elems [yy]);
		}
	}

	// ... ��ü ��ġ
	// ... ��԰� Ÿ�� + ������ ���� �����, �ظ��� �ٴڱ��
	// �� (����)					����(��)
	// �� (�ܺ�)					...
	// �� (�պ�)					...
	// �� (�Ķ���)					...
	// �� (�����)					...
	// ������ (����)				�Ϻ�
	// ������ (RC)					...
	// ������ (��ũ)				...
	// ������ (����)				...
	// ��							����(��), �Ϻ�
	// ��� (����)					����(��)
	// ��� (��ü)					...

	delete []	elems;

	return	err;
}

// [���̾�α�] �������� ���� ���̾� ����
short DGCALLBACK quantityPlywoodUIHandler (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;
	short	xx, yy;
	short	foundExistLayerInd;
	short	foundQLayerInd;
	short	selectedObjType;

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
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 120, 25, 200, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "���� ���̾�");
			DGShowItem (dialogID, itmIdx);

			// ��: �������� ���̾�
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 330, 25, 200, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "�������� ���̾�");
			DGShowItem (dialogID, itmIdx);

			// ��: �������� Ÿ��
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 550, 25, 150, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "�������� Ÿ��");
			DGShowItem (dialogID, itmIdx);

			itmPosX = 120;
			itmPosY = 50;

			// ���� ���̾�: ó������ 1�ุ ǥ���� ��
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 500, 25, 120, itmPosY, 200, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "����");
			for (xx = 0 ; xx < nLayers ; ++xx) {
				DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, layerNames.Get (xx).c_str ());
			}
			DGShowItem (dialogID, itmIdx);

			existingLayerPopup [0] = itmIdx;
			nPopupControl = 1;

			break;
		
		case DG_MSG_CHANGE:

			if (item == existingLayerPopup [nPopupControl - 1] && (nPopupControl < 50)) {

				// �˾� ��Ʈ��: �������� ���̾�
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 500, 25, 330, itmPosY, 200, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				for (xx = 0 ; xx < nLayers ; ++xx) {
					DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, layerNames.Get (xx).c_str ());
				}
				DGShowItem (dialogID, itmIdx);
				qLayerPopup [nPopupControl - 1] = itmIdx;

				// �˾� ��Ʈ��: ��ü Ÿ��
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 500, 25, 550, itmPosY, 150, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				for (xx = 0 ; xx < nObjects ; ++xx) {
					DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, objType.Get (xx).c_str ());
				}
				DGShowItem (dialogID, itmIdx);
				objTypePopup [nPopupControl - 1] = itmIdx;

				itmPosY += 30;

				// �˾� ��Ʈ��: ���� ���̾� (���� ����)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 500, 25, 120, itmPosY, 200, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "����");
				for (xx = 0 ; xx < nLayers ; ++xx) {
					DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, layerNames.Get (xx).c_str ());
				}
				DGShowItem (dialogID, itmIdx);
				existingLayerPopup [nPopupControl] = itmIdx;

				nPopupControl ++;

				DGGrowDialog (dialogID, 0, 30);
			}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					for (xx = 0 ; xx < nPopupControl ; ++xx) {
						// ���� ���̾� "����"�� �ƴ� ���
						if (DGGetItemValLong (dialogID, existingLayerPopup [xx]) != 0) {

							// ���� ���̾� �˾����� ������ �̸��� �ش��ϴ� ���̾� �ε��� ã��
							foundExistLayerInd = findLayerIndex (DGPopUpGetItemText (dialogID, existingLayerPopup [xx], static_cast<short>(DGGetItemValLong (dialogID, existingLayerPopup [xx]))).ToCStr ().Get ());

							// �������� ���̾� �˾����� ������ �̸��� �ش��ϴ� ���̾� �ε��� ã��
							foundQLayerInd = findLayerIndex (DGPopUpGetItemText (dialogID, qLayerPopup [xx], static_cast<short>(DGGetItemValLong (dialogID, qLayerPopup [xx]))).ToCStr ().Get ());

							// �������� ��ü Ÿ��
							selectedObjType = static_cast<short> (DGGetItemValLong (dialogID, objTypePopup [xx]));

							for (yy = 0 ; yy < nElems ; ++yy) {
								if (elems [yy].layerInd == foundExistLayerInd) {
									elems [yy].qLayerInd = foundQLayerInd;
									elems [yy].typeOfQPlywood = selectedObjType - 1;	// ������ �� �Ҵ��
								}
							}
						}
					}

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

// ���̾� �̸����� ���̾� �ε��� ã��
short findLayerIndex (const char* layerName)
{
	short	nLayers;
	short	xx;
	GSErrCode	err;

	API_Attribute	attrib;

	// ������Ʈ �� ���̾� ������ �˾Ƴ�
	BNZeroMemory (&attrib, sizeof (API_Attribute));
	attrib.header.typeID = API_LayerID;
	err = ACAPI_Attribute_GetNum (API_LayerID, &nLayers);

	// �Է��� ���̾� �̸��� ��ġ�ϴ� ���̾��� �ε��� ��ȣ ����
	for (xx = 1; xx <= nLayers ; ++xx) {
		attrib.header.index = xx;
		err = ACAPI_Attribute_Get (&attrib);

		if (err == NoError) {
			if (strncmp (attrib.layer.head.name, layerName, strlen (layerName)) == 0) {
				return	attrib.layer.head.index;
			}
		}
	}

	return 0;
}

// 4���� ���� �� ª�� 2���� ������ ��ȿȭ�ϰ�, ��ȿ�� �� ������ ������. ���� ���� ���� ���� �����Դϴ�. ����/����(2), ����/����(1), ����(0)
short invalidateShortTwoSide (qElem* element)
{
	short	result = 0;

	double	northWidth = GetDistance (element->NorthLeftBottom.x, element->NorthLeftBottom.y, element->NorthRightTop.x, element->NorthRightTop.y);
	double	eastWidth = GetDistance (element->EastLeftBottom.x, element->EastLeftBottom.y, element->EastRightTop.x, element->EastRightTop.y);

	// ����/������ �� ���
	if (northWidth - eastWidth > EPS) {
		// ����/���� ���� ��ȿȭ
		element->validEast = false;
		element->validWest = false;

		result = 2;

	// ����/������ �� ���
	} else {
		// ����/���� ���� ��ȿȭ
		element->validNorth = false;
		element->validSouth = false;

		result = 1;
	}

	return result;
}

// 4���� ������ ��� ��ȿȭ��
void invalidateAllSide (qElem* element)
{
	element->validNorth = false;
	element->validSouth = false;
	element->validEast = false;
	element->validWest = false;
}

// �ظ��� ��ȿȭ��
void invalidateBase (qElem* element)
{
	element->validBase = false;
}

// src ����� ����, �ظ� ������ operand ��ҿ� ���� ���ҵ�
bool subtractArea (qElem* src, qElem operand)
{
	bool precondition = false;
	bool result = false;

	// ���� ����: src, operand�� ��, ������, ��, ����̾�� ��
	if ((src->typeOfElem == ELEM_WALL) || (src->typeOfElem == ELEM_SLAB) || (src->typeOfElem == ELEM_BEAM) || (src->typeOfElem == ELEM_COLUMN))
		if ((operand.typeOfElem == ELEM_WALL) || (operand.typeOfElem == ELEM_SLAB) || (operand.typeOfElem == ELEM_BEAM) || (operand.typeOfElem == ELEM_COLUMN))
			precondition = true;

	// ���� ������ �������� ������ ����
	if (precondition == false)
		return result;

	// ���� ���鿡 ���������� ���� ���
	if (src->validNorth == true) {
		//��������
		//	(1) src ������ X�� ���� �ȿ� operand�� X�� ������ ħ���ϴ°�?
		//	(2) src ������ Y���� operand�� Y�� ���� �ȿ� ���°�?
		//	(3) src ������ Z�� ���� �ȿ� operand�� Z�� ������ ħ���ϴ°�?
		//���� ������ ��� ���̸�,
		//	(1) src ������ X�� �������� operand�� X�� ������ �����Ѵ�.
		//	(2) src ������ Z�� �������� operand�� Z�� ������ �����Ѵ�.

		if (overlapRange (src->NorthRightTop.x, src->NorthLeftBottom.x, operand.bottomPoint.x, operand.topPoint.x) &&
			inRange (src->NorthLeftBottom.y, operand.bottomPoint.y, operand.topPoint.y) &&
			overlapRange (src->NorthLeftBottom.z, src->NorthRightTop.z, operand.bottomPoint.z, operand.topPoint.z)) {

				excludeRange (&src->NorthLeftBottom, &src->NorthRightTop, NORTH_SIDE, MODE_X, operand.NorthRightTop.x, operand.NorthLeftBottom.x);
				excludeRange (&src->NorthLeftBottom, &src->NorthRightTop, NORTH_SIDE, MODE_Z, operand.NorthLeftBottom.z, operand.NorthRightTop.z);
		}

		result = true;
	}

	// ���� ���鿡 ���������� ���� ���
	if (src->validSouth == true) {
		//��������
		//	(1) src ������ X�� ���� �ȿ� operand�� X�� ������ ħ���ϴ°�?
		//	(2) src ������ Y���� operand�� Y�� ���� �ȿ� ���°�?
		//	(3) src ������ Z�� ���� �ȿ� operand�� Z�� ������ ħ���ϴ°�?
		//���� ������ ��� ���̸�,
		//	(1) src ������ X�� �������� operand�� X�� ������ �����Ѵ�.
		//	(2) src ������ Z�� �������� operand�� Z�� ������ �����Ѵ�.

		if (overlapRange (src->SouthLeftBottom.x, src->SouthRightTop.x, operand.bottomPoint.x, operand.topPoint.x) &&
			inRange (src->SouthLeftBottom.y, operand.bottomPoint.y, operand.topPoint.y) &&
			overlapRange (src->SouthLeftBottom.z, src->SouthRightTop.z, operand.bottomPoint.z, operand.topPoint.z)) {

				excludeRange (&src->SouthLeftBottom, &src->SouthRightTop, SOUTH_SIDE, MODE_X, operand.SouthLeftBottom.x, operand.SouthRightTop.x);
				excludeRange (&src->SouthLeftBottom, &src->SouthRightTop, SOUTH_SIDE, MODE_Z, operand.SouthLeftBottom.z, operand.SouthRightTop.z);
		}

		result = true;
	}

	// ���� ���鿡 ���������� ���� ���
	if (src->validEast == true) {
		//��������
		//	(1) src ������ X���� operand�� X�� ���� �ȿ� ���°�?
		//	(2) src ������ Y�� ���� �ȿ� operand�� Y�� ������ ħ���ϴ°�?
		//	(3) src ������ Z�� ���� �ȿ� operand�� Z�� ������ ħ���ϴ°�?
		//���� ������ ��� ���̸�,
		//	(1) src ������ Y�� �������� operand�� Y�� ������ �����Ѵ�.
		//	(2) src ������ Z�� �������� operand�� Z�� ������ �����Ѵ�.

		if (inRange (src->EastLeftBottom.x, operand.bottomPoint.x, operand.topPoint.x) &&
			overlapRange (src->EastLeftBottom.y, src->EastRightTop.y, operand.bottomPoint.y, operand.topPoint.y) &&
			overlapRange (src->EastLeftBottom.z, src->EastRightTop.z, operand.bottomPoint.z, operand.topPoint.z)) {

				excludeRange (&src->EastLeftBottom, &src->EastRightTop, EAST_SIDE, MODE_X, operand.EastLeftBottom.x, operand.EastRightTop.x);
				excludeRange (&src->EastLeftBottom, &src->EastRightTop, EAST_SIDE, MODE_Z, operand.EastLeftBottom.z, operand.EastRightTop.z);
		}

		result = true;
	}

	// ���� ���鿡 ���������� ���� ���
	if (src->validWest == true) {
		//��������
		//	(1) src ������ X���� operand�� X�� ���� �ȿ� ���°�?
		//	(2) src ������ Y�� ���� �ȿ� operand�� Y�� ������ ħ���ϴ°�?
		//	(3) src ������ Z�� ���� �ȿ� operand�� Z�� ������ ħ���ϴ°�?
		//���� ������ ��� ���̸�,
		//	(1) src ������ Y�� �������� operand�� Y�� ������ �����Ѵ�.
		//	(2) src ������ Z�� �������� operand�� Z�� ������ �����Ѵ�.

		if (inRange (src->WestLeftBottom.x, operand.bottomPoint.x, operand.topPoint.x) &&
			overlapRange (src->WestRightTop.y, src->WestLeftBottom.y, operand.bottomPoint.y, operand.topPoint.y) &&
			overlapRange (src->WestLeftBottom.z, src->WestRightTop.z, operand.bottomPoint.z, operand.topPoint.z)) {

				excludeRange (&src->WestLeftBottom, &src->WestRightTop, WEST_SIDE, MODE_X, operand.WestRightTop.x, operand.WestLeftBottom.x);
				excludeRange (&src->WestLeftBottom, &src->WestRightTop, WEST_SIDE, MODE_Z, operand.WestLeftBottom.z, operand.WestRightTop.z);
		}

		result = true;
	}

	// �ظ鿡 ���������� ���� ���
	if (src->validBase == true) {
		//��������
		//	(1) src �ظ��� X�� ���� �ȿ� operand�� X�� ������ ħ���ϴ°�?
		//	(2) src �ظ��� Y�� ���� �ȿ� operand�� Y�� ������ ħ���ϴ°�?
		//	(3) src �ظ��� Z���� operand�� Z�� ���� �ȿ� ���°�?
		//���� ������ ��� ���̸�,
		//	(1) src �ظ��� X�� �������� operand�� X�� ������ �����Ѵ�.
		//	(2) src �ظ��� Y�� �������� operand�� Y�� ������ �����Ѵ�.

		if (overlapRange (src->BaseLeftBottom.x, src->BaseRightTop.x, operand.bottomPoint.x, operand.topPoint.x) &&
			overlapRange (src->BaseLeftBottom.y, src->BaseRightTop.y, operand.bottomPoint.y, operand.topPoint.y) &&
			inRange (src->BaseLeftBottom.z, operand.bottomPoint.z, operand.topPoint.z)) {

				excludeRange (&src->BaseLeftBottom, &src->BaseRightTop, BASE_SIDE, MODE_X, operand.BaseLeftBottom.x, operand.BaseRightTop.x);
				excludeRange (&src->BaseLeftBottom, &src->BaseRightTop, BASE_SIDE, MODE_Z, operand.BaseLeftBottom.z, operand.BaseRightTop.z);
		}

		result = true;
	}

	return	result;
}

// srcPoint ���� target ���� �ȿ� ��� �ִ°�?
bool	inRange (double srcPoint, double targetMin, double targetMax)
{
	if ((srcPoint > targetMin - EPS) && (srcPoint < targetMax + EPS))
		return true;
	else
		return false;
}

// src ������ target ������ ��ġ�°�?
bool	overlapRange (double srcMin, double srcMax, double targetMin, double targetMax)
{
	bool	result = false;

	// srcMin�� srcMax �� �ϳ��� target ���� �ȿ� ���� ���
	if (inRange (srcMin, targetMin, targetMax) || inRange (srcMax, targetMin, targetMax))
		result = true;
	// srcMin�� targetMin���� �۰� srcMax�� targetMax���� ū ���
	if ((srcMin < targetMin + EPS) && (srcMax > targetMax - EPS))
		result = true;
	// targetMin�� srcMin���� �۰� targetMax�� srcMax���� ū ���
	if ((targetMin < srcMin + EPS) && (targetMax > srcMax - EPS))
		result = true;

	return result;
}

// ���� �Ǵ� �ظ��� ������ �����, side�� ������ �� ���� (����(0), ����(1), ����(2), ����(3), �ظ�(5)), mode�� ���� X(0), Y(1), Z(2)�� ������, minVal ~ maxVal�� ���� ������ ����
bool	excludeRange (API_Coord3D* srcLeftBottom, API_Coord3D* srcRightTop, short side, short mode, double minVal, double maxVal)
{
	bool	result = false;

	// �ּҰ�, �ִ밪�� �ݴ�� �Ǿ� ������ ����
	if (minVal > maxVal)
		return false;

	// ���� ����
	if (side == NORTH_SIDE) {
		if (mode == MODE_X) {
			// srcRightTop.x ~ srcLeftBottom.x ���� ��
				// minVal�� maxVal�� ��� ��� �ִ� ��� -> ���� ��� ����
				// minVal�� ��� ������ maxVal�� srcLeftBottom.x - EPS���� ū ��� -> srcLeftBottom.x = minVal
				// maxVal�� ��� ������ minVal�� srcRightTop.x + EPS���� ���� ��� -> srcRightTop.x = maxVal
				// minVal�� srcRightTop.x + EPS���� �۰� maxVal�� srcLeftBottom.x - EPS���� ū ��� -> ���� ��� ����

			if ((srcRightTop->x < minVal) && (minVal < srcLeftBottom->x) && (srcRightTop->x < maxVal) && (maxVal < srcLeftBottom->x)) {
				srcRightTop->x = srcLeftBottom->x;
				srcRightTop->z = srcLeftBottom->z;
			}
			if ((srcRightTop->x < minVal) && (minVal < srcLeftBottom->x) && (srcLeftBottom->x - EPS < maxVal)) {
				srcLeftBottom->x = minVal;
			}
			if ((srcRightTop->x < maxVal) && (maxVal < srcRightTop->x) && (minVal < srcRightTop->x + EPS)) {
				srcRightTop->x = maxVal;
			}
			if ((minVal < srcRightTop->x + EPS) && (srcLeftBottom->x - EPS < maxVal)) {
				srcRightTop->x = srcLeftBottom->x;
				srcRightTop->z = srcLeftBottom->z;
			}

			result = true;
		} else if (mode == MODE_Y) {
			// ��ȿ
			result = false;
		} else if (mode == MODE_Z) {
			// srcLeftBottom.z ~ srcRightTop.z ���� ��
				// minVal�� maxVal�� ��� ��� �ִ� ��� -> ���� ��� ����
				// minVal�� ��� ������ maxVal�� srcRightTop.z - EPS���� ū ��� -> srcRightTop.z = minVal
				// maxVal�� ��� ������ minVal�� srcLeftBottom.z + EPS���� ���� ��� -> srcLeftBottom.z = maxVal
				// minVal�� srcLeftBottom.z + EPS���� �۰� maxVal�� srcRightTop.z - EPS���� ū ��� -> ���� ��� ����

			if ((srcLeftBottom->z < minVal) && (minVal < srcRightTop->z) && (srcLeftBottom->z < maxVal) && (maxVal < srcRightTop->z)) {
				srcRightTop->x = srcLeftBottom->x;
				srcRightTop->z = srcLeftBottom->z;
			}
			if ((srcLeftBottom->z < minVal) && (minVal < srcRightTop->z) && (srcRightTop->z - EPS < maxVal)) {
				srcRightTop->z = minVal;
			}
			if ((srcLeftBottom->z < maxVal) && (maxVal < srcRightTop->z) && (minVal < srcLeftBottom->z + EPS)) {
				srcLeftBottom->z = maxVal;
			}
			if ((minVal < srcLeftBottom->z + EPS) && (srcRightTop->z - EPS < maxVal)) {
				srcRightTop->x = srcLeftBottom->x;
				srcRightTop->z = srcLeftBottom->z;
			}

			result = true;
		}

	// ���� ����
	} else if (side == SOUTH_SIDE) {
		if (mode == MODE_X) {
			// srcLeftBottom.x ~ srcRightTop.x ���� ��
				// minVal�� maxVal�� ��� ��� �ִ� ��� -> ���� ��� ����
				// minVal�� ��� ������ maxVal�� srcRightTop.x - EPS���� ū ��� -> srcRightTop.x = minVal
				// maxVal�� ��� ������ minVal�� srcLeftBottom.x + EPS���� ���� ��� -> srcLeftBottom.x = maxVal
				// minVal�� srcLeftBottom.x + EPS���� �۰� maxVal�� srcRightTop.x - EPS���� ū ��� -> ���� ��� ����

			if ((srcLeftBottom->x < minVal) && (minVal < srcRightTop->x) && (srcLeftBottom->x < maxVal) && (maxVal < srcRightTop->x)) {
				srcRightTop->x = srcLeftBottom->x;
				srcRightTop->z = srcLeftBottom->z;
			}
			if ((srcRightTop->x < minVal) && (minVal < srcLeftBottom->x) && (srcLeftBottom->x - EPS < maxVal)) {
				srcLeftBottom->x = minVal;
			}
			if ((srcRightTop->x < maxVal) && (maxVal < srcRightTop->x) && (minVal < srcRightTop->x + EPS)) {
				srcRightTop->x = maxVal;
			}
			if ((minVal < srcRightTop->x + EPS) && (srcLeftBottom->x - EPS < maxVal)) {
				srcRightTop->x = srcLeftBottom->x;
				srcRightTop->z = srcLeftBottom->z;
			}

			result = true;
		} else if (mode == MODE_Y) {
			// ��ȿ
			result = false;
		} else if (mode == MODE_Z) {
			// srcLeftBottom.z ~ srcRightTop.z ���� ��
				// minVal�� maxVal�� ��� ��� �ִ� ��� -> ���� ��� ����
				// minVal�� ��� ������ maxVal�� srcRightTop.z - EPS���� ū ��� -> srcRightTop.z = minVal
				// maxVal�� ��� ������ minVal�� srcLeftBottom.z + EPS���� ���� ��� -> srcLeftBottom.z = maxVal
				// minVal�� srcLeftBottom.z + EPS���� �۰� maxVal�� srcRightTop.z - EPS���� ū ��� -> ���� ��� ����

			if ((srcLeftBottom->z < minVal) && (minVal < srcRightTop->z) && (srcLeftBottom->z < maxVal) && (maxVal < srcRightTop->z)) {
				srcRightTop->x = srcLeftBottom->x;
				srcRightTop->z = srcLeftBottom->z;
			}
			if ((srcLeftBottom->z < minVal) && (minVal < srcRightTop->z) && (srcRightTop->z - EPS < maxVal)) {
				srcRightTop->z = minVal;
			}
			if ((srcLeftBottom->z < maxVal) && (maxVal < srcRightTop->z) && (minVal < srcLeftBottom->z + EPS)) {
				srcLeftBottom->z = maxVal;
			}
			if ((minVal < srcLeftBottom->z + EPS) && (srcRightTop->z - EPS < maxVal)) {
				srcRightTop->x = srcLeftBottom->x;
				srcRightTop->z = srcLeftBottom->z;
			}

			result = true;
		}

	// ���� ����
	} else if (side == EAST_SIDE) {
		if (mode == MODE_X) {
			// ��ȿ
			result = false;
		} else if (mode == MODE_Y) {
			// srcLeftBottom.y ~ srcRightTop.y ���� ��
				// minVal�� maxVal�� ��� ��� �ִ� ��� -> ���� ��� ����
				// minVal�� ��� ������ maxVal�� srcRightTop.y - EPS���� ū ��� -> srcRightTop.y = minVal
				// maxVal�� ��� ������ minVal�� srcLeftBottom.y + EPS���� ���� ��� -> srcLeftBottom.y = maxVal
				// minVal�� srcLeftBottom.y + EPS���� �۰� maxVal�� srcRightTop.y - EPS���� ū ��� -> ���� ��� ����

			if ((srcLeftBottom->y < minVal) && (minVal < srcRightTop->y) && (srcLeftBottom->y < maxVal) && (maxVal < srcRightTop->y)) {
				srcRightTop->y = srcLeftBottom->y;
				srcRightTop->z = srcLeftBottom->z;
			}
			if ((srcRightTop->y < minVal) && (minVal < srcLeftBottom->y) && (srcLeftBottom->y - EPS < maxVal)) {
				srcLeftBottom->y = minVal;
			}
			if ((srcRightTop->y < maxVal) && (maxVal < srcRightTop->y) && (minVal < srcRightTop->y + EPS)) {
				srcRightTop->y = maxVal;
			}
			if ((minVal < srcRightTop->y + EPS) && (srcLeftBottom->y - EPS < maxVal)) {
				srcRightTop->y = srcLeftBottom->y;
				srcRightTop->z = srcLeftBottom->z;
			}

			result = true;
		} else if (mode == MODE_Z) {
			// srcLeftBottom.z ~ srcRightTop.z ���� ��
				// minVal�� maxVal�� ��� ��� �ִ� ��� -> ���� ��� ����
				// minVal�� ��� ������ maxVal�� srcRightTop.z - EPS���� ū ��� -> srcRightTop.z = minVal
				// maxVal�� ��� ������ minVal�� srcLeftBottom.z + EPS���� ���� ��� -> srcLeftBottom.z = maxVal
				// minVal�� srcLeftBottom.z + EPS���� �۰� maxVal�� srcRightTop.z - EPS���� ū ��� -> ���� ��� ����

			if ((srcLeftBottom->z < minVal) && (minVal < srcRightTop->z) && (srcLeftBottom->z < maxVal) && (maxVal < srcRightTop->z)) {
				srcRightTop->x = srcLeftBottom->x;
				srcRightTop->z = srcLeftBottom->z;
			}
			if ((srcLeftBottom->z < minVal) && (minVal < srcRightTop->z) && (srcRightTop->z - EPS < maxVal)) {
				srcRightTop->z = minVal;
			}
			if ((srcLeftBottom->z < maxVal) && (maxVal < srcRightTop->z) && (minVal < srcLeftBottom->z + EPS)) {
				srcLeftBottom->z = maxVal;
			}
			if ((minVal < srcLeftBottom->z + EPS) && (srcRightTop->z - EPS < maxVal)) {
				srcRightTop->x = srcLeftBottom->x;
				srcRightTop->z = srcLeftBottom->z;
			}

			result = true;
		}
	
	// ���� ����
	} else if (side == WEST_SIDE) {
		if (mode == MODE_X) {
			// ��ȿ
			result = false;
		} else if (mode == MODE_Y) {
			// srcRightTop.y ~ srcLeftBottom.y ���� ��
				// minVal�� maxVal�� ��� ��� �ִ� ��� -> ���� ��� ����
				// minVal�� ��� ������ maxVal�� srcLeftBottom.y - EPS���� ū ��� -> srcLeftBottom.y = minVal
				// maxVal�� ��� ������ minVal�� srcRightTop.y + EPS���� ���� ��� -> srcRightTop.y = maxVal
				// minVal�� srcRightTop.y + EPS���� �۰� maxVal�� srcLeftBottom.y - EPS���� ū ��� -> ���� ��� ����

			if ((srcRightTop->y < minVal) && (minVal < srcLeftBottom->y) && (srcRightTop->y < maxVal) && (maxVal < srcLeftBottom->y)) {
				srcRightTop->y = srcLeftBottom->y;
				srcRightTop->z = srcLeftBottom->z;
			}
			if ((srcRightTop->y < minVal) && (minVal < srcLeftBottom->y) && (srcLeftBottom->y - EPS < maxVal)) {
				srcLeftBottom->y = minVal;
			}
			if ((srcRightTop->y < maxVal) && (maxVal < srcRightTop->y) && (minVal < srcRightTop->y + EPS)) {
				srcRightTop->y = maxVal;
			}
			if ((minVal < srcRightTop->y + EPS) && (srcLeftBottom->y - EPS < maxVal)) {
				srcRightTop->y = srcLeftBottom->y;
				srcRightTop->z = srcLeftBottom->z;
			}

			result = true;
		} else if (mode == MODE_Z) {
			// srcLeftBottom.z ~ srcRightTop.z ���� ��
				// minVal�� maxVal�� ��� ��� �ִ� ��� -> ���� ��� ����
				// minVal�� ��� ������ maxVal�� srcRightTop.z - EPS���� ū ��� -> srcRightTop.z = minVal
				// maxVal�� ��� ������ minVal�� srcLeftBottom.z + EPS���� ���� ��� -> srcLeftBottom.z = maxVal
				// minVal�� srcLeftBottom.z + EPS���� �۰� maxVal�� srcRightTop.z - EPS���� ū ��� -> ���� ��� ����

			if ((srcLeftBottom->z < minVal) && (minVal < srcRightTop->z) && (srcLeftBottom->z < maxVal) && (maxVal < srcRightTop->z)) {
				srcRightTop->x = srcLeftBottom->x;
				srcRightTop->z = srcLeftBottom->z;
			}
			if ((srcLeftBottom->z < minVal) && (minVal < srcRightTop->z) && (srcRightTop->z - EPS < maxVal)) {
				srcRightTop->z = minVal;
			}
			if ((srcLeftBottom->z < maxVal) && (maxVal < srcRightTop->z) && (minVal < srcLeftBottom->z + EPS)) {
				srcLeftBottom->z = maxVal;
			}
			if ((minVal < srcLeftBottom->z + EPS) && (srcRightTop->z - EPS < maxVal)) {
				srcRightTop->x = srcLeftBottom->x;
				srcRightTop->z = srcLeftBottom->z;
			}

			result = true;
		}
	
	// �ظ�
	} else if (side == BASE_SIDE) {
		if (mode == MODE_X) {
			// srcLeftBottom.x ~ srcRightTop.x ���� ��
				// minVal�� maxVal�� ��� ��� �ִ� ��� -> ���� ��� ����
				// minVal�� ��� ������ maxVal�� srcRightTop.x - EPS���� ū ��� -> srcRightTop.x = minVal
				// maxVal�� ��� ������ minVal�� srcLeftBottom.x + EPS���� ���� ��� -> srcLeftBottom.x = maxVal
				// minVal�� srcLeftBottom.x + EPS���� �۰� maxVal�� srcRightTop.x - EPS���� ū ��� -> ���� ��� ����

			if ((srcLeftBottom->x < minVal) && (minVal < srcRightTop->x) && (srcLeftBottom->x < maxVal) && (maxVal < srcRightTop->x)) {
				srcRightTop->x = srcLeftBottom->x;
				srcRightTop->z = srcLeftBottom->z;
			}
			if ((srcRightTop->x < minVal) && (minVal < srcLeftBottom->x) && (srcLeftBottom->x - EPS < maxVal)) {
				srcLeftBottom->x = minVal;
			}
			if ((srcRightTop->x < maxVal) && (maxVal < srcRightTop->x) && (minVal < srcRightTop->x + EPS)) {
				srcRightTop->x = maxVal;
			}
			if ((minVal < srcRightTop->x + EPS) && (srcLeftBottom->x - EPS < maxVal)) {
				srcRightTop->x = srcLeftBottom->x;
				srcRightTop->z = srcLeftBottom->z;
			}

			result = true;
		} else if (mode == MODE_Y) {
			// srcLeftBottom.y ~ srcRightTop.y ���� ��
				// minVal�� maxVal�� ��� ��� �ִ� ��� -> ���� ��� ����
				// minVal�� ��� ������ maxVal�� srcRightTop.y - EPS���� ū ��� -> srcRightTop.y = minVal
				// maxVal�� ��� ������ minVal�� srcLeftBottom.y + EPS���� ���� ��� -> srcLeftBottom.y = maxVal
				// minVal�� srcLeftBottom.y + EPS���� �۰� maxVal�� srcRightTop.y - EPS���� ū ��� -> ���� ��� ����

			if ((srcLeftBottom->y < minVal) && (minVal < srcRightTop->y) && (srcLeftBottom->y < maxVal) && (maxVal < srcRightTop->y)) {
				srcRightTop->y = srcLeftBottom->y;
				srcRightTop->z = srcLeftBottom->z;
			}
			if ((srcRightTop->y < minVal) && (minVal < srcLeftBottom->y) && (srcLeftBottom->y - EPS < maxVal)) {
				srcLeftBottom->y = minVal;
			}
			if ((srcRightTop->y < maxVal) && (maxVal < srcRightTop->y) && (minVal < srcRightTop->y + EPS)) {
				srcRightTop->y = maxVal;
			}
			if ((minVal < srcRightTop->y + EPS) && (srcLeftBottom->y - EPS < maxVal)) {
				srcRightTop->y = srcLeftBottom->y;
				srcRightTop->z = srcLeftBottom->z;
			}

			result = true;
		} else if (mode == MODE_Z) {
			// ��ȿ
			result = false;
		}
	}

	return result;
}