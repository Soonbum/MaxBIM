#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "Export.hpp"

// ����(���,��,������)���� ������ �����ϰ� �����ؼ� ���� ���Ϸ� ��������
GSErrCode	exportElementInfo (void)
{
	GSErrCode	err = NoError;

	GS::Array<API_Guid> elemList;
	char	msg [200];
	short	xx, yy;

	// ��ü ���� ��������
	API_Element				elem;
	API_ElementMemo			memo;
	API_ElemInfo3D			info3D;

	// �۾� �� ����
	API_StoryInfo			storyInfo;


	// ����
	// 1. Ȱ���� ���̾���� ����� ������ ������

	// ���
	// ��� ��յ��� �߽���, ����/����/���� ���� ������
	ACAPI_Element_GetElemList (API_ColumnID, &elemList, APIFilt_OnVisLayer);	// ���̴� ���̾ ����

	for (xx = 0 ; xx < elemList.GetSize () ; ++xx) {
		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		elem.header.guid = elemList.Pop ();
		err = ACAPI_Element_Get (&elem);
		err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

		sprintf (msg, "�� �ε���: %d, �߽��� ��ǥ: (%.3f, %.3f), ����: %.3f, ����: %.3f, ����: %.3f, �Ʒ��� ��: %.3f",
			elem.header.floorInd,
			elem.column.origoPos.x,
			elem.column.origoPos.y,
			elem.column.coreWidth + elem.column.venThick*2,
			elem.column.coreDepth + elem.column.venThick*2,
			elem.column.height,
			elem.column.bottomOffset);
		ACAPI_WriteReport (msg, true);

		ACAPI_DisposeElemMemoHdls (&memo);
	}
	
	// ��
	// ��� ������ ����/���� ��ǥ, �ʺ�, ����, ���� ���� ������
	ACAPI_Element_GetElemList (API_BeamID, &elemList, APIFilt_OnVisLayer);	// ���̴� ���̾ ����

	for (xx = 0 ; xx < elemList.GetSize () ; ++xx) {
		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		elem.header.guid = elemList.Pop ();
		err = ACAPI_Element_Get (&elem);
		err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

		sprintf (msg, "�� �ε���: %d, ������: (%.3f, %.3f), ����: (%.3f, %.3f), �ʺ�: %.3f, ����: %.3f, ��: %.3f",
			elem.header.floorInd,
			elem.beam.begC.x,
			elem.beam.begC.y,
			elem.beam.endC.x,
			elem.beam.endC.y,
			elem.beam.width,
			elem.beam.height,
			elem.beam.level);
		ACAPI_WriteReport (msg, true);

		ACAPI_DisposeElemMemoHdls (&memo);
	}

	// ������
	// ��� ��������� ������ ��ǥ, ����, ���� ���� ������
	ACAPI_Element_GetElemList (API_SlabID, &elemList, APIFilt_OnVisLayer);	// ���̴� ���̾ ����

	for (xx = 0 ; xx < elemList.GetSize () ; ++xx) {
		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		elem.header.guid = elemList.Pop ();
		err = ACAPI_Element_Get (&elem);
		err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

		sprintf (msg, "�� �ε���: %d, �β�: %.3f, ����: %.3f",
			elem.header.floorInd,
			elem.slab.thickness,
			elem.slab.level);
		ACAPI_WriteReport (msg, true);

		/*
		for (yy = 0 ; yy < elem.slab.poly.nCoords ; ++yy) {
			memo.coords [0][yy].x;
			memo.coords [0][yy].y;
		}
		*/

		ACAPI_DisposeElemMemoHdls (&memo);
	}

	return	err;
}