#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "Export.hpp"

// ������������ ������ �� ����ϴ� ���Լ�
int compare (const void* first, const void* second)
{
    if (*(int*)first > *(int*)second)
        return 1;
    else if (*(int*)first < *(int*)second)
        return -1;
    else
        return 0;
}

// ����(���,��,������)���� ������ �����ϰ� �����ؼ� ���� ���Ϸ� ��������
GSErrCode	exportElementInfo (void)
{
	GSErrCode	err = NoError;

	GS::Array<API_Guid> elemList;
	char	msg [200];
	char	buffer [200];
	char	piece [20];
	short	xx, yy;

	// ��ü ���� ��������
	API_Element			elem;
	API_ElementMemo		memo;
	API_ElemInfo3D		info3D;

	// �۾� �� ����
	API_StoryInfo	storyInfo;

	// �ֿ� ��ȣ�� �����ϱ� ���� ������
	double	coords1_hor [100];
	short	nCoords1_hor;
	double	coords1_ver [100];
	short	nCoords1_ver;

	double	coords2_hor [100];
	short	nCoords2_hor;
	double	coords2_ver [100];
	short	nCoords2_ver;

	// ������ �����ϱ� ���� ����ü
	ColumnInfo		columnInfo;
	

	// 1. ��� ���� ��������
	ACAPI_Element_GetElemList (API_ColumnID, &elemList, APIFilt_OnVisLayer);	// ���̴� ���̾ ����

	nCoords1_hor = 0;
	nCoords1_ver = 0;

	BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);

	for (xx = 0 ; xx < (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
		for (yy = 0 ; yy < elemList.GetSize () ; ++yy) {
			BNZeroMemory (&elem, sizeof (API_Element));
			BNZeroMemory (&memo, sizeof (API_ElementMemo));
			elem.header.guid = elemList.Pop ();
			err = ACAPI_Element_Get (&elem);
			err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

			if (storyInfo.data [0][yy].index == elem.header.floorInd) {
				coords1_hor [nCoords1_hor++] = elem.column.origoPos.x;
				coords1_ver [nCoords1_ver++] = elem.column.origoPos.y;
			}

			ACAPI_DisposeElemMemoHdls (&memo);
		}

		// ... �� ������ ó���ؾ� ��
		// ���� �� -- ���!
		qsort (&coords1_hor, nCoords1_hor, sizeof (double), compare);
		// ���� �� -- ���!
		// - traversing�ϸ鼭 �ٸ� �迭�� �� ����: ������ 3000 �����̸�, ���� �ʰ��ϸ� ����
	}

	BMKillHandle ((GSHandle *) &storyInfo.data);



	//// ���
	//// ��� ��յ��� �߽���, ����/����/���� ���� ������
	//ACAPI_Element_GetElemList (API_ColumnID, &elemList, APIFilt_OnVisLayer);	// ���̴� ���̾ ����

	//for (xx = 0 ; xx < elemList.GetSize () ; ++xx) {
	//	BNZeroMemory (&elem, sizeof (API_Element));
	//	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	//	elem.header.guid = elemList.Pop ();
	//	err = ACAPI_Element_Get (&elem);
	//	err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

	//	sprintf (msg, "�� �ε���: %d, �߽��� ��ǥ: (%.3f, %.3f), ����: %.3f, ����: %.3f, ����: %.3f, �Ʒ��� ��: %.3f",
	//		elem.header.floorInd,
	//		elem.column.origoPos.x,
	//		elem.column.origoPos.y,
	//		elem.column.coreWidth + elem.column.venThick*2,
	//		elem.column.coreDepth + elem.column.venThick*2,
	//		elem.column.height,
	//		elem.column.bottomOffset);
	//	ACAPI_WriteReport (msg, true);

	//	ACAPI_DisposeElemMemoHdls (&memo);
	//}
	//
	//// ��
	//// ��� ������ ����/���� ��ǥ, �ʺ�, ����, ���� ���� ������
	//ACAPI_Element_GetElemList (API_BeamID, &elemList, APIFilt_OnVisLayer);	// ���̴� ���̾ ����

	//for (xx = 0 ; xx < elemList.GetSize () ; ++xx) {
	//	BNZeroMemory (&elem, sizeof (API_Element));
	//	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	//	elem.header.guid = elemList.Pop ();
	//	err = ACAPI_Element_Get (&elem);
	//	err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

	//	sprintf (msg, "�� �ε���: %d, ������: (%.3f, %.3f), ����: (%.3f, %.3f), �ʺ�: %.3f, ����: %.3f, ��: %.3f",
	//		elem.header.floorInd,
	//		elem.beam.begC.x,
	//		elem.beam.begC.y,
	//		elem.beam.endC.x,
	//		elem.beam.endC.y,
	//		elem.beam.width,
	//		elem.beam.height,
	//		elem.beam.level);
	//	ACAPI_WriteReport (msg, true);

	//	ACAPI_DisposeElemMemoHdls (&memo);
	//}

	//// ������
	//// ��� ��������� ������ ��ǥ, ����, ���� ���� ������
	//ACAPI_Element_GetElemList (API_SlabID, &elemList, APIFilt_OnVisLayer);	// ���̴� ���̾ ����

	//for (xx = 0 ; xx < elemList.GetSize () ; ++xx) {
	//	BNZeroMemory (&elem, sizeof (API_Element));
	//	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	//	elem.header.guid = elemList.Pop ();
	//	err = ACAPI_Element_Get (&elem);
	//	err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

	//	sprintf (msg, "�� �ε���: %d, �β�: %.3f, ����: %.3f",
	//		elem.header.floorInd,
	//		elem.slab.thickness,
	//		elem.slab.level);
	//	ACAPI_WriteReport (msg, true);

	//	/*
	//	for (yy = 0 ; yy < elem.slab.poly.nCoords ; ++yy) {
	//		memo.coords [0][yy].x;
	//		memo.coords [0][yy].y;
	//	}
	//	*/

	//	ACAPI_DisposeElemMemoHdls (&memo);
	//}




	/*
	// �� ���� �����ϱ�
	BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
	sprintf (msg, "ó�� ��: %d, ������ ��: %d, ������ ����: %d", storyInfo.firstStory, storyInfo.lastStory, storyInfo.skipNullFloor);
	ACAPI_WriteReport (msg, true);
	for (xx = 0 ; xx < (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
		sprintf (msg, "�� �ε���: %d, ����: %lf, �̸�: %s", storyInfo.data [0][xx].index, storyInfo.data [0][xx].level, storyInfo.data [0][xx].name);
	}
	BMKillHandle ((GSHandle *) &storyInfo.data);

	// ������ ����Ǵ� ��Ҵ� ������Ʈ ���ϰ� ������ ��ġ��
	FILE* fp = fopen ("Column.csv", "w+");

	// �����
	strcpy (buffer, "�ֿ�����,����");
	for (xx = storyInfo.firstStory; xx <= storyInfo.lastStory ; ++xx) {
		if (xx < 0) {
			sprintf (piece, ",B%d", -xx);
		} else {
			sprintf (piece, ",F%d", xx);
		}
		strcat (buffer, piece);
	}
	fprintf (fp, buffer);

	// 2��
	// ...

	fclose(fp);
	*/

	// ��� ������ ����ü�� �����ϱ�
	// ... columnInfo
	return	err;
}