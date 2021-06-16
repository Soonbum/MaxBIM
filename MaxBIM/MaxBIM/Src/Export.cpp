#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "Export.hpp"

using namespace exportDG;

double DIST_BTW_COLUMN = 2.000;		// ��� �� �ּ� ���� (�⺻��), ���� ���̾�α׿��� ������ �� ����
VisibleObjectInfo	visibleObjInfo;	// ���̴� ���̾� ���� ��ü�� ��Ī, ���� ����, ���̱� ����

// �迭 �ʱ�ȭ �Լ�
void initArray (double arr [], short arrSize)
{
	short	xx;

	for (xx = 0 ; xx < arrSize ; ++xx)
		arr [xx] = 0.0;
}

// ������������ ������ �� ����ϴ� ���Լ� (����Ʈ)
int compare (const void* first, const void* second)
{
    if (*(double*)first > *(double*)second)
        return 1;
    else if (*(double*)first < *(double*)second)
        return -1;
    else
        return 0;
}

// �����ֿ�, �����ֿ�, �� ������ �̿��Ͽ� ��� ã��
ColumnInfo	findColumn (ColumnPos* columnPos, short iHor, short iVer, short floorInd)
{
	ColumnInfo	resultColumn;
	short	xx;

	resultColumn.floorInd = 0;
	resultColumn.iHor = 0;
	resultColumn.iVer = 0;
	resultColumn.posX = 0.0;
	resultColumn.posY = 0.0;
	resultColumn.horLen = 0.0;
	resultColumn.verLen = 0.0;
	resultColumn.height = 0.0;

	for (xx = 0 ; xx < columnPos->nColumns ; ++xx) {
		if ( (columnPos->columns [xx].iHor == iHor) && (columnPos->columns [xx].iVer == iVer) && (columnPos->columns [xx].floorInd == floorInd) ) {
			resultColumn.floorInd = columnPos->columns [xx].floorInd;
			resultColumn.iHor = columnPos->columns [xx].iHor;
			resultColumn.iVer = columnPos->columns [xx].iVer;
			resultColumn.posX = columnPos->columns [xx].posX;
			resultColumn.posY = columnPos->columns [xx].posY;
			resultColumn.horLen = columnPos->columns [xx].horLen;
			resultColumn.verLen = columnPos->columns [xx].verLen;
			resultColumn.height = columnPos->columns [xx].height;
		}
	}

	return	resultColumn;
}

