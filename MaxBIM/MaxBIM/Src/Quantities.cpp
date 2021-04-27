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

const GS::uchar_t*	gsmQuantityPlywood = L("��������(�ֽ������).gsm");

// ����(��,������,��,���)���� �����簡 ���� �� �ִ� �鿡 ���������� �ڵ����� ������
GSErrCode	placeQuantityPlywood (void)
{
	GSErrCode	err = NoError;

	API_Element				elem;
	API_ElementMemo			memo;
	API_ElemInfo3D			info3D;

	short	xx, yy, zz;
	short	pp, qq;
	short	result;		// ���̾�α� ���� ��
	
	API_Attribute	attrib;

	GS::Array<API_Guid>	elemList_All;		// ��ҵ��� GUID (��ü ���� ���)
	GS::Array<API_Guid>	elemList_Wall;		// ��ҵ��� GUID (��)
	GS::Array<API_Guid>	elemList_Slab;		// ��ҵ��� GUID (������)
	GS::Array<API_Guid>	elemList_Beam;		// ��ҵ��� GUID (��)
	GS::Array<API_Guid>	elemList_Column;	// ��ҵ��� GUID (���)

	GS::Array<API_Guid> elemList_Mesh;		// ��ҵ��� GUID (�޽�)
	GS::Array<API_Guid> elemList_Roof;		// ��ҵ��� GUID (����)
	GS::Array<API_Guid> elemList_Object;	// ��ҵ��� GUID (��ü, ��� ����)


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

	// ��, ������, ��, ��� ��ü�� ������
	ACAPI_Element_GetElemList (API_WallID, &elemList_Wall, APIFilt_In3D);
	ACAPI_Element_GetElemList (API_SlabID, &elemList_Slab, APIFilt_In3D);
	ACAPI_Element_GetElemList (API_BeamID, &elemList_Beam, APIFilt_In3D);
	ACAPI_Element_GetElemList (API_ColumnID, &elemList_Column, APIFilt_In3D);
	ACAPI_Element_GetElemList (API_MeshID, &elemList_Mesh, APIFilt_In3D);
	ACAPI_Element_GetElemList (API_RoofID, &elemList_Roof, APIFilt_In3D);
	ACAPI_Element_GetElemList (API_ObjectID, &elemList_Object, APIFilt_In3D);
	while (!elemList_Wall.IsEmpty ())
		elemList_All.Push (elemList_Wall.Pop ());
	while (!elemList_Slab.IsEmpty ())
		elemList_All.Push (elemList_Slab.Pop ());
	while (!elemList_Beam.IsEmpty ())
		elemList_All.Push (elemList_Beam.Pop ());
	while (!elemList_Column.IsEmpty ())
		elemList_All.Push (elemList_Column.Pop ());
	while (!elemList_Mesh.IsEmpty ())
		elemList_All.Push (elemList_Mesh.Pop ());
	while (!elemList_Roof.IsEmpty ())
		elemList_All.Push (elemList_Roof.Pop ());
	while (!elemList_Object.IsEmpty ())
		elemList_All.Push (elemList_Object.Pop ());

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

		// �� ���� ����
		if ((elem.wall.hasDoor == TRUE) || (elem.wall.hasWindow == TRUE))
			elems [xx].hasHole = true;
		else
			elems [xx].hasHole = false;

		// ��� Ÿ�� ����
		if (elem.header.typeID == API_WallID) {
			elems [xx].typeOfElem = ELEM_WALL;

			// ��: �ظ��� ��ȿ���� ����
			elems [xx].invalidateBase (&elems [xx]);

			// ���� ���̳� â�� ���� ������ ������ �� ���� ��ȿ���� ����
			if (elems [xx].hasHole == true)
				elems [xx].invalidateLongTwoSide (&elems [xx]);
		} else if (elem.header.typeID == API_SlabID) {
			elems [xx].typeOfElem = ELEM_SLAB;

			// ������: ��� ������ ��ȿ���� ����
			elems [xx].invalidateAllSide (&elems [xx]);
		} else if (elem.header.typeID == API_BeamID) {
			elems [xx].typeOfElem = ELEM_BEAM;

			// ��: ª�� ���� ��ȿ���� ����
			elems [xx].invalidateShortTwoSide (&elems [xx]);
		} else if (elem.header.typeID == API_ColumnID) {
			elems [xx].typeOfElem = ELEM_COLUMN;

			// ���: �ظ��� ��ȿ���� ����
			elems [xx].invalidateBase (&elems [xx]);
		} else {
			elems [xx].typeOfElem = ELEM_UNKNOWN;

			// �ٸ� ��ü���� ��� ����� �ظ��� ��ȿ���� ����
			elems [xx].invalidateAllSide (&elems [xx]);
			elems [xx].invalidateBase (&elems [xx]);
		}

		elems [xx].nQPlywoods = 0;		// ������ �������ǵ��� GUID
		elems [xx].nOtherElems = 0;		// ������ ����Ű�� �ٸ� ��ҵ��� GUID

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

	if (result != DG_OK) {
		delete []	elems;
		return err;
	}

	for (xx = 0 ; xx < nElems ; ++xx) {
		for (yy = 0 ; yy < nElems ; ++yy) {
			// ������ ��ҳ����� ������ ����
			if (xx == yy)	continue;

			// �����ϴ� ��Ҹ� ã�Ƴ��� GUID�� ������
			elems [xx].subtractArea (&elems [xx], elems [yy]);
		}
	}

	// �������� ��ü ��ġ
	for (xx = 0 ; xx < nElems ; ++xx) {
		elems [xx].placeQuantityPlywood (&elems [xx]);
	}

	// �ָ��� ������ ���� ���������� ������
	for (xx = 0 ; xx < nElems ; ++xx) {
		for (yy = 0 ; yy < elems [xx].nQPlywoods ; ++yy) {
			for (zz = 0 ; zz < elems [xx].nOtherElems ; ++zz) {

				// ��ġ�� ��ҿ� ���� ���������� ������ (1��° �Ķ����: Target, 2��° �Ķ����: Operator)
				err = ACAPI_Element_SolidLink_Create (elems [xx].qPlywoodGuids [yy], elems [xx].otherGuids [zz], APISolid_Substract, APISolidFlag_OperatorAttrib);

				// Operator���� �����Ǿ� �ִ� �������ǵ��� GUID�� �����ͼ� ���� ������Ŵ
				for (pp = 0 ; pp < nElems ; ++pp) {
					if ((elems [xx].otherGuids [zz] == elems [pp].guid) && (pp != xx)) {
						for (qq = 0 ; qq < elems [pp].nQPlywoods ; ++qq) {
							err = ACAPI_Element_SolidLink_Create (elems [xx].qPlywoodGuids [yy], elems [pp].qPlywoodGuids [qq], APISolid_Substract, APISolidFlag_OperatorAttrib);
						}
					}
				}
			}
		}
	}

	delete []	elems;

	// ȭ�� ���ΰ�ħ
	ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	bool	regenerate = true;
	ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

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
				DGPopUpDisableDraw (dialogID, itmIdx);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				for (xx = 0 ; xx < nLayers ; ++xx) {
					DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, layerNames.Get (xx).c_str ());
				}
				DGShowItem (dialogID, itmIdx);
				DGPopUpEnableDraw (dialogID, itmIdx);
				qLayerPopup [nPopupControl - 1] = itmIdx;

				// �˾� ��Ʈ��: ��ü Ÿ��
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 500, 25, 550, itmPosY, 150, 25);
				DGPopUpDisableDraw (dialogID, itmIdx);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				for (xx = 0 ; xx < nObjects ; ++xx) {
					DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, objType.Get (xx).c_str ());
				}
				DGShowItem (dialogID, itmIdx);
				DGPopUpEnableDraw (dialogID, itmIdx);
				objTypePopup [nPopupControl - 1] = itmIdx;

				itmPosY += 30;

				// �˾� ��Ʈ��: ���� ���̾� (���� ����)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 500, 25, 120, itmPosY, 200, 25);
				DGPopUpDisableDraw (dialogID, itmIdx);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "����");
				for (xx = 0 ; xx < nLayers ; ++xx) {
					DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, layerNames.Get (xx).c_str ());
				}
				DGShowItem (dialogID, itmIdx);
				DGPopUpEnableDraw (dialogID, itmIdx);
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

							// ��Һ� ���̾� ���� �Ҵ�
							for (yy = 0 ; yy < nElems ; ++yy) {
								if (elems [yy].layerInd == foundExistLayerInd) {
									elems [yy].qLayerInd = foundQLayerInd;
									elems [yy].typeOfQPlywood = selectedObjType - 1;	// ������ �� �Ҵ��
									
									// ���� �������� Ÿ���� ������ (����)�� ���, [Q_SLAB_BASE]
									if (elems [yy].typeOfQPlywood == Q_SLAB_BASE) {
										// �ظ��� ������ �ʰ� ������ ��� ����
										elems [yy].invalidateBase (&elems [yy]);
										elems [yy].validateAllSide (&elems [yy]);
									}
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

// 4���� ���� �� ª�� 2���� ������ ��ȿȭ�ϰ�, ��ȿ�� �� ������ ������. ���� ���� ���� ���� �����Դϴ�. ����/����(2), ����/����(1), ����(0)
short qElem::invalidateShortTwoSide (qElem* element)
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

// 4���� ���� �� �� 2���� ������ ��ȿȭ�ϰ�, ��ȿ�� �� ������ ������. ���� ���� ���� ���� �����Դϴ�. ����/����(2), ����/����(1), ����(0)
short qElem::invalidateLongTwoSide (qElem* element)
{
	short	result = 0;

	double	northWidth = GetDistance (element->NorthLeftBottom.x, element->NorthLeftBottom.y, element->NorthRightTop.x, element->NorthRightTop.y);
	double	eastWidth = GetDistance (element->EastLeftBottom.x, element->EastLeftBottom.y, element->EastRightTop.x, element->EastRightTop.y);

	// ����/������ �� ���
	if (northWidth - eastWidth > EPS) {
		// ����/���� ���� ��ȿȭ
		element->validNorth = false;
		element->validSouth = false;

		result = 1;

	// ����/������ �� ���
	} else {
		// ����/���� ���� ��ȿȭ
		element->validEast = false;
		element->validWest = false;

		result = 2;
	}

	return result;
}

// 4���� ������ ��� ��ȿȭ��
void qElem::invalidateAllSide (qElem* element)
{
	element->validNorth = false;
	element->validSouth = false;
	element->validEast = false;
	element->validWest = false;
}

// �ظ��� ��ȿȭ��
void qElem::invalidateBase (qElem* element)
{
	element->validBase = false;
}

// 4���� ������ ��� ��ȿȭ��
void qElem::validateAllSide (qElem* element)
{
	element->validNorth = true;
	element->validSouth = true;
	element->validEast = true;
	element->validWest = true;
}

// �ظ��� ��ȿȭ��
void qElem::validateBase (qElem* element)
{
	element->validBase = true;
}

// src ����� ����, �ظ� ������ operand ��ҿ� ���� ħ�� ���� ���, �ָ��� ������ ���� operand�� GUID�� ������
bool qElem::subtractArea (qElem* src, qElem operand)
{
	bool precondition = false;
	bool result = false;

	// ���� ����: src�� ��, ������, ��, ����̾�� ��
	if ((src->typeOfElem == ELEM_WALL) || (src->typeOfElem == ELEM_SLAB) || (src->typeOfElem == ELEM_BEAM) || (src->typeOfElem == ELEM_COLUMN))
		precondition = true;

	// ���� ������ �������� ������ ����
	if (precondition == false)
		return false;

	// ���� ���鿡 ���������� ���� ���
	if (src->validNorth == true) {
		//��������
		//	(1) src ������ X�� ���� �ȿ� operand�� X�� ������ ħ���ϴ°�?
		//	(2) src ������ Y���� operand�� Y�� ���� �ȿ� ���°�?
		//	(3) src ������ Z�� ���� �ȿ� operand�� Z�� ������ ħ���ϴ°�?
		if ((overlapRange (src->NorthRightTop.x, src->NorthLeftBottom.x, operand.bottomPoint.x, operand.topPoint.x) > EPS) &&
			inRange (src->NorthLeftBottom.y, operand.bottomPoint.y, operand.topPoint.y) &&
			(overlapRange (src->NorthLeftBottom.z, src->NorthRightTop.z, operand.bottomPoint.z, operand.topPoint.z) > EPS)) {

				// operand.guid�� src�� otherGuids�� �ְ� nOtherElems�� ���� ��
				src->otherGuids [src->nOtherElems ++] = operand.guid;
		}

		result = true;
	}

	// ���� ���鿡 ���������� ���� ���
	if (src->validSouth == true) {
		//��������
		//	(1) src ������ X�� ���� �ȿ� operand�� X�� ������ ħ���ϴ°�?
		//	(2) src ������ Y���� operand�� Y�� ���� �ȿ� ���°�?
		//	(3) src ������ Z�� ���� �ȿ� operand�� Z�� ������ ħ���ϴ°�?
		if ((overlapRange (src->SouthLeftBottom.x, src->SouthRightTop.x, operand.bottomPoint.x, operand.topPoint.x) > EPS) &&
			inRange (src->SouthLeftBottom.y, operand.bottomPoint.y, operand.topPoint.y) &&
			(overlapRange (src->SouthLeftBottom.z, src->SouthRightTop.z, operand.bottomPoint.z, operand.topPoint.z) > EPS)) {

				// operand.guid�� src�� otherGuids�� �ְ� nOtherElems�� ���� ��
				src->otherGuids [src->nOtherElems ++] = operand.guid;
		}

		result = true;
	}

	// ���� ���鿡 ���������� ���� ���
	if (src->validEast == true) {
		//��������
		//	(1) src ������ X���� operand�� X�� ���� �ȿ� ���°�?
		//	(2) src ������ Y�� ���� �ȿ� operand�� Y�� ������ ħ���ϴ°�?
		//	(3) src ������ Z�� ���� �ȿ� operand�� Z�� ������ ħ���ϴ°�?
		if (inRange (src->EastLeftBottom.x, operand.bottomPoint.x, operand.topPoint.x) &&
			(overlapRange (src->EastLeftBottom.y, src->EastRightTop.y, operand.bottomPoint.y, operand.topPoint.y) > EPS) &&
			(overlapRange (src->EastLeftBottom.z, src->EastRightTop.z, operand.bottomPoint.z, operand.topPoint.z) > EPS)) {

				// operand.guid�� src�� otherGuids�� �ְ� nOtherElems�� ���� ��
				src->otherGuids [src->nOtherElems ++] = operand.guid;
		}

		result = true;
	}

	// ���� ���鿡 ���������� ���� ���
	if (src->validWest == true) {
		//��������
		//	(1) src ������ X���� operand�� X�� ���� �ȿ� ���°�?
		//	(2) src ������ Y�� ���� �ȿ� operand�� Y�� ������ ħ���ϴ°�?
		//	(3) src ������ Z�� ���� �ȿ� operand�� Z�� ������ ħ���ϴ°�?
		if (inRange (src->WestLeftBottom.x, operand.bottomPoint.x, operand.topPoint.x) &&
			(overlapRange (src->WestRightTop.y, src->WestLeftBottom.y, operand.bottomPoint.y, operand.topPoint.y) > EPS) &&
			(overlapRange (src->WestLeftBottom.z, src->WestRightTop.z, operand.bottomPoint.z, operand.topPoint.z) > EPS)) {

				// operand.guid�� src�� otherGuids�� �ְ� nOtherElems�� ���� ��
				src->otherGuids [src->nOtherElems ++] = operand.guid;
		}

		result = true;
	}

	// �ظ鿡 ���������� ���� ���
	if (src->validBase == true) {
		//��������
		//	(1) src �ظ��� X�� ���� �ȿ� operand�� X�� ������ ħ���ϴ°�?
		//	(2) src �ظ��� Y�� ���� �ȿ� operand�� Y�� ������ ħ���ϴ°�?
		//	(3) src �ظ��� Z���� operand�� Z�� ���� �ȿ� ���°�?
		if ((overlapRange (src->BaseLeftBottom.x, src->BaseRightTop.x, operand.bottomPoint.x, operand.topPoint.x) > EPS) &&
			(overlapRange (src->BaseLeftBottom.y, src->BaseRightTop.y, operand.bottomPoint.y, operand.topPoint.y) > EPS) &&
			inRange (src->BaseLeftBottom.z, operand.bottomPoint.z, operand.topPoint.z)) {

				// operand.guid�� src�� otherGuids�� �ְ� nOtherElems�� ���� ��
				src->otherGuids [src->nOtherElems ++] = operand.guid;
		}

		result = true;
	}

	return	result;
}

// ����� ������ �ظ� ������ ���������� ������
void qElem::placeQuantityPlywood (qElem* element)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = gsmQuantityPlywood;
	double				aParam;
	double				bParam;
	Int32				addParNum;

	std::string			tempStr;
	double				horLen, verLen;

	// �۾� �� ����
	short			xx;
	API_StoryInfo	storyInfo;
	double			workLevel;		// ���� �۾� �� ����


	// �۾� �� ���� �ݿ�
	BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	workLevel = 0.0;
	ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
	for (xx = 0 ; xx < (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
		if (storyInfo.data [0][xx].index == element->floorInd) {
			workLevel = storyInfo.data [0][xx].level;
			break;
		}
	}
	BMKillHandle ((GSHandle *) &storyInfo.data);

	// ��ü �ε�
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (libPart.location != NULL)
		delete libPart.location;
	if (err != NoError)
		return;

	ACAPI_LibPart_Get (&libPart);

	elem.header.typeID = API_ObjectID;
	elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&elem, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	// ���̺귯���� �Ķ���� �� �Է� (����)
	elem.header.floorInd = element->floorInd;
	elem.object.libInd = libPart.index;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.header.layer = element->qLayerInd;	// �������� ���̾�

	// �з�: �������� ��ü�� Ÿ�� (enum qPlywoodType ����)
	if (element->typeOfQPlywood == Q_WALL_INNER) {
		tempStr = "��ü(����)";
		memo.params [0][82].value.real = 75;
	} else if (element->typeOfQPlywood == Q_WALL_OUTER) {
		tempStr = "��ü(�ܺ�)";
		memo.params [0][82].value.real = 76;
	} else if (element->typeOfQPlywood == Q_WALL_COMPOSITE) {
		tempStr = "��ü(�պ�)";
		memo.params [0][82].value.real = 72;
	} else if (element->typeOfQPlywood == Q_WALL_PARAPET) {
		tempStr = "��ü(�Ķ���)";
		memo.params [0][82].value.real = 32;
	} else if (element->typeOfQPlywood == Q_WALL_WATERPROOF) {
		tempStr = "��ü(�����)";
		memo.params [0][82].value.real = 12;
	} else if (element->typeOfQPlywood == Q_SLAB_BASE) {
		tempStr = "�����(����)";
		memo.params [0][82].value.real = 66;
	} else if (element->typeOfQPlywood == Q_SLAB_RC) {
		tempStr = "�����(RC)";
		memo.params [0][82].value.real = 100;
	} else if (element->typeOfQPlywood == Q_SLAB_DECK) {
		tempStr = "�����(��ũ)";
		memo.params [0][82].value.real = 99;
	} else if (element->typeOfQPlywood == Q_SLAB_RAMP) {
		tempStr = "�����(����)";
		memo.params [0][82].value.real = 3;
	} else if (element->typeOfQPlywood == Q_BEAM) {
		tempStr = "��";
		memo.params [0][82].value.real = 78;
	} else if (element->typeOfQPlywood == Q_COLUMN_ISOLATED) {
		tempStr = "���(����)";
		memo.params [0][82].value.real = 20;
	} else if (element->typeOfQPlywood == Q_COLUMN_INWALL) {
		tempStr = "���(��ü)";
		memo.params [0][82].value.real = 77;
	} else {
		tempStr = "��ü(�ܺ�)";
	}
	GS::ucscpy (memo.params [0][30].value.uStr, GS::UniString (tempStr.c_str ()).ToUStr ().Get ());

	// ���ǵβ� (���ڿ�)
	tempStr = "12";		// 12mm
	GS::ucscpy (memo.params [0][31].value.uStr, GS::UniString (tempStr.c_str ()).ToUStr ().Get ());

	// ��ġ��ġ
	tempStr = "��԰�";
	GS::ucscpy (memo.params [0][32].value.uStr, GS::UniString (tempStr.c_str ()).ToUStr ().Get ());

	// ǰ��
	tempStr = "���Ǹ���";
	GS::ucscpy (memo.params [0][33].value.uStr, GS::UniString (tempStr.c_str ()).ToUStr ().Get ());

	// ���� ����
	if (element->validNorth == true) {
		// ���� �� ������
		elem.object.angle = DegreeToRad (180.0);
		elem.object.pos.x = element->NorthLeftBottom.x;
		elem.object.pos.y = element->NorthLeftBottom.y;
		elem.object.level = element->NorthLeftBottom.z - workLevel;

		// ��ġ����
		tempStr = "���� �����";
		GS::ucscpy (memo.params [0][34].value.uStr, GS::UniString (tempStr.c_str ()).ToUStr ().Get ());

		// ���α���
		horLen = abs (element->NorthLeftBottom.x - element->NorthRightTop.x);
		memo.params [0][38].value.real = horLen;

		// ���α���
		verLen = abs (element->NorthRightTop.z - element->NorthLeftBottom.z);
		memo.params [0][42].value.real = verLen;

		// ��ü ��ġ
		if ((horLen > EPS) && (verLen > EPS)) {
			ACAPI_Element_Create (&elem, &memo);
			element->qPlywoodGuids [element->nQPlywoods ++] = elem.header.guid;
		}
	}

	// ���� ����
	if (element->validSouth == true) {
		// ���� �� ������
		elem.object.angle = DegreeToRad (0.0);
		elem.object.pos.x = element->SouthLeftBottom.x;
		elem.object.pos.y = element->SouthLeftBottom.y;
		elem.object.level = element->SouthLeftBottom.z - workLevel;

		// ��ġ����
		tempStr = "���� �����";
		GS::ucscpy (memo.params [0][34].value.uStr, GS::UniString (tempStr.c_str ()).ToUStr ().Get ());

		// ���α���
		horLen = abs (element->SouthRightTop.x - element->SouthLeftBottom.x);
		memo.params [0][38].value.real = horLen;

		// ���α���
		verLen = abs (element->SouthRightTop.z - element->SouthLeftBottom.z);
		memo.params [0][42].value.real = verLen;

		// ��ü ��ġ
		if ((horLen > EPS) && (verLen > EPS)) {
			ACAPI_Element_Create (&elem, &memo);
			element->qPlywoodGuids [element->nQPlywoods ++] = elem.header.guid;
		}
	}

	// ���� ����
	if (element->validEast == true) {
		// ���� �� ������
		elem.object.angle = DegreeToRad (90.0);
		elem.object.pos.x = element->EastLeftBottom.x;
		elem.object.pos.y = element->EastLeftBottom.y;
		elem.object.level = element->EastLeftBottom.z - workLevel;

		// ��ġ����
		tempStr = "���� �����";
		GS::ucscpy (memo.params [0][34].value.uStr, GS::UniString (tempStr.c_str ()).ToUStr ().Get ());

		// ���α���
		horLen = abs (element->EastRightTop.y - element->EastLeftBottom.y);
		memo.params [0][38].value.real = horLen;

		// ���α���
		verLen = abs (element->EastRightTop.z - element->EastLeftBottom.z);
		memo.params [0][42].value.real = verLen;

		// ��ü ��ġ
		if ((horLen > EPS) && (verLen > EPS)) {
			ACAPI_Element_Create (&elem, &memo);
			element->qPlywoodGuids [element->nQPlywoods ++] = elem.header.guid;
		}
	}

	// ���� ����
	if (element->validWest == true) {
		// ���� �� ������
		elem.object.angle = DegreeToRad (270.0);
		elem.object.pos.x = element->WestLeftBottom.x;
		elem.object.pos.y = element->WestLeftBottom.y;
		elem.object.level = element->WestLeftBottom.z - workLevel;

		// ��ġ����
		tempStr = "���� �����";
		GS::ucscpy (memo.params [0][34].value.uStr, GS::UniString (tempStr.c_str ()).ToUStr ().Get ());

		// ���α���
		horLen = abs (element->WestLeftBottom.y - element->WestRightTop.y);
		memo.params [0][38].value.real = horLen;

		// ���α���
		verLen = abs (element->WestRightTop.z - element->WestLeftBottom.z);
		memo.params [0][42].value.real = verLen;

		// ��ü ��ġ
		if ((horLen > EPS) && (verLen > EPS)) {
			ACAPI_Element_Create (&elem, &memo);
			element->qPlywoodGuids [element->nQPlywoods ++] = elem.header.guid;
		}
	}

	// �ظ�
	if (element->validBase == true) {
		// ���� �� ������
		elem.object.angle = DegreeToRad (0.0);
		elem.object.pos.x = element->BaseLeftBottom.x;
		elem.object.pos.y = element->BaseLeftBottom.y;
		elem.object.level = element->BaseLeftBottom.z - workLevel;

		// ��ġ����
		tempStr = "�ٴڱ��";
		GS::ucscpy (memo.params [0][34].value.uStr, GS::UniString (tempStr.c_str ()).ToUStr ().Get ());

		// ���α���
		horLen = abs (element->BaseRightTop.x - element->BaseLeftBottom.x);
		memo.params [0][38].value.real = horLen;

		// ���α���
		verLen = abs (element->BaseRightTop.y - element->BaseLeftBottom.y);
		memo.params [0][42].value.real = verLen;

		// ��ü ��ġ
		if ((horLen > EPS) && (verLen > EPS)) {
			ACAPI_Element_Create (&elem, &memo);
			element->qPlywoodGuids [element->nQPlywoods ++] = elem.header.guid;
		}
	}

	ACAPI_DisposeElemMemoHdls (&memo);
}

// ������ ���������� ǥ���� �հ� ���� ����
GSErrCode	calcAreasOfQuantityPlywood (void)
{
	GSErrCode	err = NoError;
	short	xx;

	API_Element				elem;
	API_ElementMemo			memo;

	GS::Array<API_Guid>		elemList_QuantityPlywood;	// �������ǵ��� GUID

	qElemType		qElemType;	// �������� ���� �з��� ����

	// �Ķ���� �� ����
	const char*		typeStr;
	double			thk;

	// ���� �Ķ���� ��������
	API_ElementQuantity	quantity;
	API_Quantities		quantities;
	API_QuantitiesMask	mask;
	API_QuantityPar		params;

	// �ؽ�Ʈ â
	API_NewWindowPars	windowPars;
	API_WindowInfo		windowInfo;
	char				buffer [256];


	// ���� �� �ʱ�ȭ
	qElemType.initAreas (&qElemType);

	// �ؽ�Ʈ â ����
	BNZeroMemory (&windowPars, sizeof (API_NewWindowPars));
	windowPars.typeID = APIWind_MyTextID;
	windowPars.userRefCon = 1;
	GS::snuprintf (windowPars.wTitle, sizeof (windowPars.wTitle) / sizeof (GS::uchar_t), L("�������� ������ ���� ���� ����"));
	err = ACAPI_Database (APIDb_NewWindowID, &windowPars, NULL);

	// �������� ��ü�� ������
	ACAPI_Element_GetElemList (API_ObjectID, &elemList_QuantityPlywood, APIFilt_In3D);

	// �������� ��ü�� ���� �ִ� �Ķ������ ������ ������
	nElems = elemList_QuantityPlywood.GetSize ();
	for (xx = 0 ; xx < nElems ; ++xx) {
		// ��� ��������
		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		elem.header.guid = elemList_QuantityPlywood.Pop ();
		err = ACAPI_Element_Get (&elem);
		err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

		// ���� ���� ��������
		ACAPI_ELEMENT_QUANTITY_MASK_CLEAR (mask);
		ACAPI_ELEMENT_QUANTITY_MASK_SET (mask, symb, surface);
		ACAPI_ELEMENT_QUANTITY_MASK_SET (mask, symb, volume);
		quantities.elements = &quantity;
		params.minOpeningSize = EPS;
		err = ACAPI_Element_GetQuantities (elem.header.guid, &params, &quantities, &mask);

		// �з� ��������
		typeStr = getParameterStringByName (&memo, "m_type");

		if (strncmp (typeStr, "��ü(����)", strlen ("��ü(����)")) == 0) {
			thk = atoi (getParameterStringByName (&memo, "m_size"));
			thk = thk / 1000;	// mm ������ m ������ ��ȯ
			qElemType.areas_wall_in += quantity.symb.volume / thk;

		} else if (strncmp (typeStr, "��ü(�ܺ�)", strlen ("��ü(�ܺ�)")) == 0) {
			thk = atoi (getParameterStringByName (&memo, "m_size"));
			thk = thk / 1000;	// mm ������ m ������ ��ȯ
			qElemType.areas_wall_out += quantity.symb.volume / thk;

		} else if (strncmp (typeStr, "��ü(�պ�)", strlen ("��ü(�պ�)")) == 0) {
			thk = atoi (getParameterStringByName (&memo, "m_size"));
			thk = thk / 1000;	// mm ������ m ������ ��ȯ
			qElemType.areas_wall_composite += quantity.symb.volume / thk;

		} else if (strncmp (typeStr, "��ü(�Ķ���)", strlen ("��ü(�Ķ���)")) == 0) {
			thk = atoi (getParameterStringByName (&memo, "m_size"));
			thk = thk / 1000;	// mm ������ m ������ ��ȯ
			qElemType.areas_wall_parapet += quantity.symb.volume / thk;

		} else if (strncmp (typeStr, "��ü(�����)", strlen ("��ü(�����)")) == 0) {
			thk = atoi (getParameterStringByName (&memo, "m_size"));
			thk = thk / 1000;	// mm ������ m ������ ��ȯ
			qElemType.areas_wall_waterproof += quantity.symb.volume / thk;

		} else if (strncmp (typeStr, "�����(����)", strlen ("�����(����)")) == 0) {
			thk = atoi (getParameterStringByName (&memo, "m_size"));
			thk = thk / 1000;	// mm ������ m ������ ��ȯ
			qElemType.areas_slab_base += quantity.symb.volume / thk;

		} else if (strncmp (typeStr, "�����(RC)", strlen ("�����(RC)")) == 0) {
			thk = atoi (getParameterStringByName (&memo, "m_size"));
			thk = thk / 1000;	// mm ������ m ������ ��ȯ
			qElemType.areas_slab_rc += quantity.symb.volume / thk;

		} else if (strncmp (typeStr, "�����(��ũ)", strlen ("�����(��ũ)")) == 0) {
			thk = atoi (getParameterStringByName (&memo, "m_size"));
			thk = thk / 1000;	// mm ������ m ������ ��ȯ
			qElemType.areas_slab_deck += quantity.symb.volume / thk;

		} else if (strncmp (typeStr, "�����(����)", strlen ("�����(����)")) == 0) {
			thk = atoi (getParameterStringByName (&memo, "m_size"));
			thk = thk / 1000;	// mm ������ m ������ ��ȯ
			qElemType.areas_slab_ramp += quantity.symb.volume / thk;

		} else if (strncmp (typeStr, "��", strlen ("��")) == 0) {
			thk = atoi (getParameterStringByName (&memo, "m_size"));
			thk = thk / 1000;	// mm ������ m ������ ��ȯ
			qElemType.areas_beam += quantity.symb.volume / thk;

		} else if (strncmp (typeStr, "���(����)", strlen ("���(����)")) == 0) {
			thk = atoi (getParameterStringByName (&memo, "m_size"));
			thk = thk / 1000;	// mm ������ m ������ ��ȯ
			qElemType.areas_column_iso += quantity.symb.volume / thk;

		} else if (strncmp (typeStr, "���(��ü)", strlen ("���(��ü)")) == 0) {
			thk = atoi (getParameterStringByName (&memo, "m_size"));
			thk = thk / 1000;	// mm ������ m ������ ��ȯ
			qElemType.areas_column_inwall += quantity.symb.volume / thk;

		} else if (strncmp (typeStr, "���(����)", strlen ("���(����)")) == 0) {
			thk = atoi (getParameterStringByName (&memo, "m_size"));
			thk = thk / 1000;	// mm ������ m ������ ��ȯ
			qElemType.areas_column_circle += quantity.symb.volume / thk;

		} else if (strncmp (typeStr, "��������(��)", strlen ("��������(��)")) == 0) {
			thk = getParameterValueByName (&memo, "m_size");
			thk = thk / 1000;	// mm ������ m ������ ��ȯ
			qElemType.areas_ramp_wall += quantity.symb.volume / thk;
		}

		// memo ��ü ����
		ACAPI_DisposeElemMemoHdls (&memo);
	}

	// �ؽ�Ʈ â���� �����ؼ� �����ֱ�
	BNZeroMemory (&windowInfo, sizeof (API_WindowInfo));
	windowInfo.typeID = APIWind_MyTextID;
	windowInfo.index = 1;

	ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, "[�������� �з��� ����]\n");

	sprintf (buffer, "��ü(����): %lf ��\n", qElemType.areas_wall_in);
	ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	sprintf (buffer, "��ü(�ܺ�): %lf ��\n", qElemType.areas_wall_out);
	ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	sprintf (buffer, "��ü(�պ�): %lf ��\n", qElemType.areas_wall_composite);
	ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	sprintf (buffer, "��ü(�Ķ���): %lf ��\n", qElemType.areas_wall_parapet);
	ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	sprintf (buffer, "��ü(�����): %lf ��\n", qElemType.areas_wall_waterproof);
	ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	sprintf (buffer, "������(����): %lf ��\n", qElemType.areas_slab_base);
	ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	sprintf (buffer, "������(RC): %lf ��\n", qElemType.areas_slab_rc);
	ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	sprintf (buffer, "������(��ũ): %lf ��\n", qElemType.areas_slab_deck);
	ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	sprintf (buffer, "������(����): %lf ��\n", qElemType.areas_slab_ramp);
	ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	sprintf (buffer, "��: %lf ��\n", qElemType.areas_beam);
	ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	sprintf (buffer, "���(����): %lf ��\n", qElemType.areas_column_iso);
	ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	sprintf (buffer, "���(��ü): %lf ��\n", qElemType.areas_column_inwall);
	ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	sprintf (buffer, "���(����): %lf ��\n", qElemType.areas_column_circle);
	ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	sprintf (buffer, "��������(��): %lf ��\n", qElemType.areas_ramp_wall);
	ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);

	return	err;
}

// ���� ���� �ʱ�ȭ
void	qElemType::initAreas (qElemType* elem)
{
	elem->areas_wall_in = 0.0;			// ����: ��ü(����)
	elem->areas_wall_out = 0.0;			// ����: ��ü(�ܺ�)
	elem->areas_wall_composite = 0.0;	// ����: ��ü(�պ�)
	elem->areas_wall_parapet = 0.0;		// ����: ��ü(�Ķ���)
	elem->areas_wall_waterproof = 0.0;	// ����: ��ü(�����)
	elem->areas_slab_base = 0.0;		// ����: �����(����)
	elem->areas_slab_rc = 0.0;			// ����: �����(RC)
	elem->areas_slab_deck = 0.0;		// ����: �����(��ũ)
	elem->areas_slab_ramp = 0.0;		// ����: �����(����)
	elem->areas_beam = 0.0;				// ����: ��
	elem->areas_column_iso = 0.0;		// ����: ���(����)
	elem->areas_column_inwall = 0.0;	// ����: ���(��ü)
	elem->areas_column_circle = 0.0;	// ����: ���(����)
}