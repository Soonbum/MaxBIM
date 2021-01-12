#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "Export.hpp"


const double DIST_BTW_COLUMN = 3.000;


// �迭 �ʱ�ȭ �Լ�
void initArray (double arr [], short arrSize)
{
	short	xx;

	for (xx = 0 ; xx < arrSize ; ++xx)
		arr [xx] = 0.0;
}

// ������������ ������ �� ����ϴ� ���Լ�
int compare (const void* first, const void* second)
{
    if (*(double*)first > *(double*)second)
        return 1;
    else if (*(double*)first < *(double*)second)
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
	short	xx, yy, zz;
	short	i, j;
	short	iSel, jSel;

	// ��ü ���� ��������
	API_Element			elem;
	API_ElementMemo		memo;
	API_ElemInfo3D		info3D;

	// �۾� �� ����
	API_StoryInfo	storyInfo;

	// �ֿ� ��ȣ�� �����ϱ� ���� ������
	double	coords_hor [100];
	short	nCoords_hor;
	double	coords_ver [100];
	short	nCoords_ver;

	short	iHor, iVer;

	// ������ �����ϱ� ���� ����ü
	ColumnPos		columnPos;
	

	// ================================================== 1. ��� ���� ��������

	// �� ���� ��������
	BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);

	columnPos.firstStory = storyInfo.firstStory;
	columnPos.lastStory = storyInfo.lastStory;
	columnPos.nStories = storyInfo.lastStory - storyInfo.firstStory;
	columnPos.nColumns = 0;

	// ������ ����� ��ġ(�ֿ�)�� �뷫������ ������ ����
	for (xx = 0 ; xx < (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {

		nCoords_hor = 0;
		nCoords_ver = 0;

		ACAPI_Element_GetElemList (API_ColumnID, &elemList, APIFilt_OnVisLayer);	// ���̴� ���̾ ����
		for (yy = 0 ; yy < (short)elemList.GetSize () ; ++yy) {
			BNZeroMemory (&elem, sizeof (API_Element));
			BNZeroMemory (&memo, sizeof (API_ElementMemo));
			elem.header.guid = elemList.Pop ();
			err = ACAPI_Element_Get (&elem);
			err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

			if (storyInfo.data [0][xx].index == elem.header.floorInd) {
				coords_hor [nCoords_hor++] = elem.column.origoPos.x;
				coords_ver [nCoords_ver++] = elem.column.origoPos.y;
			}

			ACAPI_DisposeElemMemoHdls (&memo);
		}

		// �������� ����
		qsort (&coords_hor, nCoords_hor, sizeof (double), compare);
		qsort (&coords_ver, nCoords_ver, sizeof (double), compare);

		iHor = 0;
		iVer = 0;

		// ���� ���� �ֿ� ���� ���� (������ 3000 �ʰ��ϸ� ����)
		if (nCoords_hor >= 2) {
			columnPos.node_hor [xx][iHor++]	= coords_hor [0];

			for (zz = 1 ; zz < nCoords_hor ; ++zz) {
				if ( abs (coords_hor [zz] - columnPos.node_hor [xx][iHor - 1]) > DIST_BTW_COLUMN) {
					columnPos.node_hor [xx][iHor++]	= coords_hor [zz];
				}
			}
			columnPos.nNodes_hor [xx] = iHor;
		}

		// ���� ���� �ֿ� ���� ���� (������ 3000 �ʰ��ϸ� ����)
		if (nCoords_ver >= 2) {
			columnPos.node_ver [xx][iVer++]	= coords_ver [0];

			for (zz = 1 ; zz < nCoords_ver ; ++zz) {
				if ( abs (coords_ver [zz] - columnPos.node_ver [xx][iVer - 1]) > DIST_BTW_COLUMN) {
					columnPos.node_ver [xx][iVer++]	= coords_ver [zz];
				}
			}
			columnPos.nNodes_ver [xx] = iVer;
		}

		// ��� ������ ������ !!! -- iSel, jSel ���� �޶�� �ϴµ�?
		ACAPI_Element_GetElemList (API_ColumnID, &elemList, APIFilt_OnVisLayer);	// ���̴� ���̾ ����
		for (yy = 0 ; yy < (short)elemList.GetSize () ; ++yy) {
			BNZeroMemory (&elem, sizeof (API_Element));
			BNZeroMemory (&memo, sizeof (API_ElementMemo));
			elem.header.guid = elemList.Pop ();
			err = ACAPI_Element_Get (&elem);
			err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

			if (storyInfo.data [0][xx].index == elem.header.floorInd) {
				iSel = 0;
				jSel = 0;

				for (i = 0 ; i < iHor ; ++i) {
					if (abs (elem.column.origoPos.x - columnPos.node_hor [xx][i]) <= DIST_BTW_COLUMN) {
						iSel = i;
						break;
					}
				}

				for (j = 0; j < iVer ; ++j) {
					if (abs (elem.column.origoPos.y - columnPos.node_ver [xx][j]) <= DIST_BTW_COLUMN) {
						jSel = i;
						break;
					}
				}

				columnPos.columns [columnPos.nColumns].floorInd		= elem.header.floorInd;
				columnPos.columns [columnPos.nColumns].posX			= elem.column.origoPos.x;
				columnPos.columns [columnPos.nColumns].posY			= elem.column.origoPos.y;
				columnPos.columns [columnPos.nColumns].horLen		= elem.column.coreWidth + elem.column.venThick*2;
				columnPos.columns [columnPos.nColumns].verLen		= elem.column.coreDepth + elem.column.venThick*2;
				columnPos.columns [columnPos.nColumns].height		= elem.column.height;
				columnPos.columns [columnPos.nColumns].iHor			= iSel;
				columnPos.columns [columnPos.nColumns].iVer			= jSel;

				columnPos.nColumns ++;
			}

			ACAPI_DisposeElemMemoHdls (&memo);
		}
	}

	sprintf (msg, "��� ����: %d", columnPos.nColumns);
	ACAPI_WriteReport (msg, true);

	for (xx = 0 ; xx < 20 ; ++xx) {
		sprintf (msg, "[%d]: %d��, ��ȣ(%d, %d), ��ġ(%.3f, %.3f), ũ��(%.3f, %.3f, %.3f)",
			xx,
			columnPos.columns [xx].floorInd,
			columnPos.columns [xx].iHor,
			columnPos.columns [xx].iVer,
			columnPos.columns [xx].posX,
			columnPos.columns [xx].posY,
			columnPos.columns [xx].horLen,
			columnPos.columns [xx].verLen,
			columnPos.columns [xx].height);
		ACAPI_WriteReport (msg, true);
	}

	// �� ���� ���
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
	// ... columnPos
	return	err;
}