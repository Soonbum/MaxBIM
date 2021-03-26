#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "Quantities.hpp"

using namespace quantitiesDG;

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
	
	GS::Array<API_Guid>	elemList_All;		// ��ҵ��� GUID (��ü ���� ���)
	GS::Array<API_Guid>	elemList_Wall;		// ��ҵ��� GUID (��)
	GS::Array<API_Guid>	elemList_Slab;		// ��ҵ��� GUID (������)
	GS::Array<API_Guid>	elemList_Beam;		// ��ҵ��� GUID (��)
	GS::Array<API_Guid>	elemList_Column;	// ��ҵ��� GUID (���)


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

	// ... (������)
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

	delete []	elems;

	return	err;
}

// [���̾�α�] �������� ���� ���̾� ����
short DGCALLBACK quantityPlywoodUIHandler (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;
	//short	itmPosX, itmPosY;
	//short	xx, yy;
	//char	tempStr [20];
	//short	dialogSizeX, dialogSizeY;

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
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 120, 25, 90, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "���� ���̾�");
			DGShowItem (dialogID, itmIdx);

			// ��: �������� ���̾�
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 280, 25, 90, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "�������� ���̾�");
			DGShowItem (dialogID, itmIdx);

			// ��: �������� Ÿ��
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 450, 25, 90, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "�������� Ÿ��");
			DGShowItem (dialogID, itmIdx);

			// ... ���̴� ���̾ �ҷ��� ��

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
			
			/*
			// ���̾� ���� �����ϱ�
			GSErrCode	showLayersEasily (void)
			{
				API_Attribute	attrib;
				short			nLayers;

				// ����ü �ʱ�ȭ
				allocateMemory (&layerInfo);
				selectedInfo = layerInfo;	// selectedInfo���� vector�� ��� �����Ƿ� �ʱ�ȭ�� ���� ������ ��
				allocateMemory (&selectedInfo);

				// ������Ʈ �� ���̾� ������ �˾Ƴ�
				BNZeroMemory (&attrib, sizeof (API_Attribute));
				attrib.header.typeID = API_LayerID;
				err = ACAPI_Attribute_GetNum (API_LayerID, &nLayers);

				for (xx = 1; xx <= nLayers && err == NoError ; ++xx) {
					attrib.header.index = xx;
					err = ACAPI_Attribute_Get (&attrib);
				}

				// OK ��ư�� �ƴϸ� �޸� �����ϰ� ����
				if (result != DG_OK) {
					deallocateMemory (&layerInfo);
					deallocateMemory (&selectedInfo);
					return	err;
				}

				// ��� ���̾� �����
				BNZeroMemory (&attrib, sizeof (API_Attribute));
				attrib.header.typeID = API_LayerID;
	
				for (xx = 1; xx <= nLayers ; ++xx) {
					attrib.header.index = xx;
					err = ACAPI_Attribute_Get (&attrib);
					if (err == NoError) {
						//if ((attrib.layer.head.flags & APILay_Hidden) == false) {
							attrib.layer.head.flags |= APILay_Hidden;
							ACAPI_Attribute_Modify (&attrib, NULL);
						//}
					}
				}

												// ������ ���̾� �̸� �˻��ϱ�
												BNZeroMemory (&attrib, sizeof (API_Attribute));
												attrib.header.typeID = API_LayerID;
												CHCopyC (fullLayerName, attrib.header.name);
												err = ACAPI_Attribute_Get (&attrib);

												// �ش� ���̾� �����ֱ�
												if ((attrib.layer.head.flags & APILay_Hidden) == true) {
													attrib.layer.head.flags ^= APILay_Hidden;
													ACAPI_Attribute_Modify (&attrib, NULL);
												}
											}
										}
									}
								}
							}
						}
					}
				}
	
				// ȭ�� ���ΰ�ħ
				ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
				bool	regenerate = true;
				ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

				// �޸� ����
				deallocateMemory (&layerInfo);
				deallocateMemory (&selectedInfo);

				return err;
			}
			*/

			//// ��: �ڵ� �����ֱ�
			//itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 0, 120, 50, 23);
			//DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, itmIdx, "CODE");
			//DGShowItem (dialogID, itmIdx);
			//LABEL_CODE = itmIdx;

			//// ��: ���� ����
			//itmPosX = 40;
			//itmPosY = 25;
			//itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			//DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, itmIdx, "���� ����");
			//DGShowItem (dialogID, itmIdx);

			//// üũ�ڽ�: ���� ���� ��ư
			//itmPosX = 150;
			//itmPosY = 20;
			//for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx) {
			//	if (layerInfo.code_state [xx] == true) {
			//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			//		DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
			//		sprintf (tempStr, "%s %s", layerInfo.code_name [xx].c_str (), layerInfo.code_desc [xx].c_str ());
			//		DGSetItemText (dialogID, itmIdx, tempStr);
			//		DGShowItem (dialogID, itmIdx);
			//		layerInfo.code_idx [xx] = itmIdx;

			//		itmPosX += 100;
			//		if (itmPosX >= 600) {
			//			itmPosX = 150;
			//			itmPosY += 30;
			//		}
			//	}
			//}
			//// ��� ����
			//itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			//DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
			//DGSetItemText (dialogID, itmIdx, "��� ����");
			//DGShowItem (dialogID, itmIdx);
			//SELECTALL_1_CONTYPE = itmIdx;
			//itmPosX += 100;
			//if (itmPosX >= 600) {
			//	itmPosX = 150;
			//	itmPosY += 30;
			//}

			//itmPosY += 30;

			//// ��: �� ����
			//itmPosX = 40;
			//itmPosY += 10;
			//itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			//DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, itmIdx, "�� ����");
			//DGShowItem (dialogID, itmIdx);

			//// üũ�ڽ�: �� ���� ��ư
			//itmPosX = 150;
			//itmPosY -= 5;
			//for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx) {
			//	if (layerInfo.dong_state [xx] == true) {
			//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			//		DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
			//		DGSetItemText (dialogID, itmIdx, layerInfo.dong_desc [xx].c_str ());
			//		DGShowItem (dialogID, itmIdx);
			//		layerInfo.dong_idx [xx] = itmIdx;

			//		itmPosX += 100;
			//		if (itmPosX >= 600) {
			//			itmPosX = 150;
			//			itmPosY += 30;
			//		}
			//	}
			//}
			//// ��� ����
			//if (layerInfo.bDongAllShow) {
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
			//	DGSetItemText (dialogID, itmIdx, "��� ����");
			//	DGShowItem (dialogID, itmIdx);
			//	SELECTALL_2_DONG = itmIdx;
			//	itmPosX += 100;
			//	if (itmPosX >= 600) {
			//		itmPosX = 150;
			//		itmPosY += 30;
			//	}
			//}

			//itmPosY += 30;

			//// ��: �� ����
			//itmPosX = 40;
			//itmPosY += 10;
			//itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			//DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, itmIdx, "�� ����");
			//DGShowItem (dialogID, itmIdx);

			//// üũ�ڽ�: �� ���� ��ư
			//itmPosX = 150;
			//itmPosY -= 5;
			//for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx) {
			//	if (layerInfo.floor_state [xx] == true) {
			//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			//		DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
			//		DGSetItemText (dialogID, itmIdx, layerInfo.floor_desc [xx].c_str ());
			//		DGShowItem (dialogID, itmIdx);
			//		layerInfo.floor_idx [xx] = itmIdx;

			//		itmPosX += 100;
			//		if (itmPosX >= 600) {
			//			itmPosX = 150;
			//			itmPosY += 30;
			//		}
			//	}
			//}
			//// ��� ����
			//itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			//DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
			//DGSetItemText (dialogID, itmIdx, "��� ����");
			//DGShowItem (dialogID, itmIdx);
			//SELECTALL_3_FLOOR = itmIdx;
			//itmPosX += 100;
			//if (itmPosX >= 600) {
			//	itmPosX = 150;
			//	itmPosY += 30;
			//}

			//itmPosY += 30;

			//// ��: CJ ����
			//itmPosX = 40;
			//itmPosY += 10;
			//itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			//DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, itmIdx, "CJ ����");
			//DGShowItem (dialogID, itmIdx);

			//// üũ�ڽ�: CJ ���� ��ư
			//itmPosX = 150;
			//itmPosY -= 5;
			//for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx) {
			//	if (layerInfo.CJ_state [xx] == true) {
			//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			//		DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
			//		DGSetItemText (dialogID, itmIdx, layerInfo.CJ_name [xx].c_str ());
			//		DGShowItem (dialogID, itmIdx);
			//		layerInfo.CJ_idx [xx] = itmIdx;

			//		itmPosX += 100;
			//		if (itmPosX >= 600) {
			//			itmPosX = 150;
			//			itmPosY += 30;
			//		}
			//	}
			//}
			//// ��� ����
			//if (layerInfo.bCJAllShow) {
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
			//	DGSetItemText (dialogID, itmIdx, "��� ����");
			//	DGShowItem (dialogID, itmIdx);
			//	SELECTALL_4_CJ = itmIdx;
			//	itmPosX += 100;
			//	if (itmPosX >= 600) {
			//		itmPosX = 150;
			//		itmPosY += 30;
			//	}
			//}

			//itmPosY += 30;

			//// ��: CJ �� �ð�����
			//itmPosX = 40;
			//itmPosY += 10;
			//itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			//DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, itmIdx, "�ð�����");
			//DGShowItem (dialogID, itmIdx);

			//// üũ�ڽ�: CJ �� �ð����� ��ư
			//itmPosX = 150;
			//itmPosY -= 5;
			//for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx) {
			//	if (layerInfo.orderInCJ_state [xx] == true) {
			//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			//		DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
			//		DGSetItemText (dialogID, itmIdx, layerInfo.orderInCJ_name [xx].c_str ());
			//		DGShowItem (dialogID, itmIdx);
			//		layerInfo.orderInCJ_idx [xx] = itmIdx;

			//		itmPosX += 100;
			//		if (itmPosX >= 600) {
			//			itmPosX = 150;
			//			itmPosY += 30;
			//		}
			//	}
			//}
			//// ��� ����
			//if (layerInfo.bOrderInCJAllShow) {
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
			//	DGSetItemText (dialogID, itmIdx, "��� ����");
			//	DGShowItem (dialogID, itmIdx);
			//	SELECTALL_5_ORDER = itmIdx;
			//	itmPosX += 100;
			//	if (itmPosX >= 600) {
			//		itmPosX = 150;
			//		itmPosY += 30;
			//	}
			//}

			//itmPosY += 30;

			//// ��: ����(����)
			//itmPosX = 40;
			//itmPosY += 10;
			//itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			//DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, itmIdx, "*����");
			//DGShowItem (dialogID, itmIdx);

			//// üũ�ڽ�: ����(����)
			//itmPosX = 150;
			//itmPosY -= 5;
			//for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
			//	if (strncmp (layerInfo.obj_cat [xx].c_str (), "01-S", 4) == 0) {
			//		if (layerInfo.obj_state [xx] == true) {
			//			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			//			DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
			//			DGSetItemText (dialogID, itmIdx, layerInfo.obj_desc [xx].c_str ());
			//			DGShowItem (dialogID, itmIdx);
			//			layerInfo.obj_idx [xx] = itmIdx;

			//			itmPosX += 100;
			//			if (itmPosX >= 600) {
			//				itmPosX = 150;
			//				itmPosY += 30;
			//			}
			//		}
			//	}
			//}

			//itmPosY += 30;

			//// ��: ����(���ึ��)
			//itmPosX = 40;
			//itmPosY += 10;
			//itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			//DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, itmIdx, "*���ึ��");
			//DGShowItem (dialogID, itmIdx);

			//// üũ�ڽ�: ����(���ึ��)
			//itmPosX = 150;
			//itmPosY -= 5;
			//for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
			//	if (strncmp (layerInfo.obj_cat [xx].c_str (), "02-A", 4) == 0) {
			//		if (layerInfo.obj_state [xx] == true) {
			//			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			//			DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
			//			DGSetItemText (dialogID, itmIdx, layerInfo.obj_desc [xx].c_str ());
			//			DGShowItem (dialogID, itmIdx);
			//			layerInfo.obj_idx [xx] = itmIdx;

			//			itmPosX += 100;
			//			if (itmPosX >= 600) {
			//				itmPosX = 150;
			//				itmPosY += 30;
			//			}
			//		}
			//	}
			//}

			//itmPosY += 30;

			//// ��: ����(������)
			//itmPosX = 40;
			//itmPosY += 10;
			//itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			//DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, itmIdx, "*������");
			//DGShowItem (dialogID, itmIdx);

			//// üũ�ڽ�: ����(������)
			//itmPosX = 150;
			//itmPosY -= 5;
			//for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
			//	if (strncmp (layerInfo.obj_cat [xx].c_str (), "05-T", 4) == 0) {
			//		if (layerInfo.obj_state [xx] == true) {
			//			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			//			DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
			//			DGSetItemText (dialogID, itmIdx, layerInfo.obj_desc [xx].c_str ());
			//			DGShowItem (dialogID, itmIdx);
			//			layerInfo.obj_idx [xx] = itmIdx;

			//			itmPosX += 100;
			//			if (itmPosX >= 600) {
			//				itmPosX = 150;
			//				itmPosY += 30;
			//			}
			//		}
			//	}
			//}

			//itmPosY += 30;

			//// ��: ����(���ü�)
			//itmPosX = 40;
			//itmPosY += 10;
			//itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			//DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, itmIdx, "*���ü�");
			//DGShowItem (dialogID, itmIdx);

			//// üũ�ڽ�: ����(���ü�)
			//itmPosX = 150;
			//itmPosY -= 5;
			//for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
			//	if (strncmp (layerInfo.obj_cat [xx].c_str (), "06-F", 4) == 0) {
			//		if (layerInfo.obj_state [xx] == true) {
			//			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			//			DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
			//			DGSetItemText (dialogID, itmIdx, layerInfo.obj_desc [xx].c_str ());
			//			DGShowItem (dialogID, itmIdx);
			//			layerInfo.obj_idx [xx] = itmIdx;

			//			itmPosX += 100;
			//			if (itmPosX >= 600) {
			//				itmPosX = 150;
			//				itmPosY += 30;
			//			}
			//		}
			//	}
			//}

			//itmPosY += 30;

			//// ��: ��ü(������)
			//itmPosX = 40;
			//itmPosY += 10;
			//itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			//DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, itmIdx, "**������(����)");
			//DGShowItem (dialogID, itmIdx);

			//// üũ�ڽ�: ��ü(������)
			//itmPosX = 150;
			//itmPosY -= 5;
			//for (xx = 0 ; xx < layerInfo.subObj_name.size () ; ++xx) {
			//	for (yy = 0 ; yy < layerInfo.code_name.size () ; ++yy) {
			//		if ((strncmp (layerInfo.subObj_cat [xx].c_str (), "05-T", 4) == 0) && (strncmp (layerInfo.code_name [yy].c_str (), layerInfo.subObj_cat [xx].c_str (), 4) == 0) && (layerInfo.code_state [yy] == true)) {
			//			if (layerInfo.obj_state [xx] == true) {
			//				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			//				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
			//				DGSetItemText (dialogID, itmIdx, layerInfo.subObj_desc [xx].c_str ());
			//				DGShowItem (dialogID, itmIdx);
			//				layerInfo.subObj_idx [xx] = itmIdx;

			//				itmPosX += 100;
			//				if (itmPosX >= 600) {
			//					itmPosX = 150;
			//					itmPosY += 30;
			//				}
			//			}
			//		}
			//	}
			//}

			//dialogSizeX = 700;
			//dialogSizeY = itmPosY + 150;
			//DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);

			break;
		
		case DG_MSG_CHANGE:
			//changedBtnItemIdx = item;

			//if (changedBtnItemIdx == SELECTALL_1_CONTYPE) {
			//	if (DGGetItemValLong (dialogID, SELECTALL_1_CONTYPE) == TRUE) {
			//		// ��� ����
			//		for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx)
			//			DGSetItemValLong (dialogID, layerInfo.code_idx [xx], TRUE);
			//	} else {
			//		// ��� ����
			//		for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx)
			//			DGSetItemValLong (dialogID, layerInfo.code_idx [xx], FALSE);
			//	}
			//}
			//if (changedBtnItemIdx == SELECTALL_2_DONG) {
			//	if (DGGetItemValLong (dialogID, SELECTALL_2_DONG) == TRUE) {
			//		// ��� ����
			//		for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx)
			//			DGSetItemValLong (dialogID, layerInfo.dong_idx [xx], TRUE);
			//	} else {
			//		// ��� ����
			//		for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx)
			//			DGSetItemValLong (dialogID, layerInfo.dong_idx [xx], FALSE);
			//	}
			//}
			//if (changedBtnItemIdx == SELECTALL_3_FLOOR) {
			//	if (DGGetItemValLong (dialogID, SELECTALL_3_FLOOR) == TRUE) {
			//		// ��� ����
			//		for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx)
			//			DGSetItemValLong (dialogID, layerInfo.floor_idx [xx], TRUE);
			//	} else {
			//		// ��� ����
			//		for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx)
			//			DGSetItemValLong (dialogID, layerInfo.floor_idx [xx], FALSE);
			//	}
			//}
			//if (changedBtnItemIdx == SELECTALL_4_CJ) {
			//	if (DGGetItemValLong (dialogID, SELECTALL_4_CJ) == TRUE) {
			//		// ��� ����
			//		for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx)
			//			DGSetItemValLong (dialogID, layerInfo.CJ_idx [xx], TRUE);
			//	} else {
			//		// ��� ����
			//		for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx)
			//			DGSetItemValLong (dialogID, layerInfo.CJ_idx [xx], FALSE);
			//	}
			//}
			//if (changedBtnItemIdx == SELECTALL_5_ORDER) {
			//	if (DGGetItemValLong (dialogID, SELECTALL_5_ORDER) == TRUE) {
			//		// ��� ����
			//		for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx)
			//			DGSetItemValLong (dialogID, layerInfo.orderInCJ_idx [xx], TRUE);
			//	} else {
			//		// ��� ����
			//		for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx)
			//			DGSetItemValLong (dialogID, layerInfo.orderInCJ_idx [xx], FALSE);
			//	}
			//}

			//// ���縦 �����ϸ� ���� �ڵ带 �ڵ� ����
			//for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
			//	if (DGGetItemValLong (dialogID, layerInfo.obj_idx [xx]) == TRUE) {
			//		for (yy = 0 ; yy < layerInfo.code_name.size () ; ++yy) {
			//			if (strncmp (layerInfo.obj_cat [xx].c_str (), layerInfo.code_name [yy].c_str (), 4) == 0) {
			//				DGSetItemValLong (dialogID, layerInfo.code_idx [yy], TRUE);
			//			}
			//		}
			//	}
			//}

			//// ��ü�� �����ϸ� ���� �ڵ带 �ڵ� ����
			//for (xx = 0 ; xx < layerInfo.subObj_name.size () ; ++xx) {
			//	if (DGGetItemValLong (dialogID, layerInfo.subObj_idx [xx]) == TRUE) {
			//		for (yy = 0 ; yy < layerInfo.code_name.size () ; ++yy) {
			//			if (strncmp (layerInfo.subObj_cat [xx].c_str (), layerInfo.code_name [yy].c_str (), 4) == 0) {
			//				DGSetItemValLong (dialogID, layerInfo.code_idx [yy], TRUE);
			//			}
			//		}
			//	}
			//}

			//// ��ư�� �̸� �����ֱ�
			//for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx) {
			//	if ((layerInfo.code_idx [xx] == changedBtnItemIdx) && (layerInfo.code_state [xx] == true)) {
			//		sprintf (tempStr, "%s", layerInfo.code_name [xx].c_str ());
			//		DGSetItemText (dialogID, LABEL_CODE, tempStr);
			//	}
			//}
			//for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx) {
			//	if ((layerInfo.dong_idx [xx] == changedBtnItemIdx) && (layerInfo.dong_state [xx] == true)) {
			//		sprintf (tempStr, "%s", layerInfo.dong_name [xx].c_str ());
			//		DGSetItemText (dialogID, LABEL_CODE, tempStr);
			//	}
			//}
			//for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx) {
			//	if ((layerInfo.floor_idx [xx] == changedBtnItemIdx) && (layerInfo.floor_state [xx] == true)) {
			//		sprintf (tempStr, "%s", layerInfo.floor_name [xx].c_str ());
			//		DGSetItemText (dialogID, LABEL_CODE, tempStr);
			//	}
			//}
			//for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx) {
			//	if ((layerInfo.CJ_idx [xx] == changedBtnItemIdx) && (layerInfo.CJ_state [xx] == true)) {
			//		sprintf (tempStr, "%s", layerInfo.CJ_name [xx].c_str ());
			//		DGSetItemText (dialogID, LABEL_CODE, tempStr);
			//	}
			//}
			//for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx) {
			//	if ((layerInfo.orderInCJ_idx [xx] == changedBtnItemIdx) && (layerInfo.orderInCJ_state [xx] == true)) {
			//		sprintf (tempStr, "%s", layerInfo.orderInCJ_name [xx].c_str ());
			//		DGSetItemText (dialogID, LABEL_CODE, tempStr);
			//	}
			//}
			//for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
			//	if ((layerInfo.obj_idx [xx] == changedBtnItemIdx) && (layerInfo.obj_state [xx] == true)) {
			//		sprintf (tempStr, "%s", layerInfo.obj_name [xx].c_str ());
			//		DGSetItemText (dialogID, LABEL_CODE, tempStr);
			//	}
			//}
			//for (xx = 0 ; xx < layerInfo.subObj_name.size () ; ++xx) {
			//	if ((layerInfo.subObj_idx [xx] == changedBtnItemIdx) && (layerInfo.subObj_state [xx] == true)) {
			//		sprintf (tempStr, "%s", layerInfo.subObj_name [xx].c_str ());
			//		DGSetItemText (dialogID, LABEL_CODE, tempStr);
			//	}
			//}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					//// ���� ����
					//for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx)
					//	(DGGetItemValLong (dialogID, layerInfo.code_idx [xx]) == TRUE) ? selectedInfo.code_state [xx] = true : selectedInfo.code_state [xx] = false;

					//// �� ����
					//for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx)
					//	(DGGetItemValLong (dialogID, layerInfo.dong_idx [xx]) == TRUE) ? selectedInfo.dong_state [xx] = true : selectedInfo.dong_state [xx] = false;

					//// �� ����
					//for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx)
					//	(DGGetItemValLong (dialogID, layerInfo.floor_idx [xx]) == TRUE) ? selectedInfo.floor_state [xx] = true : selectedInfo.floor_state [xx] = false;

					//// CJ
					//for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx)
					//	(DGGetItemValLong (dialogID, layerInfo.CJ_idx [xx]) == TRUE) ? selectedInfo.CJ_state [xx] = true : selectedInfo.CJ_state [xx] = false;

					//// CJ �� �ð�����
					//for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx)
					//	(DGGetItemValLong (dialogID, layerInfo.orderInCJ_idx [xx]) == TRUE) ? selectedInfo.orderInCJ_state [xx] = true : selectedInfo.orderInCJ_state [xx] = false;

					//// ����
					//for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx)
					//	(DGGetItemValLong (dialogID, layerInfo.obj_idx [xx]) == TRUE) ? selectedInfo.obj_state [xx] = true : selectedInfo.obj_state [xx] = false;

					//// ��ü
					//for (xx = 0 ; xx < layerInfo.subObj_name.size () ; ++xx)
					//	(DGGetItemValLong (dialogID, layerInfo.subObj_idx [xx]) == TRUE) ? selectedInfo.subObj_state [xx] = true : selectedInfo.subObj_state [xx] = false;

					//// ��ư ���� ����
					//saveButtonStatus ();

					break;

				case DG_CANCEL:
					break;

				default:
					//clickedBtnItemIdx = item;
					//item = 0;

					//// ����� ��ư ���¸� �ҷ���
					//if (clickedBtnItemIdx == BUTTON_LOAD) {
					//	BNZeroMemory (&info, sizeof (API_ModulData));
					//	err = ACAPI_ModulData_Get (&info, "ButtonStatus");

					//	if (err == NoError && info.dataVersion == 1) {
					//		selectedInfoSaved = *(reinterpret_cast<StatusOfLayerNameSystem*> (*info.dataHdl));
					//		
					//		for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx) {
					//			if (selectedInfoSaved.code_state [xx] == true)
					//				DGSetItemValLong (dialogID, layerInfo.code_idx [xx], TRUE);
					//		}
					//		for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx) {
					//			if (selectedInfoSaved.dong_state [xx] == true)
					//				DGSetItemValLong (dialogID, layerInfo.dong_idx [xx], TRUE);
					//		}
					//		for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx) {
					//			if (selectedInfoSaved.floor_state [xx] == true)
					//				DGSetItemValLong (dialogID, layerInfo.floor_idx [xx], TRUE);
					//		}
					//		for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx) {
					//			if (selectedInfoSaved.CJ_state [xx] == true)
					//				DGSetItemValLong (dialogID, layerInfo.CJ_idx [xx], TRUE);
					//		}
					//		for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx) {
					//			if (selectedInfoSaved.orderInCJ_state [xx] == true)
					//				DGSetItemValLong (dialogID, layerInfo.orderInCJ_idx [xx], TRUE);
					//		}
					//		for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
					//			if (selectedInfoSaved.obj_state [xx] == true)
					//				DGSetItemValLong (dialogID, layerInfo.obj_idx [xx], TRUE);
					//		}
					//		for (xx = 0 ; xx < layerInfo.subObj_name.size () ; ++xx) {
					//			if (selectedInfoSaved.subObj_state [xx] == true)
					//				DGSetItemValLong (dialogID, layerInfo.subObj_idx [xx], TRUE);
					//		}
					//	}

					//	BMKillHandle (&info.dataHdl);
					//}

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