// ����(���,��,������)���� ������ �����ϰ� �����ؼ� ���� ���Ϸ� ��������
GSErrCode	exportGridElementInfo (void)
{
	GSErrCode	err = NoError;

	GS::Array<API_Guid> elemList;
	long	nElems;
	char	msg [512];
	char	buffer [256];
	char	piece [20];
	short	xx, yy, zz;
	short	i, j;
	short	result;

	// ��ü ���� ��������
	API_Element			elem;
	API_ElementMemo		memo;
	//API_ElemInfo3D		info3D;

	// �۾� �� ����
	API_StoryInfo	storyInfo;

	// �ֿ� ��ȣ�� �����ϱ� ���� ������ (���ο� ���� ���� ���ؾ� �ϹǷ� ��������)
	double	coords_hor [100];
	short	nCoords_hor;
	double	coords_ver [100];
	short	nCoords_ver;

	short	iHor, iVer;
	short	iSel, jSel;
	short	maxHor, maxVer;

	// ���� ������ ���� ����
	API_SpecFolderID	specFolderID = API_ApplicationFolderID;
	IO::Location		location;
	GS::UniString		resultString;
	API_MiscAppInfo		miscAppInfo;

	FILE	*fp;
	char	file_column [256];
	bool	bFileCreationSuccess_column;

	// ������ �����ϱ� ���� ����ü
	ColumnPos		columnPos;
	ColumnInfo		resultColumn;
	

	// ================================================== ���
	// [���̾�α�] ��� �� �ּ� ���� �Ÿ��� ����ڿ��� �Է� ���� (�⺻��: 2000 mm)
	result = DGBlankModalDialog (250, 100, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, inputThresholdHandler, 0);

	if (result != DG_OK) {
		ACAPI_WriteReport ("�������⸦ �ߴ��մϴ�.", true);
		return	err;
	}

	// �� ���� ��������
	BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);

	columnPos.firstStory = storyInfo.firstStory;	// ������ �� �ε��� (��: ���� 4���� ���, -4)
	columnPos.lastStory = storyInfo.lastStory;		// �ֻ��� �� �ε��� (��: ���� 35���� ���, 34)
	columnPos.nStories = storyInfo.lastStory - storyInfo.firstStory + (storyInfo.skipNullFloor) * 1;	// 0���� �����ߴٸ� +1
	columnPos.nColumns = 0;

	// ������ ����� ��ġ(�ֿ�)�� �뷫������ ������ ����
	for (xx = 0 ; xx < columnPos.nStories ; ++xx) {

		nCoords_hor = 0;
		nCoords_ver = 0;

		// ��ǥ���� �ϴ� �����ϵ�, X�� Y�� �������� ������ ����
		ACAPI_Element_GetElemList (API_ColumnID, &elemList, APIFilt_OnVisLayer);	// ���̴� ���̾ ����
		nElems = elemList.GetSize ();
		for (yy = 0 ; yy < nElems ; ++yy) {
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

		// ���� ���� �ֿ� ���� ���� (������ DIST_BTW_COLUMN �̻��̸� ����)
		if (nCoords_hor >= 2) {
			columnPos.node_hor [xx][iHor]	= coords_hor [0];
			iHor ++;

			for (zz = 1 ; zz < nCoords_hor ; ++zz) {
				if ( abs (coords_hor [zz] - columnPos.node_hor [xx][iHor - 1]) >= DIST_BTW_COLUMN) {
					columnPos.node_hor [xx][iHor]	= coords_hor [zz];
					iHor ++;
				}
			}
			columnPos.nNodes_hor [xx] = iHor;
		}

		// ���� ���� �ֿ� ���� ���� (������ DIST_BTW_COLUMN �̻��̸� ����)
		if (nCoords_ver >= 2) {
			columnPos.node_ver [xx][iVer]	= coords_ver [0];
			iVer ++;

			for (zz = 1 ; zz < nCoords_ver ; ++zz) {
				if ( abs (coords_ver [zz] - columnPos.node_ver [xx][iVer - 1]) >= DIST_BTW_COLUMN) {
					columnPos.node_ver [xx][iVer]	= coords_ver [zz];
					iVer ++;
				}
			}
			columnPos.nNodes_ver [xx] = iVer;
		}

		// ��� ������ ������
		ACAPI_Element_GetElemList (API_ColumnID, &elemList, APIFilt_OnVisLayer);	// ���̴� ���̾ ����
		nElems = elemList.GetSize ();
		for (yy = 0 ; yy < nElems ; ++yy) {
			BNZeroMemory (&elem, sizeof (API_Element));
			BNZeroMemory (&memo, sizeof (API_ElementMemo));
			elem.header.guid = elemList.Pop ();
			err = ACAPI_Element_Get (&elem);
			err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

			if (storyInfo.data [0][xx].index == elem.header.floorInd) {
				iSel = 0;
				jSel = 0;

				for (i = 0 ; i < columnPos.nNodes_hor [xx] ; ++i) {
					if (abs (elem.column.origoPos.x - columnPos.node_hor [xx][i]) < DIST_BTW_COLUMN) {
						iSel = i;
					}
				}

				for (j = 0; j < columnPos.nNodes_ver [xx] ; ++j) {
					if (abs (elem.column.origoPos.y - columnPos.node_ver [xx][j]) < DIST_BTW_COLUMN) {
						jSel = j;
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

	// �� ���� ���
	BMKillHandle ((GSHandle *) &storyInfo.data);

	// ����, ���� �ֿ� �ִ� ���� ã��
	maxHor = 0;
	maxVer = 0;
	for (xx = 0 ; xx < columnPos.nStories ; ++xx) {
		if (columnPos.nNodes_hor [xx] > maxHor)
			maxHor = columnPos.nNodes_hor [xx];
		if (columnPos.nNodes_ver [xx] > maxVer) {
			maxVer = columnPos.nNodes_ver [xx];
		}
	}

	// ���� ���Ϸ� ��� ���� ��������
	ACAPI_Environment (APIEnv_GetMiscAppInfoID, &miscAppInfo);
	sprintf (file_column, "%s - Column.csv", miscAppInfo.caption);
	fp = fopen (file_column, "w+");

	if (fp != NULL) {
		// �����
		strcpy (buffer, "�ֿ�����,����");
		for (xx = storyInfo.firstStory ; xx <= storyInfo.lastStory ; ++xx) {
			if (xx < 0) {
				sprintf (piece, ",B%d", -xx);	// ������
			} else {
				sprintf (piece, ",F%d", xx+1);	// ������ (0�� ����)
			}
			strcat (buffer, piece);
		}
		strcat (buffer, "\n");
		fprintf (fp, buffer);

		for (xx = 0 ; xx < maxHor ; ++xx) {
			for (yy = 0 ; yy < maxVer ; ++yy) {
				// ��� �� ����
				sprintf (buffer, "X%d - Y%d,����,", xx+1, yy+1);
				for (zz = columnPos.firstStory ; zz <= columnPos.lastStory ; ++zz) {
					resultColumn = findColumn (&columnPos, xx, yy, zz);
					sprintf (piece, "%.0f,", resultColumn.horLen * 1000);
					strcat (buffer, piece);
				}
				strcat (buffer, "\n");
				fprintf (fp, buffer);

				// ����
				strcpy (buffer, ",����,");
				for (zz = columnPos.firstStory ; zz <= columnPos.lastStory ; ++zz) {
					resultColumn = findColumn (&columnPos, xx, yy, zz);
					sprintf (piece, "%.0f,", resultColumn.verLen * 1000);
					strcat (buffer, piece);
				}
				strcat (buffer, "\n");
				fprintf (fp, buffer);

				// ����
				strcpy (buffer, ",����,");
				for (zz = columnPos.firstStory ; zz <= columnPos.lastStory ; ++zz) {
					resultColumn = findColumn (&columnPos, xx, yy, zz);
					sprintf (piece, "%.0f,", resultColumn.height * 1000);
					strcat (buffer, piece);
				}
				strcat (buffer, "\n");
				fprintf (fp, buffer);
			}
		}

		fclose (fp);

		bFileCreationSuccess_column = true;
	} else {
		bFileCreationSuccess_column = false;
	}

	if (bFileCreationSuccess_column == true) {
		ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
		location.ToDisplayText (&resultString);
		sprintf (msg, "%s�� ���� ��ġ�� �����߽��ϴ�.\n\n%s\n�Ǵ� ������Ʈ ������ �ִ� ����", file_column, resultString.ToCStr ().Get ());
		ACAPI_WriteReport (msg, true);
	} else {
		sprintf (msg, "%s�� ������ �� �����ϴ�.", file_column);
		ACAPI_WriteReport (msg, true);
	}

	// ================================================== ��
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

	return	err;
}

// ������ ������� ��� ����: ������
SummaryOfObjectInfo::SummaryOfObjectInfo ()
{
	FILE	*fp;			// ���� ������
	char	line [10240];	// ���Ͽ��� �о�� ���� �ϳ�
	char	*token;			// �о�� ���ڿ��� ��ū
	short	lineCount;		// �о�� ���� ��
	short	tokCount;		// �о�� ��ū ����
	short	xx;

	char	nthToken [200][50];	// n��° ��ū

	// ��ü ���� ���� ��������
	fp = fopen ("C:\\objectInfo.csv", "r");

	if (fp == NULL) {
		ACAPI_WriteReport ("objectInfo.csv ������ C:\\�� �����Ͻʽÿ�.", true);
	} else {
		lineCount = 0;

		while (!feof (fp)) {
			tokCount = 0;
			fgets (line, sizeof(line), fp);

			token = strtok (line, ",");
			tokCount ++;
			lineCount ++;

			// �� ���ξ� ó��
			while (token != NULL) {
				if (strlen (token) > 0) {
					strncpy (nthToken [tokCount-1], token, strlen (token)+1);
				}
				token = strtok (NULL, ",");
				tokCount ++;
			}

			// ǥ���� ���� �ʵ� ����
			this->nInfo.push_back ((short)(((tokCount-1) - 2) / 3));

			// ��ū ������ 2�� �̻��� �� ��ȿ��
			if ((tokCount-1) >= 2) {
				// ��ū ������ 2 + 3�� ��� ������ ����� (�ʰ��� �׸��� ����)
				if (((tokCount-1) - 2) % 3 != 0) {
					tokCount --;
				}

				this->nameKey.push_back (nthToken [0]);
				this->nameVal.push_back (nthToken [1]);

				if ((tokCount-1) >= 3)		this->var1name.push_back (nthToken [2]);				else this->var1name.push_back ("");
				if ((tokCount-1) >= 4)		this->var1desc.push_back (nthToken [3]);				else this->var1desc.push_back ("");
				if ((tokCount-1) >= 5)		this->var1showFlag.push_back ((short)atoi (nthToken [4]));		else this->var1showFlag.push_back (0);

				if ((tokCount-1) >= 6)		this->var2name.push_back (nthToken [5]);				else this->var2name.push_back ("");
				if ((tokCount-1) >= 7)		this->var2desc.push_back (nthToken [6]);				else this->var2desc.push_back ("");
				if ((tokCount-1) >= 8)		this->var2showFlag.push_back ((short)atoi (nthToken [7]));		else this->var2showFlag.push_back (0);
				
				if ((tokCount-1) >= 9)		this->var3name.push_back (nthToken [8]);				else this->var3name.push_back ("");
				if ((tokCount-1) >= 10)		this->var3desc.push_back (nthToken [9]);				else this->var3desc.push_back ("");
				if ((tokCount-1) >= 11)		this->var3showFlag.push_back ((short)atoi (nthToken [10]));		else this->var3showFlag.push_back (0);

				if ((tokCount-1) >= 12)		this->var4name.push_back (nthToken [11]);				else this->var4name.push_back ("");
				if ((tokCount-1) >= 13)		this->var4desc.push_back (nthToken [12]);				else this->var4desc.push_back ("");
				if ((tokCount-1) >= 14)		this->var4showFlag.push_back ((short)atoi (nthToken [13]));		else this->var4showFlag.push_back (0);

				if ((tokCount-1) >= 15)		this->var5name.push_back (nthToken [14]);				else this->var5name.push_back ("");
				if ((tokCount-1) >= 16)		this->var5desc.push_back (nthToken [15]);				else this->var5desc.push_back ("");
				if ((tokCount-1) >= 17)		this->var5showFlag.push_back ((short)atoi (nthToken [16]));		else this->var5showFlag.push_back (0);

				if ((tokCount-1) >= 18)		this->var6name.push_back (nthToken [17]);				else this->var6name.push_back ("");
				if ((tokCount-1) >= 19)		this->var6desc.push_back (nthToken [18]);				else this->var6desc.push_back ("");
				if ((tokCount-1) >= 20)		this->var6showFlag.push_back ((short)atoi (nthToken [19]));		else this->var6showFlag.push_back (0);

				if ((tokCount-1) >= 21)		this->var7name.push_back (nthToken [20]);				else this->var7name.push_back ("");
				if ((tokCount-1) >= 22)		this->var7desc.push_back (nthToken [21]);				else this->var7desc.push_back ("");
				if ((tokCount-1) >= 23)		this->var7showFlag.push_back ((short)atoi (nthToken [22]));		else this->var7showFlag.push_back (0);

				if ((tokCount-1) >= 24)		this->var8name.push_back (nthToken [23]);				else this->var8name.push_back ("");
				if ((tokCount-1) >= 25)		this->var8desc.push_back (nthToken [24]);				else this->var8desc.push_back ("");
				if ((tokCount-1) >= 26)		this->var8showFlag.push_back ((short)atoi (nthToken [25]));		else this->var8showFlag.push_back (0);

				if ((tokCount-1) >= 27)		this->var9name.push_back (nthToken [26]);				else this->var9name.push_back ("");
				if ((tokCount-1) >= 28)		this->var9desc.push_back (nthToken [27]);				else this->var9desc.push_back ("");
				if ((tokCount-1) >= 29)		this->var9showFlag.push_back ((short)atoi (nthToken [28]));		else this->var9showFlag.push_back (0);

				if ((tokCount-1) >= 30)		this->var10name.push_back (nthToken [29]);				else this->var10name.push_back ("");
				if ((tokCount-1) >= 31)		this->var10desc.push_back (nthToken [30]);				else this->var10desc.push_back ("");
				if ((tokCount-1) >= 32)		this->var10showFlag.push_back ((short)atoi (nthToken [31]));	else this->var10showFlag.push_back (0);

				if ((tokCount-1) >= 33)		this->var11name.push_back (nthToken [32]);				else this->var11name.push_back ("");
				if ((tokCount-1) >= 34)		this->var11desc.push_back (nthToken [33]);				else this->var11desc.push_back ("");
				if ((tokCount-1) >= 35)		this->var11showFlag.push_back ((short)atoi (nthToken [34]));	else this->var11showFlag.push_back (0);

				if ((tokCount-1) >= 36)		this->var12name.push_back (nthToken [35]);				else this->var12name.push_back ("");
				if ((tokCount-1) >= 37)		this->var12desc.push_back (nthToken [36]);				else this->var12desc.push_back ("");
				if ((tokCount-1) >= 38)		this->var12showFlag.push_back ((short)atoi (nthToken [37]));	else this->var12showFlag.push_back (0);

				if ((tokCount-1) >= 39)		this->var13name.push_back (nthToken [38]);				else this->var13name.push_back ("");
				if ((tokCount-1) >= 40)		this->var13desc.push_back (nthToken [39]);				else this->var13desc.push_back ("");
				if ((tokCount-1) >= 41)		this->var13showFlag.push_back ((short)atoi (nthToken [40]));	else this->var13showFlag.push_back (0);

				if ((tokCount-1) >= 42)		this->var14name.push_back (nthToken [41]);				else this->var14name.push_back ("");
				if ((tokCount-1) >= 43)		this->var14desc.push_back (nthToken [42]);				else this->var14desc.push_back ("");
				if ((tokCount-1) >= 44)		this->var14showFlag.push_back ((short)atoi (nthToken [43]));	else this->var14showFlag.push_back (0);

				if ((tokCount-1) >= 45)		this->var15name.push_back (nthToken [44]);				else this->var15name.push_back ("");
				if ((tokCount-1) >= 46)		this->var15desc.push_back (nthToken [45]);				else this->var15desc.push_back ("");
				if ((tokCount-1) >= 47)		this->var15showFlag.push_back ((short)atoi (nthToken [46]));	else this->var15showFlag.push_back (0);

				if ((tokCount-1) >= 48)		this->var16name.push_back (nthToken [47]);				else this->var16name.push_back ("");
				if ((tokCount-1) >= 49)		this->var16desc.push_back (nthToken [48]);				else this->var16desc.push_back ("");
				if ((tokCount-1) >= 50)		this->var16showFlag.push_back ((short)atoi (nthToken [49]));	else this->var16showFlag.push_back (0);

				if ((tokCount-1) >= 51)		this->var17name.push_back (nthToken [50]);				else this->var17name.push_back ("");
				if ((tokCount-1) >= 52)		this->var17desc.push_back (nthToken [51]);				else this->var17desc.push_back ("");
				if ((tokCount-1) >= 53)		this->var17showFlag.push_back ((short)atoi (nthToken [52]));	else this->var17showFlag.push_back (0);

				if ((tokCount-1) >= 54)		this->var18name.push_back (nthToken [53]);				else this->var18name.push_back ("");
				if ((tokCount-1) >= 55)		this->var18desc.push_back (nthToken [54]);				else this->var18desc.push_back ("");
				if ((tokCount-1) >= 56)		this->var18showFlag.push_back ((short)atoi (nthToken [55]));	else this->var18showFlag.push_back (0);

				if ((tokCount-1) >= 57)		this->var19name.push_back (nthToken [56]);				else this->var19name.push_back ("");
				if ((tokCount-1) >= 58)		this->var19desc.push_back (nthToken [57]);				else this->var19desc.push_back ("");
				if ((tokCount-1) >= 59)		this->var19showFlag.push_back ((short)atoi (nthToken [58]));	else this->var19showFlag.push_back (0);

				if ((tokCount-1) >= 60)		this->var20name.push_back (nthToken [59]);				else this->var20name.push_back ("");
				if ((tokCount-1) >= 61)		this->var20desc.push_back (nthToken [60]);				else this->var20desc.push_back ("");
				if ((tokCount-1) >= 62)		this->var20showFlag.push_back ((short)atoi (nthToken [61]));	else this->var20showFlag.push_back (0);

				if ((tokCount-1) >= 63)		this->var21name.push_back (nthToken [62]);				else this->var21name.push_back ("");
				if ((tokCount-1) >= 64)		this->var21desc.push_back (nthToken [63]);				else this->var21desc.push_back ("");
				if ((tokCount-1) >= 65)		this->var21showFlag.push_back ((short)atoi (nthToken [64]));	else this->var21showFlag.push_back (0);

				if ((tokCount-1) >= 66)		this->var22name.push_back (nthToken [65]);				else this->var22name.push_back ("");
				if ((tokCount-1) >= 67)		this->var22desc.push_back (nthToken [66]);				else this->var22desc.push_back ("");
				if ((tokCount-1) >= 68)		this->var22showFlag.push_back ((short)atoi (nthToken [67]));	else this->var22showFlag.push_back (0);

				if ((tokCount-1) >= 69)		this->var23name.push_back (nthToken [68]);				else this->var23name.push_back ("");
				if ((tokCount-1) >= 70)		this->var23desc.push_back (nthToken [69]);				else this->var23desc.push_back ("");
				if ((tokCount-1) >= 71)		this->var23showFlag.push_back ((short)atoi (nthToken [70]));	else this->var23showFlag.push_back (0);

				if ((tokCount-1) >= 72)		this->var24name.push_back (nthToken [71]);				else this->var24name.push_back ("");
				if ((tokCount-1) >= 73)		this->var24desc.push_back (nthToken [72]);				else this->var24desc.push_back ("");
				if ((tokCount-1) >= 74)		this->var24showFlag.push_back ((short)atoi (nthToken [73]));	else this->var24showFlag.push_back (0);

				if ((tokCount-1) >= 75)		this->var25name.push_back (nthToken [74]);				else this->var25name.push_back ("");
				if ((tokCount-1) >= 76)		this->var25desc.push_back (nthToken [75]);				else this->var25desc.push_back ("");
				if ((tokCount-1) >= 77)		this->var25showFlag.push_back ((short)atoi (nthToken [76]));	else this->var25showFlag.push_back (0);

				if ((tokCount-1) >= 78)		this->var26name.push_back (nthToken [77]);				else this->var26name.push_back ("");
				if ((tokCount-1) >= 79)		this->var26desc.push_back (nthToken [78]);				else this->var26desc.push_back ("");
				if ((tokCount-1) >= 80)		this->var26showFlag.push_back ((short)atoi (nthToken [79]));	else this->var26showFlag.push_back (0);

				if ((tokCount-1) >= 81)		this->var27name.push_back (nthToken [80]);				else this->var27name.push_back ("");
				if ((tokCount-1) >= 82)		this->var27desc.push_back (nthToken [81]);				else this->var27desc.push_back ("");
				if ((tokCount-1) >= 83)		this->var27showFlag.push_back ((short)atoi (nthToken [82]));	else this->var27showFlag.push_back (0);

				if ((tokCount-1) >= 84)		this->var28name.push_back (nthToken [83]);				else this->var28name.push_back ("");
				if ((tokCount-1) >= 85)		this->var28desc.push_back (nthToken [84]);				else this->var28desc.push_back ("");
				if ((tokCount-1) >= 86)		this->var28showFlag.push_back ((short)atoi (nthToken [85]));	else this->var28showFlag.push_back (0);

				if ((tokCount-1) >= 87)		this->var29name.push_back (nthToken [86]);				else this->var29name.push_back ("");
				if ((tokCount-1) >= 88)		this->var29desc.push_back (nthToken [87]);				else this->var29desc.push_back ("");
				if ((tokCount-1) >= 89)		this->var29showFlag.push_back ((short)atoi (nthToken [88]));	else this->var29showFlag.push_back (0);

				if ((tokCount-1) >= 90)		this->var30name.push_back (nthToken [89]);				else this->var30name.push_back ("");
				if ((tokCount-1) >= 91)		this->var30desc.push_back (nthToken [90]);				else this->var30desc.push_back ("");
				if ((tokCount-1) >= 92)		this->var30showFlag.push_back ((short)atoi (nthToken [91]));	else this->var30showFlag.push_back (0);

				if ((tokCount-1) >= 93)		this->var31name.push_back (nthToken [92]);				else this->var31name.push_back ("");
				if ((tokCount-1) >= 94)		this->var31desc.push_back (nthToken [93]);				else this->var31desc.push_back ("");
				if ((tokCount-1) >= 95)		this->var31showFlag.push_back ((short)atoi (nthToken [94]));	else this->var31showFlag.push_back (0);

				if ((tokCount-1) >= 96)		this->var32name.push_back (nthToken [95]);				else this->var32name.push_back ("");
				if ((tokCount-1) >= 97)		this->var32desc.push_back (nthToken [96]);				else this->var32desc.push_back ("");
				if ((tokCount-1) >= 98)		this->var32showFlag.push_back ((short)atoi (nthToken [97]));	else this->var32showFlag.push_back (0);

				if ((tokCount-1) >= 99)		this->var33name.push_back (nthToken [98]);				else this->var33name.push_back ("");
				if ((tokCount-1) >= 100)	this->var33desc.push_back (nthToken [99]);				else this->var33desc.push_back ("");
				if ((tokCount-1) >= 101)	this->var33showFlag.push_back ((short)atoi (nthToken [100]));	else this->var33showFlag.push_back (0);

				if ((tokCount-1) >= 102)	this->var34name.push_back (nthToken [101]);				else this->var34name.push_back ("");
				if ((tokCount-1) >= 103)	this->var34desc.push_back (nthToken [102]);				else this->var34desc.push_back ("");
				if ((tokCount-1) >= 104)	this->var34showFlag.push_back ((short)atoi (nthToken [103]));	else this->var34showFlag.push_back (0);

				if ((tokCount-1) >= 105)	this->var35name.push_back (nthToken [104]);				else this->var35name.push_back ("");
				if ((tokCount-1) >= 106)	this->var35desc.push_back (nthToken [105]);				else this->var35desc.push_back ("");
				if ((tokCount-1) >= 107)	this->var35showFlag.push_back ((short)atoi (nthToken [106]));	else this->var35showFlag.push_back (0);

				if ((tokCount-1) >= 108)	this->var36name.push_back (nthToken [107]);				else this->var36name.push_back ("");
				if ((tokCount-1) >= 109)	this->var36desc.push_back (nthToken [108]);				else this->var36desc.push_back ("");
				if ((tokCount-1) >= 110)	this->var36showFlag.push_back ((short)atoi (nthToken [109]));	else this->var36showFlag.push_back (0);

				if ((tokCount-1) >= 111)	this->var37name.push_back (nthToken [110]);				else this->var37name.push_back ("");
				if ((tokCount-1) >= 112)	this->var37desc.push_back (nthToken [111]);				else this->var37desc.push_back ("");
				if ((tokCount-1) >= 113)	this->var37showFlag.push_back ((short)atoi (nthToken [112]));	else this->var37showFlag.push_back (0);

				if ((tokCount-1) >= 114)	this->var38name.push_back (nthToken [113]);				else this->var38name.push_back ("");
				if ((tokCount-1) >= 115)	this->var38desc.push_back (nthToken [114]);				else this->var38desc.push_back ("");
				if ((tokCount-1) >= 116)	this->var38showFlag.push_back ((short)atoi (nthToken [115]));	else this->var38showFlag.push_back (0);

				if ((tokCount-1) >= 117)	this->var39name.push_back (nthToken [116]);				else this->var39name.push_back ("");
				if ((tokCount-1) >= 118)	this->var39desc.push_back (nthToken [117]);				else this->var39desc.push_back ("");
				if ((tokCount-1) >= 119)	this->var39showFlag.push_back ((short)atoi (nthToken [118]));	else this->var39showFlag.push_back (0);

				if ((tokCount-1) >= 120)	this->var40name.push_back (nthToken [119]);				else this->var40name.push_back ("");
				if ((tokCount-1) >= 121)	this->var40desc.push_back (nthToken [120]);				else this->var40desc.push_back ("");
				if ((tokCount-1) >= 122)	this->var40showFlag.push_back ((short)atoi (nthToken [121]));	else this->var40showFlag.push_back (0);

				if ((tokCount-1) >= 123)	this->var41name.push_back (nthToken [122]);				else this->var41name.push_back ("");
				if ((tokCount-1) >= 124)	this->var41desc.push_back (nthToken [123]);				else this->var41desc.push_back ("");
				if ((tokCount-1) >= 125)	this->var41showFlag.push_back ((short)atoi (nthToken [124]));	else this->var41showFlag.push_back (0);

				if ((tokCount-1) >= 126)	this->var42name.push_back (nthToken [125]);				else this->var42name.push_back ("");
				if ((tokCount-1) >= 127)	this->var42desc.push_back (nthToken [126]);				else this->var42desc.push_back ("");
				if ((tokCount-1) >= 128)	this->var42showFlag.push_back ((short)atoi (nthToken [127]));	else this->var42showFlag.push_back (0);

				if ((tokCount-1) >= 129)	this->var43name.push_back (nthToken [128]);				else this->var43name.push_back ("");
				if ((tokCount-1) >= 130)	this->var43desc.push_back (nthToken [129]);				else this->var43desc.push_back ("");
				if ((tokCount-1) >= 131)	this->var43showFlag.push_back ((short)atoi (nthToken [130]));	else this->var43showFlag.push_back (0);

				if ((tokCount-1) >= 132)	this->var44name.push_back (nthToken [131]);				else this->var44name.push_back ("");
				if ((tokCount-1) >= 133)	this->var44desc.push_back (nthToken [132]);				else this->var44desc.push_back ("");
				if ((tokCount-1) >= 134)	this->var44showFlag.push_back ((short)atoi (nthToken [133]));	else this->var44showFlag.push_back (0);

				if ((tokCount-1) >= 135)	this->var45name.push_back (nthToken [134]);				else this->var45name.push_back ("");
				if ((tokCount-1) >= 136)	this->var45desc.push_back (nthToken [135]);				else this->var45desc.push_back ("");
				if ((tokCount-1) >= 137)	this->var45showFlag.push_back ((short)atoi (nthToken [136]));	else this->var45showFlag.push_back (0);

				if ((tokCount-1) >= 138)	this->var46name.push_back (nthToken [137]);				else this->var46name.push_back ("");
				if ((tokCount-1) >= 139)	this->var46desc.push_back (nthToken [138]);				else this->var46desc.push_back ("");
				if ((tokCount-1) >= 140)	this->var46showFlag.push_back ((short)atoi (nthToken [139]));	else this->var46showFlag.push_back (0);

				if ((tokCount-1) >= 141)	this->var47name.push_back (nthToken [140]);				else this->var47name.push_back ("");
				if ((tokCount-1) >= 142)	this->var47desc.push_back (nthToken [141]);				else this->var47desc.push_back ("");
				if ((tokCount-1) >= 143)	this->var47showFlag.push_back ((short)atoi (nthToken [142]));	else this->var47showFlag.push_back (0);

				if ((tokCount-1) >= 144)	this->var48name.push_back (nthToken [143]);				else this->var48name.push_back ("");
				if ((tokCount-1) >= 145)	this->var48desc.push_back (nthToken [144]);				else this->var48desc.push_back ("");
				if ((tokCount-1) >= 146)	this->var48showFlag.push_back ((short)atoi (nthToken [145]));	else this->var48showFlag.push_back (0);

				if ((tokCount-1) >= 147)	this->var49name.push_back (nthToken [146]);				else this->var49name.push_back ("");
				if ((tokCount-1) >= 148)	this->var49desc.push_back (nthToken [147]);				else this->var49desc.push_back ("");
				if ((tokCount-1) >= 149)	this->var49showFlag.push_back ((short)atoi (nthToken [148]));	else this->var49showFlag.push_back (0);

				if ((tokCount-1) >= 150)	this->var50name.push_back (nthToken [149]);				else this->var50name.push_back ("");
				if ((tokCount-1) >= 151)	this->var50desc.push_back (nthToken [150]);				else this->var50desc.push_back ("");
				if ((tokCount-1) >= 152)	this->var50showFlag.push_back ((short)atoi (nthToken [151]));	else this->var50showFlag.push_back (0);

			}
		}

		// ���� �ݱ�
		fclose (fp);

		// ��ü ���� ����
		this->nKnownObjects = lineCount - 1;

		// �ٸ� ��� ���� �ʱ�ȭ
		vector<string>			vec_empty_string = vector<string> (200, "");
		vector<short>			vec_empty_short = vector<short> (200, 0);
		vector<API_AddParID>	vec_empty_type = vector<API_AddParID> (200, API_ZombieParT);

		this->var1type = vec_empty_type;
		this->var2type = vec_empty_type;
		this->var3type = vec_empty_type;
		this->var4type = vec_empty_type;
		this->var5type = vec_empty_type;
		this->var6type = vec_empty_type;
		this->var7type = vec_empty_type;
		this->var8type = vec_empty_type;
		this->var9type = vec_empty_type;
		this->var10type = vec_empty_type;
		this->var11type = vec_empty_type;
		this->var12type = vec_empty_type;
		this->var13type = vec_empty_type;
		this->var14type = vec_empty_type;
		this->var15type = vec_empty_type;
		this->var16type = vec_empty_type;
		this->var17type = vec_empty_type;
		this->var18type = vec_empty_type;
		this->var19type = vec_empty_type;
		this->var20type = vec_empty_type;
		this->var21type = vec_empty_type;
		this->var22type = vec_empty_type;
		this->var23type = vec_empty_type;
		this->var24type = vec_empty_type;
		this->var25type = vec_empty_type;
		this->var26type = vec_empty_type;
		this->var27type = vec_empty_type;
		this->var28type = vec_empty_type;
		this->var29type = vec_empty_type;
		this->var30type = vec_empty_type;
		this->var31type = vec_empty_type;
		this->var32type = vec_empty_type;
		this->var33type = vec_empty_type;
		this->var34type = vec_empty_type;
		this->var35type = vec_empty_type;
		this->var36type = vec_empty_type;
		this->var37type = vec_empty_type;
		this->var38type = vec_empty_type;
		this->var39type = vec_empty_type;
		this->var40type = vec_empty_type;
		this->var41type = vec_empty_type;
		this->var42type = vec_empty_type;
		this->var43type = vec_empty_type;
		this->var44type = vec_empty_type;
		this->var45type = vec_empty_type;
		this->var46type = vec_empty_type;
		this->var47type = vec_empty_type;
		this->var48type = vec_empty_type;
		this->var49type = vec_empty_type;
		this->var50type = vec_empty_type;

		this->nCounts = vec_empty_short;
		this->nCountsBeam = 0;
		this->nUnknownObjects = 0;

		for (xx = 0 ; xx < lineCount-1 ; ++xx) {
			this->var1value.push_back (vec_empty_string);
			this->var2value.push_back (vec_empty_string);
			this->var3value.push_back (vec_empty_string);
			this->var4value.push_back (vec_empty_string);
			this->var5value.push_back (vec_empty_string);
			this->var6value.push_back (vec_empty_string);
			this->var7value.push_back (vec_empty_string);
			this->var8value.push_back (vec_empty_string);
			this->var9value.push_back (vec_empty_string);
			this->var10value.push_back (vec_empty_string);
			this->var11value.push_back (vec_empty_string);
			this->var12value.push_back (vec_empty_string);
			this->var13value.push_back (vec_empty_string);
			this->var14value.push_back (vec_empty_string);
			this->var15value.push_back (vec_empty_string);
			this->var16value.push_back (vec_empty_string);
			this->var17value.push_back (vec_empty_string);
			this->var18value.push_back (vec_empty_string);
			this->var19value.push_back (vec_empty_string);
			this->var20value.push_back (vec_empty_string);
			this->var21value.push_back (vec_empty_string);
			this->var22value.push_back (vec_empty_string);
			this->var23value.push_back (vec_empty_string);
			this->var24value.push_back (vec_empty_string);
			this->var25value.push_back (vec_empty_string);
			this->var26value.push_back (vec_empty_string);
			this->var27value.push_back (vec_empty_string);
			this->var28value.push_back (vec_empty_string);
			this->var29value.push_back (vec_empty_string);
			this->var30value.push_back (vec_empty_string);
			this->var31value.push_back (vec_empty_string);
			this->var32value.push_back (vec_empty_string);
			this->var33value.push_back (vec_empty_string);
			this->var34value.push_back (vec_empty_string);
			this->var35value.push_back (vec_empty_string);
			this->var36value.push_back (vec_empty_string);
			this->var37value.push_back (vec_empty_string);
			this->var38value.push_back (vec_empty_string);
			this->var39value.push_back (vec_empty_string);
			this->var40value.push_back (vec_empty_string);
			this->var41value.push_back (vec_empty_string);
			this->var42value.push_back (vec_empty_string);
			this->var43value.push_back (vec_empty_string);
			this->var44value.push_back (vec_empty_string);
			this->var45value.push_back (vec_empty_string);
			this->var46value.push_back (vec_empty_string);
			this->var47value.push_back (vec_empty_string);
			this->var48value.push_back (vec_empty_string);
			this->var49value.push_back (vec_empty_string);
			this->var50value.push_back (vec_empty_string);
			this->combinationCount.push_back (vec_empty_short);
		}
	}
}

// ������ ���� ���� �������� (Single ���)
GSErrCode	exportSelectedElementInfo (void)
{
	GSErrCode	err = NoError;
	long		nSel;
	short		xx, yy, zz;
	bool		regenerate = true;
	
	// ������ ��Ұ� ������ ����
	API_SelectionInfo		selectionInfo;
	API_Neig				**selNeigs;
	API_Element				tElem;
	GS::Array<API_Guid>&	objects = GS::Array<API_Guid> ();
	GS::Array<API_Guid>&	beams = GS::Array<API_Guid> ();
	long					nObjects = 0;
	long					nBeams = 0;


	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err != NoError) {
		BMKillHandle ((GSHandle *) &selNeigs);
		ACAPI_WriteReport ("��ҵ��� �����ؾ� �մϴ�.", true);
		return err;
	}

	// ��ü Ÿ���� ��� GUID�� ������
	if (selectionInfo.typeID != API_SelEmpty) {
		nSel = BMGetHandleSize ((GSHandle) selNeigs) / sizeof (API_Neig);
		for (xx = 0 ; xx < nSel && err == NoError ; ++xx) {
			tElem.header.typeID = Neig_To_ElemID ((*selNeigs)[xx].neigID);

			tElem.header.guid = (*selNeigs)[xx].guid;
			if (ACAPI_Element_Get (&tElem) != NoError)	// ������ �� �ִ� ����ΰ�?
				continue;

			if (tElem.header.typeID == API_ObjectID)	// ��ü Ÿ�� ����ΰ�?
				objects.Push (tElem.header.guid);

			if (tElem.header.typeID == API_BeamID)		// �� Ÿ�� ����ΰ�?
				beams.Push (tElem.header.guid);
		}
	}
	BMKillHandle ((GSHandle *) &selNeigs);
	nObjects = objects.GetSize ();
	nBeams = beams.GetSize ();

	// ������ ��ҵ��� 3D�� ������
	err = ACAPI_Automate (APIDo_ShowSelectionIn3DID);

	// ������ ��ҵ��� ���� ����ϱ�
	API_Element			elem;
	API_ElementMemo		memo;
	bool				foundExistValue;

	SummaryOfObjectInfo		objectInfo;

	char				buffer [256];
	char				filename [256];

	// ���� ���Ϸ� ��� ���� ��������
	// ���� ������ ���� ����
	API_SpecFolderID	specFolderID = API_ApplicationFolderID;
	IO::Location		location;
	GS::UniString		resultString;
	API_MiscAppInfo		miscAppInfo;
	FILE				*fp;

	ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
	ACAPI_Environment (APIEnv_GetMiscAppInfoID, &miscAppInfo);
	sprintf (filename, "%s - ������ ���� ����.csv", miscAppInfo.caption);
	fp = fopen (filename, "w+");

	if (fp == NULL) {
		ACAPI_WriteReport ("������ �� �� �����ϴ�.", true);
		return err;
	}

	double			value_numeric [50];
	string			value_string [50];
	API_AddParID	value_type [50];
	char			tempStr [50];
	const char*		foundStr;
	bool			foundObject;

	for (xx = 0 ; xx < nObjects ; ++xx) {
		foundObject = false;

		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		elem.header.guid = objects.Pop ();
		err = ACAPI_Element_Get (&elem);
		err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

		// �Ķ���� ��ũ��Ʈ�� ������ �����Ŵ
		ACAPI_Goodies (APIAny_RunGDLParScriptID, &elem.header, 0);

		for (yy = 0 ; yy < objectInfo.nameKey.size () ; ++yy) {

			strcpy (tempStr, objectInfo.nameKey [yy].c_str ());
			foundStr = getParameterStringByName (&memo, tempStr);

			// ��ü ������ ã�Ҵٸ�,
			if (foundStr != NULL) {
				foundObject = true;

				if (my_strcmp (foundStr, objectInfo.nameVal [yy].c_str ()) == 0) {
					foundExistValue = false;

					value_string [0] = "";
					value_string [1] = "";
					value_string [2] = "";
					value_string [3] = "";
					value_string [4] = "";
					value_string [5] = "";
					value_string [6] = "";
					value_string [7] = "";
					value_string [8] = "";
					value_string [9] = "";
					value_string [10] = "";
					value_string [11] = "";
					value_string [12] = "";
					value_string [13] = "";
					value_string [14] = "";
					value_string [15] = "";
					value_string [16] = "";
					value_string [17] = "";
					value_string [18] = "";
					value_string [19] = "";
					value_string [20] = "";
					value_string [21] = "";
					value_string [22] = "";
					value_string [23] = "";
					value_string [24] = "";
					value_string [25] = "";
					value_string [26] = "";
					value_string [27] = "";
					value_string [28] = "";
					value_string [29] = "";
					value_string [30] = "";
					value_string [31] = "";
					value_string [32] = "";
					value_string [33] = "";
					value_string [34] = "";
					value_string [35] = "";
					value_string [36] = "";
					value_string [37] = "";
					value_string [38] = "";
					value_string [39] = "";
					value_string [40] = "";
					value_string [41] = "";
					value_string [42] = "";
					value_string [43] = "";
					value_string [44] = "";
					value_string [45] = "";
					value_string [46] = "";
					value_string [47] = "";
					value_string [48] = "";
					value_string [49] = "";

					// ���� 1
					strcpy (tempStr, objectInfo.var1name [yy].c_str ());
					value_type [0] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var1type [yy] = value_type [0];

					if ((value_type [0] != APIParT_Separator) || (value_type [0] != APIParT_Title) || (value_type [0] != API_ZombieParT)) {
						if (value_type [0] == APIParT_CString) {
							// ���ڿ�
							value_string [0] = getParameterStringByName (&memo, tempStr);
							value_numeric [0] = 0.0;
						} else {
							// ����
							value_numeric [0] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [0]);
							value_string [0] = tempStr;
						}
					}

					// ���� 2
					strcpy (tempStr, objectInfo.var2name [yy].c_str ());
					value_type [1] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var2type [yy] = value_type [1];

					if ((value_type [1] != APIParT_Separator) || (value_type [1] != APIParT_Title) || (value_type [1] != API_ZombieParT)) {
						if (value_type [1] == APIParT_CString) {
							// ���ڿ�
							value_string [1] = getParameterStringByName (&memo, tempStr);
							value_numeric [1] = 0.0;
						} else {
							// ����
							value_numeric [1] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [1]);
							value_string [1] = tempStr;
						}
					}

					// ���� 3
					strcpy (tempStr, objectInfo.var3name [yy].c_str ());
					value_type [2] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var3type [yy] = value_type [2];

					if ((value_type [2] != APIParT_Separator) || (value_type [2] != APIParT_Title) || (value_type [2] != API_ZombieParT)) {
						if (value_type [2] == APIParT_CString) {
							// ���ڿ�
							value_string [2] = getParameterStringByName (&memo, tempStr);
							value_numeric [2] = 0.0;
						} else {
							// ����
							value_numeric [2] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [2]);
							value_string [2] = tempStr;
						}
					}

					// ���� 4
					strcpy (tempStr, objectInfo.var4name [yy].c_str ());
					value_type [3] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var4type [yy] = value_type [3];

					if ((value_type [3] != APIParT_Separator) || (value_type [3] != APIParT_Title) || (value_type [3] != API_ZombieParT)) {
						if (value_type [3] == APIParT_CString) {
							// ���ڿ�
							value_string [3] = getParameterStringByName (&memo, tempStr);
							value_numeric [3] = 0.0;
						} else {
							// ����
							value_numeric [3] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [3]);
							value_string [3] = tempStr;
						}
					}

					// ���� 5
					strcpy (tempStr, objectInfo.var5name [yy].c_str ());
					value_type [4] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var5type [yy] = value_type [4];

					if ((value_type [4] != APIParT_Separator) || (value_type [4] != APIParT_Title) || (value_type [4] != API_ZombieParT)) {
						if (value_type [4] == APIParT_CString) {
							// ���ڿ�
							value_string [4] = getParameterStringByName (&memo, tempStr);
							value_numeric [4] = 0.0;
						} else {
							// ����
							value_numeric [4] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [4]);
							value_string [4] = tempStr;
						}
					}

					// ���� 6
					strcpy (tempStr, objectInfo.var6name [yy].c_str ());
					value_type [5] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var6type [yy] = value_type [5];

					if ((value_type [5] != APIParT_Separator) || (value_type [5] != APIParT_Title) || (value_type [5] != API_ZombieParT)) {
						if (value_type [5] == APIParT_CString) {
							// ���ڿ�
							value_string [5] = getParameterStringByName (&memo, tempStr);
							value_numeric [5] = 0.0;
						} else {
							// ����
							value_numeric [5] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [5]);
							value_string [5] = tempStr;
						}
					}

					// ���� 7
					strcpy (tempStr, objectInfo.var7name [yy].c_str ());
					value_type [6] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var7type [yy] = value_type [6];

					if ((value_type [6] != APIParT_Separator) || (value_type [6] != APIParT_Title) || (value_type [6] != API_ZombieParT)) {
						if (value_type [6] == APIParT_CString) {
							// ���ڿ�
							value_string [6] = getParameterStringByName (&memo, tempStr);
							value_numeric [6] = 0.0;
						} else {
							// ����
							value_numeric [6] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [6]);
							value_string [6] = tempStr;
						}
					}

					// ���� 8
					strcpy (tempStr, objectInfo.var8name [yy].c_str ());
					value_type [7] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var8type [yy] = value_type [7];

					if ((value_type [7] != APIParT_Separator) || (value_type [7] != APIParT_Title) || (value_type [7] != API_ZombieParT)) {
						if (value_type [7] == APIParT_CString) {
							// ���ڿ�
							value_string [7] = getParameterStringByName (&memo, tempStr);
							value_numeric [7] = 0.0;
						} else {
							// ����
							value_numeric [7] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [7]);
							value_string [7] = tempStr;
						}
					}

					// ���� 9
					strcpy (tempStr, objectInfo.var9name [yy].c_str ());
					value_type [8] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var9type [yy] = value_type [8];

					if ((value_type [8] != APIParT_Separator) || (value_type [8] != APIParT_Title) || (value_type [8] != API_ZombieParT)) {
						if (value_type [8] == APIParT_CString) {
							// ���ڿ�
							value_string [8] = getParameterStringByName (&memo, tempStr);
							value_numeric [8] = 0.0;
						} else {
							// ����
							value_numeric [8] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [8]);
							value_string [8] = tempStr;
						}
					}

					// ���� 10
					strcpy (tempStr, objectInfo.var10name [yy].c_str ());
					value_type [9] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var10type [yy] = value_type [9];

					if ((value_type [9] != APIParT_Separator) || (value_type [9] != APIParT_Title) || (value_type [9] != API_ZombieParT)) {
						if (value_type [9] == APIParT_CString) {
							// ���ڿ�
							value_string [9] = getParameterStringByName (&memo, tempStr);
							value_numeric [9] = 0.0;
						} else {
							// ����
							value_numeric [9] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [9]);
							value_string [9] = tempStr;
						}
					}

					// ���� 11
					strcpy (tempStr, objectInfo.var11name [yy].c_str ());
					value_type [10] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var11type [yy] = value_type [10];

					if ((value_type [10] != APIParT_Separator) || (value_type [10] != APIParT_Title) || (value_type [10] != API_ZombieParT)) {
						if (value_type [10] == APIParT_CString) {
							// ���ڿ�
							value_string [10] = getParameterStringByName (&memo, tempStr);
							value_numeric [10] = 0.0;
						} else {
							// ����
							value_numeric [10] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [10]);
							value_string [10] = tempStr;
						}
					}

					// ���� 12
					strcpy (tempStr, objectInfo.var12name [yy].c_str ());
					value_type [11] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var12type [yy] = value_type [11];

					if ((value_type [11] != APIParT_Separator) || (value_type [11] != APIParT_Title) || (value_type [11] != API_ZombieParT)) {
						if (value_type [11] == APIParT_CString) {
							// ���ڿ�
							value_string [11] = getParameterStringByName (&memo, tempStr);
							value_numeric [11] = 0.0;
						} else {
							// ����
							value_numeric [11] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [11]);
							value_string [11] = tempStr;
						}
					}

					// ���� 13
					strcpy (tempStr, objectInfo.var13name [yy].c_str ());
					value_type [12] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var13type [yy] = value_type [12];

					if ((value_type [12] != APIParT_Separator) || (value_type [12] != APIParT_Title) || (value_type [12] != API_ZombieParT)) {
						if (value_type [12] == APIParT_CString) {
							// ���ڿ�
							value_string [12] = getParameterStringByName (&memo, tempStr);
							value_numeric [12] = 0.0;
						} else {
							// ����
							value_numeric [12] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [12]);
							value_string [12] = tempStr;
						}
					}

					// ���� 14
					strcpy (tempStr, objectInfo.var14name [yy].c_str ());
					value_type [13] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var14type [yy] = value_type [13];

					if ((value_type [13] != APIParT_Separator) || (value_type [13] != APIParT_Title) || (value_type [13] != API_ZombieParT)) {
						if (value_type [13] == APIParT_CString) {
							// ���ڿ�
							value_string [13] = getParameterStringByName (&memo, tempStr);
							value_numeric [13] = 0.0;
						} else {
							// ����
							value_numeric [13] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [13]);
							value_string [13] = tempStr;
						}
					}

					// ���� 15
					strcpy (tempStr, objectInfo.var15name [yy].c_str ());
					value_type [14] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var15type [yy] = value_type [14];

					if ((value_type [14] != APIParT_Separator) || (value_type [14] != APIParT_Title) || (value_type [14] != API_ZombieParT)) {
						if (value_type [14] == APIParT_CString) {
							// ���ڿ�
							value_string [14] = getParameterStringByName (&memo, tempStr);
							value_numeric [14] = 0.0;
						} else {
							// ����
							value_numeric [14] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [14]);
							value_string [14] = tempStr;
						}
					}

					// ���� 16
					strcpy (tempStr, objectInfo.var16name [yy].c_str ());
					value_type [15] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var16type [yy] = value_type [15];

					if ((value_type [15] != APIParT_Separator) || (value_type [15] != APIParT_Title) || (value_type [15] != API_ZombieParT)) {
						if (value_type [15] == APIParT_CString) {
							// ���ڿ�
							value_string [15] = getParameterStringByName (&memo, tempStr);
							value_numeric [15] = 0.0;
						} else {
							// ����
							value_numeric [15] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [15]);
							value_string [15] = tempStr;
						}
					}

					// ���� 17
					strcpy (tempStr, objectInfo.var17name [yy].c_str ());
					value_type [16] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var17type [yy] = value_type [16];

					if ((value_type [16] != APIParT_Separator) || (value_type [16] != APIParT_Title) || (value_type [16] != API_ZombieParT)) {
						if (value_type [16] == APIParT_CString) {
							// ���ڿ�
							value_string [16] = getParameterStringByName (&memo, tempStr);
							value_numeric [16] = 0.0;
						} else {
							// ����
							value_numeric [16] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [16]);
							value_string [16] = tempStr;
						}
					}

					// ���� 18
					strcpy (tempStr, objectInfo.var18name [yy].c_str ());
					value_type [17] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var18type [yy] = value_type [17];

					if ((value_type [17] != APIParT_Separator) || (value_type [17] != APIParT_Title) || (value_type [17] != API_ZombieParT)) {
						if (value_type [17] == APIParT_CString) {
							// ���ڿ�
							value_string [17] = getParameterStringByName (&memo, tempStr);
							value_numeric [17] = 0.0;
						} else {
							// ����
							value_numeric [17] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [17]);
							value_string [17] = tempStr;
						}
					}

					// ���� 19
					strcpy (tempStr, objectInfo.var19name [yy].c_str ());
					value_type [18] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var19type [yy] = value_type [18];

					if ((value_type [18] != APIParT_Separator) || (value_type [18] != APIParT_Title) || (value_type [18] != API_ZombieParT)) {
						if (value_type [18] == APIParT_CString) {
							// ���ڿ�
							value_string [18] = getParameterStringByName (&memo, tempStr);
							value_numeric [18] = 0.0;
						} else {
							// ����
							value_numeric [18] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [18]);
							value_string [18] = tempStr;
						}
					}

					// ���� 20
					strcpy (tempStr, objectInfo.var20name [yy].c_str ());
					value_type [19] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var20type [yy] = value_type [19];

					if ((value_type [19] != APIParT_Separator) || (value_type [19] != APIParT_Title) || (value_type [19] != API_ZombieParT)) {
						if (value_type [19] == APIParT_CString) {
							// ���ڿ�
							value_string [19] = getParameterStringByName (&memo, tempStr);
							value_numeric [19] = 0.0;
						} else {
							// ����
							value_numeric [19] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [19]);
							value_string [19] = tempStr;
						}
					}

					// ���� 21
					strcpy (tempStr, objectInfo.var21name [yy].c_str ());
					value_type [20] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var21type [yy] = value_type [20];

					if ((value_type [20] != APIParT_Separator) || (value_type [20] != APIParT_Title) || (value_type [20] != API_ZombieParT)) {
						if (value_type [20] == APIParT_CString) {
							// ���ڿ�
							value_string [20] = getParameterStringByName (&memo, tempStr);
							value_numeric [20] = 0.0;
						} else {
							// ����
							value_numeric [20] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [20]);
							value_string [20] = tempStr;
						}
					}

					// ���� 22
					strcpy (tempStr, objectInfo.var22name [yy].c_str ());
					value_type [21] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var22type [yy] = value_type [21];

					if ((value_type [21] != APIParT_Separator) || (value_type [21] != APIParT_Title) || (value_type [21] != API_ZombieParT)) {
						if (value_type [21] == APIParT_CString) {
							// ���ڿ�
							value_string [21] = getParameterStringByName (&memo, tempStr);
							value_numeric [21] = 0.0;
						} else {
							// ����
							value_numeric [21] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [21]);
							value_string [21] = tempStr;
						}
					}

					// ���� 23
					strcpy (tempStr, objectInfo.var23name [yy].c_str ());
					value_type [22] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var23type [yy] = value_type [22];

					if ((value_type [22] != APIParT_Separator) || (value_type [22] != APIParT_Title) || (value_type [22] != API_ZombieParT)) {
						if (value_type [22] == APIParT_CString) {
							// ���ڿ�
							value_string [22] = getParameterStringByName (&memo, tempStr);
							value_numeric [22] = 0.0;
						} else {
							// ����
							value_numeric [22] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [22]);
							value_string [22] = tempStr;
						}
					}

					// ���� 24
					strcpy (tempStr, objectInfo.var24name [yy].c_str ());
					value_type [23] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var24type [yy] = value_type [23];

					if ((value_type [23] != APIParT_Separator) || (value_type [23] != APIParT_Title) || (value_type [23] != API_ZombieParT)) {
						if (value_type [23] == APIParT_CString) {
							// ���ڿ�
							value_string [23] = getParameterStringByName (&memo, tempStr);
							value_numeric [23] = 0.0;
						} else {
							// ����
							value_numeric [23] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [23]);
							value_string [23] = tempStr;
						}
					}

					// ���� 25
					strcpy (tempStr, objectInfo.var25name [yy].c_str ());
					value_type [24] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var25type [yy] = value_type [24];

					if ((value_type [24] != APIParT_Separator) || (value_type [24] != APIParT_Title) || (value_type [24] != API_ZombieParT)) {
						if (value_type [24] == APIParT_CString) {
							// ���ڿ�
							value_string [24] = getParameterStringByName (&memo, tempStr);
							value_numeric [24] = 0.0;
						} else {
							// ����
							value_numeric [24] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [24]);
							value_string [24] = tempStr;
						}
					}

					// ���� 26
					strcpy (tempStr, objectInfo.var26name [yy].c_str ());
					value_type [25] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var26type [yy] = value_type [25];

					if ((value_type [25] != APIParT_Separator) || (value_type [25] != APIParT_Title) || (value_type [25] != API_ZombieParT)) {
						if (value_type [25] == APIParT_CString) {
							// ���ڿ�
							value_string [25] = getParameterStringByName (&memo, tempStr);
							value_numeric [25] = 0.0;
						} else {
							// ����
							value_numeric [25] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [25]);
							value_string [25] = tempStr;
						}
					}

					// ���� 27
					strcpy (tempStr, objectInfo.var27name [yy].c_str ());
					value_type [26] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var27type [yy] = value_type [26];

					if ((value_type [26] != APIParT_Separator) || (value_type [26] != APIParT_Title) || (value_type [26] != API_ZombieParT)) {
						if (value_type [26] == APIParT_CString) {
							// ���ڿ�
							value_string [26] = getParameterStringByName (&memo, tempStr);
							value_numeric [26] = 0.0;
						} else {
							// ����
							value_numeric [26] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [26]);
							value_string [26] = tempStr;
						}
					}

					// ���� 28
					strcpy (tempStr, objectInfo.var28name [yy].c_str ());
					value_type [27] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var28type [yy] = value_type [27];

					if ((value_type [27] != APIParT_Separator) || (value_type [27] != APIParT_Title) || (value_type [27] != API_ZombieParT)) {
						if (value_type [27] == APIParT_CString) {
							// ���ڿ�
							value_string [27] = getParameterStringByName (&memo, tempStr);
							value_numeric [27] = 0.0;
						} else {
							// ����
							value_numeric [27] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [27]);
							value_string [27] = tempStr;
						}
					}

					// ���� 29
					strcpy (tempStr, objectInfo.var29name [yy].c_str ());
					value_type [28] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var29type [yy] = value_type [28];

					if ((value_type [28] != APIParT_Separator) || (value_type [28] != APIParT_Title) || (value_type [28] != API_ZombieParT)) {
						if (value_type [28] == APIParT_CString) {
							// ���ڿ�
							value_string [28] = getParameterStringByName (&memo, tempStr);
							value_numeric [28] = 0.0;
						} else {
							// ����
							value_numeric [28] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [28]);
							value_string [28] = tempStr;
						}
					}

					// ���� 30
					strcpy (tempStr, objectInfo.var30name [yy].c_str ());
					value_type [29] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var30type [yy] = value_type [29];

					if ((value_type [29] != APIParT_Separator) || (value_type [29] != APIParT_Title) || (value_type [29] != API_ZombieParT)) {
						if (value_type [29] == APIParT_CString) {
							// ���ڿ�
							value_string [29] = getParameterStringByName (&memo, tempStr);
							value_numeric [29] = 0.0;
						} else {
							// ����
							value_numeric [29] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [29]);
							value_string [29] = tempStr;
						}
					}

					// ���� 31
					strcpy (tempStr, objectInfo.var31name [yy].c_str ());
					value_type [30] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var31type [yy] = value_type [30];

					if ((value_type [30] != APIParT_Separator) || (value_type [30] != APIParT_Title) || (value_type [30] != API_ZombieParT)) {
						if (value_type [30] == APIParT_CString) {
							// ���ڿ�
							value_string [30] = getParameterStringByName (&memo, tempStr);
							value_numeric [30] = 0.0;
						} else {
							// ����
							value_numeric [30] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [30]);
							value_string [30] = tempStr;
						}
					}

					// ���� 32
					strcpy (tempStr, objectInfo.var32name [yy].c_str ());
					value_type [31] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var32type [yy] = value_type [31];

					if ((value_type [31] != APIParT_Separator) || (value_type [31] != APIParT_Title) || (value_type [31] != API_ZombieParT)) {
						if (value_type [31] == APIParT_CString) {
							// ���ڿ�
							value_string [31] = getParameterStringByName (&memo, tempStr);
							value_numeric [31] = 0.0;
						} else {
							// ����
							value_numeric [31] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [31]);
							value_string [31] = tempStr;
						}
					}

					// ���� 33
					strcpy (tempStr, objectInfo.var33name [yy].c_str ());
					value_type [32] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var33type [yy] = value_type [32];

					if ((value_type [32] != APIParT_Separator) || (value_type [32] != APIParT_Title) || (value_type [32] != API_ZombieParT)) {
						if (value_type [32] == APIParT_CString) {
							// ���ڿ�
							value_string [32] = getParameterStringByName (&memo, tempStr);
							value_numeric [32] = 0.0;
						} else {
							// ����
							value_numeric [32] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [32]);
							value_string [32] = tempStr;
						}
					}

					// ���� 34
					strcpy (tempStr, objectInfo.var34name [yy].c_str ());
					value_type [33] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var34type [yy] = value_type [33];

					if ((value_type [33] != APIParT_Separator) || (value_type [33] != APIParT_Title) || (value_type [33] != API_ZombieParT)) {
						if (value_type [33] == APIParT_CString) {
							// ���ڿ�
							value_string [33] = getParameterStringByName (&memo, tempStr);
							value_numeric [33] = 0.0;
						} else {
							// ����
							value_numeric [33] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [33]);
							value_string [33] = tempStr;
						}
					}

					// ���� 35
					strcpy (tempStr, objectInfo.var35name [yy].c_str ());
					value_type [34] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var35type [yy] = value_type [34];

					if ((value_type [34] != APIParT_Separator) || (value_type [34] != APIParT_Title) || (value_type [34] != API_ZombieParT)) {
						if (value_type [34] == APIParT_CString) {
							// ���ڿ�
							value_string [34] = getParameterStringByName (&memo, tempStr);
							value_numeric [34] = 0.0;
						} else {
							// ����
							value_numeric [34] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [34]);
							value_string [34] = tempStr;
						}
					}

					// ���� 36
					strcpy (tempStr, objectInfo.var36name [yy].c_str ());
					value_type [35] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var36type [yy] = value_type [35];

					if ((value_type [35] != APIParT_Separator) || (value_type [35] != APIParT_Title) || (value_type [35] != API_ZombieParT)) {
						if (value_type [35] == APIParT_CString) {
							// ���ڿ�
							value_string [35] = getParameterStringByName (&memo, tempStr);
							value_numeric [35] = 0.0;
						} else {
							// ����
							value_numeric [35] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [35]);
							value_string [35] = tempStr;
						}
					}

					// ���� 37
					strcpy (tempStr, objectInfo.var37name [yy].c_str ());
					value_type [36] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var37type [yy] = value_type [36];

					if ((value_type [36] != APIParT_Separator) || (value_type [36] != APIParT_Title) || (value_type [36] != API_ZombieParT)) {
						if (value_type [36] == APIParT_CString) {
							// ���ڿ�
							value_string [36] = getParameterStringByName (&memo, tempStr);
							value_numeric [36] = 0.0;
						} else {
							// ����
							value_numeric [36] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [36]);
							value_string [36] = tempStr;
						}
					}

					// ���� 38
					strcpy (tempStr, objectInfo.var38name [yy].c_str ());
					value_type [37] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var38type [yy] = value_type [37];

					if ((value_type [37] != APIParT_Separator) || (value_type [37] != APIParT_Title) || (value_type [37] != API_ZombieParT)) {
						if (value_type [37] == APIParT_CString) {
							// ���ڿ�
							value_string [37] = getParameterStringByName (&memo, tempStr);
							value_numeric [37] = 0.0;
						} else {
							// ����
							value_numeric [37] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [37]);
							value_string [37] = tempStr;
						}
					}

					// ���� 39
					strcpy (tempStr, objectInfo.var39name [yy].c_str ());
					value_type [38] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var39type [yy] = value_type [38];

					if ((value_type [38] != APIParT_Separator) || (value_type [38] != APIParT_Title) || (value_type [38] != API_ZombieParT)) {
						if (value_type [38] == APIParT_CString) {
							// ���ڿ�
							value_string [38] = getParameterStringByName (&memo, tempStr);
							value_numeric [38] = 0.0;
						} else {
							// ����
							value_numeric [38] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [38]);
							value_string [38] = tempStr;
						}
					}

					// ���� 40
					strcpy (tempStr, objectInfo.var40name [yy].c_str ());
					value_type [39] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var40type [yy] = value_type [39];

					if ((value_type [39] != APIParT_Separator) || (value_type [39] != APIParT_Title) || (value_type [39] != API_ZombieParT)) {
						if (value_type [39] == APIParT_CString) {
							// ���ڿ�
							value_string [39] = getParameterStringByName (&memo, tempStr);
							value_numeric [39] = 0.0;
						} else {
							// ����
							value_numeric [39] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [39]);
							value_string [39] = tempStr;
						}
					}

					// ���� 41
					strcpy (tempStr, objectInfo.var41name [yy].c_str ());
					value_type [40] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var41type [yy] = value_type [40];

					if ((value_type [40] != APIParT_Separator) || (value_type [40] != APIParT_Title) || (value_type [40] != API_ZombieParT)) {
						if (value_type [40] == APIParT_CString) {
							// ���ڿ�
							value_string [40] = getParameterStringByName (&memo, tempStr);
							value_numeric [40] = 0.0;
						} else {
							// ����
							value_numeric [40] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [40]);
							value_string [40] = tempStr;
						}
					}

					// ���� 42
					strcpy (tempStr, objectInfo.var42name [yy].c_str ());
					value_type [41] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var42type [yy] = value_type [41];

					if ((value_type [41] != APIParT_Separator) || (value_type [41] != APIParT_Title) || (value_type [41] != API_ZombieParT)) {
						if (value_type [41] == APIParT_CString) {
							// ���ڿ�
							value_string [41] = getParameterStringByName (&memo, tempStr);
							value_numeric [41] = 0.0;
						} else {
							// ����
							value_numeric [41] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [41]);
							value_string [41] = tempStr;
						}
					}

					// ���� 43
					strcpy (tempStr, objectInfo.var43name [yy].c_str ());
					value_type [42] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var43type [yy] = value_type [42];

					if ((value_type [42] != APIParT_Separator) || (value_type [42] != APIParT_Title) || (value_type [42] != API_ZombieParT)) {
						if (value_type [42] == APIParT_CString) {
							// ���ڿ�
							value_string [42] = getParameterStringByName (&memo, tempStr);
							value_numeric [42] = 0.0;
						} else {
							// ����
							value_numeric [42] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [42]);
							value_string [42] = tempStr;
						}
					}

					// ���� 44
					strcpy (tempStr, objectInfo.var44name [yy].c_str ());
					value_type [43] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var44type [yy] = value_type [43];

					if ((value_type [43] != APIParT_Separator) || (value_type [43] != APIParT_Title) || (value_type [43] != API_ZombieParT)) {
						if (value_type [43] == APIParT_CString) {
							// ���ڿ�
							value_string [43] = getParameterStringByName (&memo, tempStr);
							value_numeric [43] = 0.0;
						} else {
							// ����
							value_numeric [43] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [43]);
							value_string [43] = tempStr;
						}
					}

					// ���� 45
					strcpy (tempStr, objectInfo.var45name [yy].c_str ());
					value_type [44] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var45type [yy] = value_type [44];

					if ((value_type [44] != APIParT_Separator) || (value_type [44] != APIParT_Title) || (value_type [44] != API_ZombieParT)) {
						if (value_type [44] == APIParT_CString) {
							// ���ڿ�
							value_string [44] = getParameterStringByName (&memo, tempStr);
							value_numeric [44] = 0.0;
						} else {
							// ����
							value_numeric [44] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [44]);
							value_string [44] = tempStr;
						}
					}

					// ���� 46
					strcpy (tempStr, objectInfo.var46name [yy].c_str ());
					value_type [45] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var46type [yy] = value_type [45];

					if ((value_type [45] != APIParT_Separator) || (value_type [45] != APIParT_Title) || (value_type [45] != API_ZombieParT)) {
						if (value_type [45] == APIParT_CString) {
							// ���ڿ�
							value_string [45] = getParameterStringByName (&memo, tempStr);
							value_numeric [45] = 0.0;
						} else {
							// ����
							value_numeric [45] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [45]);
							value_string [45] = tempStr;
						}
					}

					// ���� 47
					strcpy (tempStr, objectInfo.var47name [yy].c_str ());
					value_type [46] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var47type [yy] = value_type [46];

					if ((value_type [46] != APIParT_Separator) || (value_type [46] != APIParT_Title) || (value_type [46] != API_ZombieParT)) {
						if (value_type [46] == APIParT_CString) {
							// ���ڿ�
							value_string [46] = getParameterStringByName (&memo, tempStr);
							value_numeric [46] = 0.0;
						} else {
							// ����
							value_numeric [46] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [46]);
							value_string [46] = tempStr;
						}
					}

					// ���� 48
					strcpy (tempStr, objectInfo.var48name [yy].c_str ());
					value_type [47] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var48type [yy] = value_type [47];

					if ((value_type [47] != APIParT_Separator) || (value_type [47] != APIParT_Title) || (value_type [47] != API_ZombieParT)) {
						if (value_type [47] == APIParT_CString) {
							// ���ڿ�
							value_string [47] = getParameterStringByName (&memo, tempStr);
							value_numeric [47] = 0.0;
						} else {
							// ����
							value_numeric [47] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [47]);
							value_string [47] = tempStr;
						}
					}

					// ���� 49
					strcpy (tempStr, objectInfo.var49name [yy].c_str ());
					value_type [48] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var49type [yy] = value_type [48];

					if ((value_type [48] != APIParT_Separator) || (value_type [48] != APIParT_Title) || (value_type [48] != API_ZombieParT)) {
						if (value_type [48] == APIParT_CString) {
							// ���ڿ�
							value_string [48] = getParameterStringByName (&memo, tempStr);
							value_numeric [48] = 0.0;
						} else {
							// ����
							value_numeric [48] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [48]);
							value_string [48] = tempStr;
						}
					}

					// ���� 50
					strcpy (tempStr, objectInfo.var50name [yy].c_str ());
					value_type [49] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var50type [yy] = value_type [49];

					if ((value_type [49] != APIParT_Separator) || (value_type [49] != APIParT_Title) || (value_type [49] != API_ZombieParT)) {
						if (value_type [49] == APIParT_CString) {
							// ���ڿ�
							value_string [49] = getParameterStringByName (&memo, tempStr);
							value_numeric [49] = 0.0;
						} else {
							// ����
							value_numeric [49] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [49]);
							value_string [49] = tempStr;
						}
					}

					// �ߺ� �׸��� ������ ����
					for (zz = 0 ; zz < objectInfo.nCounts [yy] ; ++zz) {
						if ((objectInfo.var1value [yy][zz].compare (value_string [0]) == 0) && (objectInfo.var2value [yy][zz].compare (value_string [1]) == 0) && (objectInfo.var3value [yy][zz].compare (value_string [2]) == 0) && 
							(objectInfo.var4value [yy][zz].compare (value_string [3]) == 0) && (objectInfo.var5value [yy][zz].compare (value_string [4]) == 0) && (objectInfo.var6value [yy][zz].compare (value_string [5]) == 0) && 
							(objectInfo.var7value [yy][zz].compare (value_string [6]) == 0) && (objectInfo.var8value [yy][zz].compare (value_string [7]) == 0) && (objectInfo.var9value [yy][zz].compare (value_string [8]) == 0) &&
							(objectInfo.var10value [yy][zz].compare (value_string [9]) == 0) && (objectInfo.var11value [yy][zz].compare (value_string [10]) == 0) && (objectInfo.var12value [yy][zz].compare (value_string [11]) == 0) &&
							(objectInfo.var13value [yy][zz].compare (value_string [12]) == 0) && (objectInfo.var14value [yy][zz].compare (value_string [13]) == 0) && (objectInfo.var15value [yy][zz].compare (value_string [14]) == 0) &&
							(objectInfo.var16value [yy][zz].compare (value_string [15]) == 0) && (objectInfo.var17value [yy][zz].compare (value_string [16]) == 0) && (objectInfo.var18value [yy][zz].compare (value_string [17]) == 0) &&
							(objectInfo.var19value [yy][zz].compare (value_string [18]) == 0) && (objectInfo.var20value [yy][zz].compare (value_string [19]) == 0) && (objectInfo.var21value [yy][zz].compare (value_string [20]) == 0) &&
							(objectInfo.var22value [yy][zz].compare (value_string [21]) == 0) && (objectInfo.var23value [yy][zz].compare (value_string [22]) == 0) && (objectInfo.var24value [yy][zz].compare (value_string [23]) == 0) &&
							(objectInfo.var25value [yy][zz].compare (value_string [24]) == 0) && (objectInfo.var26value [yy][zz].compare (value_string [25]) == 0) && (objectInfo.var27value [yy][zz].compare (value_string [26]) == 0) &&
							(objectInfo.var28value [yy][zz].compare (value_string [27]) == 0) && (objectInfo.var29value [yy][zz].compare (value_string [28]) == 0) && (objectInfo.var30value [yy][zz].compare (value_string [29]) == 0) &&
							(objectInfo.var31value [yy][zz].compare (value_string [30]) == 0) && (objectInfo.var32value [yy][zz].compare (value_string [31]) == 0) && (objectInfo.var33value [yy][zz].compare (value_string [32]) == 0) &&
							(objectInfo.var34value [yy][zz].compare (value_string [33]) == 0) && (objectInfo.var35value [yy][zz].compare (value_string [34]) == 0) && (objectInfo.var36value [yy][zz].compare (value_string [35]) == 0) &&
							(objectInfo.var37value [yy][zz].compare (value_string [36]) == 0) && (objectInfo.var38value [yy][zz].compare (value_string [37]) == 0) && (objectInfo.var39value [yy][zz].compare (value_string [38]) == 0) &&
							(objectInfo.var40value [yy][zz].compare (value_string [39]) == 0) && (objectInfo.var41value [yy][zz].compare (value_string [40]) == 0) && (objectInfo.var42value [yy][zz].compare (value_string [41]) == 0) &&
							(objectInfo.var43value [yy][zz].compare (value_string [42]) == 0) && (objectInfo.var44value [yy][zz].compare (value_string [43]) == 0) && (objectInfo.var45value [yy][zz].compare (value_string [44]) == 0) &&
							(objectInfo.var46value [yy][zz].compare (value_string [45]) == 0) && (objectInfo.var47value [yy][zz].compare (value_string [46]) == 0) && (objectInfo.var48value [yy][zz].compare (value_string [47]) == 0) &&
							(objectInfo.var49value [yy][zz].compare (value_string [48]) == 0) && (objectInfo.var50value [yy][zz].compare (value_string [49]) == 0)) {

								objectInfo.combinationCount [yy][zz] ++;
								foundExistValue = true;
								break;
						}
					}

					// �ű� �׸��̸�
					if (!foundExistValue) {
						objectInfo.var1value [yy][objectInfo.nCounts [yy]] = value_string [0];
						objectInfo.var2value [yy][objectInfo.nCounts [yy]] = value_string [1];
						objectInfo.var3value [yy][objectInfo.nCounts [yy]] = value_string [2];
						objectInfo.var4value [yy][objectInfo.nCounts [yy]] = value_string [3];
						objectInfo.var5value [yy][objectInfo.nCounts [yy]] = value_string [4];
						objectInfo.var6value [yy][objectInfo.nCounts [yy]] = value_string [5];
						objectInfo.var7value [yy][objectInfo.nCounts [yy]] = value_string [6];
						objectInfo.var8value [yy][objectInfo.nCounts [yy]] = value_string [7];
						objectInfo.var9value [yy][objectInfo.nCounts [yy]] = value_string [8];
						objectInfo.var10value [yy][objectInfo.nCounts [yy]] = value_string [9];
						objectInfo.var11value [yy][objectInfo.nCounts [yy]] = value_string [10];
						objectInfo.var12value [yy][objectInfo.nCounts [yy]] = value_string [11];
						objectInfo.var13value [yy][objectInfo.nCounts [yy]] = value_string [12];
						objectInfo.var14value [yy][objectInfo.nCounts [yy]] = value_string [13];
						objectInfo.var15value [yy][objectInfo.nCounts [yy]] = value_string [14];
						objectInfo.var16value [yy][objectInfo.nCounts [yy]] = value_string [15];
						objectInfo.var17value [yy][objectInfo.nCounts [yy]] = value_string [16];
						objectInfo.var18value [yy][objectInfo.nCounts [yy]] = value_string [17];
						objectInfo.var19value [yy][objectInfo.nCounts [yy]] = value_string [18];
						objectInfo.var20value [yy][objectInfo.nCounts [yy]] = value_string [19];
						objectInfo.var21value [yy][objectInfo.nCounts [yy]] = value_string [20];
						objectInfo.var22value [yy][objectInfo.nCounts [yy]] = value_string [21];
						objectInfo.var23value [yy][objectInfo.nCounts [yy]] = value_string [22];
						objectInfo.var24value [yy][objectInfo.nCounts [yy]] = value_string [23];
						objectInfo.var25value [yy][objectInfo.nCounts [yy]] = value_string [24];
						objectInfo.var26value [yy][objectInfo.nCounts [yy]] = value_string [25];
						objectInfo.var27value [yy][objectInfo.nCounts [yy]] = value_string [26];
						objectInfo.var28value [yy][objectInfo.nCounts [yy]] = value_string [27];
						objectInfo.var29value [yy][objectInfo.nCounts [yy]] = value_string [28];
						objectInfo.var30value [yy][objectInfo.nCounts [yy]] = value_string [29];
						objectInfo.var31value [yy][objectInfo.nCounts [yy]] = value_string [30];
						objectInfo.var32value [yy][objectInfo.nCounts [yy]] = value_string [31];
						objectInfo.var33value [yy][objectInfo.nCounts [yy]] = value_string [32];
						objectInfo.var34value [yy][objectInfo.nCounts [yy]] = value_string [33];
						objectInfo.var35value [yy][objectInfo.nCounts [yy]] = value_string [34];
						objectInfo.var36value [yy][objectInfo.nCounts [yy]] = value_string [35];
						objectInfo.var37value [yy][objectInfo.nCounts [yy]] = value_string [36];
						objectInfo.var38value [yy][objectInfo.nCounts [yy]] = value_string [37];
						objectInfo.var39value [yy][objectInfo.nCounts [yy]] = value_string [38];
						objectInfo.var40value [yy][objectInfo.nCounts [yy]] = value_string [39];
						objectInfo.var41value [yy][objectInfo.nCounts [yy]] = value_string [40];
						objectInfo.var42value [yy][objectInfo.nCounts [yy]] = value_string [41];
						objectInfo.var43value [yy][objectInfo.nCounts [yy]] = value_string [42];
						objectInfo.var44value [yy][objectInfo.nCounts [yy]] = value_string [43];
						objectInfo.var45value [yy][objectInfo.nCounts [yy]] = value_string [44];
						objectInfo.var46value [yy][objectInfo.nCounts [yy]] = value_string [45];
						objectInfo.var47value [yy][objectInfo.nCounts [yy]] = value_string [46];
						objectInfo.var48value [yy][objectInfo.nCounts [yy]] = value_string [47];
						objectInfo.var49value [yy][objectInfo.nCounts [yy]] = value_string [48];
						objectInfo.var50value [yy][objectInfo.nCounts [yy]] = value_string [49];

						objectInfo.combinationCount [yy][objectInfo.nCounts [yy]] = 1;
						objectInfo.nCounts [yy] ++;
					}
				}
			}
		}

		// ���� ã�� ���ϸ� �� �� ���� ��ü�� �����
		if (foundObject == false)
			objectInfo.nUnknownObjects ++;

		ACAPI_DisposeElemMemoHdls (&memo);
	}

	// �� ���� ����
	for (xx = 0 ; xx < nBeams ; ++xx) {
		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		elem.header.guid = beams.Pop ();
		err = ACAPI_Element_Get (&elem);
		err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

		foundExistValue = false;

		int len;

		len = static_cast<int> (round (GetDistance (elem.beam.begC, elem.beam.endC) * 1000, 0));

		// �ߺ� �׸��� ������ ����
		for (zz = 0 ; zz < objectInfo.nCountsBeam ; ++zz) {
			if (objectInfo.beamLength [zz] == len) {
				objectInfo.beamCount [zz] ++;
				foundExistValue = true;
				break;
			}
		}

		// �ű� �׸� �߰��ϰ� ������ ����
		if ( !foundExistValue ) {
			objectInfo.beamLength.push_back (len);
			objectInfo.beamCount.push_back (1);
			objectInfo.nCountsBeam ++;
		}

		ACAPI_DisposeElemMemoHdls (&memo);
	}

	// ���� �ؽ�Ʈ ǥ��
	// APIParT_Length�� ��� 1000�� ���ؼ� ǥ��
	// APIParT_Boolean�� ��� ��/�ƴϿ� ǥ��
	double	length, length2, length3;
	bool	bShow;

	for (xx = 0 ; xx < objectInfo.nKnownObjects ; ++xx) {
		for (yy = 0 ; yy < objectInfo.nCounts [xx] ; ++yy) {
			// ����
			if (yy == 0) {
				sprintf (buffer, "\n[%s]\n", objectInfo.nameVal [xx].c_str ());
				fprintf (fp, buffer);
			}

			if (my_strcmp (objectInfo.nameVal [xx].c_str (), "������ ��ũ") == 0) {
				// ����
				if (my_strcmp (objectInfo.var2value [xx][yy].c_str (), "����") == 0) {
					sprintf (buffer, "���� / %s ", objectInfo.var1value [xx][yy].c_str ());

				// �簢
				} else {
					sprintf (buffer, "�簢 / %s ", objectInfo.var1value [xx][yy].c_str ());
				}
				fprintf (fp, buffer);

			} else if (my_strcmp (objectInfo.nameVal [xx].c_str (), "������") == 0) {
				// �԰���
				if (atoi (objectInfo.var1value [xx][yy].c_str ()) > 0) {
					sprintf (buffer, "%s X %s ", objectInfo.var2value [xx][yy], objectInfo.var3value [xx][yy]);
				// ��԰���
				} else {
					// 4�� X 5��
					length = atof (objectInfo.var4value [xx][yy].c_str ());
					length2 = atof (objectInfo.var5value [xx][yy].c_str ());
					sprintf (buffer, "%.0f X %.0f ", round (length*1000, 0), round (length2*1000, 0));
				}
				fprintf (fp, buffer);

			} else if (my_strcmp (objectInfo.nameVal [xx].c_str (), "��ƿ��") == 0) {
				// �԰���
				if (atoi (objectInfo.var1value [xx][yy].c_str ()) > 0) {
					sprintf (buffer, "%s X %s ", objectInfo.var2value [xx][yy], objectInfo.var3value [xx][yy]);
				// ��԰���
				} else {
					// 4�� X 5��
					length = atof (objectInfo.var4value [xx][yy].c_str ());
					length2 = atof (objectInfo.var5value [xx][yy].c_str ());
					sprintf (buffer, "%.0f X %.0f ", round (length*1000, 0), round (length2*1000, 0));
				}
				fprintf (fp, buffer);

			} else if (my_strcmp (objectInfo.nameVal [xx].c_str (), "����") == 0) {
				length = atof (objectInfo.var1value [xx][yy].c_str ());
				length2 = atof (objectInfo.var2value [xx][yy].c_str ());
				length3 = atof (objectInfo.var3value [xx][yy].c_str ());
				sprintf (buffer, "%.0f X %.0f X %.0f ", round (length*1000, 0), round (length2*1000, 0), round (length3*1000, 0));
				fprintf (fp, buffer);

			} else if (my_strcmp (objectInfo.nameVal [xx].c_str (), "����(�ٰ���)") == 0) {
				sprintf (buffer, "����(�ٰ���) ���� %s ", objectInfo.var1value [xx][yy].c_str ());
				fprintf (fp, buffer);

				if (atoi (objectInfo.var2value [xx][yy].c_str ()) > 0) {
					sprintf (buffer, "(���� �ѱ���: %s) ", objectInfo.var3value [xx][yy].c_str ());
					fprintf (fp, buffer);
				}

			} else if (my_strcmp (objectInfo.nameVal [xx].c_str (), "����") == 0) {
				if (my_strcmp (objectInfo.var1value [xx][yy].c_str (), "3x6 [910x1820]") == 0) {
					sprintf (buffer, "910 X 1820 X %s ", objectInfo.var2value [xx][yy].c_str ());
					fprintf (fp, buffer);

					// ����Ʋ ON
					if (atoi (objectInfo.var5value [xx][yy].c_str ()) > 0) {
						sprintf (buffer, "(���� �ѱ���: %s) ", objectInfo.var6value [xx][yy].c_str ());
						fprintf (fp, buffer);
					}
				} else if (my_strcmp (objectInfo.var1value [xx][yy].c_str (), "4x8 [1220x2440]") == 0) {
					sprintf (buffer, "1220 X 2440 X %s ", objectInfo.var2value [xx][yy].c_str ());
					fprintf (fp, buffer);

					// ����Ʋ ON
					if (atoi (objectInfo.var5value [xx][yy].c_str ()) > 0) {
						sprintf (buffer, "(���� �ѱ���: %s) ", objectInfo.var6value [xx][yy].c_str ());
						fprintf (fp, buffer);
					}
				} else if (my_strcmp (objectInfo.var1value [xx][yy].c_str (), "2x5 [606x1520]") == 0) {
					sprintf (buffer, "606 X 1520 X %s ", objectInfo.var2value [xx][yy].c_str ());
					fprintf (fp, buffer);

					// ����Ʋ ON
					if (atoi (objectInfo.var5value [xx][yy].c_str ()) > 0) {
						sprintf (buffer, "(���� �ѱ���: %s) ", objectInfo.var6value [xx][yy].c_str ());
						fprintf (fp, buffer);
					}
				} else if (my_strcmp (objectInfo.var1value [xx][yy].c_str (), "2x6 [606x1820]") == 0) {
					sprintf (buffer, "606 X 1820 X %s ", objectInfo.var2value [xx][yy].c_str ());
					fprintf (fp, buffer);

					// ����Ʋ ON
					if (atoi (objectInfo.var5value [xx][yy].c_str ()) > 0) {
						sprintf (buffer, "(���� �ѱ���: %s) ", objectInfo.var6value [xx][yy].c_str ());
						fprintf (fp, buffer);
					}
				} else if (my_strcmp (objectInfo.var1value [xx][yy].c_str (), "3x5 [910x1520]") == 0) {
					sprintf (buffer, "910 X 1520 X %s ", objectInfo.var2value [xx][yy].c_str ());
					fprintf (fp, buffer);

					// ����Ʋ ON
					if (atoi (objectInfo.var5value [xx][yy].c_str ()) > 0) {
						sprintf (buffer, "(���� �ѱ���: %s) ", objectInfo.var6value [xx][yy].c_str ());
						fprintf (fp, buffer);
					}
				} else if (my_strcmp (objectInfo.var1value [xx][yy].c_str (), "��԰�") == 0) {
					// ���� X ���� X �β�
					length = atof (objectInfo.var3value [xx][yy].c_str ());
					length2 = atof (objectInfo.var4value [xx][yy].c_str ());
					sprintf (buffer, "%.0f X %.0f X %s ", round (length*1000, 0), round (length2*1000, 0), objectInfo.var2value [xx][yy].c_str ());
					fprintf (fp, buffer);

					// ����Ʋ ON
					if (atoi (objectInfo.var5value [xx][yy].c_str ()) > 0) {
						sprintf (buffer, "(���� �ѱ���: %s) ", objectInfo.var6value [xx][yy].c_str ());
						fprintf (fp, buffer);
					}
				} else if (my_strcmp (objectInfo.var1value [xx][yy].c_str (), "������") == 0) {
					sprintf (buffer, "������ ");
					fprintf (fp, buffer);

					// ����Ʋ ON
					if (atoi (objectInfo.var5value [xx][yy].c_str ()) > 0) {
						sprintf (buffer, "(���� �ѱ���: %s) ", objectInfo.var6value [xx][yy].c_str ());
						fprintf (fp, buffer);
					}
				} else {
					sprintf (buffer, "����(�ٰ���) ");
					fprintf (fp, buffer);
				}

			} else if (my_strcmp (objectInfo.nameVal [xx].c_str (), "RS Push-Pull Props") == 0) {
				// ���̽� �÷���Ʈ ����
				if (atoi (objectInfo.var1value [xx][yy].c_str ()) == 1) {
					sprintf (buffer, "���̽� �÷���Ʈ(����) ");
				} else {
					sprintf (buffer, "���̽� �÷���Ʈ(����) ");
				}
				fprintf (fp, buffer);

				// �԰�(���)
				sprintf (buffer, "�԰�(���): %s ", objectInfo.var2value [xx][yy].c_str ());
				fprintf (fp, buffer);

				// �԰�(�Ϻ�) - ���û���
				if (atoi (objectInfo.var4value [xx][yy].c_str ()) == 1) {
					sprintf (buffer, "�԰�(�Ϻ�): %s ", objectInfo.var3value [xx][yy].c_str ());
				}
				fprintf (fp, buffer);

			} else if (my_strcmp (objectInfo.nameVal [xx].c_str (), "�簢������") == 0) {
				// �簢������
				if (atoi (objectInfo.var1value [xx][yy].c_str ()) == 0) {
					length = atof (objectInfo.var2value [xx][yy].c_str ());
					sprintf (buffer, "50 x 50 x %.0f ", round (length*1000, 0));

				// ���簢������
				} else {
					length = atof (objectInfo.var2value [xx][yy].c_str ());
					sprintf (buffer, "%s x %.0f ", objectInfo.var1value [xx][yy].c_str (), round (length*1000, 0));
				}
				fprintf (fp, buffer);

			} else if (my_strcmp (objectInfo.nameVal [xx].c_str (), "����������") == 0) {
				length = atof (objectInfo.var1value [xx][yy].c_str ());
				sprintf (buffer, "%.0f ", round (length*1000, 0));
				fprintf (fp, buffer);

			} else if (my_strcmp (objectInfo.nameVal [xx].c_str (), "�ƿ��ڳʾޱ�") == 0) {
				length = atof (objectInfo.var1value [xx][yy].c_str ());
				sprintf (buffer, "%.0f ", round (length*1000, 0));
				fprintf (fp, buffer);

			} else if (my_strcmp (objectInfo.nameVal [xx].c_str (), "������") == 0) {
				if (atoi (objectInfo.var2value [xx][yy].c_str ()) > 0) {
					length = atof (objectInfo.var1value [xx][yy].c_str ());
					length2 = atof (objectInfo.var5value [xx][yy].c_str ());
					sprintf (buffer, "%.0f, ���� %.0f", round (length*1000, 0), round (length2*1000, 0));
				} else {
					length = atof (objectInfo.var1value [xx][yy].c_str ());
					sprintf (buffer, "%.0f ", round (length*1000, 0));
				}
				fprintf (fp, buffer);

			} else if (my_strcmp (objectInfo.nameVal [xx].c_str (), "�����") == 0) {
				sprintf (buffer, "%s ", objectInfo.var1value [xx][yy].c_str ());
				fprintf (fp, buffer);

			} else if (my_strcmp (objectInfo.nameVal [xx].c_str (), "�� �ۿ���") == 0) {
				length = atof (objectInfo.var1value [xx][yy].c_str ());
				sprintf (buffer, "%.0f ", round (length*1000, 0));
				fprintf (fp, buffer);

			} else if (my_strcmp (objectInfo.nameVal [xx].c_str (), "��������") == 0) {
				sprintf (buffer, "%s �� ", objectInfo.var1value [xx][yy].c_str ());
				fprintf (fp, buffer);

			} else {
				// ������ �� ǥ��
				if (objectInfo.var1name [xx].size () > 1) {
					bShow = false;
					if (objectInfo.var1showFlag [xx] == 0)																bShow = true;
					if ((objectInfo.var1showFlag [xx] ==  2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var1showFlag [xx] == -2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var1showFlag [xx] ==  3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var1showFlag [xx] == -3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var1showFlag [xx] ==  4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var1showFlag [xx] == -4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var1showFlag [xx] ==  5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var1showFlag [xx] == -5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var1showFlag [xx] ==  6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var1showFlag [xx] == -6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var1showFlag [xx] ==  7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var1showFlag [xx] == -7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var1showFlag [xx] ==  8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var1showFlag [xx] == -8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var1showFlag [xx] ==  9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var1showFlag [xx] == -9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 0))	bShow = true;

					if (objectInfo.var1type [xx] == APIParT_Length) {
						length = atof (objectInfo.var1value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var1desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var1type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var1value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var1desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var1desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var1desc [xx].c_str (), objectInfo.var1value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var2name [xx].size () > 1) {
					bShow = false;
					if (objectInfo.var2showFlag [xx] == 0)																bShow = true;
					if ((objectInfo.var2showFlag [xx] ==  1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var2showFlag [xx] == -1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var2showFlag [xx] ==  3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var2showFlag [xx] == -3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var2showFlag [xx] ==  4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var2showFlag [xx] == -4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var2showFlag [xx] ==  5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var2showFlag [xx] == -5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var2showFlag [xx] ==  6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var2showFlag [xx] == -6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var2showFlag [xx] ==  7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var2showFlag [xx] == -7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var2showFlag [xx] ==  8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var2showFlag [xx] == -8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var2showFlag [xx] ==  9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var2showFlag [xx] == -9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 0))	bShow = true;

					if (objectInfo.var2type [xx] == APIParT_Length) {
						length = atof (objectInfo.var2value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var2desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var2type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var2value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var2desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var2desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var2desc [xx].c_str (), objectInfo.var2value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var3name [xx].size () > 1) {
					bShow = false;
					if (objectInfo.var3showFlag [xx] == 0)																bShow = true;
					if ((objectInfo.var3showFlag [xx] ==  1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var3showFlag [xx] == -1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var3showFlag [xx] ==  2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var3showFlag [xx] == -2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var3showFlag [xx] ==  4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var3showFlag [xx] == -4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var3showFlag [xx] ==  5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var3showFlag [xx] == -5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var3showFlag [xx] ==  6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var3showFlag [xx] == -6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var3showFlag [xx] ==  7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var3showFlag [xx] == -7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var3showFlag [xx] ==  8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var3showFlag [xx] == -8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var3showFlag [xx] ==  9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var3showFlag [xx] == -9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 0))	bShow = true;

					if (objectInfo.var3type [xx] == APIParT_Length) {
						length = atof (objectInfo.var3value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var3desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var3type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var3value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var3desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var3desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var3desc [xx].c_str (), objectInfo.var3value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var4name [xx].size () > 1) {
					bShow = false;
					if (objectInfo.var4showFlag [xx] == 0)																bShow = true;
					if ((objectInfo.var4showFlag [xx] ==  1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var4showFlag [xx] == -1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var4showFlag [xx] ==  2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var4showFlag [xx] == -2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var4showFlag [xx] ==  3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var4showFlag [xx] == -3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var4showFlag [xx] ==  5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var4showFlag [xx] == -5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var4showFlag [xx] ==  6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var4showFlag [xx] == -6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var4showFlag [xx] ==  7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var4showFlag [xx] == -7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var4showFlag [xx] ==  8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var4showFlag [xx] == -8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var4showFlag [xx] ==  9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var4showFlag [xx] == -9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 0))	bShow = true;

					if (objectInfo.var4type [xx] == APIParT_Length) {
						length = atof (objectInfo.var4value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var4desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var4type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var4value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var4desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var4desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var4desc [xx].c_str (), objectInfo.var4value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var5name [xx].size () > 1) {
					bShow = false;
					if (objectInfo.var5showFlag [xx] == 0)																bShow = true;
					if ((objectInfo.var5showFlag [xx] ==  1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var5showFlag [xx] == -1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var5showFlag [xx] ==  2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var5showFlag [xx] == -2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var5showFlag [xx] ==  3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var5showFlag [xx] == -3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var5showFlag [xx] ==  4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var5showFlag [xx] == -4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var5showFlag [xx] ==  6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var5showFlag [xx] == -6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var5showFlag [xx] ==  7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var5showFlag [xx] == -7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var5showFlag [xx] ==  8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var5showFlag [xx] == -8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var5showFlag [xx] ==  9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var5showFlag [xx] == -9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 0))	bShow = true;

					if (objectInfo.var5type [xx] == APIParT_Length) {
						length = atof (objectInfo.var5value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var5desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var5type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var5value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var5desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var5desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var5desc [xx].c_str (), objectInfo.var5value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var6name [xx].size () > 1) {
					bShow = false;
					if (objectInfo.var6showFlag [xx] == 0)																bShow = true;
					if ((objectInfo.var6showFlag [xx] ==  1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var6showFlag [xx] == -1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var6showFlag [xx] ==  2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var6showFlag [xx] == -2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var6showFlag [xx] ==  3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var6showFlag [xx] == -3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var6showFlag [xx] ==  4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var6showFlag [xx] == -4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var6showFlag [xx] ==  5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var6showFlag [xx] == -5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var6showFlag [xx] ==  7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var6showFlag [xx] == -7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var6showFlag [xx] ==  8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var6showFlag [xx] == -8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var6showFlag [xx] ==  9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var6showFlag [xx] == -9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 0))	bShow = true;

					if (objectInfo.var6type [xx] == APIParT_Length) {
						length = atof (objectInfo.var6value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var6desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var6type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var6value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var6desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var6desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var6desc [xx].c_str (), objectInfo.var6value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var7name [xx].size () > 1) {
					bShow = false;
					if (objectInfo.var7showFlag [xx] == 0)																bShow = true;
					if ((objectInfo.var7showFlag [xx] ==  1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var7showFlag [xx] == -1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var7showFlag [xx] ==  2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var7showFlag [xx] == -2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var7showFlag [xx] ==  3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var7showFlag [xx] == -3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var7showFlag [xx] ==  4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var7showFlag [xx] == -4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var7showFlag [xx] ==  5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var7showFlag [xx] == -5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var7showFlag [xx] ==  6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var7showFlag [xx] == -6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var7showFlag [xx] ==  8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var7showFlag [xx] == -8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var7showFlag [xx] ==  9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var7showFlag [xx] == -9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 0))	bShow = true;

					if (objectInfo.var7type [xx] == APIParT_Length) {
						length = atof (objectInfo.var7value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var7desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var7type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var7value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var7desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var7desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var7desc [xx].c_str (), objectInfo.var7value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var8name [xx].size () > 1) {
					bShow = false;
					if (objectInfo.var8showFlag [xx] == 0)																bShow = true;
					if ((objectInfo.var8showFlag [xx] ==  1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var8showFlag [xx] == -1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var8showFlag [xx] ==  2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var8showFlag [xx] == -2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var8showFlag [xx] ==  3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var8showFlag [xx] == -3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var8showFlag [xx] ==  4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var8showFlag [xx] == -4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var8showFlag [xx] ==  5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var8showFlag [xx] == -5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var8showFlag [xx] ==  6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var8showFlag [xx] == -6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var8showFlag [xx] ==  7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var8showFlag [xx] == -7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var8showFlag [xx] ==  9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var8showFlag [xx] == -9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 0))	bShow = true;

					if (objectInfo.var8type [xx] == APIParT_Length) {
						length = atof (objectInfo.var8value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var8desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var8type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var8value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var8desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var8desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var8desc [xx].c_str (), objectInfo.var8value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var9name [xx].size () > 1) {
					bShow = false;
					if (objectInfo.var9showFlag [xx] == 0)																bShow = true;
					if ((objectInfo.var9showFlag [xx] ==  1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var9showFlag [xx] == -1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var9showFlag [xx] ==  2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var9showFlag [xx] == -2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var9showFlag [xx] ==  3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var9showFlag [xx] == -3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var9showFlag [xx] ==  4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var9showFlag [xx] == -4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var9showFlag [xx] ==  5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var9showFlag [xx] == -5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var9showFlag [xx] ==  6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var9showFlag [xx] == -6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var9showFlag [xx] ==  7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var9showFlag [xx] == -7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var9showFlag [xx] ==  8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var9showFlag [xx] == -8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 0))	bShow = true;

					if (objectInfo.var9type [xx] == APIParT_Length) {
						length = atof (objectInfo.var9value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var9desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var9type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var9value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var9desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var9desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var9desc [xx].c_str (), objectInfo.var9value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var10name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var10type [xx] == APIParT_Length) {
						length = atof (objectInfo.var10value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var10desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var10type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var10value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var10desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var10desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var10desc [xx].c_str (), objectInfo.var10value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var11name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var11type [xx] == APIParT_Length) {
						length = atof (objectInfo.var11value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var11desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var11type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var11value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var11desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var11desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var11desc [xx].c_str (), objectInfo.var11value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var12name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var12type [xx] == APIParT_Length) {
						length = atof (objectInfo.var12value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var12desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var12type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var12value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var12desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var12desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var12desc [xx].c_str (), objectInfo.var12value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var13name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var13type [xx] == APIParT_Length) {
						length = atof (objectInfo.var13value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var13desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var13type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var13value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var13desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var13desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var13desc [xx].c_str (), objectInfo.var13value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var14name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var14type [xx] == APIParT_Length) {
						length = atof (objectInfo.var14value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var14desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var14type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var14value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var14desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var14desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var14desc [xx].c_str (), objectInfo.var14value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var15name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var15type [xx] == APIParT_Length) {
						length = atof (objectInfo.var15value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var15desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var15type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var15value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var15desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var15desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var15desc [xx].c_str (), objectInfo.var15value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var16name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var16type [xx] == APIParT_Length) {
						length = atof (objectInfo.var16value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var16desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var16type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var16value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var16desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var16desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var16desc [xx].c_str (), objectInfo.var16value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var17name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var17type [xx] == APIParT_Length) {
						length = atof (objectInfo.var17value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var17desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var17type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var17value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var17desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var17desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var17desc [xx].c_str (), objectInfo.var17value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var18name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var18type [xx] == APIParT_Length) {
						length = atof (objectInfo.var18value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var18desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var18type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var18value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var18desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var18desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var18desc [xx].c_str (), objectInfo.var18value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var19name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var19type [xx] == APIParT_Length) {
						length = atof (objectInfo.var19value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var19desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var19type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var19value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var19desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var19desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var19desc [xx].c_str (), objectInfo.var19value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var20name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var20type [xx] == APIParT_Length) {
						length = atof (objectInfo.var20value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var20desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var20type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var20value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var20desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var20desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var20desc [xx].c_str (), objectInfo.var20value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var21name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var21type [xx] == APIParT_Length) {
						length = atof (objectInfo.var21value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var21desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var21type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var21value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var21desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var21desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var21desc [xx].c_str (), objectInfo.var21value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var22name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var22type [xx] == APIParT_Length) {
						length = atof (objectInfo.var22value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var22desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var22type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var22value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var22desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var22desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var22desc [xx].c_str (), objectInfo.var22value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var23name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var23type [xx] == APIParT_Length) {
						length = atof (objectInfo.var23value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var23desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var23type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var23value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var23desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var23desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var23desc [xx].c_str (), objectInfo.var23value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var24name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var24type [xx] == APIParT_Length) {
						length = atof (objectInfo.var24value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var24desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var24type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var24value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var24desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var24desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var24desc [xx].c_str (), objectInfo.var24value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var25name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var25type [xx] == APIParT_Length) {
						length = atof (objectInfo.var25value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var25desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var25type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var25value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var25desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var25desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var25desc [xx].c_str (), objectInfo.var25value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var26name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var26type [xx] == APIParT_Length) {
						length = atof (objectInfo.var26value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var26desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var26type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var26value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var26desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var26desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var26desc [xx].c_str (), objectInfo.var26value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var27name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var27type [xx] == APIParT_Length) {
						length = atof (objectInfo.var27value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var27desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var27type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var27value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var27desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var27desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var27desc [xx].c_str (), objectInfo.var27value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var28name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var28type [xx] == APIParT_Length) {
						length = atof (objectInfo.var28value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var28desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var28type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var28value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var28desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var28desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var28desc [xx].c_str (), objectInfo.var28value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var29name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var29type [xx] == APIParT_Length) {
						length = atof (objectInfo.var29value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var29desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var29type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var29value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var29desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var29desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var29desc [xx].c_str (), objectInfo.var29value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var30name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var30type [xx] == APIParT_Length) {
						length = atof (objectInfo.var30value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var30desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var30type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var30value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var30desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var30desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var30desc [xx].c_str (), objectInfo.var30value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var31name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var31type [xx] == APIParT_Length) {
						length = atof (objectInfo.var31value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var31desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var31type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var31value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var31desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var31desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var31desc [xx].c_str (), objectInfo.var31value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var32name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var32type [xx] == APIParT_Length) {
						length = atof (objectInfo.var32value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var32desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var32type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var32value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var32desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var32desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var32desc [xx].c_str (), objectInfo.var32value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var33name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var33type [xx] == APIParT_Length) {
						length = atof (objectInfo.var33value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var33desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var33type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var33value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var33desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var33desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var33desc [xx].c_str (), objectInfo.var33value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var34name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var34type [xx] == APIParT_Length) {
						length = atof (objectInfo.var34value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var34desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var34type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var34value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var34desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var34desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var34desc [xx].c_str (), objectInfo.var34value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var35name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var35type [xx] == APIParT_Length) {
						length = atof (objectInfo.var35value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var35desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var35type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var35value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var35desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var35desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var35desc [xx].c_str (), objectInfo.var35value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var36name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var36type [xx] == APIParT_Length) {
						length = atof (objectInfo.var36value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var36desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var36type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var36value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var36desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var36desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var36desc [xx].c_str (), objectInfo.var36value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var37name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var37type [xx] == APIParT_Length) {
						length = atof (objectInfo.var37value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var37desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var37type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var37value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var37desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var37desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var37desc [xx].c_str (), objectInfo.var37value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var38name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var38type [xx] == APIParT_Length) {
						length = atof (objectInfo.var38value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var38desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var38type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var38value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var38desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var38desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var38desc [xx].c_str (), objectInfo.var38value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var39name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var39type [xx] == APIParT_Length) {
						length = atof (objectInfo.var39value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var39desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var39type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var39value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var39desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var39desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var39desc [xx].c_str (), objectInfo.var39value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var40name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var40type [xx] == APIParT_Length) {
						length = atof (objectInfo.var40value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var40desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var40type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var40value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var40desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var40desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var40desc [xx].c_str (), objectInfo.var40value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var41name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var41type [xx] == APIParT_Length) {
						length = atof (objectInfo.var41value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var41desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var41type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var41value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var41desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var41desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var41desc [xx].c_str (), objectInfo.var41value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var42name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var42type [xx] == APIParT_Length) {
						length = atof (objectInfo.var42value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var42desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var42type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var42value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var42desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var42desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var42desc [xx].c_str (), objectInfo.var42value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var43name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var43type [xx] == APIParT_Length) {
						length = atof (objectInfo.var43value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var43desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var43type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var43value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var43desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var43desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var43desc [xx].c_str (), objectInfo.var43value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var44name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var44type [xx] == APIParT_Length) {
						length = atof (objectInfo.var44value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var44desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var44type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var44value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var44desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var44desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var44desc [xx].c_str (), objectInfo.var44value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var45name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var45type [xx] == APIParT_Length) {
						length = atof (objectInfo.var45value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var45desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var45type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var45value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var45desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var45desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var45desc [xx].c_str (), objectInfo.var45value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var46name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var46type [xx] == APIParT_Length) {
						length = atof (objectInfo.var46value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var46desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var46type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var46value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var46desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var46desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var46desc [xx].c_str (), objectInfo.var46value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var47name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var47type [xx] == APIParT_Length) {
						length = atof (objectInfo.var47value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var47desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var47type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var47value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var47desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var47desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var47desc [xx].c_str (), objectInfo.var47value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var48name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var48type [xx] == APIParT_Length) {
						length = atof (objectInfo.var48value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var48desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var48type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var48value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var48desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var48desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var48desc [xx].c_str (), objectInfo.var48value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var49name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var49type [xx] == APIParT_Length) {
						length = atof (objectInfo.var49value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var49desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var49type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var49value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var49desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var49desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var49desc [xx].c_str (), objectInfo.var49value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
				if (objectInfo.var50name [xx].size () > 1) {
					bShow = true;

					if (objectInfo.var50type [xx] == APIParT_Length) {
						length = atof (objectInfo.var50value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var50desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var50type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var50value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var50desc [xx].c_str (), "��");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var50desc [xx].c_str (), "�ƴϿ�");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var50desc [xx].c_str (), objectInfo.var50value [xx][yy].c_str ());
					}
					if (bShow) fprintf (fp, buffer);
				}
			}

			// ���� ǥ��
			if (objectInfo.combinationCount [xx][yy] > 0) {
				sprintf (buffer, ": %d EA\n", objectInfo.combinationCount [xx][yy]);
				fprintf (fp, buffer);
			}
		}
	}

	// �Ϲ� ��� - ��
	for (xx = 0 ; xx < objectInfo.nCountsBeam ; ++xx) {
		if (xx == 0) {
			fprintf (fp, "\n[��]\n");
		}
		sprintf (buffer, "%d : %d EA\n", objectInfo.beamLength [xx], objectInfo.beamCount [xx]);
		fprintf (fp, buffer);
	}

	// �� �� ���� ��ü
	if (objectInfo.nUnknownObjects > 0) {
		sprintf (buffer, "\n�� �� ���� ��ü : %d EA\n", objectInfo.nUnknownObjects);
		fprintf (fp, buffer);
	}

	fclose (fp);

	// ȭ�� ���ΰ�ħ
	ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

	ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
	location.ToDisplayText (&resultString);
	sprintf (buffer, "������� ���� ��ġ�� �����߽��ϴ�.\n\n%s\n�Ǵ� ������Ʈ ������ �ִ� ����", resultString.ToCStr ().Get ());
	ACAPI_WriteReport (buffer, true);

	return	err;
}

// ������ ���� ���� �������� (Multi ���)
GSErrCode	exportElementInfoOnVisibleLayers (void)
{
	GSErrCode	err = NoError;
	short		xx, yy, zz;
	short		mm;
	bool		regenerate = true;
	short		result;

	// ��� ��ü���� ���� ��ǥ�� ���� ������
	vector<API_Coord3D>	vecPos;

	// ��� ��ü, �� ����
	GS::Array<API_Guid>		elemList;
	GS::Array<API_Guid>&	objects = GS::Array<API_Guid> ();
	GS::Array<API_Guid>&	beams = GS::Array<API_Guid> ();
	long					nObjects = 0;
	long					nBeams = 0;

	// ���̾� ���� ����
	short			nLayers;
	API_Attribute	attrib;
	short			nVisibleLayers = 0;
	short			visLayerList [1024];
	char			fullLayerName [128];

	// ���̾� Ÿ�Կ� ���� ĸ�� ���� ����
	char*			foundLayerName;
	short			layerType = UNDEFINED;

	// ��Ÿ
	char			buffer [256];
	char			filename [256];

	// �۾� �� ����
	API_StoryInfo	storyInfo;
	double			workLevel_object;		// ���� �۾� �� ����

	// ���� ���Ϸ� ��� ���� ��������
	// ���� ������ ���� ����
	API_SpecFolderID	specFolderID = API_ApplicationFolderID;
	IO::Location		location;
	GS::UniString		resultString;
	API_MiscAppInfo		miscAppInfo;
	FILE				*fp;
	FILE				*fp_unite;

	ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);

	// [DIALOG] ���̾�α׿��� ��ü �̹����� ĸ������ ���θ� ���
	result = DGAlert (DG_INFORMATION, "ĸ�� ���� ����", "ĸ�� �۾��� �����Ͻðڽ��ϱ�?", "", "��", "�ƴϿ�", "");

	// ������Ʈ �� ���̾� ������ �˾Ƴ�
	BNZeroMemory (&attrib, sizeof (API_Attribute));
	attrib.layer.head.typeID = API_LayerID;
	err = ACAPI_Attribute_GetNum (API_LayerID, &nLayers);

	// ���̴� ���̾���� ��� �����ϱ�
	for (xx = 1 ; xx <= nLayers ; ++xx) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = xx;
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			if (!((attrib.layer.head.flags & APILay_Hidden) == true)) {
				visLayerList [nVisibleLayers++] = attrib.layer.head.index;
			}
		}
	}

	// �Ͻ������� ��� ���̾� �����
	for (xx = 1 ; xx <= nLayers ; ++xx) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = xx;
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			attrib.layer.head.flags |= APILay_Hidden;
			ACAPI_Attribute_Modify (&attrib, NULL);
		}
	}

	ACAPI_Environment (APIEnv_GetMiscAppInfoID, &miscAppInfo);
	sprintf (filename, "%s - ������ ���� ���� (����).csv", miscAppInfo.caption);
	fp_unite = fopen (filename, "w+");

	if (fp_unite == NULL) {
		ACAPI_WriteReport ("���� ���� ���������� ���� �� �����ϴ�.", true);
		return	NoError;
	}

	// ���̴� ���̾���� �ϳ��� ��ȸ�ϸ鼭 ��ü ��ҵ��� ������ �� "������ ���� ���� ��������" ��ƾ ����
	for (mm = 1 ; mm <= nVisibleLayers ; ++mm) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = visLayerList [mm-1];
		err = ACAPI_Attribute_Get (&attrib);

		// �ʱ�ȭ
		objects.Clear ();
		beams.Clear ();
		vecPos.clear ();

		if (err == NoError) {
			// ���̾� ���̱�
			if ((attrib.layer.head.flags & APILay_Hidden) == true) {
				attrib.layer.head.flags ^= APILay_Hidden;
				ACAPI_Attribute_Modify (&attrib, NULL);
			}

			// ��� ��� ��������
			ACAPI_Element_GetElemList (API_ObjectID, &elemList, APIFilt_OnVisLayer);	// ���̴� ���̾ ����, ��ü Ÿ�Ը�
			while (elemList.GetSize () > 0) {
				objects.Push (elemList.Pop ());
			}

			ACAPI_Element_GetElemList (API_BeamID, &elemList, APIFilt_OnVisLayer);		// ���̴� ���̾ ����, �� Ÿ�Ը�
			while (elemList.GetSize () > 0) {
				beams.Push (elemList.Pop ());
			}
			nObjects = objects.GetSize ();
			nBeams = beams.GetSize ();

			if ((nObjects == 0) && (nBeams == 0))
				continue;

			// ���̾� �̸� ������
			sprintf (fullLayerName, "%s", attrib.layer.head.name);
			fullLayerName [strlen (fullLayerName)] = '\0';

			// ���̾� �̸� �ĺ��ϱ� (WALL: ��, SLAB: ������, COLU: ���, BEAM: ��, WLBM: ���纸)
			layerType = UNDEFINED;
			foundLayerName = strstr (fullLayerName, "WALL");
			if (foundLayerName != NULL)	layerType = WALL;
			foundLayerName = strstr (fullLayerName, "SLAB");
			if (foundLayerName != NULL)	layerType = SLAB;
			foundLayerName = strstr (fullLayerName, "COLU");
			if (foundLayerName != NULL)	layerType = COLU;
			foundLayerName = strstr (fullLayerName, "BEAM");
			if (foundLayerName != NULL)	layerType = BEAM;
			foundLayerName = strstr (fullLayerName, "WLBM");
			if (foundLayerName != NULL)	layerType = WLBM;

			// ��� ��ҵ��� 3D�� ������
			err = ACAPI_Automate (APIDo_ShowAllIn3DID);

			sprintf (filename, "%s - ������ ���� ����.csv", fullLayerName);
			fp = fopen (filename, "w+");

			if (fp == NULL) {
				sprintf (buffer, "���̾� %s�� ���ϸ��� �� �� �����Ƿ� �����մϴ�.", fullLayerName);
				ACAPI_WriteReport (buffer, true);
				continue;
			}

			// ���̾� �̸� (���� ��������)
			sprintf (buffer, "\n\n<< ���̾� : %s >>\n", fullLayerName);
			fprintf (fp_unite, buffer);

			// ������ ��ҵ��� ���� ����ϱ�
			API_Element			elem;
			API_ElementMemo		memo;
			bool				foundExistValue;

			SummaryOfObjectInfo		objectInfo;

			double			value_numeric [50];
			string			value_string [50];
			API_AddParID	value_type [50];
			char			tempStr [50];
			const char*		foundStr;
			bool			foundObject;

			for (xx = 0 ; xx < nObjects ; ++xx) {
				foundObject = false;

				BNZeroMemory (&elem, sizeof (API_Element));
				BNZeroMemory (&memo, sizeof (API_ElementMemo));
				elem.header.guid = objects.Pop ();
				err = ACAPI_Element_Get (&elem);
				err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

				// !!! ��ü�� ���� �����ϱ� ==================================
				API_Coord3D	coord;

				coord.x = elem.object.pos.x;
				coord.y = elem.object.pos.y;
				coord.z = elem.object.level;
					
				vecPos.push_back (coord);
				// !!! ��ü�� ���� �����ϱ� ==================================

				// �۾� �� ���� �ݿ� -- ��ü
				if (xx == 0) {
					BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
					workLevel_object = 0.0;
					ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
					for (yy = 0 ; yy < (storyInfo.lastStory - storyInfo.firstStory) ; ++yy) {
						if (storyInfo.data [0][yy].index == elem.header.floorInd) {
							workLevel_object = storyInfo.data [0][yy].level;
							break;
						}
					}
					BMKillHandle ((GSHandle *) &storyInfo.data);
				}

				// �Ķ���� ��ũ��Ʈ�� ������ �����Ŵ
				ACAPI_Goodies (APIAny_RunGDLParScriptID, &elem.header, 0);

				for (yy = 0 ; yy < objectInfo.nameKey.size () ; ++yy) {

					strcpy (tempStr, objectInfo.nameKey [yy].c_str ());
					foundStr = getParameterStringByName (&memo, tempStr);

					// ��ü ������ ã�Ҵٸ�,
					if (foundStr != NULL) {
						foundObject = true;

						if (my_strcmp (foundStr, objectInfo.nameVal [yy].c_str ()) == 0) {
							foundExistValue = false;

							value_string [0] = "";
							value_string [1] = "";
							value_string [2] = "";
							value_string [3] = "";
							value_string [4] = "";
							value_string [5] = "";
							value_string [6] = "";
							value_string [7] = "";
							value_string [8] = "";
							value_string [9] = "";
							value_string [10] = "";
							value_string [11] = "";
							value_string [12] = "";
							value_string [13] = "";
							value_string [14] = "";
							value_string [15] = "";
							value_string [16] = "";
							value_string [17] = "";
							value_string [18] = "";
							value_string [19] = "";
							value_string [20] = "";
							value_string [21] = "";
							value_string [22] = "";
							value_string [23] = "";
							value_string [24] = "";
							value_string [25] = "";
							value_string [26] = "";
							value_string [27] = "";
							value_string [28] = "";
							value_string [29] = "";
							value_string [30] = "";
							value_string [31] = "";
							value_string [32] = "";
							value_string [33] = "";
							value_string [34] = "";
							value_string [35] = "";
							value_string [36] = "";
							value_string [37] = "";
							value_string [38] = "";
							value_string [39] = "";
							value_string [40] = "";
							value_string [41] = "";
							value_string [42] = "";
							value_string [43] = "";
							value_string [44] = "";
							value_string [45] = "";
							value_string [46] = "";
							value_string [47] = "";
							value_string [48] = "";
							value_string [49] = "";

							// ���� 1
							strcpy (tempStr, objectInfo.var1name [yy].c_str ());
							value_type [0] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var1type [yy] = value_type [0];

							if ((value_type [0] != APIParT_Separator) || (value_type [0] != APIParT_Title) || (value_type [0] != API_ZombieParT)) {
								if (value_type [0] == APIParT_CString) {
									// ���ڿ�
									value_string [0] = getParameterStringByName (&memo, tempStr);
									value_numeric [0] = 0.0;
								} else {
									// ����
									value_numeric [0] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [0]);
									value_string [0] = tempStr;
								}
							}

							// ���� 2
							strcpy (tempStr, objectInfo.var2name [yy].c_str ());
							value_type [1] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var2type [yy] = value_type [1];

							if ((value_type [1] != APIParT_Separator) || (value_type [1] != APIParT_Title) || (value_type [1] != API_ZombieParT)) {
								if (value_type [1] == APIParT_CString) {
									// ���ڿ�
									value_string [1] = getParameterStringByName (&memo, tempStr);
									value_numeric [1] = 0.0;
								} else {
									// ����
									value_numeric [1] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [1]);
									value_string [1] = tempStr;
								}
							}

							// ���� 3
							strcpy (tempStr, objectInfo.var3name [yy].c_str ());
							value_type [2] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var3type [yy] = value_type [2];

							if ((value_type [2] != APIParT_Separator) || (value_type [2] != APIParT_Title) || (value_type [2] != API_ZombieParT)) {
								if (value_type [2] == APIParT_CString) {
									// ���ڿ�
									value_string [2] = getParameterStringByName (&memo, tempStr);
									value_numeric [2] = 0.0;
								} else {
									// ����
									value_numeric [2] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [2]);
									value_string [2] = tempStr;
								}
							}

							// ���� 4
							strcpy (tempStr, objectInfo.var4name [yy].c_str ());
							value_type [3] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var4type [yy] = value_type [3];

							if ((value_type [3] != APIParT_Separator) || (value_type [3] != APIParT_Title) || (value_type [3] != API_ZombieParT)) {
								if (value_type [3] == APIParT_CString) {
									// ���ڿ�
									value_string [3] = getParameterStringByName (&memo, tempStr);
									value_numeric [3] = 0.0;
								} else {
									// ����
									value_numeric [3] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [3]);
									value_string [3] = tempStr;
								}
							}

							// ���� 5
							strcpy (tempStr, objectInfo.var5name [yy].c_str ());
							value_type [4] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var5type [yy] = value_type [4];

							if ((value_type [4] != APIParT_Separator) || (value_type [4] != APIParT_Title) || (value_type [4] != API_ZombieParT)) {
								if (value_type [4] == APIParT_CString) {
									// ���ڿ�
									value_string [4] = getParameterStringByName (&memo, tempStr);
									value_numeric [4] = 0.0;
								} else {
									// ����
									value_numeric [4] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [4]);
									value_string [4] = tempStr;
								}
							}

							// ���� 6
							strcpy (tempStr, objectInfo.var6name [yy].c_str ());
							value_type [5] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var6type [yy] = value_type [5];

							if ((value_type [5] != APIParT_Separator) || (value_type [5] != APIParT_Title) || (value_type [5] != API_ZombieParT)) {
								if (value_type [5] == APIParT_CString) {
									// ���ڿ�
									value_string [5] = getParameterStringByName (&memo, tempStr);
									value_numeric [5] = 0.0;
								} else {
									// ����
									value_numeric [5] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [5]);
									value_string [5] = tempStr;
								}
							}

							// ���� 7
							strcpy (tempStr, objectInfo.var7name [yy].c_str ());
							value_type [6] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var7type [yy] = value_type [6];

							if ((value_type [6] != APIParT_Separator) || (value_type [6] != APIParT_Title) || (value_type [6] != API_ZombieParT)) {
								if (value_type [6] == APIParT_CString) {
									// ���ڿ�
									value_string [6] = getParameterStringByName (&memo, tempStr);
									value_numeric [6] = 0.0;
								} else {
									// ����
									value_numeric [6] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [6]);
									value_string [6] = tempStr;
								}
							}

							// ���� 8
							strcpy (tempStr, objectInfo.var8name [yy].c_str ());
							value_type [7] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var8type [yy] = value_type [7];

							if ((value_type [7] != APIParT_Separator) || (value_type [7] != APIParT_Title) || (value_type [7] != API_ZombieParT)) {
								if (value_type [7] == APIParT_CString) {
									// ���ڿ�
									value_string [7] = getParameterStringByName (&memo, tempStr);
									value_numeric [7] = 0.0;
								} else {
									// ����
									value_numeric [7] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [7]);
									value_string [7] = tempStr;
								}
							}

							// ���� 9
							strcpy (tempStr, objectInfo.var9name [yy].c_str ());
							value_type [8] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var9type [yy] = value_type [8];

							if ((value_type [8] != APIParT_Separator) || (value_type [8] != APIParT_Title) || (value_type [8] != API_ZombieParT)) {
								if (value_type [8] == APIParT_CString) {
									// ���ڿ�
									value_string [8] = getParameterStringByName (&memo, tempStr);
									value_numeric [8] = 0.0;
								} else {
									// ����
									value_numeric [8] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [8]);
									value_string [8] = tempStr;
								}
							}

							// ���� 10
							strcpy (tempStr, objectInfo.var10name [yy].c_str ());
							value_type [9] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var10type [yy] = value_type [9];

							if ((value_type [9] != APIParT_Separator) || (value_type [9] != APIParT_Title) || (value_type [9] != API_ZombieParT)) {
								if (value_type [9] == APIParT_CString) {
									// ���ڿ�
									value_string [9] = getParameterStringByName (&memo, tempStr);
									value_numeric [9] = 0.0;
								} else {
									// ����
									value_numeric [9] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [9]);
									value_string [9] = tempStr;
								}
							}

							// ���� 11
							strcpy (tempStr, objectInfo.var11name [yy].c_str ());
							value_type [10] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var11type [yy] = value_type [10];

							if ((value_type [10] != APIParT_Separator) || (value_type [10] != APIParT_Title) || (value_type [10] != API_ZombieParT)) {
								if (value_type [10] == APIParT_CString) {
									// ���ڿ�
									value_string [10] = getParameterStringByName (&memo, tempStr);
									value_numeric [10] = 0.0;
								} else {
									// ����
									value_numeric [10] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [10]);
									value_string [10] = tempStr;
								}
							}

							// ���� 12
							strcpy (tempStr, objectInfo.var12name [yy].c_str ());
							value_type [11] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var12type [yy] = value_type [11];

							if ((value_type [11] != APIParT_Separator) || (value_type [11] != APIParT_Title) || (value_type [11] != API_ZombieParT)) {
								if (value_type [11] == APIParT_CString) {
									// ���ڿ�
									value_string [11] = getParameterStringByName (&memo, tempStr);
									value_numeric [11] = 0.0;
								} else {
									// ����
									value_numeric [11] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [11]);
									value_string [11] = tempStr;
								}
							}

							// ���� 13
							strcpy (tempStr, objectInfo.var13name [yy].c_str ());
							value_type [12] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var13type [yy] = value_type [12];

							if ((value_type [12] != APIParT_Separator) || (value_type [12] != APIParT_Title) || (value_type [12] != API_ZombieParT)) {
								if (value_type [12] == APIParT_CString) {
									// ���ڿ�
									value_string [12] = getParameterStringByName (&memo, tempStr);
									value_numeric [12] = 0.0;
								} else {
									// ����
									value_numeric [12] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [12]);
									value_string [12] = tempStr;
								}
							}

							// ���� 14
							strcpy (tempStr, objectInfo.var14name [yy].c_str ());
							value_type [13] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var14type [yy] = value_type [13];

							if ((value_type [13] != APIParT_Separator) || (value_type [13] != APIParT_Title) || (value_type [13] != API_ZombieParT)) {
								if (value_type [13] == APIParT_CString) {
									// ���ڿ�
									value_string [13] = getParameterStringByName (&memo, tempStr);
									value_numeric [13] = 0.0;
								} else {
									// ����
									value_numeric [13] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [13]);
									value_string [13] = tempStr;
								}
							}

							// ���� 15
							strcpy (tempStr, objectInfo.var15name [yy].c_str ());
							value_type [14] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var15type [yy] = value_type [14];

							if ((value_type [14] != APIParT_Separator) || (value_type [14] != APIParT_Title) || (value_type [14] != API_ZombieParT)) {
								if (value_type [14] == APIParT_CString) {
									// ���ڿ�
									value_string [14] = getParameterStringByName (&memo, tempStr);
									value_numeric [14] = 0.0;
								} else {
									// ����
									value_numeric [14] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [14]);
									value_string [14] = tempStr;
								}
							}

							// ���� 16
							strcpy (tempStr, objectInfo.var16name [yy].c_str ());
							value_type [15] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var16type [yy] = value_type [15];

							if ((value_type [15] != APIParT_Separator) || (value_type [15] != APIParT_Title) || (value_type [15] != API_ZombieParT)) {
								if (value_type [15] == APIParT_CString) {
									// ���ڿ�
									value_string [15] = getParameterStringByName (&memo, tempStr);
									value_numeric [15] = 0.0;
								} else {
									// ����
									value_numeric [15] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [15]);
									value_string [15] = tempStr;
								}
							}

							// ���� 17
							strcpy (tempStr, objectInfo.var17name [yy].c_str ());
							value_type [16] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var17type [yy] = value_type [16];

							if ((value_type [16] != APIParT_Separator) || (value_type [16] != APIParT_Title) || (value_type [16] != API_ZombieParT)) {
								if (value_type [16] == APIParT_CString) {
									// ���ڿ�
									value_string [16] = getParameterStringByName (&memo, tempStr);
									value_numeric [16] = 0.0;
								} else {
									// ����
									value_numeric [16] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [16]);
									value_string [16] = tempStr;
								}
							}

							// ���� 18
							strcpy (tempStr, objectInfo.var18name [yy].c_str ());
							value_type [17] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var18type [yy] = value_type [17];

							if ((value_type [17] != APIParT_Separator) || (value_type [17] != APIParT_Title) || (value_type [17] != API_ZombieParT)) {
								if (value_type [17] == APIParT_CString) {
									// ���ڿ�
									value_string [17] = getParameterStringByName (&memo, tempStr);
									value_numeric [17] = 0.0;
								} else {
									// ����
									value_numeric [17] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [17]);
									value_string [17] = tempStr;
								}
							}

							// ���� 19
							strcpy (tempStr, objectInfo.var19name [yy].c_str ());
							value_type [18] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var19type [yy] = value_type [18];

							if ((value_type [18] != APIParT_Separator) || (value_type [18] != APIParT_Title) || (value_type [18] != API_ZombieParT)) {
								if (value_type [18] == APIParT_CString) {
									// ���ڿ�
									value_string [18] = getParameterStringByName (&memo, tempStr);
									value_numeric [18] = 0.0;
								} else {
									// ����
									value_numeric [18] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [18]);
									value_string [18] = tempStr;
								}
							}

							// ���� 20
							strcpy (tempStr, objectInfo.var20name [yy].c_str ());
							value_type [19] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var20type [yy] = value_type [19];

							if ((value_type [19] != APIParT_Separator) || (value_type [19] != APIParT_Title) || (value_type [19] != API_ZombieParT)) {
								if (value_type [19] == APIParT_CString) {
									// ���ڿ�
									value_string [19] = getParameterStringByName (&memo, tempStr);
									value_numeric [19] = 0.0;
								} else {
									// ����
									value_numeric [19] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [19]);
									value_string [19] = tempStr;
								}
							}

							// ���� 21
							strcpy (tempStr, objectInfo.var21name [yy].c_str ());
							value_type [20] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var21type [yy] = value_type [20];

							if ((value_type [20] != APIParT_Separator) || (value_type [20] != APIParT_Title) || (value_type [20] != API_ZombieParT)) {
								if (value_type [20] == APIParT_CString) {
									// ���ڿ�
									value_string [20] = getParameterStringByName (&memo, tempStr);
									value_numeric [20] = 0.0;
								} else {
									// ����
									value_numeric [20] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [20]);
									value_string [20] = tempStr;
								}
							}

							// ���� 22
							strcpy (tempStr, objectInfo.var22name [yy].c_str ());
							value_type [21] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var22type [yy] = value_type [21];

							if ((value_type [21] != APIParT_Separator) || (value_type [21] != APIParT_Title) || (value_type [21] != API_ZombieParT)) {
								if (value_type [21] == APIParT_CString) {
									// ���ڿ�
									value_string [21] = getParameterStringByName (&memo, tempStr);
									value_numeric [21] = 0.0;
								} else {
									// ����
									value_numeric [21] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [21]);
									value_string [21] = tempStr;
								}
							}

							// ���� 23
							strcpy (tempStr, objectInfo.var23name [yy].c_str ());
							value_type [22] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var23type [yy] = value_type [22];

							if ((value_type [22] != APIParT_Separator) || (value_type [22] != APIParT_Title) || (value_type [22] != API_ZombieParT)) {
								if (value_type [22] == APIParT_CString) {
									// ���ڿ�
									value_string [22] = getParameterStringByName (&memo, tempStr);
									value_numeric [22] = 0.0;
								} else {
									// ����
									value_numeric [22] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [22]);
									value_string [22] = tempStr;
								}
							}

							// ���� 24
							strcpy (tempStr, objectInfo.var24name [yy].c_str ());
							value_type [23] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var24type [yy] = value_type [23];

							if ((value_type [23] != APIParT_Separator) || (value_type [23] != APIParT_Title) || (value_type [23] != API_ZombieParT)) {
								if (value_type [23] == APIParT_CString) {
									// ���ڿ�
									value_string [23] = getParameterStringByName (&memo, tempStr);
									value_numeric [23] = 0.0;
								} else {
									// ����
									value_numeric [23] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [23]);
									value_string [23] = tempStr;
								}
							}

							// ���� 25
							strcpy (tempStr, objectInfo.var25name [yy].c_str ());
							value_type [24] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var25type [yy] = value_type [24];

							if ((value_type [24] != APIParT_Separator) || (value_type [24] != APIParT_Title) || (value_type [24] != API_ZombieParT)) {
								if (value_type [24] == APIParT_CString) {
									// ���ڿ�
									value_string [24] = getParameterStringByName (&memo, tempStr);
									value_numeric [24] = 0.0;
								} else {
									// ����
									value_numeric [24] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [24]);
									value_string [24] = tempStr;
								}
							}

							// ���� 26
							strcpy (tempStr, objectInfo.var26name [yy].c_str ());
							value_type [25] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var26type [yy] = value_type [25];

							if ((value_type [25] != APIParT_Separator) || (value_type [25] != APIParT_Title) || (value_type [25] != API_ZombieParT)) {
								if (value_type [25] == APIParT_CString) {
									// ���ڿ�
									value_string [25] = getParameterStringByName (&memo, tempStr);
									value_numeric [25] = 0.0;
								} else {
									// ����
									value_numeric [25] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [25]);
									value_string [25] = tempStr;
								}
							}

							// ���� 27
							strcpy (tempStr, objectInfo.var27name [yy].c_str ());
							value_type [26] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var27type [yy] = value_type [26];

							if ((value_type [26] != APIParT_Separator) || (value_type [26] != APIParT_Title) || (value_type [26] != API_ZombieParT)) {
								if (value_type [26] == APIParT_CString) {
									// ���ڿ�
									value_string [26] = getParameterStringByName (&memo, tempStr);
									value_numeric [26] = 0.0;
								} else {
									// ����
									value_numeric [26] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [26]);
									value_string [26] = tempStr;
								}
							}

							// ���� 28
							strcpy (tempStr, objectInfo.var28name [yy].c_str ());
							value_type [27] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var28type [yy] = value_type [27];

							if ((value_type [27] != APIParT_Separator) || (value_type [27] != APIParT_Title) || (value_type [27] != API_ZombieParT)) {
								if (value_type [27] == APIParT_CString) {
									// ���ڿ�
									value_string [27] = getParameterStringByName (&memo, tempStr);
									value_numeric [27] = 0.0;
								} else {
									// ����
									value_numeric [27] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [27]);
									value_string [27] = tempStr;
								}
							}

							// ���� 29
							strcpy (tempStr, objectInfo.var29name [yy].c_str ());
							value_type [28] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var29type [yy] = value_type [28];

							if ((value_type [28] != APIParT_Separator) || (value_type [28] != APIParT_Title) || (value_type [28] != API_ZombieParT)) {
								if (value_type [28] == APIParT_CString) {
									// ���ڿ�
									value_string [28] = getParameterStringByName (&memo, tempStr);
									value_numeric [28] = 0.0;
								} else {
									// ����
									value_numeric [28] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [28]);
									value_string [28] = tempStr;
								}
							}

							// ���� 30
							strcpy (tempStr, objectInfo.var30name [yy].c_str ());
							value_type [29] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var30type [yy] = value_type [29];

							if ((value_type [29] != APIParT_Separator) || (value_type [29] != APIParT_Title) || (value_type [29] != API_ZombieParT)) {
								if (value_type [29] == APIParT_CString) {
									// ���ڿ�
									value_string [29] = getParameterStringByName (&memo, tempStr);
									value_numeric [29] = 0.0;
								} else {
									// ����
									value_numeric [29] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [29]);
									value_string [29] = tempStr;
								}
							}

							// ���� 31
							strcpy (tempStr, objectInfo.var31name [yy].c_str ());
							value_type [30] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var31type [yy] = value_type [30];

							if ((value_type [30] != APIParT_Separator) || (value_type [30] != APIParT_Title) || (value_type [30] != API_ZombieParT)) {
								if (value_type [30] == APIParT_CString) {
									// ���ڿ�
									value_string [30] = getParameterStringByName (&memo, tempStr);
									value_numeric [30] = 0.0;
								} else {
									// ����
									value_numeric [30] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [30]);
									value_string [30] = tempStr;
								}
							}

							// ���� 32
							strcpy (tempStr, objectInfo.var32name [yy].c_str ());
							value_type [31] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var32type [yy] = value_type [31];

							if ((value_type [31] != APIParT_Separator) || (value_type [31] != APIParT_Title) || (value_type [31] != API_ZombieParT)) {
								if (value_type [31] == APIParT_CString) {
									// ���ڿ�
									value_string [31] = getParameterStringByName (&memo, tempStr);
									value_numeric [31] = 0.0;
								} else {
									// ����
									value_numeric [31] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [31]);
									value_string [31] = tempStr;
								}
							}

							// ���� 33
							strcpy (tempStr, objectInfo.var33name [yy].c_str ());
							value_type [32] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var33type [yy] = value_type [32];

							if ((value_type [32] != APIParT_Separator) || (value_type [32] != APIParT_Title) || (value_type [32] != API_ZombieParT)) {
								if (value_type [32] == APIParT_CString) {
									// ���ڿ�
									value_string [32] = getParameterStringByName (&memo, tempStr);
									value_numeric [32] = 0.0;
								} else {
									// ����
									value_numeric [32] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [32]);
									value_string [32] = tempStr;
								}
							}

							// ���� 34
							strcpy (tempStr, objectInfo.var34name [yy].c_str ());
							value_type [33] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var34type [yy] = value_type [33];

							if ((value_type [33] != APIParT_Separator) || (value_type [33] != APIParT_Title) || (value_type [33] != API_ZombieParT)) {
								if (value_type [33] == APIParT_CString) {
									// ���ڿ�
									value_string [33] = getParameterStringByName (&memo, tempStr);
									value_numeric [33] = 0.0;
								} else {
									// ����
									value_numeric [33] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [33]);
									value_string [33] = tempStr;
								}
							}

							// ���� 35
							strcpy (tempStr, objectInfo.var35name [yy].c_str ());
							value_type [34] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var35type [yy] = value_type [34];

							if ((value_type [34] != APIParT_Separator) || (value_type [34] != APIParT_Title) || (value_type [34] != API_ZombieParT)) {
								if (value_type [34] == APIParT_CString) {
									// ���ڿ�
									value_string [34] = getParameterStringByName (&memo, tempStr);
									value_numeric [34] = 0.0;
								} else {
									// ����
									value_numeric [34] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [34]);
									value_string [34] = tempStr;
								}
							}

							// ���� 36
							strcpy (tempStr, objectInfo.var36name [yy].c_str ());
							value_type [35] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var36type [yy] = value_type [35];

							if ((value_type [35] != APIParT_Separator) || (value_type [35] != APIParT_Title) || (value_type [35] != API_ZombieParT)) {
								if (value_type [35] == APIParT_CString) {
									// ���ڿ�
									value_string [35] = getParameterStringByName (&memo, tempStr);
									value_numeric [35] = 0.0;
								} else {
									// ����
									value_numeric [35] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [35]);
									value_string [35] = tempStr;
								}
							}

							// ���� 37
							strcpy (tempStr, objectInfo.var37name [yy].c_str ());
							value_type [36] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var37type [yy] = value_type [36];

							if ((value_type [36] != APIParT_Separator) || (value_type [36] != APIParT_Title) || (value_type [36] != API_ZombieParT)) {
								if (value_type [36] == APIParT_CString) {
									// ���ڿ�
									value_string [36] = getParameterStringByName (&memo, tempStr);
									value_numeric [36] = 0.0;
								} else {
									// ����
									value_numeric [36] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [36]);
									value_string [36] = tempStr;
								}
							}

							// ���� 38
							strcpy (tempStr, objectInfo.var38name [yy].c_str ());
							value_type [37] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var38type [yy] = value_type [37];

							if ((value_type [37] != APIParT_Separator) || (value_type [37] != APIParT_Title) || (value_type [37] != API_ZombieParT)) {
								if (value_type [37] == APIParT_CString) {
									// ���ڿ�
									value_string [37] = getParameterStringByName (&memo, tempStr);
									value_numeric [37] = 0.0;
								} else {
									// ����
									value_numeric [37] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [37]);
									value_string [37] = tempStr;
								}
							}

							// ���� 39
							strcpy (tempStr, objectInfo.var39name [yy].c_str ());
							value_type [38] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var39type [yy] = value_type [38];

							if ((value_type [38] != APIParT_Separator) || (value_type [38] != APIParT_Title) || (value_type [38] != API_ZombieParT)) {
								if (value_type [38] == APIParT_CString) {
									// ���ڿ�
									value_string [38] = getParameterStringByName (&memo, tempStr);
									value_numeric [38] = 0.0;
								} else {
									// ����
									value_numeric [38] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [38]);
									value_string [38] = tempStr;
								}
							}

							// ���� 40
							strcpy (tempStr, objectInfo.var40name [yy].c_str ());
							value_type [39] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var40type [yy] = value_type [39];

							if ((value_type [39] != APIParT_Separator) || (value_type [39] != APIParT_Title) || (value_type [39] != API_ZombieParT)) {
								if (value_type [39] == APIParT_CString) {
									// ���ڿ�
									value_string [39] = getParameterStringByName (&memo, tempStr);
									value_numeric [39] = 0.0;
								} else {
									// ����
									value_numeric [39] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [39]);
									value_string [39] = tempStr;
								}
							}

							// ���� 41
							strcpy (tempStr, objectInfo.var41name [yy].c_str ());
							value_type [40] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var41type [yy] = value_type [40];

							if ((value_type [40] != APIParT_Separator) || (value_type [40] != APIParT_Title) || (value_type [40] != API_ZombieParT)) {
								if (value_type [40] == APIParT_CString) {
									// ���ڿ�
									value_string [40] = getParameterStringByName (&memo, tempStr);
									value_numeric [40] = 0.0;
								} else {
									// ����
									value_numeric [40] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [40]);
									value_string [40] = tempStr;
								}
							}

							// ���� 42
							strcpy (tempStr, objectInfo.var42name [yy].c_str ());
							value_type [41] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var42type [yy] = value_type [41];

							if ((value_type [41] != APIParT_Separator) || (value_type [41] != APIParT_Title) || (value_type [41] != API_ZombieParT)) {
								if (value_type [41] == APIParT_CString) {
									// ���ڿ�
									value_string [41] = getParameterStringByName (&memo, tempStr);
									value_numeric [41] = 0.0;
								} else {
									// ����
									value_numeric [41] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [41]);
									value_string [41] = tempStr;
								}
							}

							// ���� 43
							strcpy (tempStr, objectInfo.var43name [yy].c_str ());
							value_type [42] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var43type [yy] = value_type [42];

							if ((value_type [42] != APIParT_Separator) || (value_type [42] != APIParT_Title) || (value_type [42] != API_ZombieParT)) {
								if (value_type [42] == APIParT_CString) {
									// ���ڿ�
									value_string [42] = getParameterStringByName (&memo, tempStr);
									value_numeric [42] = 0.0;
								} else {
									// ����
									value_numeric [42] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [42]);
									value_string [42] = tempStr;
								}
							}

							// ���� 44
							strcpy (tempStr, objectInfo.var44name [yy].c_str ());
							value_type [43] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var44type [yy] = value_type [43];

							if ((value_type [43] != APIParT_Separator) || (value_type [43] != APIParT_Title) || (value_type [43] != API_ZombieParT)) {
								if (value_type [43] == APIParT_CString) {
									// ���ڿ�
									value_string [43] = getParameterStringByName (&memo, tempStr);
									value_numeric [43] = 0.0;
								} else {
									// ����
									value_numeric [43] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [43]);
									value_string [43] = tempStr;
								}
							}

							// ���� 45
							strcpy (tempStr, objectInfo.var45name [yy].c_str ());
							value_type [44] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var45type [yy] = value_type [44];

							if ((value_type [44] != APIParT_Separator) || (value_type [44] != APIParT_Title) || (value_type [44] != API_ZombieParT)) {
								if (value_type [44] == APIParT_CString) {
									// ���ڿ�
									value_string [44] = getParameterStringByName (&memo, tempStr);
									value_numeric [44] = 0.0;
								} else {
									// ����
									value_numeric [44] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [44]);
									value_string [44] = tempStr;
								}
							}

							// ���� 46
							strcpy (tempStr, objectInfo.var46name [yy].c_str ());
							value_type [45] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var46type [yy] = value_type [45];

							if ((value_type [45] != APIParT_Separator) || (value_type [45] != APIParT_Title) || (value_type [45] != API_ZombieParT)) {
								if (value_type [45] == APIParT_CString) {
									// ���ڿ�
									value_string [45] = getParameterStringByName (&memo, tempStr);
									value_numeric [45] = 0.0;
								} else {
									// ����
									value_numeric [45] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [45]);
									value_string [45] = tempStr;
								}
							}

							// ���� 47
							strcpy (tempStr, objectInfo.var47name [yy].c_str ());
							value_type [46] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var47type [yy] = value_type [46];

							if ((value_type [46] != APIParT_Separator) || (value_type [46] != APIParT_Title) || (value_type [46] != API_ZombieParT)) {
								if (value_type [46] == APIParT_CString) {
									// ���ڿ�
									value_string [46] = getParameterStringByName (&memo, tempStr);
									value_numeric [46] = 0.0;
								} else {
									// ����
									value_numeric [46] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [46]);
									value_string [46] = tempStr;
								}
							}

							// ���� 48
							strcpy (tempStr, objectInfo.var48name [yy].c_str ());
							value_type [47] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var48type [yy] = value_type [47];

							if ((value_type [47] != APIParT_Separator) || (value_type [47] != APIParT_Title) || (value_type [47] != API_ZombieParT)) {
								if (value_type [47] == APIParT_CString) {
									// ���ڿ�
									value_string [47] = getParameterStringByName (&memo, tempStr);
									value_numeric [47] = 0.0;
								} else {
									// ����
									value_numeric [47] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [47]);
									value_string [47] = tempStr;
								}
							}

							// ���� 49
							strcpy (tempStr, objectInfo.var49name [yy].c_str ());
							value_type [48] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var49type [yy] = value_type [48];

							if ((value_type [48] != APIParT_Separator) || (value_type [48] != APIParT_Title) || (value_type [48] != API_ZombieParT)) {
								if (value_type [48] == APIParT_CString) {
									// ���ڿ�
									value_string [48] = getParameterStringByName (&memo, tempStr);
									value_numeric [48] = 0.0;
								} else {
									// ����
									value_numeric [48] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [48]);
									value_string [48] = tempStr;
								}
							}

							// ���� 50
							strcpy (tempStr, objectInfo.var50name [yy].c_str ());
							value_type [49] = getParameterTypeByName (&memo, tempStr);
							objectInfo.var50type [yy] = value_type [49];

							if ((value_type [49] != APIParT_Separator) || (value_type [49] != APIParT_Title) || (value_type [49] != API_ZombieParT)) {
								if (value_type [49] == APIParT_CString) {
									// ���ڿ�
									value_string [49] = getParameterStringByName (&memo, tempStr);
									value_numeric [49] = 0.0;
								} else {
									// ����
									value_numeric [49] = getParameterValueByName (&memo, tempStr);
									sprintf (tempStr, "%f", value_numeric [49]);
									value_string [49] = tempStr;
								}
							}

							// �ߺ� �׸��� ������ ����
							for (zz = 0 ; zz < objectInfo.nCounts [yy] ; ++zz) {
								if ((objectInfo.var1value [yy][zz].compare (value_string [0]) == 0) && (objectInfo.var2value [yy][zz].compare (value_string [1]) == 0) && (objectInfo.var3value [yy][zz].compare (value_string [2]) == 0) && 
									(objectInfo.var4value [yy][zz].compare (value_string [3]) == 0) && (objectInfo.var5value [yy][zz].compare (value_string [4]) == 0) && (objectInfo.var6value [yy][zz].compare (value_string [5]) == 0) && 
									(objectInfo.var7value [yy][zz].compare (value_string [6]) == 0) && (objectInfo.var8value [yy][zz].compare (value_string [7]) == 0) && (objectInfo.var9value [yy][zz].compare (value_string [8]) == 0) &&
									(objectInfo.var10value [yy][zz].compare (value_string [9]) == 0) && (objectInfo.var11value [yy][zz].compare (value_string [10]) == 0) && (objectInfo.var12value [yy][zz].compare (value_string [11]) == 0) &&
									(objectInfo.var13value [yy][zz].compare (value_string [12]) == 0) && (objectInfo.var14value [yy][zz].compare (value_string [13]) == 0) && (objectInfo.var15value [yy][zz].compare (value_string [14]) == 0) &&
									(objectInfo.var16value [yy][zz].compare (value_string [15]) == 0) && (objectInfo.var17value [yy][zz].compare (value_string [16]) == 0) && (objectInfo.var18value [yy][zz].compare (value_string [17]) == 0) &&
									(objectInfo.var19value [yy][zz].compare (value_string [18]) == 0) && (objectInfo.var20value [yy][zz].compare (value_string [19]) == 0) && (objectInfo.var21value [yy][zz].compare (value_string [20]) == 0) &&
									(objectInfo.var22value [yy][zz].compare (value_string [21]) == 0) && (objectInfo.var23value [yy][zz].compare (value_string [22]) == 0) && (objectInfo.var24value [yy][zz].compare (value_string [23]) == 0) &&
									(objectInfo.var25value [yy][zz].compare (value_string [24]) == 0) && (objectInfo.var26value [yy][zz].compare (value_string [25]) == 0) && (objectInfo.var27value [yy][zz].compare (value_string [26]) == 0) &&
									(objectInfo.var28value [yy][zz].compare (value_string [27]) == 0) && (objectInfo.var29value [yy][zz].compare (value_string [28]) == 0) && (objectInfo.var30value [yy][zz].compare (value_string [29]) == 0) &&
									(objectInfo.var31value [yy][zz].compare (value_string [30]) == 0) && (objectInfo.var32value [yy][zz].compare (value_string [31]) == 0) && (objectInfo.var33value [yy][zz].compare (value_string [32]) == 0) &&
									(objectInfo.var34value [yy][zz].compare (value_string [33]) == 0) && (objectInfo.var35value [yy][zz].compare (value_string [34]) == 0) && (objectInfo.var36value [yy][zz].compare (value_string [35]) == 0) &&
									(objectInfo.var37value [yy][zz].compare (value_string [36]) == 0) && (objectInfo.var38value [yy][zz].compare (value_string [37]) == 0) && (objectInfo.var39value [yy][zz].compare (value_string [38]) == 0) &&
									(objectInfo.var40value [yy][zz].compare (value_string [39]) == 0) && (objectInfo.var41value [yy][zz].compare (value_string [40]) == 0) && (objectInfo.var42value [yy][zz].compare (value_string [41]) == 0) &&
									(objectInfo.var43value [yy][zz].compare (value_string [42]) == 0) && (objectInfo.var44value [yy][zz].compare (value_string [43]) == 0) && (objectInfo.var45value [yy][zz].compare (value_string [44]) == 0) &&
									(objectInfo.var46value [yy][zz].compare (value_string [45]) == 0) && (objectInfo.var47value [yy][zz].compare (value_string [46]) == 0) && (objectInfo.var48value [yy][zz].compare (value_string [47]) == 0) &&
									(objectInfo.var49value [yy][zz].compare (value_string [48]) == 0) && (objectInfo.var50value [yy][zz].compare (value_string [49]) == 0)) {

										objectInfo.combinationCount [yy][zz] ++;
										foundExistValue = true;
										break;
								}
							}

							// �ű� �׸��̸�
							if (!foundExistValue) {
								objectInfo.var1value [yy][objectInfo.nCounts [yy]] = value_string [0];
								objectInfo.var2value [yy][objectInfo.nCounts [yy]] = value_string [1];
								objectInfo.var3value [yy][objectInfo.nCounts [yy]] = value_string [2];
								objectInfo.var4value [yy][objectInfo.nCounts [yy]] = value_string [3];
								objectInfo.var5value [yy][objectInfo.nCounts [yy]] = value_string [4];
								objectInfo.var6value [yy][objectInfo.nCounts [yy]] = value_string [5];
								objectInfo.var7value [yy][objectInfo.nCounts [yy]] = value_string [6];
								objectInfo.var8value [yy][objectInfo.nCounts [yy]] = value_string [7];
								objectInfo.var9value [yy][objectInfo.nCounts [yy]] = value_string [8];
								objectInfo.var10value [yy][objectInfo.nCounts [yy]] = value_string [9];
								objectInfo.var11value [yy][objectInfo.nCounts [yy]] = value_string [10];
								objectInfo.var12value [yy][objectInfo.nCounts [yy]] = value_string [11];
								objectInfo.var13value [yy][objectInfo.nCounts [yy]] = value_string [12];
								objectInfo.var14value [yy][objectInfo.nCounts [yy]] = value_string [13];
								objectInfo.var15value [yy][objectInfo.nCounts [yy]] = value_string [14];
								objectInfo.var16value [yy][objectInfo.nCounts [yy]] = value_string [15];
								objectInfo.var17value [yy][objectInfo.nCounts [yy]] = value_string [16];
								objectInfo.var18value [yy][objectInfo.nCounts [yy]] = value_string [17];
								objectInfo.var19value [yy][objectInfo.nCounts [yy]] = value_string [18];
								objectInfo.var20value [yy][objectInfo.nCounts [yy]] = value_string [19];
								objectInfo.var21value [yy][objectInfo.nCounts [yy]] = value_string [20];
								objectInfo.var22value [yy][objectInfo.nCounts [yy]] = value_string [21];
								objectInfo.var23value [yy][objectInfo.nCounts [yy]] = value_string [22];
								objectInfo.var24value [yy][objectInfo.nCounts [yy]] = value_string [23];
								objectInfo.var25value [yy][objectInfo.nCounts [yy]] = value_string [24];
								objectInfo.var26value [yy][objectInfo.nCounts [yy]] = value_string [25];
								objectInfo.var27value [yy][objectInfo.nCounts [yy]] = value_string [26];
								objectInfo.var28value [yy][objectInfo.nCounts [yy]] = value_string [27];
								objectInfo.var29value [yy][objectInfo.nCounts [yy]] = value_string [28];
								objectInfo.var30value [yy][objectInfo.nCounts [yy]] = value_string [29];
								objectInfo.var31value [yy][objectInfo.nCounts [yy]] = value_string [30];
								objectInfo.var32value [yy][objectInfo.nCounts [yy]] = value_string [31];
								objectInfo.var33value [yy][objectInfo.nCounts [yy]] = value_string [32];
								objectInfo.var34value [yy][objectInfo.nCounts [yy]] = value_string [33];
								objectInfo.var35value [yy][objectInfo.nCounts [yy]] = value_string [34];
								objectInfo.var36value [yy][objectInfo.nCounts [yy]] = value_string [35];
								objectInfo.var37value [yy][objectInfo.nCounts [yy]] = value_string [36];
								objectInfo.var38value [yy][objectInfo.nCounts [yy]] = value_string [37];
								objectInfo.var39value [yy][objectInfo.nCounts [yy]] = value_string [38];
								objectInfo.var40value [yy][objectInfo.nCounts [yy]] = value_string [39];
								objectInfo.var41value [yy][objectInfo.nCounts [yy]] = value_string [40];
								objectInfo.var42value [yy][objectInfo.nCounts [yy]] = value_string [41];
								objectInfo.var43value [yy][objectInfo.nCounts [yy]] = value_string [42];
								objectInfo.var44value [yy][objectInfo.nCounts [yy]] = value_string [43];
								objectInfo.var45value [yy][objectInfo.nCounts [yy]] = value_string [44];
								objectInfo.var46value [yy][objectInfo.nCounts [yy]] = value_string [45];
								objectInfo.var47value [yy][objectInfo.nCounts [yy]] = value_string [46];
								objectInfo.var48value [yy][objectInfo.nCounts [yy]] = value_string [47];
								objectInfo.var49value [yy][objectInfo.nCounts [yy]] = value_string [48];
								objectInfo.var50value [yy][objectInfo.nCounts [yy]] = value_string [49];
								objectInfo.combinationCount [yy][objectInfo.nCounts [yy]] = 1;
								objectInfo.nCounts [yy] ++;
							}
						}
					}
				}

				// ���� ã�� ���ϸ� �� �� ���� ��ü�� �����
				if (foundObject == false)
					objectInfo.nUnknownObjects ++;

				ACAPI_DisposeElemMemoHdls (&memo);
			}

			// �� ���� ����
			for (xx = 0 ; xx < nBeams ; ++xx) {
				BNZeroMemory (&elem, sizeof (API_Element));
				BNZeroMemory (&memo, sizeof (API_ElementMemo));
				elem.header.guid = beams.Pop ();
				err = ACAPI_Element_Get (&elem);
				err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

				foundExistValue = false;

				int len;

				len = static_cast<int> (round (GetDistance (elem.beam.begC, elem.beam.endC) * 1000, 0));

				// �ߺ� �׸��� ������ ����
				for (zz = 0 ; zz < objectInfo.nCountsBeam ; ++zz) {
					if (objectInfo.beamLength [zz] == len) {
						objectInfo.beamCount [zz] ++;
						foundExistValue = true;
						break;
					}
				}

				// �ű� �׸� �߰��ϰ� ������ ����
				if ( !foundExistValue ) {
					objectInfo.beamLength.push_back (len);
					objectInfo.beamCount.push_back (1);
					objectInfo.nCountsBeam ++;
				}

				ACAPI_DisposeElemMemoHdls (&memo);
			}

			// APIParT_Length�� ��� 1000�� ���ؼ� ǥ��
			// APIParT_Boolean�� ��� ��/�ƴϿ� ǥ��
			double	length, length2, length3;
			bool	bShow;

			for (xx = 0 ; xx < objectInfo.nKnownObjects ; ++xx) {
				for (yy = 0 ; yy < objectInfo.nCounts [xx] ; ++yy) {
					// ����
					if (yy == 0) {
						sprintf (buffer, "\n[%s]\n", objectInfo.nameVal [xx].c_str ());
						fprintf (fp, buffer);
						fprintf (fp_unite, buffer);
					}

					if (my_strcmp (objectInfo.nameVal [xx].c_str (), "������ ��ũ") == 0) {
						// ����
						if (my_strcmp (objectInfo.var2value [xx][yy].c_str (), "����") == 0) {
							sprintf (buffer, "���� / %s ", objectInfo.var1value [xx][yy].c_str ());

						// �簢
						} else {
							sprintf (buffer, "�簢 / %s ", objectInfo.var1value [xx][yy].c_str ());
						}
						fprintf (fp, buffer);
						fprintf (fp_unite, buffer);

					} else if (my_strcmp (objectInfo.nameVal [xx].c_str (), "������") == 0) {
						// �԰���
						if (atoi (objectInfo.var1value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s X %s ", objectInfo.var2value [xx][yy], objectInfo.var3value [xx][yy]);
						// ��԰���
						} else {
							// 4�� X 5��
							length = atof (objectInfo.var4value [xx][yy].c_str ());
							length2 = atof (objectInfo.var5value [xx][yy].c_str ());
							sprintf (buffer, "%.0f X %.0f ", round (length*1000, 0), round (length2*1000, 0));
						}
						fprintf (fp, buffer);
						fprintf (fp_unite, buffer);

					} else if (my_strcmp (objectInfo.nameVal [xx].c_str (), "��ƿ��") == 0) {
						// �԰���
						if (atoi (objectInfo.var1value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s X %s ", objectInfo.var2value [xx][yy], objectInfo.var3value [xx][yy]);
						// ��԰���
						} else {
							// 4�� X 5��
							length = atof (objectInfo.var4value [xx][yy].c_str ());
							length2 = atof (objectInfo.var5value [xx][yy].c_str ());
							sprintf (buffer, "%.0f X %.0f ", round (length*1000, 0), round (length2*1000, 0));
						}
						fprintf (fp, buffer);
						fprintf (fp_unite, buffer);

					} else if (my_strcmp (objectInfo.nameVal [xx].c_str (), "����") == 0) {
						length = atof (objectInfo.var1value [xx][yy].c_str ());
						length2 = atof (objectInfo.var2value [xx][yy].c_str ());
						length3 = atof (objectInfo.var3value [xx][yy].c_str ());
						sprintf (buffer, "%.0f X %.0f X %.0f ", round (length*1000, 0), round (length2*1000, 0), round (length3*1000, 0));
						fprintf (fp, buffer);
						fprintf (fp_unite, buffer);

					} else if (my_strcmp (objectInfo.nameVal [xx].c_str (), "����(�ٰ���)") == 0) {
						sprintf (buffer, "����(�ٰ���) ���� %s ", objectInfo.var1value [xx][yy].c_str ());
						fprintf (fp, buffer);
						fprintf (fp_unite, buffer);

						if (atoi (objectInfo.var2value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "(���� �ѱ���: %s) ", objectInfo.var3value [xx][yy].c_str ());
							fprintf (fp, buffer);
							fprintf (fp_unite, buffer);
						}

					} else if (my_strcmp (objectInfo.nameVal [xx].c_str (), "����") == 0) {
						if (my_strcmp (objectInfo.var1value [xx][yy].c_str (), "3x6 [910x1820]") == 0) {
							sprintf (buffer, "910 X 1820 X %s ", objectInfo.var2value [xx][yy].c_str ());
							fprintf (fp, buffer);
							fprintf (fp_unite, buffer);

							// ����Ʋ ON
							if (atoi (objectInfo.var5value [xx][yy].c_str ()) > 0) {
								sprintf (buffer, "(���� �ѱ���: %s) ", objectInfo.var6value [xx][yy].c_str ());
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						} else if (my_strcmp (objectInfo.var1value [xx][yy].c_str (), "4x8 [1220x2440]") == 0) {
							sprintf (buffer, "1220 X 2440 X %s ", objectInfo.var2value [xx][yy].c_str ());
							fprintf (fp, buffer);
							fprintf (fp_unite, buffer);

							// ����Ʋ ON
							if (atoi (objectInfo.var5value [xx][yy].c_str ()) > 0) {
								sprintf (buffer, "(���� �ѱ���: %s) ", objectInfo.var6value [xx][yy].c_str ());
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						} else if (my_strcmp (objectInfo.var1value [xx][yy].c_str (), "2x5 [606x1520]") == 0) {
							sprintf (buffer, "606 X 1520 X %s ", objectInfo.var2value [xx][yy].c_str ());
							fprintf (fp, buffer);
							fprintf (fp_unite, buffer);

							// ����Ʋ ON
							if (atoi (objectInfo.var5value [xx][yy].c_str ()) > 0) {
								sprintf (buffer, "(���� �ѱ���: %s) ", objectInfo.var6value [xx][yy].c_str ());
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						} else if (my_strcmp (objectInfo.var1value [xx][yy].c_str (), "2x6 [606x1820]") == 0) {
							sprintf (buffer, "606 X 1820 X %s ", objectInfo.var2value [xx][yy].c_str ());
							fprintf (fp, buffer);
							fprintf (fp_unite, buffer);

							// ����Ʋ ON
							if (atoi (objectInfo.var5value [xx][yy].c_str ()) > 0) {
								sprintf (buffer, "(���� �ѱ���: %s) ", objectInfo.var6value [xx][yy].c_str ());
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						} else if (my_strcmp (objectInfo.var1value [xx][yy].c_str (), "3x5 [910x1520]") == 0) {
							sprintf (buffer, "910 X 1520 X %s ", objectInfo.var2value [xx][yy].c_str ());
							fprintf (fp, buffer);
							fprintf (fp_unite, buffer);

							// ����Ʋ ON
							if (atoi (objectInfo.var5value [xx][yy].c_str ()) > 0) {
								sprintf (buffer, "(���� �ѱ���: %s) ", objectInfo.var6value [xx][yy].c_str ());
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						} else if (my_strcmp (objectInfo.var1value [xx][yy].c_str (), "��԰�") == 0) {
							// ���� X ���� X �β�
							length = atof (objectInfo.var3value [xx][yy].c_str ());
							length2 = atof (objectInfo.var4value [xx][yy].c_str ());
							sprintf (buffer, "%.0f X %.0f X %s ", round (length*1000, 0), round (length2*1000, 0), objectInfo.var2value [xx][yy].c_str ());
							fprintf (fp, buffer);
							fprintf (fp_unite, buffer);

							// ����Ʋ ON
							if (atoi (objectInfo.var5value [xx][yy].c_str ()) > 0) {
								sprintf (buffer, "(���� �ѱ���: %s) ", objectInfo.var6value [xx][yy].c_str ());
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						} else if (my_strcmp (objectInfo.var1value [xx][yy].c_str (), "������") == 0) {
							sprintf (buffer, "������ ");
							fprintf (fp, buffer);
							fprintf (fp_unite, buffer);

							// ����Ʋ ON
							if (atoi (objectInfo.var5value [xx][yy].c_str ()) > 0) {
								sprintf (buffer, "(���� �ѱ���: %s) ", objectInfo.var6value [xx][yy].c_str ());
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						} else {
							sprintf (buffer, "����(�ٰ���) ");
							fprintf (fp, buffer);
							fprintf (fp_unite, buffer);
						}

					} else if (my_strcmp (objectInfo.nameVal [xx].c_str (), "RS Push-Pull Props") == 0) {
						// ���̽� �÷���Ʈ ����
						if (atoi (objectInfo.var1value [xx][yy].c_str ()) == 1) {
							sprintf (buffer, "���̽� �÷���Ʈ(����) ");
						} else {
							sprintf (buffer, "���̽� �÷���Ʈ(����) ");
						}
						fprintf (fp, buffer);
						fprintf (fp_unite, buffer);

						// �԰�(���)
						sprintf (buffer, "�԰�(���): %s ", objectInfo.var2value [xx][yy].c_str ());
						fprintf (fp, buffer);
						fprintf (fp_unite, buffer);

						// �԰�(�Ϻ�) - ���û���
						if (atoi (objectInfo.var4value [xx][yy].c_str ()) == 1) {
							sprintf (buffer, "�԰�(�Ϻ�): %s ", objectInfo.var3value [xx][yy].c_str ());
						}
						fprintf (fp, buffer);
						fprintf (fp_unite, buffer);

					} else if (my_strcmp (objectInfo.nameVal [xx].c_str (), "�簢������") == 0) {
						// �簢������
						if (atoi (objectInfo.var1value [xx][yy].c_str ()) == 0) {
							length = atof (objectInfo.var2value [xx][yy].c_str ());
							sprintf (buffer, "50 x 50 x %.0f ", round (length*1000, 0));

						// ���簢������
						} else {
							length = atof (objectInfo.var2value [xx][yy].c_str ());
							sprintf (buffer, "%s x %.0f ", objectInfo.var1value [xx][yy].c_str (), round (length*1000, 0));
						}
						fprintf (fp, buffer);
						fprintf (fp_unite, buffer);

					} else if (my_strcmp (objectInfo.nameVal [xx].c_str (), "����������") == 0) {
						length = atof (objectInfo.var1value [xx][yy].c_str ());
						sprintf (buffer, "%.0f ", round (length*1000, 0));
						fprintf (fp, buffer);
						fprintf (fp_unite, buffer);

					} else if (my_strcmp (objectInfo.nameVal [xx].c_str (), "�ƿ��ڳʾޱ�") == 0) {
						length = atof (objectInfo.var1value [xx][yy].c_str ());
						sprintf (buffer, "%.0f ", round (length*1000, 0));
						fprintf (fp, buffer);
						fprintf (fp_unite, buffer);

					} else if (my_strcmp (objectInfo.nameVal [xx].c_str (), "������") == 0) {
						if (atoi (objectInfo.var2value [xx][yy].c_str ()) > 0) {
							length = atof (objectInfo.var1value [xx][yy].c_str ());
							length2 = atof (objectInfo.var5value [xx][yy].c_str ());
							sprintf (buffer, "%.0f, ���� %.0f", round (length*1000, 0), round (length2*1000, 0));
						} else {
							length = atof (objectInfo.var1value [xx][yy].c_str ());
							sprintf (buffer, "%.0f ", round (length*1000, 0));
						}
						fprintf (fp, buffer);
						fprintf (fp_unite, buffer);

					} else if (my_strcmp (objectInfo.nameVal [xx].c_str (), "�����") == 0) {
						sprintf (buffer, "%s ", objectInfo.var1value [xx][yy].c_str ());
						fprintf (fp, buffer);
						fprintf (fp_unite, buffer);

					} else if (my_strcmp (objectInfo.nameVal [xx].c_str (), "�� �ۿ���") == 0) {
						length = atof (objectInfo.var1value [xx][yy].c_str ());
						sprintf (buffer, "%.0f ", round (length*1000, 0));
						fprintf (fp, buffer);
						fprintf (fp_unite, buffer);

					} else if (my_strcmp (objectInfo.nameVal [xx].c_str (), "��������") == 0) {
						sprintf (buffer, "%s �� ", objectInfo.var1value [xx][yy].c_str ());
						fprintf (fp, buffer);
						fprintf (fp_unite, buffer);

					} else {
						// ������ �� ǥ��
						if (objectInfo.var1name [xx].size () > 1) {
							bShow = false;
							if (objectInfo.var1showFlag [xx] == 0)																bShow = true;
							if ((objectInfo.var1showFlag [xx] ==  2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var1showFlag [xx] == -2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var1showFlag [xx] ==  3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var1showFlag [xx] == -3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var1showFlag [xx] ==  4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var1showFlag [xx] == -4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var1showFlag [xx] ==  5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var1showFlag [xx] == -5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var1showFlag [xx] ==  6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var1showFlag [xx] == -6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var1showFlag [xx] ==  7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var1showFlag [xx] == -7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var1showFlag [xx] ==  8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var1showFlag [xx] == -8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var1showFlag [xx] ==  9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var1showFlag [xx] == -9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 0))	bShow = true;

							if (objectInfo.var1type [xx] == APIParT_Length) {
								length = atof (objectInfo.var1value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var1desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var1type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var1value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var1desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var1desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var1desc [xx].c_str (), objectInfo.var1value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var2name [xx].size () > 1) {
							bShow = false;
							if (objectInfo.var2showFlag [xx] == 0)																bShow = true;
							if ((objectInfo.var2showFlag [xx] ==  1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var2showFlag [xx] == -1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var2showFlag [xx] ==  3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var2showFlag [xx] == -3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var2showFlag [xx] ==  4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var2showFlag [xx] == -4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var2showFlag [xx] ==  5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var2showFlag [xx] == -5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var2showFlag [xx] ==  6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var2showFlag [xx] == -6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var2showFlag [xx] ==  7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var2showFlag [xx] == -7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var2showFlag [xx] ==  8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var2showFlag [xx] == -8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var2showFlag [xx] ==  9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var2showFlag [xx] == -9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 0))	bShow = true;

							if (objectInfo.var2type [xx] == APIParT_Length) {
								length = atof (objectInfo.var2value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var2desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var2type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var2value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var2desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var2desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var2desc [xx].c_str (), objectInfo.var2value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var3name [xx].size () > 1) {
							bShow = false;
							if (objectInfo.var3showFlag [xx] == 0)																bShow = true;
							if ((objectInfo.var3showFlag [xx] ==  1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var3showFlag [xx] == -1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var3showFlag [xx] ==  2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var3showFlag [xx] == -2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var3showFlag [xx] ==  4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var3showFlag [xx] == -4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var3showFlag [xx] ==  5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var3showFlag [xx] == -5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var3showFlag [xx] ==  6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var3showFlag [xx] == -6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var3showFlag [xx] ==  7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var3showFlag [xx] == -7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var3showFlag [xx] ==  8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var3showFlag [xx] == -8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var3showFlag [xx] ==  9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var3showFlag [xx] == -9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 0))	bShow = true;

							if (objectInfo.var3type [xx] == APIParT_Length) {
								length = atof (objectInfo.var3value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var3desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var3type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var3value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var3desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var3desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var3desc [xx].c_str (), objectInfo.var3value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var4name [xx].size () > 1) {
							bShow = false;
							if (objectInfo.var4showFlag [xx] == 0)																bShow = true;
							if ((objectInfo.var4showFlag [xx] ==  1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var4showFlag [xx] == -1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var4showFlag [xx] ==  2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var4showFlag [xx] == -2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var4showFlag [xx] ==  3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var4showFlag [xx] == -3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var4showFlag [xx] ==  5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var4showFlag [xx] == -5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var4showFlag [xx] ==  6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var4showFlag [xx] == -6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var4showFlag [xx] ==  7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var4showFlag [xx] == -7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var4showFlag [xx] ==  8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var4showFlag [xx] == -8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var4showFlag [xx] ==  9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var4showFlag [xx] == -9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 0))	bShow = true;

							if (objectInfo.var4type [xx] == APIParT_Length) {
								length = atof (objectInfo.var4value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var4desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var4type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var4value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var4desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var4desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var4desc [xx].c_str (), objectInfo.var4value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var5name [xx].size () > 1) {
							bShow = false;
							if (objectInfo.var5showFlag [xx] == 0)																bShow = true;
							if ((objectInfo.var5showFlag [xx] ==  1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var5showFlag [xx] == -1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var5showFlag [xx] ==  2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var5showFlag [xx] == -2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var5showFlag [xx] ==  3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var5showFlag [xx] == -3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var5showFlag [xx] ==  4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var5showFlag [xx] == -4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var5showFlag [xx] ==  6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var5showFlag [xx] == -6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var5showFlag [xx] ==  7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var5showFlag [xx] == -7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var5showFlag [xx] ==  8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var5showFlag [xx] == -8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var5showFlag [xx] ==  9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var5showFlag [xx] == -9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 0))	bShow = true;

							if (objectInfo.var5type [xx] == APIParT_Length) {
								length = atof (objectInfo.var5value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var5desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var5type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var5value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var5desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var5desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var5desc [xx].c_str (), objectInfo.var5value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var6name [xx].size () > 1) {
							bShow = false;
							if (objectInfo.var6showFlag [xx] == 0)																bShow = true;
							if ((objectInfo.var6showFlag [xx] ==  1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var6showFlag [xx] == -1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var6showFlag [xx] ==  2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var6showFlag [xx] == -2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var6showFlag [xx] ==  3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var6showFlag [xx] == -3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var6showFlag [xx] ==  4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var6showFlag [xx] == -4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var6showFlag [xx] ==  5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var6showFlag [xx] == -5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var6showFlag [xx] ==  7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var6showFlag [xx] == -7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var6showFlag [xx] ==  8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var6showFlag [xx] == -8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var6showFlag [xx] ==  9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var6showFlag [xx] == -9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 0))	bShow = true;

							if (objectInfo.var6type [xx] == APIParT_Length) {
								length = atof (objectInfo.var6value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var6desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var6type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var6value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var6desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var6desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var6desc [xx].c_str (), objectInfo.var6value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var7name [xx].size () > 1) {
							bShow = false;
							if (objectInfo.var7showFlag [xx] == 0)																bShow = true;
							if ((objectInfo.var7showFlag [xx] ==  1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var7showFlag [xx] == -1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var7showFlag [xx] ==  2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var7showFlag [xx] == -2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var7showFlag [xx] ==  3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var7showFlag [xx] == -3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var7showFlag [xx] ==  4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var7showFlag [xx] == -4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var7showFlag [xx] ==  5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var7showFlag [xx] == -5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var7showFlag [xx] ==  6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var7showFlag [xx] == -6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var7showFlag [xx] ==  8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var7showFlag [xx] == -8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var7showFlag [xx] ==  9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var7showFlag [xx] == -9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 0))	bShow = true;

							if (objectInfo.var7type [xx] == APIParT_Length) {
								length = atof (objectInfo.var7value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var7desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var7type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var7value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var7desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var7desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var7desc [xx].c_str (), objectInfo.var7value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var8name [xx].size () > 1) {
							bShow = false;
							if (objectInfo.var8showFlag [xx] == 0)																bShow = true;
							if ((objectInfo.var8showFlag [xx] ==  1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var8showFlag [xx] == -1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var8showFlag [xx] ==  2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var8showFlag [xx] == -2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var8showFlag [xx] ==  3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var8showFlag [xx] == -3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var8showFlag [xx] ==  4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var8showFlag [xx] == -4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var8showFlag [xx] ==  5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var8showFlag [xx] == -5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var8showFlag [xx] ==  6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var8showFlag [xx] == -6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var8showFlag [xx] ==  7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var8showFlag [xx] == -7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var8showFlag [xx] ==  9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var8showFlag [xx] == -9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 0))	bShow = true;

							if (objectInfo.var8type [xx] == APIParT_Length) {
								length = atof (objectInfo.var8value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var8desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var8type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var8value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var8desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var8desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var8desc [xx].c_str (), objectInfo.var8value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var9name [xx].size () > 1) {
							bShow = false;
							if (objectInfo.var9showFlag [xx] == 0)																bShow = true;
							if ((objectInfo.var9showFlag [xx] ==  1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var9showFlag [xx] == -1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var9showFlag [xx] ==  2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var9showFlag [xx] == -2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var9showFlag [xx] ==  3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var9showFlag [xx] == -3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var9showFlag [xx] ==  4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var9showFlag [xx] == -4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var9showFlag [xx] ==  5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var9showFlag [xx] == -5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var9showFlag [xx] ==  6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var9showFlag [xx] == -6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var9showFlag [xx] ==  7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var9showFlag [xx] == -7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 0))	bShow = true;
							if ((objectInfo.var9showFlag [xx] ==  8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 1))	bShow = true;
							if ((objectInfo.var9showFlag [xx] == -8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 0))	bShow = true;

							if (objectInfo.var9type [xx] == APIParT_Length) {
								length = atof (objectInfo.var9value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var9desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var9type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var9value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var9desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var9desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var9desc [xx].c_str (), objectInfo.var9value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var10name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var10type [xx] == APIParT_Length) {
								length = atof (objectInfo.var10value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var10desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var10type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var10value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var10desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var10desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var10desc [xx].c_str (), objectInfo.var10value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var11name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var11type [xx] == APIParT_Length) {
								length = atof (objectInfo.var11value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var11desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var11type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var11value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var11desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var11desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var11desc [xx].c_str (), objectInfo.var11value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var12name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var12type [xx] == APIParT_Length) {
								length = atof (objectInfo.var12value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var12desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var12type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var12value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var12desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var12desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var12desc [xx].c_str (), objectInfo.var12value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var13name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var13type [xx] == APIParT_Length) {
								length = atof (objectInfo.var13value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var13desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var13type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var13value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var13desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var13desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var13desc [xx].c_str (), objectInfo.var13value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var14name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var14type [xx] == APIParT_Length) {
								length = atof (objectInfo.var14value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var14desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var14type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var14value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var14desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var14desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var14desc [xx].c_str (), objectInfo.var14value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var15name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var15type [xx] == APIParT_Length) {
								length = atof (objectInfo.var15value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var15desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var15type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var15value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var15desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var15desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var15desc [xx].c_str (), objectInfo.var15value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var16name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var16type [xx] == APIParT_Length) {
								length = atof (objectInfo.var16value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var16desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var16type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var16value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var16desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var16desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var16desc [xx].c_str (), objectInfo.var16value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var17name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var17type [xx] == APIParT_Length) {
								length = atof (objectInfo.var17value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var17desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var17type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var17value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var17desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var17desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var17desc [xx].c_str (), objectInfo.var17value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var18name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var18type [xx] == APIParT_Length) {
								length = atof (objectInfo.var18value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var18desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var18type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var18value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var18desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var18desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var18desc [xx].c_str (), objectInfo.var18value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var19name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var19type [xx] == APIParT_Length) {
								length = atof (objectInfo.var19value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var19desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var19type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var19value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var19desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var19desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var19desc [xx].c_str (), objectInfo.var19value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var20name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var20type [xx] == APIParT_Length) {
								length = atof (objectInfo.var20value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var20desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var20type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var20value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var20desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var20desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var20desc [xx].c_str (), objectInfo.var20value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var21name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var21type [xx] == APIParT_Length) {
								length = atof (objectInfo.var21value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var21desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var21type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var21value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var21desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var21desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var21desc [xx].c_str (), objectInfo.var21value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var22name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var22type [xx] == APIParT_Length) {
								length = atof (objectInfo.var22value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var22desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var22type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var22value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var22desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var22desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var22desc [xx].c_str (), objectInfo.var22value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var23name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var23type [xx] == APIParT_Length) {
								length = atof (objectInfo.var23value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var23desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var23type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var23value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var23desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var23desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var23desc [xx].c_str (), objectInfo.var23value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var24name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var24type [xx] == APIParT_Length) {
								length = atof (objectInfo.var24value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var24desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var24type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var24value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var24desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var24desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var24desc [xx].c_str (), objectInfo.var24value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var25name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var25type [xx] == APIParT_Length) {
								length = atof (objectInfo.var25value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var25desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var25type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var25value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var25desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var25desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var25desc [xx].c_str (), objectInfo.var25value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var26name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var26type [xx] == APIParT_Length) {
								length = atof (objectInfo.var26value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var26desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var26type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var26value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var26desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var26desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var26desc [xx].c_str (), objectInfo.var26value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var27name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var27type [xx] == APIParT_Length) {
								length = atof (objectInfo.var27value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var27desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var27type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var27value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var27desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var27desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var27desc [xx].c_str (), objectInfo.var27value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var28name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var28type [xx] == APIParT_Length) {
								length = atof (objectInfo.var28value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var28desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var28type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var28value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var28desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var28desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var28desc [xx].c_str (), objectInfo.var28value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var29name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var29type [xx] == APIParT_Length) {
								length = atof (objectInfo.var29value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var29desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var29type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var29value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var29desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var29desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var29desc [xx].c_str (), objectInfo.var29value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var30name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var30type [xx] == APIParT_Length) {
								length = atof (objectInfo.var30value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var30desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var30type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var30value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var30desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var30desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var30desc [xx].c_str (), objectInfo.var30value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var31name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var31type [xx] == APIParT_Length) {
								length = atof (objectInfo.var31value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var31desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var31type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var31value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var31desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var31desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var31desc [xx].c_str (), objectInfo.var31value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var32name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var32type [xx] == APIParT_Length) {
								length = atof (objectInfo.var32value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var32desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var32type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var32value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var32desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var32desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var32desc [xx].c_str (), objectInfo.var32value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var33name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var33type [xx] == APIParT_Length) {
								length = atof (objectInfo.var33value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var33desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var33type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var33value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var33desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var33desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var33desc [xx].c_str (), objectInfo.var33value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var34name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var34type [xx] == APIParT_Length) {
								length = atof (objectInfo.var34value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var34desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var34type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var34value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var34desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var34desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var34desc [xx].c_str (), objectInfo.var34value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var35name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var35type [xx] == APIParT_Length) {
								length = atof (objectInfo.var35value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var35desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var35type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var35value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var35desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var35desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var35desc [xx].c_str (), objectInfo.var35value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var36name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var36type [xx] == APIParT_Length) {
								length = atof (objectInfo.var36value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var36desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var36type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var36value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var36desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var36desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var36desc [xx].c_str (), objectInfo.var36value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var37name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var37type [xx] == APIParT_Length) {
								length = atof (objectInfo.var37value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var37desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var37type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var37value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var37desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var37desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var37desc [xx].c_str (), objectInfo.var37value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var38name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var38type [xx] == APIParT_Length) {
								length = atof (objectInfo.var38value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var38desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var38type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var38value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var38desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var38desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var38desc [xx].c_str (), objectInfo.var38value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var39name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var39type [xx] == APIParT_Length) {
								length = atof (objectInfo.var39value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var39desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var39type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var39value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var39desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var39desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var39desc [xx].c_str (), objectInfo.var39value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var40name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var40type [xx] == APIParT_Length) {
								length = atof (objectInfo.var40value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var40desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var40type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var40value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var40desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var40desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var40desc [xx].c_str (), objectInfo.var40value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var41name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var41type [xx] == APIParT_Length) {
								length = atof (objectInfo.var41value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var41desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var41type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var41value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var41desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var41desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var41desc [xx].c_str (), objectInfo.var41value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var42name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var42type [xx] == APIParT_Length) {
								length = atof (objectInfo.var42value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var42desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var42type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var42value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var42desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var42desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var42desc [xx].c_str (), objectInfo.var42value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var43name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var43type [xx] == APIParT_Length) {
								length = atof (objectInfo.var43value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var43desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var43type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var43value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var43desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var43desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var43desc [xx].c_str (), objectInfo.var43value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var44name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var44type [xx] == APIParT_Length) {
								length = atof (objectInfo.var44value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var44desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var44type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var44value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var44desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var44desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var44desc [xx].c_str (), objectInfo.var44value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var45name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var45type [xx] == APIParT_Length) {
								length = atof (objectInfo.var45value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var45desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var45type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var45value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var45desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var45desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var45desc [xx].c_str (), objectInfo.var45value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var46name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var46type [xx] == APIParT_Length) {
								length = atof (objectInfo.var46value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var46desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var46type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var46value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var46desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var46desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var46desc [xx].c_str (), objectInfo.var46value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var47name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var47type [xx] == APIParT_Length) {
								length = atof (objectInfo.var47value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var47desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var47type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var47value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var47desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var47desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var47desc [xx].c_str (), objectInfo.var47value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var48name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var48type [xx] == APIParT_Length) {
								length = atof (objectInfo.var48value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var48desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var48type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var48value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var48desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var48desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var48desc [xx].c_str (), objectInfo.var48value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var49name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var49type [xx] == APIParT_Length) {
								length = atof (objectInfo.var49value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var49desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var49type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var49value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var49desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var49desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var49desc [xx].c_str (), objectInfo.var49value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
						if (objectInfo.var50name [xx].size () > 1) {
							bShow = true;

							if (objectInfo.var50type [xx] == APIParT_Length) {
								length = atof (objectInfo.var50value [xx][yy].c_str ());
								sprintf (buffer, "%s(%.0f) ", objectInfo.var50desc [xx].c_str (), round (length*1000, 0));
							} else if (objectInfo.var50type [xx] == APIParT_Boolean) {
								if (atoi (objectInfo.var50value [xx][yy].c_str ()) > 0) {
									sprintf (buffer, "%s(%s) ", objectInfo.var50desc [xx].c_str (), "��");
								} else {
									sprintf (buffer, "%s(%s) ", objectInfo.var50desc [xx].c_str (), "�ƴϿ�");
								}
							} else {
								sprintf (buffer, "%s(%s) ", objectInfo.var50desc [xx].c_str (), objectInfo.var50value [xx][yy].c_str ());
							}
							if (bShow) {
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}
					}

					// ���� ǥ��
					if (objectInfo.combinationCount [xx][yy] > 0) {
						sprintf (buffer, ": %d EA\n", objectInfo.combinationCount [xx][yy]);
						fprintf (fp, buffer);
						fprintf (fp_unite, buffer);
					}
				}
			}

			// �Ϲ� ��� - ��
			for (xx = 0 ; xx < objectInfo.nCountsBeam ; ++xx) {
				if (xx == 0) {
					sprintf (buffer, "\n[��]\n");
					fprintf (fp, buffer);
					fprintf (fp_unite, buffer);
				}
				sprintf (buffer, "%d : %d EA\n", objectInfo.beamLength [xx], objectInfo.beamCount [xx]);
				fprintf (fp, buffer);
				fprintf (fp_unite, buffer);
			}

			// �� �� ���� ��ü
			if (objectInfo.nUnknownObjects > 0) {
				sprintf (buffer, "\n�� �� ���� ��ü : %d EA\n", objectInfo.nUnknownObjects);
				fprintf (fp, buffer);
				fprintf (fp_unite, buffer);
			}

			fclose (fp);

			// !!! 3D ���� ���� ==================================
			if (result == DG_OK) {
				API_3DProjectionInfo  proj3DInfo_beforeCapture;

				BNZeroMemory (&proj3DInfo_beforeCapture, sizeof (API_3DProjectionInfo));
				err = ACAPI_Environment (APIEnv_Get3DProjectionSetsID, &proj3DInfo_beforeCapture, NULL);


				API_3DProjectionInfo  proj3DInfo;

				BNZeroMemory (&proj3DInfo, sizeof (API_3DProjectionInfo));
				err = ACAPI_Environment (APIEnv_Get3DProjectionSetsID, &proj3DInfo, NULL);

				double	lowestZ, highestZ, cameraZ, targetZ;	// ���� ���� ����, ���� ���� ����, ī�޶� �� ��� ����
				API_Coord3D		p1, p2, p3, p4, p5;				// �� ��ǥ ����
				double	distanceOfPoints;						// �� �� ���� �Ÿ�
				double	angleOfPoints;							// �� �� ���� ����
				API_Coord3D		camPos1, camPos2;				// ī�޶� ���� �� �ִ� �� 2��

				API_FileSavePars		fsp;			// ���� ������ ���� ����
				API_SavePars_Picture	pars_pict;		// �׸� ���Ͽ� ���� ����

				if (err == NoError && proj3DInfo.isPersp) {
					// �� Ÿ�� ���̾��� ���
					if (layerType == WALL) {
						// ���� ���� ������ ����
						// ���� ���� x���� ���� �� p1�� ã�Ƴ�
						lowestZ = highestZ = vecPos [0].z;
						p1 = vecPos [0];
						for (xx = 1 ; xx < vecPos.size () ; ++xx) {
							if (lowestZ > vecPos [xx].z)	lowestZ = vecPos [xx].z;
							if (highestZ < vecPos [xx].z)	highestZ = vecPos [xx].z;

							if (vecPos [xx].x < p1.x)	p1 = vecPos [xx];
						}
						cameraZ = (highestZ - lowestZ)/2 + workLevel_object;

						distanceOfPoints = 0.0;
						for (xx = 0 ; xx < vecPos.size () ; ++xx) {
							if (distanceOfPoints < GetDistance (p1, vecPos [xx])) {
								distanceOfPoints = GetDistance (p1, vecPos [xx]);
								p2 = vecPos [xx];
							}
						}

						// �� ��(p1, p2) ���� ���� ���ϱ�
						angleOfPoints = RadToDegree (atan2 ((p2.y - p1.y), (p2.x - p1.x)));

						// ī�޶�� ����� ���� �� �ִ� ��ġ 2���� ã��
						camPos1 = p1;
						moveIn3D ('x', DegreeToRad (angleOfPoints), distanceOfPoints/2, &camPos1.x, &camPos1.y, &camPos1.z);
						if (distanceOfPoints > (highestZ - lowestZ))
							moveIn3D ('y', DegreeToRad (angleOfPoints), -distanceOfPoints * 1.5, &camPos1.x, &camPos1.y, &camPos1.z);
						else
							moveIn3D ('y', DegreeToRad (angleOfPoints), -(highestZ - lowestZ) * 1.5, &camPos1.x, &camPos1.y, &camPos1.z);
						camPos2 = p1;
						moveIn3D ('x', DegreeToRad (angleOfPoints), distanceOfPoints/2, &camPos2.x, &camPos2.y, &camPos2.z);
						if (distanceOfPoints > (highestZ - lowestZ))
							moveIn3D ('y', DegreeToRad (angleOfPoints), distanceOfPoints * 1.5, &camPos2.x, &camPos2.y, &camPos2.z);
						else
							moveIn3D ('y', DegreeToRad (angleOfPoints), (highestZ - lowestZ) * 1.5, &camPos2.x, &camPos2.y, &camPos2.z);

						camPos1.z = cameraZ;
						camPos2.z = cameraZ;

						// ========== 1��° ĸ��
						// ī�޶� �� ��� ��ġ ����
						proj3DInfo.isPersp = true;				// �۽���Ƽ�� ��
						proj3DInfo.u.persp.viewCone = 90.0;		// ī�޶� �þ߰�
						proj3DInfo.u.persp.rollAngle = 0.0;		// ī�޶� �� ����
						proj3DInfo.u.persp.azimuth = angleOfPoints + 90.0;	// ī�޶� ������

						proj3DInfo.u.persp.pos.x = camPos1.x;
						proj3DInfo.u.persp.pos.y = camPos1.y;
						proj3DInfo.u.persp.cameraZ = camPos1.z;

						proj3DInfo.u.persp.target.x = camPos2.x;
						proj3DInfo.u.persp.target.y = camPos2.y;
						proj3DInfo.u.persp.targetZ = camPos2.z;

						err = ACAPI_Environment (APIEnv_Change3DProjectionSetsID, &proj3DInfo, NULL, NULL);

						// ȭ�� ���ΰ�ħ
						ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
						ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

						// ȭ�� ĸ��
						ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
						BNZeroMemory (&fsp, sizeof (API_FileSavePars));
						fsp.fileTypeID = APIFType_PNGFile;
						sprintf (filename, "%s - ĸ�� (1).png", fullLayerName);
						fsp.file = new IO::Location (location, IO::Name (filename));

						BNZeroMemory (&pars_pict, sizeof (API_SavePars_Picture));
						pars_pict.colorDepth	= APIColorDepth_TC24;
						pars_pict.dithered		= false;
						pars_pict.view2D		= false;
						pars_pict.crop			= false;
						err = ACAPI_Automate (APIDo_SaveID, &fsp, &pars_pict);	// ���� ���������� �۵����� ����
					
						delete fsp.file;

						// ========== 2��° ĸ��
						// ī�޶� �� ��� ��ġ ����
						proj3DInfo.isPersp = true;				// �۽���Ƽ�� ��
						proj3DInfo.u.persp.viewCone = 90.0;		// ī�޶� �þ߰�
						proj3DInfo.u.persp.rollAngle = 0.0;		// ī�޶� �� ����
						proj3DInfo.u.persp.azimuth = angleOfPoints - 90.0;	// ī�޶� ������

						proj3DInfo.u.persp.pos.x = camPos2.x;
						proj3DInfo.u.persp.pos.y = camPos2.y;
						proj3DInfo.u.persp.cameraZ = camPos2.z;

						proj3DInfo.u.persp.target.x = camPos1.x;
						proj3DInfo.u.persp.target.y = camPos1.y;
						proj3DInfo.u.persp.targetZ = camPos1.z;

						err = ACAPI_Environment (APIEnv_Change3DProjectionSetsID, &proj3DInfo, NULL, NULL);

						// ȭ�� ���ΰ�ħ
						ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
						ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

						// ȭ�� ĸ��
						ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
						BNZeroMemory (&fsp, sizeof (API_FileSavePars));
						fsp.fileTypeID = APIFType_PNGFile;
						sprintf (filename, "%s - ĸ�� (2).png", fullLayerName);
						fsp.file = new IO::Location (location, IO::Name (filename));

						BNZeroMemory (&pars_pict, sizeof (API_SavePars_Picture));
						pars_pict.colorDepth	= APIColorDepth_TC24;
						pars_pict.dithered		= false;
						pars_pict.view2D		= false;
						pars_pict.crop			= false;
						err = ACAPI_Automate (APIDo_SaveID, &fsp, &pars_pict);	// ���� ���������� �۵����� ����
					
						delete fsp.file;
					}
					// ������ Ÿ�� ���̾��� ���
					else if (layerType == SLAB) {
						// p1: ���� ���� x�� ã��
						// p2: ���� ���� y�� ã��
						// p3: ���� ū x�� ã��
						// p4: ���� ū y�� ã��
						// lowestZ: ���� ���� z�� ã��
						// highestZ: ���� ���� z�� ã��
						p1 = vecPos [0];
						p2 = vecPos [0];
						p3 = vecPos [0];
						p4 = vecPos [0];
						lowestZ = highestZ = vecPos [0].z;
						for (xx = 1 ; xx < vecPos.size () ; ++xx) {
							if (lowestZ > vecPos [xx].z)	lowestZ = vecPos [xx].z;
							if (highestZ < vecPos [xx].z)	highestZ = vecPos [xx].z;
							if (vecPos [xx].x < p1.x)	p1 = vecPos [xx];
							if (vecPos [xx].y < p2.y)	p2 = vecPos [xx];
							if (vecPos [xx].x > p3.x)	p3 = vecPos [xx];
							if (vecPos [xx].y > p4.y)	p4 = vecPos [xx];
						}

						// p5: ���� �߽� ã��
						p5.x = (p1.x + p3.x) / 2;
						p5.y = (p2.y + p4.y) / 2;
						p5.z = lowestZ;

						// p1�� p3 ���� �Ÿ�, p2�� p4 ���� �Ÿ� �� ���� �� �Ÿ��� ã��
						if (GetDistance (p1, p3) > GetDistance (p2, p4))
							distanceOfPoints = GetDistance (p1, p3);
						else
							distanceOfPoints = GetDistance (p2, p4);

						// ������ ȸ�� ������ ���� (p1�� p3 ���� ���� - 45��)
						angleOfPoints = RadToDegree (atan2 ((p3.y - p1.y), (p3.x - p1.x))) - 45.0;

						// ī�޶� ����, ��� ���� ����
						cameraZ = lowestZ - (distanceOfPoints * 10) + workLevel_object;		// �� ���� ���� ���������� �Ÿ��� �޶���
						targetZ = highestZ + (distanceOfPoints * 2) + workLevel_object;

						// ī�޶� �� ��� ��ġ ����
						proj3DInfo.isPersp = true;						// �۽���Ƽ�� ��
						proj3DInfo.u.persp.viewCone = 90.0;				// ī�޶� �þ߰�
						proj3DInfo.u.persp.rollAngle = 0.0;				// ī�޶� �� ����
						proj3DInfo.u.persp.azimuth = angleOfPoints;		// ī�޶� ������
						proj3DInfo.u.persp.distance = (targetZ - cameraZ) * 1000;	// �Ÿ�

						proj3DInfo.u.persp.pos.x = p5.x;
						proj3DInfo.u.persp.pos.y = p5.y;
						proj3DInfo.u.persp.cameraZ = cameraZ;

						proj3DInfo.u.persp.target.x = p5.x + 0.010;		// ī�޶�� ��� ���� X, Y ��ǥ�� ��Ȯ�ϰ� ��ġ�� ä�� �� ���̸� ������ ĸ�Ŀ� �����ϹǷ� ���� �־�� ��
						proj3DInfo.u.persp.target.y = p5.y;
						proj3DInfo.u.persp.targetZ = targetZ;

						err = ACAPI_Environment (APIEnv_Change3DProjectionSetsID, &proj3DInfo, NULL, NULL);

						// ========== 1��° ĸ��
						// ȭ�� ���ΰ�ħ
						ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
						ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

						// ȭ�� ĸ��
						ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
						BNZeroMemory (&fsp, sizeof (API_FileSavePars));
						fsp.fileTypeID = APIFType_PNGFile;
						sprintf (filename, "%s - ĸ�� (1).png", fullLayerName);
						fsp.file = new IO::Location (location, IO::Name (filename));

						BNZeroMemory (&pars_pict, sizeof (API_SavePars_Picture));
						pars_pict.colorDepth	= APIColorDepth_TC24;
						pars_pict.dithered		= false;
						pars_pict.view2D		= false;
						pars_pict.crop			= false;
						err = ACAPI_Automate (APIDo_SaveID, &fsp, &pars_pict);	// ���� ���������� �۵����� ����
					
						delete fsp.file;

						// ========== 2��° ĸ��
						proj3DInfo.u.persp.rollAngle = 90.0;			// ī�޶� �� ����

						err = ACAPI_Environment (APIEnv_Change3DProjectionSetsID, &proj3DInfo, NULL, NULL);

						// ȭ�� ���ΰ�ħ
						ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
						ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

						// ȭ�� ĸ��
						ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
						BNZeroMemory (&fsp, sizeof (API_FileSavePars));
						fsp.fileTypeID = APIFType_PNGFile;
						sprintf (filename, "%s - ĸ�� (2).png", fullLayerName);
						fsp.file = new IO::Location (location, IO::Name (filename));

						BNZeroMemory (&pars_pict, sizeof (API_SavePars_Picture));
						pars_pict.colorDepth	= APIColorDepth_TC24;
						pars_pict.dithered		= false;
						pars_pict.view2D		= false;
						pars_pict.crop			= false;
						err = ACAPI_Automate (APIDo_SaveID, &fsp, &pars_pict);	// ���� ���������� �۵����� ����
					
						delete fsp.file;

						// �� ���� �ʱ�ȭ
						proj3DInfo.u.persp.rollAngle = 0.0;			// ī�޶� �� ����

						err = ACAPI_Environment (APIEnv_Change3DProjectionSetsID, &proj3DInfo, NULL, NULL);
					}
					// ��� Ÿ�� ���̾��� ���
					else if (layerType == COLU) {
						// p1: ���� ���� x�� ã��
						// p2: ���� ���� y�� ã��
						// p3: ���� ū x�� ã��
						// p4: ���� ū y�� ã��
						// lowestZ: ���� ���� z�� ã��
						// highestZ: ���� ���� z�� ã��
						p1 = vecPos [0];
						p2 = vecPos [0];
						p3 = vecPos [0];
						p4 = vecPos [0];
						lowestZ = highestZ = vecPos [0].z;
						for (xx = 1 ; xx < vecPos.size () ; ++xx) {
							if (lowestZ > vecPos [xx].z)	lowestZ = vecPos [xx].z;
							if (highestZ < vecPos [xx].z)	highestZ = vecPos [xx].z;
							if (vecPos [xx].x < p1.x)	p1 = vecPos [xx];
							if (vecPos [xx].y < p2.y)	p2 = vecPos [xx];
							if (vecPos [xx].x > p3.x)	p3 = vecPos [xx];
							if (vecPos [xx].y > p4.y)	p4 = vecPos [xx];
						}

						// p5: ���� �߽� ã��
						p5.x = (p1.x + p3.x) / 2;
						p5.y = (p2.y + p4.y) / 2;
						p5.z = lowestZ;

						// p1�� p3 ���� �Ÿ�, p2�� p4 ���� �Ÿ� �� ���� �� �Ÿ��� ã��
						if (GetDistance (p1, p3) > GetDistance (p2, p4))
							distanceOfPoints = GetDistance (p1, p3);
						else
							distanceOfPoints = GetDistance (p2, p4);

						// ��� ȸ�� ������ ���� (p1�� p3 ���� ���� - 45��)
						angleOfPoints = RadToDegree (atan2 ((p3.y - p1.y), (p3.x - p1.x))) - 45.0;

						// ī�޶� ����, ��� ���� ����
						targetZ = cameraZ = (highestZ - lowestZ)/2 + workLevel_object;

						// ========== 1��° ĸ�� (���ʿ���)
						// ī�޶� �� ��� ��ġ ����
						proj3DInfo.isPersp = true;						// �۽���Ƽ�� ��
						proj3DInfo.u.persp.viewCone = 90.0;				// ī�޶� �þ߰�
						proj3DInfo.u.persp.rollAngle = 0.0;				// ī�޶� �� ����
						proj3DInfo.u.persp.azimuth = 270.0;				// ī�޶� ������
						proj3DInfo.u.persp.distance = distanceOfPoints * 2;		// �Ÿ�

						proj3DInfo.u.persp.pos.x = p5.x;
						proj3DInfo.u.persp.pos.y = p5.y + distanceOfPoints;
						proj3DInfo.u.persp.cameraZ = cameraZ;

						proj3DInfo.u.persp.target.x = p5.x;
						proj3DInfo.u.persp.target.y = p5.y - distanceOfPoints;
						proj3DInfo.u.persp.targetZ = targetZ;

						err = ACAPI_Environment (APIEnv_Change3DProjectionSetsID, &proj3DInfo, NULL, NULL);

						// ȭ�� ���ΰ�ħ
						ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
						ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

						// ȭ�� ĸ��
						ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
						BNZeroMemory (&fsp, sizeof (API_FileSavePars));
						fsp.fileTypeID = APIFType_PNGFile;
						sprintf (filename, "%s - ĸ�� (1).png", fullLayerName);
						fsp.file = new IO::Location (location, IO::Name (filename));

						BNZeroMemory (&pars_pict, sizeof (API_SavePars_Picture));
						pars_pict.colorDepth	= APIColorDepth_TC24;
						pars_pict.dithered		= false;
						pars_pict.view2D		= false;
						pars_pict.crop			= false;
						err = ACAPI_Automate (APIDo_SaveID, &fsp, &pars_pict);	// ���� ���������� �۵����� ����
					
						delete fsp.file;

						// ========== 2��° ĸ�� (���ʿ���)
						// ī�޶� �� ��� ��ġ ����
						proj3DInfo.isPersp = true;						// �۽���Ƽ�� ��
						proj3DInfo.u.persp.viewCone = 90.0;				// ī�޶� �þ߰�
						proj3DInfo.u.persp.rollAngle = 0.0;				// ī�޶� �� ����
						proj3DInfo.u.persp.azimuth = 90.0;				// ī�޶� ������
						proj3DInfo.u.persp.distance = distanceOfPoints * 2;		// �Ÿ�

						proj3DInfo.u.persp.pos.x = p5.x;
						proj3DInfo.u.persp.pos.y = p5.y - distanceOfPoints;
						proj3DInfo.u.persp.cameraZ = cameraZ;

						proj3DInfo.u.persp.target.x = p5.x;
						proj3DInfo.u.persp.target.y = p5.y + distanceOfPoints;
						proj3DInfo.u.persp.targetZ = targetZ;

						err = ACAPI_Environment (APIEnv_Change3DProjectionSetsID, &proj3DInfo, NULL, NULL);

						// ȭ�� ���ΰ�ħ
						ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
						ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

						// ȭ�� ĸ��
						ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
						BNZeroMemory (&fsp, sizeof (API_FileSavePars));
						fsp.fileTypeID = APIFType_PNGFile;
						sprintf (filename, "%s - ĸ�� (2).png", fullLayerName);
						fsp.file = new IO::Location (location, IO::Name (filename));

						BNZeroMemory (&pars_pict, sizeof (API_SavePars_Picture));
						pars_pict.colorDepth	= APIColorDepth_TC24;
						pars_pict.dithered		= false;
						pars_pict.view2D		= false;
						pars_pict.crop			= false;
						err = ACAPI_Automate (APIDo_SaveID, &fsp, &pars_pict);	// ���� ���������� �۵����� ����
					
						delete fsp.file;

						// ========== 3��° ĸ�� (���ʿ���)
						// ī�޶� �� ��� ��ġ ����
						proj3DInfo.isPersp = true;						// �۽���Ƽ�� ��
						proj3DInfo.u.persp.viewCone = 90.0;				// ī�޶� �þ߰�
						proj3DInfo.u.persp.rollAngle = 0.0;				// ī�޶� �� ����
						proj3DInfo.u.persp.azimuth = 180.0;				// ī�޶� ������
						proj3DInfo.u.persp.distance = distanceOfPoints * 2;		// �Ÿ�

						proj3DInfo.u.persp.pos.x = p5.x + distanceOfPoints;
						proj3DInfo.u.persp.pos.y = p5.y;
						proj3DInfo.u.persp.cameraZ = cameraZ;

						proj3DInfo.u.persp.target.x = p5.x - distanceOfPoints;
						proj3DInfo.u.persp.target.y = p5.y;
						proj3DInfo.u.persp.targetZ = targetZ;

						err = ACAPI_Environment (APIEnv_Change3DProjectionSetsID, &proj3DInfo, NULL, NULL);

						// ȭ�� ���ΰ�ħ
						ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
						ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

						// ȭ�� ĸ��
						ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
						BNZeroMemory (&fsp, sizeof (API_FileSavePars));
						fsp.fileTypeID = APIFType_PNGFile;
						sprintf (filename, "%s - ĸ�� (3).png", fullLayerName);
						fsp.file = new IO::Location (location, IO::Name (filename));

						BNZeroMemory (&pars_pict, sizeof (API_SavePars_Picture));
						pars_pict.colorDepth	= APIColorDepth_TC24;
						pars_pict.dithered		= false;
						pars_pict.view2D		= false;
						pars_pict.crop			= false;
						err = ACAPI_Automate (APIDo_SaveID, &fsp, &pars_pict);	// ���� ���������� �۵����� ����
					
						delete fsp.file;

						// ========== 4��° ĸ�� (���ʿ���)
						// ī�޶� �� ��� ��ġ ����
						proj3DInfo.isPersp = true;						// �۽���Ƽ�� ��
						proj3DInfo.u.persp.viewCone = 90.0;				// ī�޶� �þ߰�
						proj3DInfo.u.persp.rollAngle = 0.0;				// ī�޶� �� ����
						proj3DInfo.u.persp.azimuth = 0.0;				// ī�޶� ������
						proj3DInfo.u.persp.distance = distanceOfPoints * 2;		// �Ÿ�

						proj3DInfo.u.persp.pos.x = p5.x - distanceOfPoints;
						proj3DInfo.u.persp.pos.y = p5.y;
						proj3DInfo.u.persp.cameraZ = cameraZ;

						proj3DInfo.u.persp.target.x = p5.x + distanceOfPoints;
						proj3DInfo.u.persp.target.y = p5.y;
						proj3DInfo.u.persp.targetZ = targetZ;

						err = ACAPI_Environment (APIEnv_Change3DProjectionSetsID, &proj3DInfo, NULL, NULL);

						// ȭ�� ���ΰ�ħ
						ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
						ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

						// ȭ�� ĸ��
						ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
						BNZeroMemory (&fsp, sizeof (API_FileSavePars));
						fsp.fileTypeID = APIFType_PNGFile;
						sprintf (filename, "%s - ĸ�� (4).png", fullLayerName);
						fsp.file = new IO::Location (location, IO::Name (filename));

						BNZeroMemory (&pars_pict, sizeof (API_SavePars_Picture));
						pars_pict.colorDepth	= APIColorDepth_TC24;
						pars_pict.dithered		= false;
						pars_pict.view2D		= false;
						pars_pict.crop			= false;
						err = ACAPI_Automate (APIDo_SaveID, &fsp, &pars_pict);	// ���� ���������� �۵����� ����
					
						delete fsp.file;
					}
					// ��, ���纸 Ÿ�� ���̾��� ���
					else if ((layerType == BEAM) || (layerType == WLBM)) {
						// ���� ���� ������ ����
						// ���� ���� x���� ���� �� p1�� ã�Ƴ�
						lowestZ = highestZ = vecPos [0].z;
						p1 = vecPos [0];
						for (xx = 1 ; xx < vecPos.size () ; ++xx) {
							if (lowestZ > vecPos [xx].z)	lowestZ = vecPos [xx].z;
							if (highestZ < vecPos [xx].z)	highestZ = vecPos [xx].z;

							if (vecPos [xx].x < p1.x)	p1 = vecPos [xx];
						}
						cameraZ = (highestZ - lowestZ)/2 + workLevel_object;

						distanceOfPoints = 0.0;
						for (xx = 0 ; xx < vecPos.size () ; ++xx) {
							if (distanceOfPoints < GetDistance (p1, vecPos [xx])) {
								distanceOfPoints = GetDistance (p1, vecPos [xx]);
								p2 = vecPos [xx];
							}
						}

						// �� ��(p1, p2) ���� ���� ���ϱ�
						angleOfPoints = RadToDegree (atan2 ((p2.y - p1.y), (p2.x - p1.x)));

						// �߽��� ���ϱ�
						p3 = p1;
						moveIn3D ('x', DegreeToRad (angleOfPoints), distanceOfPoints/2, &p3.x, &p3.y, &p3.z);
						p3.z += workLevel_object;

						// ī�޶�� ����� ���� �� �ִ� ��ġ 2���� ã��
						camPos1 = p1;
						moveIn3D ('x', DegreeToRad (angleOfPoints), distanceOfPoints/2, &camPos1.x, &camPos1.y, &camPos1.z);
						if (distanceOfPoints > (highestZ - lowestZ))
							moveIn3D ('y', DegreeToRad (angleOfPoints), -distanceOfPoints * 1.5, &camPos1.x, &camPos1.y, &camPos1.z);
						else
							moveIn3D ('y', DegreeToRad (angleOfPoints), -(highestZ - lowestZ) * 1.5, &camPos1.x, &camPos1.y, &camPos1.z);
						camPos2 = p1;
						moveIn3D ('x', DegreeToRad (angleOfPoints), distanceOfPoints/2, &camPos2.x, &camPos2.y, &camPos2.z);
						if (distanceOfPoints > (highestZ - lowestZ))
							moveIn3D ('y', DegreeToRad (angleOfPoints), distanceOfPoints * 1.5, &camPos2.x, &camPos2.y, &camPos2.z);
						else
							moveIn3D ('y', DegreeToRad (angleOfPoints), (highestZ - lowestZ) * 1.5, &camPos2.x, &camPos2.y, &camPos2.z);

						camPos1.z = cameraZ;
						camPos2.z = cameraZ;

						// ========== 1��° ĸ��
						// ī�޶� �� ��� ��ġ ����
						proj3DInfo.isPersp = true;				// �۽���Ƽ�� ��
						proj3DInfo.u.persp.viewCone = 90.0;		// ī�޶� �þ߰�
						proj3DInfo.u.persp.rollAngle = 0.0;		// ī�޶� �� ����
						proj3DInfo.u.persp.azimuth = angleOfPoints + 90.0;	// ī�޶� ������

						proj3DInfo.u.persp.pos.x = camPos1.x;
						proj3DInfo.u.persp.pos.y = camPos1.y;
						proj3DInfo.u.persp.cameraZ = camPos1.z;

						proj3DInfo.u.persp.target.x = camPos2.x;
						proj3DInfo.u.persp.target.y = camPos2.y;
						proj3DInfo.u.persp.targetZ = camPos2.z;

						err = ACAPI_Environment (APIEnv_Change3DProjectionSetsID, &proj3DInfo, NULL, NULL);

						// ȭ�� ���ΰ�ħ
						ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
						ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

						// ȭ�� ĸ��
						ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
						BNZeroMemory (&fsp, sizeof (API_FileSavePars));
						fsp.fileTypeID = APIFType_PNGFile;
						sprintf (filename, "%s - ĸ�� (1).png", fullLayerName);
						fsp.file = new IO::Location (location, IO::Name (filename));

						BNZeroMemory (&pars_pict, sizeof (API_SavePars_Picture));
						pars_pict.colorDepth	= APIColorDepth_TC24;
						pars_pict.dithered		= false;
						pars_pict.view2D		= false;
						pars_pict.crop			= false;
						err = ACAPI_Automate (APIDo_SaveID, &fsp, &pars_pict);	// ���� ���������� �۵����� ����
					
						delete fsp.file;

						// ========== 2��° ĸ��
						// ī�޶� �� ��� ��ġ ����
						proj3DInfo.isPersp = true;				// �۽���Ƽ�� ��
						proj3DInfo.u.persp.viewCone = 90.0;		// ī�޶� �þ߰�
						proj3DInfo.u.persp.rollAngle = 0.0;		// ī�޶� �� ����
						proj3DInfo.u.persp.azimuth = angleOfPoints - 90.0;	// ī�޶� ������

						proj3DInfo.u.persp.pos.x = camPos2.x;
						proj3DInfo.u.persp.pos.y = camPos2.y;
						proj3DInfo.u.persp.cameraZ = camPos2.z;

						proj3DInfo.u.persp.target.x = camPos1.x;
						proj3DInfo.u.persp.target.y = camPos1.y;
						proj3DInfo.u.persp.targetZ = camPos1.z;

						err = ACAPI_Environment (APIEnv_Change3DProjectionSetsID, &proj3DInfo, NULL, NULL);

						// ȭ�� ���ΰ�ħ
						ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
						ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

						// ȭ�� ĸ��
						ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
						BNZeroMemory (&fsp, sizeof (API_FileSavePars));
						fsp.fileTypeID = APIFType_PNGFile;
						sprintf (filename, "%s - ĸ�� (2).png", fullLayerName);
						fsp.file = new IO::Location (location, IO::Name (filename));

						BNZeroMemory (&pars_pict, sizeof (API_SavePars_Picture));
						pars_pict.colorDepth	= APIColorDepth_TC24;
						pars_pict.dithered		= false;
						pars_pict.view2D		= false;
						pars_pict.crop			= false;
						err = ACAPI_Automate (APIDo_SaveID, &fsp, &pars_pict);	// ���� ���������� �۵����� ����
					
						delete fsp.file;

						// ========== 3��° ĸ��
						// ī�޶� �� ��� ��ġ ����
						proj3DInfo.isPersp = true;				// �۽���Ƽ�� ��
						proj3DInfo.u.persp.viewCone = 90.0;		// ī�޶� �þ߰�
						proj3DInfo.u.persp.rollAngle = 0.0;		// ī�޶� �� ����
						proj3DInfo.u.persp.azimuth = angleOfPoints - 90.0;	// ī�޶� ������

						proj3DInfo.u.persp.pos.x = p3.x;
						proj3DInfo.u.persp.pos.y = p3.y;
						proj3DInfo.u.persp.cameraZ = p3.z - distanceOfPoints;

						proj3DInfo.u.persp.target.x = p3.x - 0.001;
						proj3DInfo.u.persp.target.y = p3.y;
						proj3DInfo.u.persp.targetZ = p3.z;

						err = ACAPI_Environment (APIEnv_Change3DProjectionSetsID, &proj3DInfo, NULL, NULL);

						// ȭ�� ���ΰ�ħ
						ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
						ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

						// ȭ�� ĸ��
						ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
						BNZeroMemory (&fsp, sizeof (API_FileSavePars));
						fsp.fileTypeID = APIFType_PNGFile;
						sprintf (filename, "%s - ĸ�� (3).png", fullLayerName);
						fsp.file = new IO::Location (location, IO::Name (filename));

						BNZeroMemory (&pars_pict, sizeof (API_SavePars_Picture));
						pars_pict.colorDepth	= APIColorDepth_TC24;
						pars_pict.dithered		= false;
						pars_pict.view2D		= false;
						pars_pict.crop			= false;
						err = ACAPI_Automate (APIDo_SaveID, &fsp, &pars_pict);	// ���� ���������� �۵����� ����
					
						delete fsp.file;
					}
				}

				// ȭ���� ĸ�� ���� ���·� ��������
				err = ACAPI_Environment (APIEnv_Change3DProjectionSetsID, &proj3DInfo_beforeCapture, NULL, NULL);

				// ȭ�� ���ΰ�ħ
				ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
				ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);
				// !!! 3D ���� ���� ==================================
			}

			// ���̾� �����
			attrib.layer.head.flags |= APILay_Hidden;
			ACAPI_Attribute_Modify (&attrib, NULL);
		}
	}

	fclose (fp_unite);

	// ��� ���μ����� ��ġ�� ó���� �����ߴ� ���̴� ���̾���� �ٽ� �ѳ��� ��
	for (xx = 1 ; xx <= nVisibleLayers ; ++xx) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = visLayerList [xx-1];
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			if ((attrib.layer.head.flags & APILay_Hidden) == true) {
				attrib.layer.head.flags ^= APILay_Hidden;
				ACAPI_Attribute_Modify (&attrib, NULL);
			}
		}
	}

	// ȭ�� ���ΰ�ħ
	ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

	ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
	location.ToDisplayText (&resultString);
	sprintf (buffer, "������� ���� ��ġ�� �����߽��ϴ�.\n\n%s\n�Ǵ� ������Ʈ ������ �ִ� ����", resultString.ToCStr ().Get ());
	ACAPI_WriteReport (buffer, true);

	return	err;
}

// ���纰 ���� �� �����ֱ�
GSErrCode	filterSelection (void)
{
	GSErrCode	err = NoError;
	short		xx, yy;
	short		result;
	const char*	tempStr;
	bool		foundObj;

	short		selCount;
	API_Neig**	selNeig;

	FILE	*fp;				// ���� ������
	char	line [10240];		// ���Ͽ��� �о�� ���� �ϳ�
	char	*token;				// �о�� ���ڿ��� ��ū
	short	lineCount;			// �о�� ���� ��
	short	tokCount;			// �о�� ��ū ����
	char	nthToken [200][50];	// n��° ��ū

	API_Element			elem;
	API_ElementMemo		memo;

	// GUID ������ ���� ����
	GS::Array<API_Guid>	objects	= GS::Array<API_Guid> ();	long nObjects	= 0;
	GS::Array<API_Guid>	walls	= GS::Array<API_Guid> ();	long nWalls		= 0;
	GS::Array<API_Guid>	columns	= GS::Array<API_Guid> ();	long nColumns	= 0;
	GS::Array<API_Guid>	beams	= GS::Array<API_Guid> ();	long nBeams		= 0;
	GS::Array<API_Guid>	slabs	= GS::Array<API_Guid> ();	long nSlabs		= 0;
	GS::Array<API_Guid>	roofs	= GS::Array<API_Guid> ();	long nRoofs		= 0;
	GS::Array<API_Guid>	meshes	= GS::Array<API_Guid> ();	long nMeshes	= 0;
	GS::Array<API_Guid>	morphs	= GS::Array<API_Guid> ();	long nMorphs	= 0;
	GS::Array<API_Guid>	shells	= GS::Array<API_Guid> ();	long nShells	= 0;

	// ���ǿ� �´� ��ü���� GUID ����
	GS::Array<API_Guid> selection_known = GS::Array<API_Guid> ();
	GS::Array<API_Guid> selection_unknown = GS::Array<API_Guid> ();

	
	ACAPI_Element_GetElemList (API_ObjectID, &objects, APIFilt_OnVisLayer | APIFilt_In3D);	nObjects = objects.GetSize ();	// ���̴� ���̾� ���� ��ü Ÿ�Ը� ��������
	ACAPI_Element_GetElemList (API_WallID, &walls, APIFilt_OnVisLayer | APIFilt_In3D);		nWalls = walls.GetSize ();		// ���̴� ���̾� ���� �� Ÿ�Ը� ��������
	ACAPI_Element_GetElemList (API_ColumnID, &columns, APIFilt_OnVisLayer | APIFilt_In3D);	nColumns = columns.GetSize ();	// ���̴� ���̾� ���� ��� Ÿ�Ը� ��������
	ACAPI_Element_GetElemList (API_BeamID, &beams, APIFilt_OnVisLayer | APIFilt_In3D);		nBeams = beams.GetSize ();		// ���̴� ���̾� ���� �� Ÿ�Ը� ��������
	ACAPI_Element_GetElemList (API_SlabID, &slabs, APIFilt_OnVisLayer | APIFilt_In3D);		nSlabs = slabs.GetSize ();		// ���̴� ���̾� ���� ������ Ÿ�Ը� ��������
	ACAPI_Element_GetElemList (API_RoofID, &roofs, APIFilt_OnVisLayer | APIFilt_In3D);		nRoofs = roofs.GetSize ();		// ���̴� ���̾� ���� ���� Ÿ�Ը� ��������
	ACAPI_Element_GetElemList (API_MeshID, &meshes, APIFilt_OnVisLayer | APIFilt_In3D);		nMeshes = meshes.GetSize ();	// ���̴� ���̾� ���� �޽� Ÿ�Ը� ��������
	ACAPI_Element_GetElemList (API_MorphID, &morphs, APIFilt_OnVisLayer | APIFilt_In3D);	nMorphs = morphs.GetSize ();	// ���̴� ���̾� ���� ���� Ÿ�Ը� ��������
	ACAPI_Element_GetElemList (API_ShellID, &shells, APIFilt_OnVisLayer | APIFilt_In3D);	nShells = shells.GetSize ();	// ���̴� ���̾� ���� �� Ÿ�Ը� ��������

	if (nObjects == 0 && nWalls == 0 && nColumns == 0 && nBeams == 0 && nSlabs == 0 && nRoofs == 0 && nMeshes == 0 && nMorphs == 0 && nShells == 0) {
		result = DGAlert (DG_INFORMATION, "���� �˸�", "�ƹ� ��ü�� �������� �ʽ��ϴ�.", "", "Ȯ��", "", "");
		return	err;
	}

	// ��ü ���� ���� ��������
	fp = fopen ("C:\\objectInfo.csv", "r");

	if (fp == NULL) {
		result = DGAlert (DG_WARNING, "���� ����", "objectInfo.csv ������ C:\\�� �����Ͻʽÿ�.", "", "Ȯ��", "", "");
		return	err;
	}

	lineCount = 0;

	while (!feof (fp)) {
		tokCount = 0;
		fgets (line, sizeof (line), fp);

		token = strtok (line, ",");
		tokCount ++;
		lineCount ++;

		// �� ���ξ� ó��
		while (token != NULL) {
			if (strlen (token) > 0) {
				strncpy (nthToken [tokCount-1], token, strlen (token)+1);
			}
			token = strtok (NULL, ",");
			tokCount ++;
		}

		sprintf (visibleObjInfo.varName [lineCount-1], "%s", nthToken [0]);
		sprintf (visibleObjInfo.objName [lineCount-1], "%s", nthToken [1]);
	}

	visibleObjInfo.nKinds = lineCount;

	// ���� ���� �׸��� 2�� �� �� �����Ƿ� �ߺ� ����
	if (lineCount >= 2) {
		if (my_strcmp (visibleObjInfo.varName [lineCount-1], visibleObjInfo.varName [lineCount-2]) == 0) {
			visibleObjInfo.nKinds --;
		}
	}

	// ���� �ݱ�
	fclose (fp);

	// ���� ����, ǥ�� ���� �ʱ�ȭ
	for (xx = 0 ; xx < 50 ; ++xx) {
		visibleObjInfo.bExist [xx] = false;
		visibleObjInfo.bShow [xx] = false;
	}
	visibleObjInfo.bExist_Walls = false;
	visibleObjInfo.bShow_Walls = false;
	visibleObjInfo.bExist_Columns = false;
	visibleObjInfo.bShow_Columns = false;
	visibleObjInfo.bExist_Beams = false;
	visibleObjInfo.bShow_Beams = false;
	visibleObjInfo.bExist_Slabs = false;
	visibleObjInfo.bShow_Slabs = false;
	visibleObjInfo.bExist_Roofs = false;
	visibleObjInfo.bShow_Roofs = false;
	visibleObjInfo.bExist_Meshes = false;
	visibleObjInfo.bShow_Meshes = false;
	visibleObjInfo.bExist_Morphs = false;
	visibleObjInfo.bShow_Morphs = false;
	visibleObjInfo.bExist_Shells = false;
	visibleObjInfo.bShow_Shells = false;

	// ���� ���� üũ
	for (xx = 0 ; xx < nObjects ; ++xx) {
		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		elem.header.guid = objects [xx];
		err = ACAPI_Element_Get (&elem);
		err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

		foundObj = false;

		for (yy = 0 ; yy < visibleObjInfo.nKinds ; ++yy) {
			tempStr = getParameterStringByName (&memo, visibleObjInfo.varName [yy]);
			if (tempStr != NULL) {
				if (my_strcmp (tempStr, visibleObjInfo.objName [yy]) == 0) {
					visibleObjInfo.bExist [yy] = true;
					foundObj = true;
				}
			}
		}

		// ���� ã�� ���ϸ� �˷����� ���� Object Ÿ�� ����Ʈ�� �߰�
		if (foundObj == false)
			selection_unknown.Push (objects [xx]);

		ACAPI_DisposeElemMemoHdls (&memo);
	}
	
	char msg [256];
	sprintf (msg, "nObjects : %d\nselection_unknown.GetSize () : %d", nObjects, selection_unknown.GetSize ());
	ACAPI_WriteReport (msg, true);
	visibleObjInfo.nUnknownObjects = selection_unknown.GetSize ();

	if (nWalls > 0)		visibleObjInfo.bExist_Walls = true;
	if (nColumns > 0)	visibleObjInfo.bExist_Columns = true;
	if (nBeams > 0)		visibleObjInfo.bExist_Beams = true;
	if (nSlabs > 0)		visibleObjInfo.bExist_Slabs = true;
	if (nRoofs > 0)		visibleObjInfo.bExist_Roofs = true;
	if (nMeshes > 0)	visibleObjInfo.bExist_Meshes = true;
	if (nMorphs > 0)	visibleObjInfo.bExist_Morphs = true;
	if (nShells > 0)	visibleObjInfo.bExist_Shells = true;

	visibleObjInfo.nItems = visibleObjInfo.nKinds +
		(visibleObjInfo.bExist_Walls * 1) +
		(visibleObjInfo.bExist_Columns * 1) +
		(visibleObjInfo.bExist_Beams * 1) +
		(visibleObjInfo.bExist_Slabs * 1) +
		(visibleObjInfo.bExist_Roofs * 1) +
		(visibleObjInfo.bExist_Meshes * 1) +
		(visibleObjInfo.bExist_Morphs * 1) +
		(visibleObjInfo.bExist_Shells * 1);

	// [DIALOG] ���̾�α׿��� ���̴� ���̾� �� �ִ� ��ü���� ������ �����ְ�, üũ�� ������ ��ü�鸸 ���� �� ������
	result = DGBlankModalDialog (750, 500, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, filterSelectionHandler, 0);

	if (result == DG_OK) {
		// ������ ���ǿ� �ش��ϴ� ��ü�� �����ϱ�
		for (xx = 0 ; xx < nObjects ; ++xx) {
			BNZeroMemory (&elem, sizeof (API_Element));
			BNZeroMemory (&memo, sizeof (API_ElementMemo));
			elem.header.guid = objects [xx];
			err = ACAPI_Element_Get (&elem);
			err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

			for (yy = 0 ; yy < visibleObjInfo.nKinds ; ++yy) {
				tempStr = getParameterStringByName (&memo, visibleObjInfo.varName [yy]);
				
				if (tempStr != NULL) {
					if ((my_strcmp (tempStr, visibleObjInfo.objName [yy]) == 0) && (visibleObjInfo.bShow [yy] == true)) {
						selection_known.Push (objects [xx]);
					}
				}
			}

			ACAPI_DisposeElemMemoHdls (&memo);
		}

		// �˷��� Object Ÿ�� ����
		selCount = (short)selection_known.GetSize ();
		selNeig = (API_Neig**)(BMAllocateHandle (selCount * sizeof (API_Neig), ALLOCATE_CLEAR, 0));
		for (xx = 0 ; xx < selCount ; ++xx)
			(*selNeig)[xx].guid = selection_known [xx];

		ACAPI_Element_Select (selNeig, selCount, true);
		BMKillHandle (reinterpret_cast<GSHandle*> (&selNeig));

		// �˷����� ���� Object Ÿ�� ����
		if (visibleObjInfo.bShow_Unknown == true) {
			selCount = (short)selection_unknown.GetSize ();
			selNeig = (API_Neig**)(BMAllocateHandle (selCount * sizeof (API_Neig), ALLOCATE_CLEAR, 0));
			for (xx = 0 ; xx < selCount ; ++xx)
				(*selNeig)[xx].guid = selection_unknown [xx];

			ACAPI_Element_Select (selNeig, selCount, true);
			BMKillHandle (reinterpret_cast<GSHandle*> (&selNeig));
		}

		// ������ Ÿ��
		if (visibleObjInfo.bShow_Walls == true) {
			selCount = (short)nWalls;
			selNeig = (API_Neig**)(BMAllocateHandle (selCount * sizeof (API_Neig), ALLOCATE_CLEAR, 0));
			for (xx = 0 ; xx < selCount ; ++xx)
				(*selNeig)[xx].guid = walls [xx];

			ACAPI_Element_Select (selNeig, selCount, true);
			BMKillHandle (reinterpret_cast<GSHandle*> (&selNeig));
		}
		if (visibleObjInfo.bShow_Columns == true) {
			selCount = (short)nColumns;
			selNeig = (API_Neig**)(BMAllocateHandle (selCount * sizeof (API_Neig), ALLOCATE_CLEAR, 0));
			for (xx = 0 ; xx < selCount ; ++xx)
				(*selNeig)[xx].guid = columns [xx];

			ACAPI_Element_Select (selNeig, selCount, true);
			BMKillHandle (reinterpret_cast<GSHandle*> (&selNeig));
		}
		if (visibleObjInfo.bShow_Beams == true) {
			selCount = (short)nBeams;
			selNeig = (API_Neig**)(BMAllocateHandle (selCount * sizeof (API_Neig), ALLOCATE_CLEAR, 0));
			for (xx = 0 ; xx < selCount ; ++xx)
				(*selNeig)[xx].guid = beams [xx];

			ACAPI_Element_Select (selNeig, selCount, true);
			BMKillHandle (reinterpret_cast<GSHandle*> (&selNeig));
		}
		if (visibleObjInfo.bShow_Slabs == true) {
			selCount = (short)nSlabs;
			selNeig = (API_Neig**)(BMAllocateHandle (selCount * sizeof (API_Neig), ALLOCATE_CLEAR, 0));
			for (xx = 0 ; xx < selCount ; ++xx)
				(*selNeig)[xx].guid = slabs [xx];

			ACAPI_Element_Select (selNeig, selCount, true);
			BMKillHandle (reinterpret_cast<GSHandle*> (&selNeig));
		}
		if (visibleObjInfo.bShow_Roofs == true) {
			selCount = (short)nRoofs;
			selNeig = (API_Neig**)(BMAllocateHandle (selCount * sizeof (API_Neig), ALLOCATE_CLEAR, 0));
			for (xx = 0 ; xx < selCount ; ++xx)
				(*selNeig)[xx].guid = roofs [xx];

			ACAPI_Element_Select (selNeig, selCount, true);
			BMKillHandle (reinterpret_cast<GSHandle*> (&selNeig));
		}
		if (visibleObjInfo.bShow_Meshes == true) {
			selCount = (short)nMeshes;
			selNeig = (API_Neig**)(BMAllocateHandle (selCount * sizeof (API_Neig), ALLOCATE_CLEAR, 0));
			for (xx = 0 ; xx < selCount ; ++xx)
				(*selNeig)[xx].guid = meshes [xx];

			ACAPI_Element_Select (selNeig, selCount, true);
			BMKillHandle (reinterpret_cast<GSHandle*> (&selNeig));
		}
		if (visibleObjInfo.bShow_Morphs == true) {
			selCount = (short)nMorphs;
			selNeig = (API_Neig**)(BMAllocateHandle (selCount * sizeof (API_Neig), ALLOCATE_CLEAR, 0));
			for (xx = 0 ; xx < selCount ; ++xx)
				(*selNeig)[xx].guid = morphs [xx];

			ACAPI_Element_Select (selNeig, selCount, true);
			BMKillHandle (reinterpret_cast<GSHandle*> (&selNeig));
		}
		if (visibleObjInfo.bShow_Shells == true) {
			selCount = (short)nShells;
			selNeig = (API_Neig**)(BMAllocateHandle (selCount * sizeof (API_Neig), ALLOCATE_CLEAR, 0));
			for (xx = 0 ; xx < selCount ; ++xx)
				(*selNeig)[xx].guid = shells [xx];

			ACAPI_Element_Select (selNeig, selCount, true);
			BMKillHandle (reinterpret_cast<GSHandle*> (&selNeig));
		}
		
		// ������ �͸� 3D�� �����ֱ�
		ACAPI_Automate (APIDo_ShowSelectionIn3DID, NULL, NULL);
	}

	return	err;
}

// [���̾�α�] ��� �� �ּ� ���� �Ÿ��� ����ڿ��� �Է� ���� (�⺻��: 2000 mm)
short DGCALLBACK inputThresholdHandler (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;
	short	itmPosX, itmPosY;

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, "�ֿ� �ּ� ���� �Է��ϱ�");

			// Ȯ�� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 60, 80, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "Ȯ��");
			DGShowItem (dialogID, DG_OK);

			// ��� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 140, 60, 80, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "���");
			DGShowItem (dialogID, DG_CANCEL);

			// ��: �ֿ� �ּ� ����
			itmPosX = 10;
			itmPosY = 15;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 120, 23);
			DGSetItemFont (dialogID, LABEL_DIST_BTW_COLUMN, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_DIST_BTW_COLUMN, "�ֿ� �ּ� ���� (mm)");
			DGShowItem (dialogID, LABEL_DIST_BTW_COLUMN);

			// Edit��Ʈ��: �ֿ� �ּ� ����
			itmPosX = 140;
			itmPosY = 10;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, itmPosY, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_DIST_BTW_COLUMN, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemValDouble (dialogID, EDITCONTROL_DIST_BTW_COLUMN, DIST_BTW_COLUMN);
			DGShowItem (dialogID, EDITCONTROL_DIST_BTW_COLUMN);

			break;
		
		case DG_MSG_CHANGE:
			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					DIST_BTW_COLUMN = DGGetItemValDouble (dialogID, EDITCONTROL_DIST_BTW_COLUMN);
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

// [���̾�α�] ���̾�α׿��� ���̴� ���̾� �� �ִ� ��ü���� ������ �����ְ�, üũ�� ������ ��ü�鸸 ���� �� ������
short DGCALLBACK filterSelectionHandler (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	xx;
	short	itmIdx;
	short	itmPosX, itmPosY;
	char	buffer [64];

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, "������ Ÿ���� ��ü ���� �� �����ֱ�");

			// Ȯ�� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 10, 80, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "Ȯ��");
			DGShowItem (dialogID, DG_OK);

			// ��� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 120, 10, 80, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "���");
			DGShowItem (dialogID, DG_CANCEL);

			// ��ư: ��ü����
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 50, 80, 25);
			DGSetItemFont (dialogID, BUTTON_ALL_SEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_ALL_SEL, "��ü����");
			DGShowItem (dialogID, BUTTON_ALL_SEL);

			// ��ư: ��ü���� ����
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 120, 50, 80, 25);
			DGSetItemFont (dialogID, BUTTON_ALL_UNSEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_ALL_UNSEL, "��ü����\n����");
			DGShowItem (dialogID, BUTTON_ALL_UNSEL);

			// üũ�ڽ�: �˷����� ���� ��ü ����
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 220, 50, 250, 25);
			DGSetItemFont (dialogID, CHECKBOX_INCLUDE_UNKNOWN_OBJECT, DG_IS_LARGE | DG_IS_PLAIN);
			sprintf (buffer, "�˷����� ���� ��ü ���� (%d)", visibleObjInfo.nUnknownObjects);
			DGSetItemText (dialogID, CHECKBOX_INCLUDE_UNKNOWN_OBJECT, buffer);
			DGShowItem (dialogID, CHECKBOX_INCLUDE_UNKNOWN_OBJECT);
			if (visibleObjInfo.nUnknownObjects > 0)
				DGEnableItem (dialogID, CHECKBOX_INCLUDE_UNKNOWN_OBJECT);
			else
				DGDisableItem (dialogID, CHECKBOX_INCLUDE_UNKNOWN_OBJECT);

			// ������
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 5, 90, 740, 1);
			DGShowItem (dialogID, itmIdx);

			// üũ�ڽ� �׸�� ��ġ�� ��
			itmPosX = 20;	itmPosY = 105;	// Y�� ���� 105 ~ 500����

			if (visibleObjInfo.bExist_Walls == true) {
				visibleObjInfo.itmIdx_Walls = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, "��");
				DGShowItem (dialogID, itmIdx);
				itmPosY += 30;
			}
			if (visibleObjInfo.bExist_Columns == true) {
				visibleObjInfo.itmIdx_Columns = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, "���");
				DGShowItem (dialogID, itmIdx);
				itmPosY += 30;
			}
			if (visibleObjInfo.bExist_Beams == true) {
				visibleObjInfo.itmIdx_Beams = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, "��");
				DGShowItem (dialogID, itmIdx);
				itmPosY += 30;
			}
			if (visibleObjInfo.bExist_Slabs == true) {
				visibleObjInfo.itmIdx_Slabs = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, "������");
				DGShowItem (dialogID, itmIdx);
				itmPosY += 30;
			}
			if (visibleObjInfo.bExist_Roofs == true) {
				visibleObjInfo.itmIdx_Roofs = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);
				itmPosY += 30;
			}
			if (visibleObjInfo.bExist_Meshes == true) {
				visibleObjInfo.itmIdx_Meshes = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, "�޽�");
				DGShowItem (dialogID, itmIdx);
				itmPosY += 30;
			}
			if (visibleObjInfo.bExist_Morphs == true) {
				visibleObjInfo.itmIdx_Morphs = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);
				itmPosY += 30;
			}
			if (visibleObjInfo.bExist_Shells == true) {
				visibleObjInfo.itmIdx_Shells = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, "��");
				DGShowItem (dialogID, itmIdx);
				itmPosY += 30;
			}

			for (xx = 0 ; xx < visibleObjInfo.nKinds ; ++xx) {
				if (visibleObjInfo.bExist [xx] == true) {
					visibleObjInfo.itmIdx [xx] = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, visibleObjInfo.objName [xx]);
					DGShowItem (dialogID, itmIdx);
					itmPosY += 30;

					// 1�࿡ 12��
					if (itmPosY > 430) {
						itmPosX += 200;
						itmPosY = 105;
					}
				}
			}

			break;
		
		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// Object Ÿ��
					for (xx = 0 ; xx < visibleObjInfo.nKinds ; ++xx) {
						if (DGGetItemValLong (dialogID, visibleObjInfo.itmIdx [xx]) == TRUE) {
							visibleObjInfo.bShow [xx] = true;
						} else {
							visibleObjInfo.bShow [xx] = false;
						}
					}

					// �˷����� ���� Object Ÿ���� ��ü ���̱� ����
					(DGGetItemValLong (dialogID, CHECKBOX_INCLUDE_UNKNOWN_OBJECT) == TRUE)	? visibleObjInfo.bShow_Unknown = true	: visibleObjInfo.bShow_Unknown = false;

					// ������ Ÿ��
					(DGGetItemValLong (dialogID, visibleObjInfo.itmIdx_Walls) == TRUE)		? visibleObjInfo.bShow_Walls = true		: visibleObjInfo.bShow_Walls = false;
					(DGGetItemValLong (dialogID, visibleObjInfo.itmIdx_Columns) == TRUE)	? visibleObjInfo.bShow_Columns = true	: visibleObjInfo.bShow_Columns = false;
					(DGGetItemValLong (dialogID, visibleObjInfo.itmIdx_Beams) == TRUE)		? visibleObjInfo.bShow_Beams = true		: visibleObjInfo.bShow_Beams = false;
					(DGGetItemValLong (dialogID, visibleObjInfo.itmIdx_Slabs) == TRUE)		? visibleObjInfo.bShow_Slabs = true		: visibleObjInfo.bShow_Slabs = false;
					(DGGetItemValLong (dialogID, visibleObjInfo.itmIdx_Roofs) == TRUE)		? visibleObjInfo.bShow_Roofs = true		: visibleObjInfo.bShow_Roofs = false;
					(DGGetItemValLong (dialogID, visibleObjInfo.itmIdx_Meshes) == TRUE)		? visibleObjInfo.bShow_Meshes = true	: visibleObjInfo.bShow_Meshes = false;
					(DGGetItemValLong (dialogID, visibleObjInfo.itmIdx_Morphs) == TRUE)		? visibleObjInfo.bShow_Morphs = true	: visibleObjInfo.bShow_Morphs = false;
					(DGGetItemValLong (dialogID, visibleObjInfo.itmIdx_Shells) == TRUE)		? visibleObjInfo.bShow_Shells = true	: visibleObjInfo.bShow_Shells = false;

					break;

				case DG_CANCEL:
					break;

				case BUTTON_ALL_SEL:
					item = 0;

					// ��� üũ�ڽ��� ��
					if (visibleObjInfo.bExist_Walls == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Walls, TRUE);
					if (visibleObjInfo.bExist_Columns == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Columns, TRUE);
					if (visibleObjInfo.bExist_Beams == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Beams, TRUE);
					if (visibleObjInfo.bExist_Slabs == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Slabs, TRUE);
					if (visibleObjInfo.bExist_Roofs == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Roofs, TRUE);
					if (visibleObjInfo.bExist_Meshes == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Meshes, TRUE);
					if (visibleObjInfo.bExist_Morphs == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Morphs, TRUE);
					if (visibleObjInfo.bExist_Shells == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Shells, TRUE);
					for (xx = 0 ; xx < visibleObjInfo.nKinds ; ++xx) {
						if (visibleObjInfo.bExist [xx] == true) {
							DGSetItemValLong (dialogID, visibleObjInfo.itmIdx [xx], TRUE);
						}
					}

					break;

				case BUTTON_ALL_UNSEL:
					item = 0;

					// ��� üũ�ڽ��� ��
					if (visibleObjInfo.bExist_Walls == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Walls, FALSE);
					if (visibleObjInfo.bExist_Columns == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Columns, FALSE);
					if (visibleObjInfo.bExist_Beams == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Beams, FALSE);
					if (visibleObjInfo.bExist_Slabs == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Slabs, FALSE);
					if (visibleObjInfo.bExist_Roofs == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Roofs, FALSE);
					if (visibleObjInfo.bExist_Meshes == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Meshes, FALSE);
					if (visibleObjInfo.bExist_Morphs == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Morphs, FALSE);
					if (visibleObjInfo.bExist_Shells == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Shells, FALSE);
					for (xx = 0 ; xx < visibleObjInfo.nKinds ; ++xx) {
						if (visibleObjInfo.bExist [xx] == true) {
							DGSetItemValLong (dialogID, visibleObjInfo.itmIdx [xx], FALSE);
						}
					}

					break;

				default:
					item = 0;
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