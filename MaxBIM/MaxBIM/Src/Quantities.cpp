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
	// ...

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

	//for (xx = 0 ; xx < nElems ; ++xx) {
	//	for (yy = 0 ; yy < nElems ; ++yy) {
	// ��, �ڱ� �ڽŰ��� �񱳴� ���� �ʴ´�. (���� xx��° guid�� yy��° guid�� �����ϸ� continue)

	// *** (�߿�) ������ ��Ҵ� ���� �Ǵ� �ظ鿡 ���� X, Y, Z�� �� ������ ȸ�� ���� ������ ����ü�� ���� �־�� ��

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

	// ������ü ������ ������ 8���� ��ǥ ��ü ��ġ (��: ��� xx - �� ?)
	// ...
	//BNZeroMemory (&elem, sizeof (API_Element));
	//BNZeroMemory (&memo, sizeof (API_ElementMemo));
	//ACAPI_Element_Get (&elem);
	//ACAPI_Element_GetMemo (elem.header.guid, &memo);

	//// ���������� ���,
	//if (elem.header.typeID == API_PolyLineID) {
	//	for (int yy = 1 ; yy <= elem.polyLine.poly.nCoords ; ++yy) {
	//		err = placeCoordinateLabel (memo.coords [0][yy].x, memo.coords [0][yy].y, 0, false, "", layerInd);
	//	}
	//}

	//// ������ ���,
	//if (elem.header.typeID == API_MorphID) {
	//	ACAPI_Element_Get3DInfo (elem.header, &info3D);

	//	// ������ 3D �ٵ� ������
	//	BNZeroMemory (&component, sizeof (API_Component3D));
	//	component.header.typeID = API_BodyID;
	//	component.header.index = info3D.fbody;
	//	err = ACAPI_3D_GetComponent (&component);

	//	// ������ 3D ���� �������� ���ϸ� ����
	//	if (err != NoError) {
	//		ACAPI_WriteReport ("������ 3D ���� �������� ���߽��ϴ�.", true);
	//		return err;
	//	}

	//	nVert = component.body.nVert;
	//	nEdge = component.body.nEdge;
	//	nPgon = component.body.nPgon;
	//	tm = component.body.tranmat;
	//	elemIdx = component.body.head.elemIndex - 1;
	//	bodyIdx = component.body.head.bodyIndex - 1;
	//
	//	// ���� ��ǥ�� ���� ������� ������
	//	for (xx = 1 ; xx <= nVert ; ++xx) {
	//		component.header.typeID	= API_VertID;
	//		component.header.index	= xx;
	//		err = ACAPI_3D_GetComponent (&component);
	//		if (err == NoError) {
	//			trCoord.x = tm.tmx[0]*component.vert.x + tm.tmx[1]*component.vert.y + tm.tmx[2]*component.vert.z + tm.tmx[3];
	//			trCoord.y = tm.tmx[4]*component.vert.x + tm.tmx[5]*component.vert.y + tm.tmx[6]*component.vert.z + tm.tmx[7];
	//			trCoord.z = tm.tmx[8]*component.vert.x + tm.tmx[9]*component.vert.y + tm.tmx[10]*component.vert.z + tm.tmx[11];
	//			coords.Push (trCoord);
	//		}
	//	}
	//	nNodes = coords.GetSize ();

	//	tempString = format_string ("%s", "MIN ��");
	//	err = placeCoordinateLabel (info3D.bounds.xMin, info3D.bounds.yMin, info3D.bounds.zMin, true, tempString, layerInd);
	//	tempString = format_string ("%s", "MAX ��");
	//	err = placeCoordinateLabel (info3D.bounds.xMax, info3D.bounds.yMax, info3D.bounds.zMax, true, tempString, layerInd);

	//	for (xx = 1 ; xx <= nNodes ; ++xx) {
	//		point3D = coords.Pop ();

	//		tempString = format_string ("%d��", xx);
	//		err = placeCoordinateLabel (point3D.x, point3D.y, point3D.z, true, tempString, layerInd);
	//	}
	//}

	//// ���� ���,
	//if (elem.header.typeID == API_BeamID) {
	//	tempString = format_string ("����");
	//	err = placeCoordinateLabel (elem.beam.begC.x, elem.beam.begC.y, elem.beam.level, true, tempString, layerInd);
	//	tempString = format_string ("��");
	//	err = placeCoordinateLabel (elem.beam.endC.x, elem.beam.endC.y, elem.beam.level, true, tempString, layerInd);
	//}

	//ACAPI_DisposeElemMemoHdls (&memo);

	return	err;
}