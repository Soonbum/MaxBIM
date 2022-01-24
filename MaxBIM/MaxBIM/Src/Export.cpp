#include <cstdio>
#include <cstdlib>
#include <algorithm>
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

// vector �� ���� ���� ����ü ������ ���� �� �Լ� (��ǥ�� X ����)
bool comparePosX (const objectInBeamTableform& a, const objectInBeamTableform& b)
{
	return a.origin.x < b.origin.x;
}

// vector �� ���� ���� ����ü ������ ���� �� �Լ� (��ǥ�� Y ����)
bool comparePosY (const objectInBeamTableform& a, const objectInBeamTableform& b)
{
	return a.origin.y < b.origin.y;
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

			if (err == NoError && elem.header.hasMemo) {
				err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

				if (err == NoError) {
					if (storyInfo.data [0][xx].index == elem.header.floorInd) {
						coords_hor [nCoords_hor++] = elem.column.origoPos.x;
						coords_ver [nCoords_ver++] = elem.column.origoPos.y;
					}
				}

				ACAPI_DisposeElemMemoHdls (&memo);
			}
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

			if (err == NoError && elem.header.hasMemo) {
				err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

				if (err == NoError) {
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
				}

				ACAPI_DisposeElemMemoHdls (&memo);
			}
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
		sprintf (buffer, "�ֿ�����,����");
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
				sprintf (buffer, ",����,");
				for (zz = columnPos.firstStory ; zz <= columnPos.lastStory ; ++zz) {
					resultColumn = findColumn (&columnPos, xx, yy, zz);
					sprintf (piece, "%.0f,", resultColumn.verLen * 1000);
					strcat (buffer, piece);
				}
				strcat (buffer, "\n");
				fprintf (fp, buffer);

				// ����
				sprintf (buffer, ",����,");
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
	char	line [2048];	// ���Ͽ��� �о�� ���� �ϳ�
	char	*token;			// �о�� ���ڿ��� ��ū
	short	lineCount;		// �о�� ���� ��
	short	tokCount;		// �о�� ��ū ����
	short	xx;
	int		count;

	char	nthToken [200][256];	// n��° ��ū

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

			// ��ū ������ 2�� �̻��� �� ��ȿ��
			if ((tokCount-1) >= 2) {
				// ��ū ������ 2 + 3�� ��� ������ ����� (�ʰ��� �׸��� ����)
				if (((tokCount-1) - 2) % 3 != 0) {
					tokCount --;
				}

				this->keyName.push_back (nthToken [0]);		// ��: u_comp
				this->keyDesc.push_back (nthToken [1]);		// ��: ������
				count = atoi (nthToken [2]);
				this->nInfo.push_back (count);				// ��: 5

				vector<string>	varNames;	// �ش� ��ü�� ���� �̸���
				vector<string>	varDescs;	// �ش� ��ü�� ���� �̸��� ���� �����

				for (xx = 1 ; xx <= count ; ++xx) {
					varNames.push_back (nthToken [1 + xx*2]);
					varDescs.push_back (nthToken [1 + xx*2 + 1]);
				}

				this->varName.push_back (varNames);
				this->varDesc.push_back (varDescs);
			}
		}

		// ���� �ݱ�
		fclose (fp);

		// ��ü ���� ����
		this->nKnownObjects = lineCount - 1;
		this->nUnknownObjects = 0;
	}
}

// ��ü�� ���ڵ� ���� 1 ���� (������ ����, ������ �ű� �߰�)
int	SummaryOfObjectInfo::quantityPlus1 (vector<string> record)
{
	int		xx, yy;
	size_t	vecLen;
	size_t	inVecLen1, inVecLen2;
	int		diff;
	int		value;
	char	tempStr [512];

	vecLen = this->records.size ();

	try {
		for (xx = 0 ; xx < vecLen ; ++xx) {
			// ���� ���� ������ ���
			inVecLen1 = this->records.at(xx).size () - 1;		// ���� ���� �ʵ带 ������ ����
			inVecLen2 = record.size ();

			if (inVecLen1 == inVecLen2) {
				// ��ġ���� �ʴ� �ʵ尡 �ϳ��� �ִ��� ã�ƺ� ��
				diff = 0;
				for (yy = 0 ; yy < inVecLen1 ; ++yy) {
					if (my_strcmp (this->records.at(xx).at(yy).c_str (), record.at(yy).c_str ()) != 0)
						diff++;
				}

				// ��� �ʵ尡 ��ġ�ϸ�
				if (diff == 0) {
					value = atoi (this->records.at(xx).back ().c_str ());
					value ++;
					sprintf (tempStr, "%d", value);
					this->records.at(xx).pop_back ();
					this->records.at(xx).push_back (tempStr);
					return value;
				}
			}
		}
	} catch (exception& ex) {
		WriteReport ("quantityPlus1 �Լ����� ���� �߻�: %s", ex.what ());
	}

	// ������ �ű� ���ڵ� �߰��ϰ� 1 ����
	record.push_back ("1");
	this->records.push_back (record);

	return 1;
}

// ���ڵ� ���� �����
void SummaryOfObjectInfo::clear ()
{
	unsigned int xx;
	
	try {
		for (xx = 0 ; xx < this->records.size () ; ++xx) {
			this->records.at(xx).clear ();
		}
	} catch (exception& ex) {
		WriteReport ("clear �Լ����� ���� �߻�: %s", ex.what ());
	}
	this->records.clear ();
}

// ������ ���� ���� �������� (Single ���)
GSErrCode	exportSelectedElementInfo (void)
{
	GSErrCode	err = NoError;
	long		nSel;
	unsigned short		xx, yy, zz;
	bool		regenerate = true;
	bool		suspGrp;
	
	// ������ ��Ұ� ������ ����
	API_SelectionInfo		selectionInfo;
	API_Neig				**selNeigs;
	API_Element				tElem;
	GS::Array<API_Guid>		objects;
	GS::Array<API_Guid>		beams;
	long					nObjects = 0;
	long					nBeams = 0;

	// ������ ��ҵ��� ���� ����ϱ�
	API_Element			elem;
	API_ElementMemo		memo;
	SummaryOfObjectInfo	objectInfo;

	char			buffer [512];
	char			filename [512];
	char			tempStr [512];
	const char*		foundStr;
	bool			foundObject;
	bool			foundExistValue;
	int				retVal;
	int				nInfo;
	API_AddParID	varType;
	vector<string>	record;

	// GS::Array �ݺ���
	GS::Array<API_Guid>::Iterator	iterObj;
	API_Guid	curGuid;


	// �׷�ȭ �Ͻ����� ON
	ACAPI_Environment (APIEnv_IsSuspendGroupOnID, &suspGrp);
	if (suspGrp == false)	ACAPI_Element_Tool (NULL, NULL, APITool_SuspendGroups, NULL);

	// ȭ�� ���ΰ�ħ
	ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

	// ������ ��� ��������
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

	// ���� ���Ϸ� ��� ���� ��������
	// ���� ������ ���� ����
	API_SpecFolderID	specFolderID = API_ApplicationFolderID;
	IO::Location		location;
	GS::UniString		resultString;
	API_MiscAppInfo		miscAppInfo;
	FILE				*fp;
	FILE				*fp_interReport;

	ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
	ACAPI_Environment (APIEnv_GetMiscAppInfoID, &miscAppInfo);
	sprintf (filename, "%s - ������ ���� ����.csv", miscAppInfo.caption);
	fp = fopen (filename, "w+");
	sprintf (filename, "%s - ������ ���� ���� (�߰�����).txt", miscAppInfo.caption);
	fp_interReport = fopen (filename, "w+");

	if ((fp == NULL) || (fp_interReport == NULL)) {
		ACAPI_WriteReport ("������ �� �� �����ϴ�.", true);
		return err;
	}

	iterObj = objects.Enumerate ();

	while (iterObj != NULL) {
		foundObject = false;

		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		curGuid = *iterObj;
		elem.header.guid = curGuid;
		err = ACAPI_Element_Get (&elem);

		if (err == NoError && elem.header.hasMemo) {
			err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

			if (err == NoError) {
				// �Ķ���� ��ũ��Ʈ�� ������ �����Ŵ
				ACAPI_Goodies (APIAny_RunGDLParScriptID, &elem.header, 0);

				try {
					for (yy = 0 ; yy < objectInfo.keyName.size () ; ++yy) {
						strcpy (tempStr, objectInfo.keyName.at(yy).c_str ());
						foundStr = getParameterStringByName (&memo, tempStr);

						// ��ü ������ ã�Ҵٸ�,
						if (my_strcmp (foundStr, "") != 0) {
							retVal = my_strcmp (objectInfo.keyDesc.at(yy).c_str (), foundStr);

							if (retVal == 0) {
								foundObject = true;
								foundExistValue = false;

								// �߰��� ��ü�� �����͸� ������� ���ڵ� �߰�
								if (!record.empty ())
									record.clear ();

								record.push_back (objectInfo.keyDesc.at(yy));		// ��ü �̸�
								nInfo = objectInfo.nInfo.at(yy);
								for (zz = 0 ; zz < nInfo ; ++zz) {
									sprintf (buffer, "%s", objectInfo.varName.at(yy).at(zz).c_str ());
									varType = getParameterTypeByName (&memo, buffer);

									if ((varType != APIParT_Separator) || (varType != APIParT_Title) || (varType != API_ZombieParT)) {
										if (varType == APIParT_CString)
											sprintf (tempStr, "%s", getParameterStringByName (&memo, buffer));	// ���ڿ�
										else
											sprintf (tempStr, "%.3f", getParameterValueByName (&memo, buffer));	// ����
									}
									record.push_back (tempStr);		// ������
								}

								objectInfo.quantityPlus1 (record);
							}
						}
					}
				} catch (exception& ex) {
					WriteReport ("��ü ���� �������� ���� �߻�: %s", ex.what ());
				}

				// ���� ã�� ���ϸ� �� �� ���� ��ü�� �����
				if (foundObject == false)
					objectInfo.nUnknownObjects ++;
			}

			ACAPI_DisposeElemMemoHdls (&memo);
		}

		++iterObj;
	}

	// �� ���� ����
	//for (xx = 0 ; xx < nBeams ; ++xx) {
	//	BNZeroMemory (&elem, sizeof (API_Element));
	//	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	//	elem.header.guid = beams.Pop ();
	//	err = ACAPI_Element_Get (&elem);
	//	err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

	//	foundExistValue = false;

	//	int len;

	//	len = static_cast<int> (round (GetDistance (elem.beam.begC, elem.beam.endC) * 1000, 0));

	//	// �ߺ� �׸��� ������ ����
	//	for (zz = 0 ; zz < objectInfo.nCountsBeam ; ++zz) {
	//		if (objectInfo.beamLength [zz] == len) {
	//			objectInfo.beamCount [zz] ++;
	//			foundExistValue = true;
	//			break;
	//		}
	//	}

	//	// �ű� �׸� �߰��ϰ� ������ ����
	//	if ( !foundExistValue ) {
	//		objectInfo.beamLength.push_back (len);
	//		objectInfo.beamCount.push_back (1);
	//		objectInfo.nCountsBeam ++;
	//	}

	//	ACAPI_DisposeElemMemoHdls (&memo);
	//}

	// ���� �ؽ�Ʈ ǥ��
	// APIParT_Length�� ��� 1000�� ���ؼ� ǥ��
	// APIParT_Boolean�� ��� ��/�ƴϿ� ǥ��
	double	length, length2, length3;
	bool	bTitleAppeared;

	// *����, ���� ���� ������ ����ϱ� ���� ����
	double	totalAreaOfPlywoods = 0.0;
	double	totalLengthOfTimbers_40x50 = 0.0;	// �ٷ糢 (40*50)
	double	totalLengthOfTimbers_50x80 = 0.0;	// ������ (50*80)
	double	totalLengthOfTimbers_80x80 = 0.0;	// ��°� (80*80)
	double	totalLengthOfTimbersEtc = 0.0;		// ��԰�
	int		count;	// ����

	// ��ü �������� ���� ���
	try {
		for (xx = 0 ; xx < objectInfo.keyDesc.size () ; ++xx) {
			bTitleAppeared = false;

			// ���ڵ带 ���� ��ȸ
			for (yy = 0 ; yy < objectInfo.records.size () ; ++yy) {
				// ��ü ���� �̸��� ���ڵ��� 1�� �ʵ尡 ��ġ�ϴ� ��츸 ã�Ƽ� �����
				retVal = my_strcmp (objectInfo.keyDesc.at(xx).c_str (), objectInfo.records.at(yy).at(0).c_str ());

				count = atoi (objectInfo.records.at(yy).at(objectInfo.records.at(yy).size ()-1).c_str ());

				if (retVal == 0) {
					// ���� ���
					if (bTitleAppeared == false) {
						sprintf (buffer, "\n[%s]\n", objectInfo.keyDesc.at(xx).c_str ());
						fprintf (fp, buffer);
						bTitleAppeared = true;
					}

					// ������ �� ���
					if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "������ ��ũ") == 0) {
						// ����
						if (objectInfo.records.at(yy).at(2).compare ("����") == 0) {
							sprintf (buffer, "���� / %s", objectInfo.records.at(yy).at(1));
						}

						// �簢
						if (objectInfo.records.at(yy).at(2).compare ("�簢") == 0) {
							sprintf (buffer, "�簢 / %s", objectInfo.records.at(yy).at(1));
						}
						fprintf (fp, buffer);

					} else if ((my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "������") == 0) || (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "��ƿ��") == 0)) {
						// �԰���
						if (atoi (objectInfo.records.at(yy).at(1).c_str ()) > 0) {
							sprintf (buffer, "%s X %s ", objectInfo.records.at(yy).at(2), objectInfo.records.at(yy).at(3));

						// ��԰�ǰ
						} else {
							length = atof (objectInfo.records.at(yy).at(4).c_str ());
							length2 = atof (objectInfo.records.at(yy).at(5).c_str ());
							sprintf (buffer, "%.0f X %.0f ", round (length*1000, 0), round (length2*1000, 0));
						}
						fprintf (fp, buffer);

					} else if (objectInfo.keyDesc.at(xx).compare ("����") == 0) {
						length = atof (objectInfo.records.at(yy).at(1).c_str ());
						length2 = atof (objectInfo.records.at(yy).at(2).c_str ());
						length3 = atof (objectInfo.records.at(yy).at(3).c_str ());
						sprintf (buffer, "%.0f X %.0f X %.0f ", round (length*1000, 0), round (length2*1000, 0), round (length3*1000, 0));
						if ( ((abs (length - 0.040) < EPS) && (abs (length2 - 0.050) < EPS)) || ((abs (length - 0.050) < EPS) && (abs (length2 - 0.040) < EPS)) )
							totalLengthOfTimbers_40x50 += (length3 * count);
						else if ( ((abs (length - 0.050) < EPS) && (abs (length2 - 0.080) < EPS)) || ((abs (length - 0.080) < EPS) && (abs (length2 - 0.050) < EPS)) )
							totalLengthOfTimbers_50x80 += (length3 * count);
						else if ((abs (length - 0.080) < EPS) && (abs (length2 - 0.080) < EPS))
							totalLengthOfTimbers_80x80 += (length3 * count);
						else
							totalLengthOfTimbersEtc += (length3 * count);
						fprintf (fp, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "���ǳ�") == 0) {
						if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x6 [910x1820]") == 0) {
							sprintf (buffer, "910 X 1820 X %s ", objectInfo.records.at(yy).at(2).c_str ());
							fprintf (fp, buffer);

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "4x8 [1220x2440]") == 0) {
							sprintf (buffer, "1220 X 2440 X %s ", objectInfo.records.at(yy).at(2).c_str ());
							fprintf (fp, buffer);

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x5 [606x1520]") == 0) {
							sprintf (buffer, "606 X 1520 X %s ", objectInfo.records.at(yy).at(2).c_str ());
							fprintf (fp, buffer);

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x6 [606x1820]") == 0) {
							sprintf (buffer, "606 X 1820 X %s ", objectInfo.records.at(yy).at(2).c_str ());
							fprintf (fp, buffer);

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x5 [910x1520]") == 0) {
							sprintf (buffer, "910 X 1520 X %s ", objectInfo.records.at(yy).at(2).c_str ());
							fprintf (fp, buffer);

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "��԰�") == 0) {
							// ���� X ���� X �β�
							length = atof (objectInfo.records.at(yy).at(3).c_str ());
							length2 = atof (objectInfo.records.at(yy).at(4).c_str ());
							sprintf (buffer, "%.0f X %.0f X %s ", round (length*1000, 0), round (length2*1000, 0), objectInfo.records.at(yy).at(2).c_str ());
							fprintf (fp, buffer);
						}

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "����") == 0) {
						if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x6 [910x1820]") == 0) {
							sprintf (buffer, "910 X 1820 X %s ", objectInfo.records.at(yy).at(2).c_str ());
							totalAreaOfPlywoods += (0.900 * 1.800 * count);
							fprintf (fp, buffer);

							// ����Ʋ ON
							if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
								sprintf (buffer, "(���� �ѱ���: %s) ", objectInfo.records.at(yy).at(6).c_str ());
								totalLengthOfTimbers_40x50 += ((atof (objectInfo.records.at(yy).at(6).c_str ()) / 1000) * count);
								fprintf (fp, buffer);

								sprintf (buffer, "(���� ���� ����: %s) ", objectInfo.records.at(yy).at(7).c_str ());
								fprintf (fp, buffer);
							}

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "4x8 [1220x2440]") == 0) {
							sprintf (buffer, "1220 X 2440 X %s ", objectInfo.records.at(yy).at(2).c_str ());
							totalAreaOfPlywoods += (1.200 * 2.400 * count);
							fprintf (fp, buffer);

							// ����Ʋ ON
							if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
								sprintf (buffer, "(���� �ѱ���: %s) ", objectInfo.records.at(yy).at(6).c_str ());
								totalLengthOfTimbers_40x50 += ((atof (objectInfo.records.at(yy).at(6).c_str ()) / 1000) * count);
								fprintf (fp, buffer);

								sprintf (buffer, "(���� ���� ����: %s) ", objectInfo.records.at(yy).at(7).c_str ());
								fprintf (fp, buffer);
							}

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x5 [606x1520]") == 0) {
							sprintf (buffer, "606 X 1520 X %s ", objectInfo.records.at(yy).at(2).c_str ());
							totalAreaOfPlywoods += (0.600 * 1.500 * count);
							fprintf (fp, buffer);

							// ����Ʋ ON
							if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
								sprintf (buffer, "(���� �ѱ���: %s) ", objectInfo.records.at(yy).at(6).c_str ());
								totalLengthOfTimbers_40x50 += ((atof (objectInfo.records.at(yy).at(6).c_str ()) / 1000) * count);
								fprintf (fp, buffer);

								sprintf (buffer, "(���� ���� ����: %s) ", objectInfo.records.at(yy).at(7).c_str ());
								fprintf (fp, buffer);
							}

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x6 [606x1820]") == 0) {
							sprintf (buffer, "606 X 1820 X %s ", objectInfo.records.at(yy).at(2).c_str ());
							totalAreaOfPlywoods += (0.600 * 1.800 * count);
							fprintf (fp, buffer);

							// ����Ʋ ON
							if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
								sprintf (buffer, "(���� �ѱ���: %s) ", objectInfo.records.at(yy).at(6).c_str ());
								totalLengthOfTimbers_40x50 += ((atof (objectInfo.records.at(yy).at(6).c_str ()) / 1000) * count);
								fprintf (fp, buffer);

								sprintf (buffer, "(���� ���� ����: %s) ", objectInfo.records.at(yy).at(7).c_str ());
								fprintf (fp, buffer);
							}

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x5 [910x1520]") == 0) {
							sprintf (buffer, "910 X 1520 X %s ", objectInfo.records.at(yy).at(2).c_str ());
							totalAreaOfPlywoods += (0.900 * 1.500 * count);
							fprintf (fp, buffer);

							// ����Ʋ ON
							if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
								sprintf (buffer, "(���� �ѱ���: %s) ", objectInfo.records.at(yy).at(6).c_str ());
								totalLengthOfTimbers_40x50 += ((atof (objectInfo.records.at(yy).at(6).c_str ()) / 1000) * count);
								fprintf (fp, buffer);

								sprintf (buffer, "(���� ���� ����: %s) ", objectInfo.records.at(yy).at(7).c_str ());
								fprintf (fp, buffer);
							}

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "��԰�") == 0) {
							// ���� X ���� X �β�
							length = atof (objectInfo.records.at(yy).at(3).c_str ());
							length2 = atof (objectInfo.records.at(yy).at(4).c_str ());
							sprintf (buffer, "%.0f X %.0f X %s ", round (length*1000, 0), round (length2*1000, 0), objectInfo.records.at(yy).at(2).c_str ());
							totalAreaOfPlywoods += (length * length2 * count);
							fprintf (fp, buffer);

							// ����Ʋ ON
							if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
								sprintf (buffer, "(���� �ѱ���: %s) ", objectInfo.records.at(yy).at(6).c_str ());
								totalLengthOfTimbers_40x50 += ((atof (objectInfo.records.at(yy).at(6).c_str ()) / 1000) * count);
								fprintf (fp, buffer);

								sprintf (buffer, "(���� ���� ����: %s) ", objectInfo.records.at(yy).at(7).c_str ());
								fprintf (fp, buffer);
							}

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "������") == 0) {
							sprintf (buffer, "������ ");
							fprintf (fp, buffer);

						} else {
							sprintf (buffer, "�ٰ��� ");
							fprintf (fp, buffer);
						}

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "����(�ٰ���)") == 0) {
						// ���� ����
						sprintf (buffer, "����: %.2f ", atof (objectInfo.records.at(yy).at(1).c_str ()));
						totalAreaOfPlywoods += (atof (objectInfo.records.at(yy).at(1).c_str ()) * count);
						fprintf (fp, buffer);

						// ����Ʋ ON
						if (atoi (objectInfo.records.at(yy).at(2).c_str ()) > 0) {
							sprintf (buffer, "(���� �ѱ���: %.0f) ", round (atof (objectInfo.records.at(yy).at(3).c_str ())*1000, 0));
							totalLengthOfTimbers_40x50 += (atof (objectInfo.records.at(yy).at(3).c_str ()) * count);
							fprintf (fp, buffer);
						}

					} else if (objectInfo.keyDesc.at(xx).compare ("RS Push-Pull Props") == 0) {
						// ���̽� �÷���Ʈ ����
						if (atoi (objectInfo.records.at(yy).at(1).c_str ()) == 1) {
							sprintf (buffer, "���̽� �÷���Ʈ(����) ");
						} else {
							sprintf (buffer, "���̽� �÷���Ʈ(����) ");
						}
						fprintf (fp, buffer);

						// �԰�(���)
						sprintf (buffer, "�԰�(���): %s ", objectInfo.records.at(yy).at(2).c_str ());
						fprintf (fp, buffer);

						// �԰�(�Ϻ�) - ���û���
						if (atoi (objectInfo.records.at(yy).at(4).c_str ()) == 1) {
							sprintf (buffer, "�԰�(�Ϻ�): %s ", objectInfo.records.at(yy).at(3).c_str ());
						}
						fprintf (fp, buffer);
				
					} else if (objectInfo.keyDesc.at(xx).compare ("Push-Pull Props (�⼺ǰ �� �������ǰ)") == 0) {
						// ���̽� �÷���Ʈ ����
						if (atoi (objectInfo.records.at(yy).at(1).c_str ()) == 1) {
							sprintf (buffer, "���̽� �÷���Ʈ(����) ");
						} else {
							sprintf (buffer, "���̽� �÷���Ʈ(����) ");
						}
						fprintf (fp, buffer);

						// �԰�(���)
						sprintf (buffer, "�԰�(���): %s ", objectInfo.records.at(yy).at(2).c_str ());
						fprintf (fp, buffer);

						// �԰�(�Ϻ�) - ���û���
						if (atoi (objectInfo.records.at(yy).at(4).c_str ()) == 1) {
							sprintf (buffer, "�԰�(�Ϻ�): %s ", objectInfo.records.at(yy).at(3).c_str ());
						}
						fprintf (fp, buffer);

					} else if (objectInfo.keyDesc.at(xx).compare ("�簢������") == 0) {
						// �簢������
						if (atof (objectInfo.records.at(yy).at(1).c_str ()) < EPS) {
							length = atof (objectInfo.records.at(yy).at(2).c_str ());
							sprintf (buffer, "50 x 50 x %.0f ", round (length*1000, 0));

						// ���簢������
						} else {
							length = atof (objectInfo.records.at(yy).at(2).c_str ());
							sprintf (buffer, "%s x %.0f ", objectInfo.records.at(yy).at(1).c_str (), round (length*1000, 0));
						}
						fprintf (fp, buffer);

					} else if (objectInfo.keyDesc.at(xx).compare ("����������") == 0) {
						length = atof (objectInfo.records.at(yy).at(1).c_str ());
						sprintf (buffer, "%.0f ", round (length*1000, 0));
						fprintf (fp, buffer);

					} else if (objectInfo.keyDesc.at(xx).compare ("�ƿ��ڳʾޱ�") == 0) {
						length = atof (objectInfo.records.at(yy).at(1).c_str ());
						sprintf (buffer, "%.0f ", round (length*1000, 0));
						fprintf (fp, buffer);

					} else if (objectInfo.keyDesc.at(xx).compare ("������") == 0) {
						if (atoi (objectInfo.records.at(yy).at(2).c_str ()) > 0) {
							length = atof (objectInfo.records.at(yy).at(3).c_str ());
							length2 = atof (objectInfo.records.at(yy).at(4).c_str ());
							length3 = atof (objectInfo.records.at(yy).at(5).c_str ());
							totalAreaOfPlywoods += (atof (objectInfo.records.at(yy).at(6).c_str ()) * count);
							sprintf (buffer, "%.0f / ����(%.0f X %.0f) ", round (atof (objectInfo.records.at(yy).at(1).c_str ())*1000, 0), round ((length - length2)*1000, 0), round (length3*1000, 0));
						} else {
							length = atof (objectInfo.records.at(yy).at(1).c_str ());
							sprintf (buffer, "%.0f ", round (length*1000, 0));
						}
						fprintf (fp, buffer);

					} else if (objectInfo.keyDesc.at(xx).compare ("�����ƿ��ڳ�") == 0) {
						sprintf (buffer, "Ÿ��(%s) %.0f ", objectInfo.records.at(yy).at(1).c_str (), round (atof (objectInfo.records.at(yy).at(2).c_str ())*1000, 0));
						fprintf (fp, buffer);
						if (atoi (objectInfo.records.at(yy).at(3).c_str ()) > 0) {
							totalAreaOfPlywoods += (atof (objectInfo.records.at(yy).at(6).c_str ()) * count);
							sprintf (buffer, "����1(%.0f X %.0f) ", round (atof (objectInfo.records.at(yy).at(2).c_str ())*1000, 0), round (atof (objectInfo.records.at(yy).at(4).c_str ())*1000, 0));
							fprintf (fp, buffer);
							sprintf (buffer, "����2(%.0f X %.0f) ", round (atof (objectInfo.records.at(yy).at(2).c_str ())*1000, 0), round (atof (objectInfo.records.at(yy).at(5).c_str ())*1000, 0));
							fprintf (fp, buffer);
						}

					} else if (objectInfo.keyDesc.at(xx).compare ("�������ڳ�") == 0) {
						if (atoi (objectInfo.records.at(yy).at(3).c_str ()) > 0) {
							length = atof (objectInfo.records.at(yy).at(4).c_str ());
							length2 = atof (objectInfo.records.at(yy).at(5).c_str ());
							length3 = atof (objectInfo.records.at(yy).at(6).c_str ());
							totalAreaOfPlywoods += (atof (objectInfo.records.at(yy).at(7).c_str ()) * count);
							sprintf (buffer, "%.0f / ����(%.0f X %.0f) ", round (atof (objectInfo.records.at(yy).at(2).c_str ())*1000, 0), round ((length - length2)*1000, 0), round (length3*1000, 0));
						} else {
							length = atof (objectInfo.records.at(yy).at(2).c_str ());
							sprintf (buffer, "%.0f ", round (length*1000, 0));
						}
						fprintf (fp, buffer);

					} else if (objectInfo.keyDesc.at(xx).compare ("���纸 ����� v2") == 0) {
						if (atoi (objectInfo.records.at(yy).at(1).c_str ()) > 0) {
							length = atof (objectInfo.records.at(yy).at(2).c_str ()) / 1000;
							totalLengthOfTimbers_40x50 += (length * count);
							sprintf (buffer, "����(%.0f) ", round (length*1000, 0));
						}
						fprintf (fp, buffer);

					} else if (objectInfo.keyDesc.at(xx).compare ("�ܿ���") == 0) {
						sprintf (buffer, "����ũ��: %.0f X %.0f / ����ũ��: %.0f X %.0f (���������� �ڸ�: %s)",
							round (atof (objectInfo.records.at(yy).at(2).c_str ())*1000, 0), round (atof (objectInfo.records.at(yy).at(3).c_str ())*1000, 0),
							round (atof (objectInfo.records.at(yy).at(4).c_str ())*1000, 0), round (atof (objectInfo.records.at(yy).at(5).c_str ())*1000, 0),
							(atoi (objectInfo.records.at(yy).at(5).c_str ()) ? "�ڸ�" : "�ڸ��� ����"));
						fprintf (fp, buffer);

					} else if (objectInfo.keyDesc.at(xx).compare ("PERI���ٸ� ������") == 0) {
						length = atof (objectInfo.records.at(yy).at(2).c_str ());
						if (atoi (objectInfo.records.at(yy).at(3).c_str ()) == 1) {
							sprintf (buffer, "�԰�(%s) ����(%.0f) ũ�ν����(%s) ", objectInfo.records.at(yy).at(1).c_str (), round (length*1000, 0), objectInfo.records.at(yy).at(4).c_str ());
						} else {
							sprintf (buffer, "�԰�(%s) ����(%.0f) ", objectInfo.records.at(yy).at(1).c_str (), round (length*1000, 0));
						}
						fprintf (fp, buffer);
						
					} else {
						for (zz = 0 ; zz < objectInfo.nInfo.at(xx) ; ++zz) {
							// ������ �� ���
							sprintf (buffer, "%s(%s) ", objectInfo.varDesc.at(xx).at(zz).c_str (), objectInfo.records.at(yy).at(zz+1).c_str ());
							fprintf (fp, buffer);
						}
					}

					// ���� ���
					sprintf (buffer, ": %s EA\n", objectInfo.records.at(yy).at(objectInfo.records.at(yy).size ()-1).c_str ());
					fprintf (fp, buffer);
				}
			}
		}
	} catch (exception& ex) {
		WriteReport ("��� �Լ����� ���� �߻�: %s", ex.what ());
	}

	// �Ϲ� ��� - ��
	//for (xx = 0 ; xx < objectInfo.nCountsBeam ; ++xx) {
	//	if (xx == 0) {
	//		fprintf (fp, "\n[��]\n");
	//	}
	//	sprintf (buffer, "%d : %d EA\n", objectInfo.beamLength [xx], objectInfo.beamCount [xx]);
	//	fprintf (fp, buffer);
	//}

	// �� �� ���� ��ü
	if (objectInfo.nUnknownObjects > 0) {
		sprintf (buffer, "\n�� �� ���� ��ü : %d EA\n", objectInfo.nUnknownObjects);
		fprintf (fp, buffer);
	}

	// *����, ���� ���� ���� ���
	// ���� 4x8 �԰� (1200*2400) �������� �� ������ ������ ���� ���� ������ ����
	if (totalAreaOfPlywoods > EPS) {
		sprintf (buffer, "\n���� ���� ������ ������ �����ϴ�.\n�� ���� (%.2f ��) �� ���� 4x8 �԰� (1200*2400) = %.0f �� (���� 5�ۼ�Ʈ �����)\n", totalAreaOfPlywoods, ceil ((totalAreaOfPlywoods / 2.88)*1.05));
		fprintf (fp, buffer);
	}
	// ���� �ٷ糢(40*50), ������(50*80), ��°�(80*80), 1���� 3600mm
	if ((totalLengthOfTimbers_40x50 > EPS) || (totalLengthOfTimbers_50x80 > EPS) || (totalLengthOfTimbers_80x80 > EPS) || (totalLengthOfTimbersEtc > EPS)) {
		sprintf (buffer, "\n���� ���� ������ ������ �����ϴ�.\n");
		fprintf (fp, buffer);
		if (totalLengthOfTimbers_40x50 > EPS) {
			sprintf (buffer, "�ٷ糢 (40*50) : �� ���� (%.3f) �� 1�� (3600) = %.0f �� (���� 5�ۼ�Ʈ �����)\n", totalLengthOfTimbers_40x50, ceil ((totalLengthOfTimbers_40x50 / 3.6)*1.05));
			fprintf (fp, buffer);
		}
		if (totalLengthOfTimbers_50x80 > EPS) {
			sprintf (buffer, "������ (50*80) : �� ���� (%.3f) �� 1�� (3600) = %.0f �� (���� 5�ۼ�Ʈ �����)\n", totalLengthOfTimbers_50x80, ceil ((totalLengthOfTimbers_50x80 / 3.6)*1.05));
			fprintf (fp, buffer);
		}
		if (totalLengthOfTimbers_80x80 > EPS) {
			sprintf (buffer, "��°� (80*80) : �� ���� (%.3f) �� 1�� (3600) = %.0f �� (���� 5�ۼ�Ʈ �����)\n", totalLengthOfTimbers_80x80, ceil ((totalLengthOfTimbers_80x80 / 3.6)*1.05));
			fprintf (fp, buffer);
		}
		if (totalLengthOfTimbersEtc > EPS) {
			sprintf (buffer, "��԰� : �� ���� (%.3f) �� 1�� (3600) = %.0f �� (���� 5�ۼ�Ʈ �����)\n", totalLengthOfTimbersEtc, ceil ((totalLengthOfTimbersEtc / 3.6)*1.05));
			fprintf (fp, buffer);
		}
	}
	if ((totalAreaOfPlywoods > EPS) || (totalLengthOfTimbers_40x50 > EPS) || (totalLengthOfTimbers_50x80 > EPS) || (totalLengthOfTimbers_80x80 > EPS) || (totalLengthOfTimbersEtc > EPS)) {
		sprintf (buffer, "\n���ǻ���: ����/���� ���� ������ ���� ��ü�� ���ؼ��� ���Ǿ����ϴ�. �߰��� ��ü�� �ִٸ� �����ڿ��� �����Ͻʽÿ�.\n���� / ����(�ٰ���) / ���� / ������ / �����ƿ��ڳ� / �������ڳ� / ���纸 ����� v2\n");
		fprintf (fp, buffer);
	}

	fclose (fp);

	// ��ü �������� ���� ��� (�߰�����)
	try {
		for (xx = 0 ; xx < objectInfo.keyDesc.size () ; ++xx) {
			// ���ڵ带 ���� ��ȸ
			for (yy = 0 ; yy < objectInfo.records.size () ; ++yy) {
				// ��ü ���� �̸��� ���ڵ��� 1�� �ʵ尡 ��ġ�ϴ� ��츸 ã�Ƽ� �����
				retVal = my_strcmp (objectInfo.keyDesc.at(xx).c_str (), objectInfo.records.at(yy).at(0).c_str ());

				if (retVal == 0) {
					// ǰ��
					sprintf (buffer, "%s | ", objectInfo.keyDesc.at(xx).c_str ());
					fprintf (fp_interReport, buffer);

					// ������ �� ���
					if ((my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "������") == 0) || (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "��ƿ��") == 0)) {
						// �԰�
						if (atoi (objectInfo.records.at(yy).at(1).c_str ()) > 0) {
							// �԰���
							sprintf (buffer, "%s X %s | ", objectInfo.records.at(yy).at(2), objectInfo.records.at(yy).at(3));
						} else {
							// ��԰�ǰ
							length = atof (objectInfo.records.at(yy).at(4).c_str ());
							length2 = atof (objectInfo.records.at(yy).at(5).c_str ());
							sprintf (buffer, "%.0f X %.0f | ", round (length*1000, 0), round (length2*1000, 0));
						}
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);
						
						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if ((my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "���ڳ��ǳ�") == 0) || (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "�ƿ��ڳ��ǳ�") == 0)) {
						length = atof (objectInfo.records.at(yy).at(1).c_str ());
						length2 = atof (objectInfo.records.at(yy).at(2).c_str ());
						length3 = atof (objectInfo.records.at(yy).at(3).c_str ());

						// �԰�
						sprintf (buffer, "%.0f X %.0f | ", round (length*1000, 0), round (length2*1000, 0));
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "%.0f | ", round (length3*1000, 0));
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "�ƿ��ڳʾޱ�") == 0) {
						length = atof (objectInfo.records.at(yy).at(1).c_str ());

						// �԰�
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "%.0f | ", round (length*1000, 0));
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "����") == 0) {
						length = atof (objectInfo.records.at(yy).at(1).c_str ());
						length2 = atof (objectInfo.records.at(yy).at(2).c_str ());
						length3 = atof (objectInfo.records.at(yy).at(3).c_str ());

						// �԰�
						sprintf (buffer, "%.0f X %.0f | ", round (length*1000, 0), round (length2*1000, 0));
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "%.0f | ", round (length3*1000, 0));
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "�� | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "�ٷ������̼�") == 0) {
						length = atof (objectInfo.records.at(yy).at(1).c_str ());
						length2 = atof (objectInfo.records.at(yy).at(2).c_str ());

						// �԰�
						sprintf (buffer, "%.0f | ", round (length*1000, 0));
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "%.0f | ", round (length2*1000, 0));
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "����������") == 0) {
						length = atof (objectInfo.records.at(yy).at(1).c_str ());

						// �԰�
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "%.0f | ", round (length*1000, 0));
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "�簢������") == 0) {
						length = atof (objectInfo.records.at(yy).at(2).c_str ());

						// �԰�
						if (atof (objectInfo.records.at(yy).at(1).c_str ()) < EPS) {
							// �簢������
							sprintf (buffer, "50 x 50 | ");
						} else {
							// ���簢������
							sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						}
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "%.0f | ", round (length*1000, 0));
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "�� | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "���ǳ�") == 0) {
						// �԰�
						if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x6 [910x1820]") == 0) {
							sprintf (buffer, "910 X 1820 X %s | ", objectInfo.records.at(yy).at(2).c_str ());

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "4x8 [1220x2440]") == 0) {
							sprintf (buffer, "1220 X 2440 X %s | ", objectInfo.records.at(yy).at(2).c_str ());

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x5 [606x1520]") == 0) {
							sprintf (buffer, "606 X 1520 X %s | ", objectInfo.records.at(yy).at(2).c_str ());

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x6 [606x1820]") == 0) {
							sprintf (buffer, "606 X 1820 X %s | ", objectInfo.records.at(yy).at(2).c_str ());

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x5 [910x1520]") == 0) {
							sprintf (buffer, "910 X 1520 X %s | ", objectInfo.records.at(yy).at(2).c_str ());

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "��԰�") == 0) {
							// ���� X ���� X �β�
							length = atof (objectInfo.records.at(yy).at(3).c_str ());
							length2 = atof (objectInfo.records.at(yy).at(4).c_str ());
							sprintf (buffer, "%.0f X %.0f X %s | ", round (length*1000, 0), round (length2*1000, 0), objectInfo.records.at(yy).at(2).c_str ());
						}
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "�� | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "����") == 0) {
						// �԰�
						if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x6 [910x1820]") == 0) {
							sprintf (buffer, "910 X 1820 X %s | ", objectInfo.records.at(yy).at(2).c_str ());

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "4x8 [1220x2440]") == 0) {
							sprintf (buffer, "1220 X 2440 X %s | ", objectInfo.records.at(yy).at(2).c_str ());

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x5 [606x1520]") == 0) {
							sprintf (buffer, "606 X 1520 X %s | ", objectInfo.records.at(yy).at(2).c_str ());

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x6 [606x1820]") == 0) {
							sprintf (buffer, "606 X 1820 X %s | ", objectInfo.records.at(yy).at(2).c_str ());

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x5 [910x1520]") == 0) {
							sprintf (buffer, "910 X 1520 X %s | ", objectInfo.records.at(yy).at(2).c_str ());

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "��԰�") == 0) {
							// ���� X ���� X �β�
							length = atof (objectInfo.records.at(yy).at(3).c_str ());
							length2 = atof (objectInfo.records.at(yy).at(4).c_str ());
							sprintf (buffer, "%.0f X %.0f X %s | ", round (length*1000, 0), round (length2*1000, 0), objectInfo.records.at(yy).at(2).c_str ());
						}
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "�� | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "RS Push-Pull Props ����ǽ� (�ξ�� ����)") == 0) {
						// �԰�
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "RS Push-Pull Props") == 0) {
						// �԰�
						if (atoi (objectInfo.records.at(yy).at(4).c_str ()) == 1) {
							// �Ϻ� ������ ���� ���
							sprintf (buffer, "%s, %s | ", objectInfo.records.at(yy).at(2).c_str (), objectInfo.records.at(yy).at(3).c_str ());
						} else {
							// �Ϻ� ������ ���� ���
							sprintf (buffer, "%s, - | ", objectInfo.records.at(yy).at(2).c_str ());
						}
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "�ɺ�Ʈ��Ʈ") == 0) {
						// �԰�
						length = atof (objectInfo.records.at(yy).at(2).c_str ());
						sprintf (buffer, "%.0f X %.0f | ", round (length*1000, 0), round (length*1000, 0));
						fprintf (fp_interReport, buffer);

						// ����
						length = atof (objectInfo.records.at(yy).at(1).c_str ());
						sprintf (buffer, "%.0f | ", round (length*1000, 0));
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "����ö�� (�簢�ͼ�Ȱ��)") == 0) {
						// �԰�
						length = atof (objectInfo.records.at(yy).at(2).c_str ());
						sprintf (buffer, "%.0f X %.0f | ", round (length*1000, 0), round (length*1000, 0));
						fprintf (fp_interReport, buffer);

						// ����
						length = atof (objectInfo.records.at(yy).at(1).c_str ());
						sprintf (buffer, "%.0f | ", round (length*1000, 0));
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "�� �ۿ���") == 0) {
						// �԰�
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "PERI���ٸ� ������") == 0) {
						// �԰�
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(2).c_str ());
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "PERI���ٸ� ������") == 0) {
						// �԰�
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "GT24 �Ŵ�") == 0) {
						// �԰�
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "������") == 0) {
						// �԰� (������ ��ü ����)
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp_interReport, buffer);

						// ���� (���� �ʺ� X ����)
						length = atof (objectInfo.records.at(yy).at(3).c_str ());
						length2 = atof (objectInfo.records.at(yy).at(4).c_str ());
						if (atoi (objectInfo.records.at(yy).at(2).c_str ()) == 1) {
							sprintf (buffer, "%s X %.0f | ", objectInfo.records.at(yy).at(5).c_str (), abs (round (length*1000, 0) - round (length2*1000, 0)));
						} else {
							sprintf (buffer, "- | ");
						}
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "�������ڳ�") == 0) {
						// �԰� (������ �ʺ� X ����)
						sprintf (buffer, "%s X %s | ", objectInfo.records.at(yy).at(1).c_str (), objectInfo.records.at(yy).at(2).c_str ());
						fprintf (fp_interReport, buffer);

						// ���� (���� �ʺ� X ����)
						length = atof (objectInfo.records.at(yy).at(4).c_str ());
						length2 = atof (objectInfo.records.at(yy).at(5).c_str ());
						if (atoi (objectInfo.records.at(yy).at(3).c_str ()) == 1) {
							sprintf (buffer, "%s X %.0f | ", objectInfo.records.at(yy).at(6).c_str (), abs (round (length*1000, 0) - round (length2*1000, 0)));
						} else {
							sprintf (buffer, "- | ");
						}
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "������ ��ũ") == 0) {
						// �԰�
						if (objectInfo.records.at(yy).at(2).compare ("����") == 0) {
							sprintf (buffer, "%s, ���� | ", objectInfo.records.at(yy).at(1).c_str ());
						} else if (objectInfo.records.at(yy).at(2).compare ("�簢") == 0) {
							sprintf (buffer, "%s, �簢 | ", objectInfo.records.at(yy).at(1).c_str ());
						}
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);
						
						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "�����") == 0) {
						// �԰�
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);
						
						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "����Ʈ") == 0) {
						// �԰�
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);
						
						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "������ ���̺��� (���ǳ�)") == 0) {
						// �԰�
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "���Ŭ����") == 0) {
						// �԰�
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);
						
						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "������Ʈ�� Push-Pull Props ����ǽ�") == 0) {
						// �԰�
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp_interReport, buffer);

						// ���� 
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "��� �� �����") == 0) {
						// �԰�
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "�ܿ���") == 0) {
						// �԰�
						if (atoi (objectInfo.records.at(yy).at(1).c_str ()) == 1) {
							// ���� ���� X ����
							sprintf (buffer, "%s X %s | ", objectInfo.records.at(yy).at(2).c_str (), objectInfo.records.at(yy).at(3).c_str ());
						} else {
							// ���� ���� X ����
							sprintf (buffer, "%s X %s | ", objectInfo.records.at(yy).at(4).c_str (), objectInfo.records.at(yy).at(5).c_str ());
						}
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "�� | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "Push-Pull Props (�⼺ǰ �� �������ǰ)") == 0) {
						// �԰�
						if (atoi (objectInfo.records.at(yy).at(4).c_str ()) == 1) {
							// �Ϻ� ������ ���� ���
							sprintf (buffer, "%s, %s | ", objectInfo.records.at(yy).at(2).c_str (), objectInfo.records.at(yy).at(3).c_str ());
						} else {
							// �Ϻ� ������ ���� ���
							sprintf (buffer, "%s, - | ", objectInfo.records.at(yy).at(2).c_str ());
						}
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);

					} else {
						// �԰�, ���� ���� ������ ǥ���� ���

						// �԰�
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// ����
						sprintf (buffer, "����(EA) | ");
						fprintf (fp_interReport, buffer);
					}

					// ���� ���
					sprintf (buffer, "%s\n", objectInfo.records.at(yy).at(objectInfo.records.at(yy).size ()-1).c_str ());
					fprintf (fp_interReport, buffer);
				}
			}
		}
	} catch (exception& ex) {
		WriteReport ("��� �Լ����� ���� �߻�: %s", ex.what ());
	}

	// �� �� ���� ��ü
	if (objectInfo.nUnknownObjects > 0) {
		sprintf (buffer, "�� �� ���� ��ü | - | - | - | %d\n", objectInfo.nUnknownObjects);
		fprintf (fp_interReport, buffer);
	}

	fclose (fp_interReport);

	// ȭ�� ���ΰ�ħ
	//ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	//ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

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
	unsigned short		xx, yy, zz;
	short		mm;
	bool		regenerate = true;
	bool		suspGrp;

	// ��� ��ü���� ���� ��ǥ�� ���� ������
	vector<API_Coord3D>	vecPos;

	// ��� ��ü, �� ����
	GS::Array<API_Guid>		elemList;
	GS::Array<API_Guid>		objects;
	GS::Array<API_Guid>		beams;
	long					nObjects = 0;
	long					nBeams = 0;

	// ������ ��ҵ��� ���� ����ϱ�
	API_Element			elem;
	API_ElementMemo		memo;
	SummaryOfObjectInfo	objectInfo;

	char			tempStr [512];
	const char*		foundStr;
	bool			foundObject;
	bool			foundExistValue;
	int				retVal;
	int				nInfo;
	API_AddParID	varType;
	vector<string>	record;

	// GS::Array �ݺ���
	//GS::Array<API_Guid>::Iterator	iterObj;
	//API_Guid	curGuid;

	// ���̾� ���� ����
	short			nLayers;
	API_Attribute	attrib;
	short			nVisibleLayers = 0;
	short			visLayerList [1024];
	char			fullLayerName [512];

	// ���̾� Ÿ�Կ� ���� ĸ�� ���� ����
	char*			foundLayerName;
	short			layerType = UNDEFINED;

	// ��Ÿ
	char			buffer [512];
	char			filename [512];

	// �۾� �� ����
	API_StoryInfo	storyInfo;
	double			workLevel_object;		// ��ü�� �۾� �� ����


	// ����ٸ� ǥ���ϱ� ���� ����
	GS::UniString       title ("�������� ���� ��Ȳ");
	GS::UniString       subtitle ("������...");
	short	nPhase;
	Int32	cur, total;

	// ���� ���Ϸ� ��� ���� ��������
	// ���� ������ ���� ����
	API_SpecFolderID	specFolderID = API_ApplicationFolderID;
	IO::Location		location;
	GS::UniString		resultString;
	API_MiscAppInfo		miscAppInfo;
	FILE				*fp;
	FILE				*fp_unite;


	// �׷�ȭ �Ͻ����� ON
	ACAPI_Environment (APIEnv_IsSuspendGroupOnID, &suspGrp);
	if (suspGrp == false)	ACAPI_Element_Tool (NULL, NULL, APITool_SuspendGroups, NULL);

	// ȭ�� ���ΰ�ħ
	ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

	ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);

	// [���] ���̾�α׿��� ��ü �̹����� ĸ������ ���θ� ���
	//result = DGAlert (DG_INFORMATION, "ĸ�� ���� ����", "ĸ�� �۾��� �����Ͻðڽ��ϱ�?", "", "��", "�ƴϿ�", "");
	//result = DG_CANCEL;

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

	// ���� ��Ȳ ǥ���ϴ� ��� - �ʱ�ȭ
	nPhase = 1;
	cur = 1;
	total = nVisibleLayers;
	ACAPI_Interface (APIIo_InitProcessWindowID, &title, &nPhase);
	ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &total);

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
		objectInfo.clear ();
		objectInfo.nUnknownObjects = 0;

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

			for (xx = 0 ; xx < nObjects ; ++xx) {
				foundObject = false;

				BNZeroMemory (&elem, sizeof (API_Element));
				BNZeroMemory (&memo, sizeof (API_ElementMemo));
				elem.header.guid = objects.Pop ();
				err = ACAPI_Element_Get (&elem);

				if (err == NoError && elem.header.hasMemo) {
					err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

					if (err == NoError) {
						// ��ü�� ���� �����ϱ� ==================================
						//API_Coord3D	coord;

						//coord.x = elem.object.pos.x;
						//coord.y = elem.object.pos.y;
						//coord.z = elem.object.level;
					
						//vecPos.push_back (coord);
						// ��ü�� ���� �����ϱ� ==================================

						// �۾� �� ���� �ݿ� -- ��ü
						if (xx == 0) {
							BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
							workLevel_object = 0.0;
							ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
							for (yy = 0 ; yy <= (storyInfo.lastStory - storyInfo.firstStory) ; ++yy) {
								if (storyInfo.data [0][yy].index == elem.header.floorInd) {
									workLevel_object = storyInfo.data [0][yy].level;
									break;
								}
							}
							BMKillHandle ((GSHandle *) &storyInfo.data);
						}

						// �Ķ���� ��ũ��Ʈ�� ������ �����Ŵ
						ACAPI_Goodies (APIAny_RunGDLParScriptID, &elem.header, 0);

						try {
							for (yy = 0 ; yy < objectInfo.keyName.size () ; ++yy) {
								strcpy (tempStr, objectInfo.keyName.at(yy).c_str ());
								foundStr = getParameterStringByName (&memo, tempStr);

								// ��ü ������ ã�Ҵٸ�,
								if (my_strcmp (foundStr, "") != 0) {
									retVal = my_strcmp (objectInfo.keyDesc.at(yy).c_str (), foundStr);

									if (retVal == 0) {
										foundObject = true;
										foundExistValue = false;

										// �߰��� ��ü�� �����͸� ������� ���ڵ� �߰�
										if (!record.empty ())
											record.clear ();

										record.push_back (objectInfo.keyDesc.at(yy));		// ��ü �̸�
										nInfo = objectInfo.nInfo.at(yy);
										for (zz = 0 ; zz < nInfo ; ++zz) {
											sprintf (buffer, "%s", objectInfo.varName.at(yy).at(zz).c_str ());
											varType = getParameterTypeByName (&memo, buffer);

											if ((varType != APIParT_Separator) || (varType != APIParT_Title) || (varType != API_ZombieParT)) {
												if (varType == APIParT_CString)
													sprintf (tempStr, "%s", getParameterStringByName (&memo, buffer));	// ���ڿ�
												else
													sprintf (tempStr, "%.3f", getParameterValueByName (&memo, buffer));	// ����
											}
											record.push_back (tempStr);		// ������
										}

										objectInfo.quantityPlus1 (record);

									}
								}
							}
						} catch (exception& ex) {
							WriteReport ("��ü ���� �������� ���� �߻�: %s", ex.what ());
						}

						// ���� ã�� ���ϸ� �� �� ���� ��ü�� �����
						if (foundObject == false)
							objectInfo.nUnknownObjects ++;
					}

					ACAPI_DisposeElemMemoHdls (&memo);
				}
			}

			// �� ���� ����
			//for (xx = 0 ; xx < nBeams ; ++xx) {
			//	BNZeroMemory (&elem, sizeof (API_Element));
			//	BNZeroMemory (&memo, sizeof (API_ElementMemo));
			//	elem.header.guid = beams.Pop ();
			//	err = ACAPI_Element_Get (&elem);
			//	err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

			//	foundExistValue = false;

			//	int len;

			//	len = static_cast<int> (round (GetDistance (elem.beam.begC, elem.beam.endC) * 1000, 0));

			//	// �ߺ� �׸��� ������ ����
			//	for (zz = 0 ; zz < objectInfo.nCountsBeam ; ++zz) {
			//		if (objectInfo.beamLength [zz] == len) {
			//			objectInfo.beamCount [zz] ++;
			//			foundExistValue = true;
			//			break;
			//		}
			//	}

			//	// �ű� �׸� �߰��ϰ� ������ ����
			//	if ( !foundExistValue ) {
			//		objectInfo.beamLength.push_back (len);
			//		objectInfo.beamCount.push_back (1);
			//		objectInfo.nCountsBeam ++;
			//	}

			//	ACAPI_DisposeElemMemoHdls (&memo);
			//}

			// APIParT_Length�� ��� 1000�� ���ؼ� ǥ��
			// APIParT_Boolean�� ��� ��/�ƴϿ� ǥ��
			double	length, length2, length3;
			bool	bTitleAppeared;

			// ��ü �������� ���� ���
			try {
				for (xx = 0 ; xx < objectInfo.keyDesc.size () ; ++xx) {
					bTitleAppeared = false;

					// ���ڵ带 ���� ��ȸ
					for (yy = 0 ; yy < objectInfo.records.size () ; ++yy) {
						// ��ü ���� �̸��� ���ڵ��� 1�� �ʵ尡 ��ġ�ϴ� ��츸 ã�Ƽ� �����
						retVal = my_strcmp (objectInfo.keyDesc.at(xx).c_str (), objectInfo.records.at(yy).at(0).c_str ());

						if (retVal == 0) {
							// ���� ���
							if (bTitleAppeared == false) {
								sprintf (buffer, "\n[%s]\n", objectInfo.keyDesc.at(xx).c_str ());
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
								bTitleAppeared = true;
							}

							// ������ �� ���
							if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "������ ��ũ") == 0) {
								// ����
								if (objectInfo.records.at(yy).at(2).compare ("����") == 0) {
									sprintf (buffer, "���� / %s", objectInfo.records.at(yy).at(1));
								}

								// �簢
								if (objectInfo.records.at(yy).at(2).compare ("�簢") == 0) {
									sprintf (buffer, "�簢 / %s", objectInfo.records.at(yy).at(1));
								}
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

							} else if ((my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "������") == 0) || (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "��ƿ��") == 0)) {
								// �԰���
								if (atoi (objectInfo.records.at(yy).at(1).c_str ()) > 0) {
									sprintf (buffer, "%s X %s ", objectInfo.records.at(yy).at(2), objectInfo.records.at(yy).at(3));

								// ��԰�ǰ
								} else {
									length = atof (objectInfo.records.at(yy).at(4).c_str ());
									length2 = atof (objectInfo.records.at(yy).at(5).c_str ());
									sprintf (buffer, "%.0f X %.0f ", round (length*1000, 0), round (length2*1000, 0));
								}
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

							} else if (objectInfo.keyDesc.at(xx).compare ("����") == 0) {
								length = atof (objectInfo.records.at(yy).at(1).c_str ());
								length2 = atof (objectInfo.records.at(yy).at(2).c_str ());
								length3 = atof (objectInfo.records.at(yy).at(3).c_str ());
								sprintf (buffer, "%.0f X %.0f X %.0f ", round (length*1000, 0), round (length2*1000, 0), round (length3*1000, 0));
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

							} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "���ǳ�") == 0) {
								if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x6 [910x1820]") == 0) {
									sprintf (buffer, "910 X 1820 X %s ", objectInfo.records.at(yy).at(2).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);

								} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "4x8 [1220x2440]") == 0) {
									sprintf (buffer, "1220 X 2440 X %s ", objectInfo.records.at(yy).at(2).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);

								} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x5 [606x1520]") == 0) {
									sprintf (buffer, "606 X 1520 X %s ", objectInfo.records.at(yy).at(2).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);

								} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x6 [606x1820]") == 0) {
									sprintf (buffer, "606 X 1820 X %s ", objectInfo.records.at(yy).at(2).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);

								} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x5 [910x1520]") == 0) {
									sprintf (buffer, "910 X 1520 X %s ", objectInfo.records.at(yy).at(2).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);

								} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "��԰�") == 0) {
									// ���� X ���� X �β�
									length = atof (objectInfo.records.at(yy).at(3).c_str ());
									length2 = atof (objectInfo.records.at(yy).at(4).c_str ());
									sprintf (buffer, "%.0f X %.0f X %s ", round (length*1000, 0), round (length2*1000, 0), objectInfo.records.at(yy).at(2).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);
								}

							} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "����") == 0) {
								if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x6 [910x1820]") == 0) {
									sprintf (buffer, "910 X 1820 X %s ", objectInfo.records.at(yy).at(2).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);

									// ����Ʋ ON
									if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
										sprintf (buffer, "(���� �ѱ���: %s) \n", objectInfo.records.at(yy).at(6).c_str ());
										fprintf (fp, buffer);
										fprintf (fp_unite, buffer);

										sprintf (buffer, "(���� ���� ����: %s) ", objectInfo.records.at(yy).at(7).c_str ());
										fprintf (fp, buffer);
										fprintf (fp_unite, buffer);
									}

								} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "4x8 [1220x2440]") == 0) {
									sprintf (buffer, "1220 X 2440 X %s ", objectInfo.records.at(yy).at(2).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);

									// ����Ʋ ON
									if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
										sprintf (buffer, "(���� �ѱ���: %s) \n", objectInfo.records.at(yy).at(6).c_str ());
										fprintf (fp, buffer);
										fprintf (fp_unite, buffer);

										sprintf (buffer, "(���� ���� ����: %s) ", objectInfo.records.at(yy).at(7).c_str ());
										fprintf (fp, buffer);
										fprintf (fp_unite, buffer);
									}

								} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x5 [606x1520]") == 0) {
									sprintf (buffer, "606 X 1520 X %s ", objectInfo.records.at(yy).at(2).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);

									// ����Ʋ ON
									if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
										sprintf (buffer, "(���� �ѱ���: %s) \n", objectInfo.records.at(yy).at(6).c_str ());
										fprintf (fp, buffer);
										fprintf (fp_unite, buffer);

										sprintf (buffer, "(���� ���� ����: %s) ", objectInfo.records.at(yy).at(7).c_str ());
										fprintf (fp, buffer);
										fprintf (fp_unite, buffer);
									}

								} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x6 [606x1820]") == 0) {
									sprintf (buffer, "606 X 1820 X %s ", objectInfo.records.at(yy).at(2).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);

									// ����Ʋ ON
									if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
										sprintf (buffer, "(���� �ѱ���: %s) \n", objectInfo.records.at(yy).at(6).c_str ());
										fprintf (fp, buffer);
										fprintf (fp_unite, buffer);

										sprintf (buffer, "(���� ���� ����: %s) ", objectInfo.records.at(yy).at(7).c_str ());
										fprintf (fp, buffer);
										fprintf (fp_unite, buffer);
									}

								} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x5 [910x1520]") == 0) {
									sprintf (buffer, "910 X 1520 X %s ", objectInfo.records.at(yy).at(2).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);

									// ����Ʋ ON
									if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
										sprintf (buffer, "(���� �ѱ���: %s) \n", objectInfo.records.at(yy).at(6).c_str ());
										fprintf (fp, buffer);
										fprintf (fp_unite, buffer);

										sprintf (buffer, "(���� ���� ����: %s) ", objectInfo.records.at(yy).at(7).c_str ());
										fprintf (fp, buffer);
										fprintf (fp_unite, buffer);
									}

								} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "��԰�") == 0) {
									// ���� X ���� X �β�
									length = atof (objectInfo.records.at(yy).at(3).c_str ());
									length2 = atof (objectInfo.records.at(yy).at(4).c_str ());
									sprintf (buffer, "%.0f X %.0f X %s ", round (length*1000, 0), round (length2*1000, 0), objectInfo.records.at(yy).at(2).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);

									// ����Ʋ ON
									if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
										sprintf (buffer, "(���� �ѱ���: %s) \n", objectInfo.records.at(yy).at(6).c_str ());
										fprintf (fp, buffer);
										fprintf (fp_unite, buffer);

										sprintf (buffer, "(���� ���� ����: %s) ", objectInfo.records.at(yy).at(7).c_str ());
										fprintf (fp, buffer);
										fprintf (fp_unite, buffer);
									}

								} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "������") == 0) {
									sprintf (buffer, "������ ");
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);

								} else {
									sprintf (buffer, "�ٰ��� ");
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);
								}

							} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "����(�ٰ���)") == 0) {
								// ���� ����
								sprintf (buffer, "����: %.2f ", atof (objectInfo.records.at(yy).at(1).c_str ()));
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

								// ����Ʋ ON
								if (atoi (objectInfo.records.at(yy).at(2).c_str ()) > 0) {
									sprintf (buffer, "(���� �ѱ���: %.0f) ", round (atof (objectInfo.records.at(yy).at(3).c_str ())*1000, 0));
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);
								}

							} else if (objectInfo.keyDesc.at(xx).compare ("RS Push-Pull Props") == 0) {
								// ���̽� �÷���Ʈ ����
								if (atoi (objectInfo.records.at(yy).at(1).c_str ()) == 1) {
									sprintf (buffer, "���̽� �÷���Ʈ(����) ");
								} else {
									sprintf (buffer, "���̽� �÷���Ʈ(����) ");
								}
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

								// �԰�(���)
								sprintf (buffer, "�԰�(���): %s ", objectInfo.records.at(yy).at(2).c_str ());
								fprintf (fp, buffer);

								// �԰�(�Ϻ�) - ���û���
								if (atoi (objectInfo.records.at(yy).at(4).c_str ()) == 1) {
									sprintf (buffer, "�԰�(�Ϻ�): %s ", objectInfo.records.at(yy).at(3).c_str ());
								}
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
				
							} else if (objectInfo.keyDesc.at(xx).compare ("Push-Pull Props (�⼺ǰ �� �������ǰ)") == 0) {
								// ���̽� �÷���Ʈ ����
								if (atoi (objectInfo.records.at(yy).at(1).c_str ()) == 1) {
									sprintf (buffer, "���̽� �÷���Ʈ(����) ");
								} else {
									sprintf (buffer, "���̽� �÷���Ʈ(����) ");
								}
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

								// �԰�(���)
								sprintf (buffer, "�԰�(���): %s ", objectInfo.records.at(yy).at(2).c_str ());
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

								// �԰�(�Ϻ�) - ���û���
								if (atoi (objectInfo.records.at(yy).at(4).c_str ()) == 1) {
									sprintf (buffer, "�԰�(�Ϻ�): %s ", objectInfo.records.at(yy).at(3).c_str ());
								}
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

							} else if (objectInfo.keyDesc.at(xx).compare ("�簢������") == 0) {
								// �簢������
								if (atof (objectInfo.records.at(yy).at(1).c_str ()) < EPS) {
									length = atof (objectInfo.records.at(yy).at(2).c_str ());
									sprintf (buffer, "50 x 50 x %.0f ", round (length*1000, 0));

								// ���簢������
								} else {
									length = atof (objectInfo.records.at(yy).at(2).c_str ());
									sprintf (buffer, "%s x %.0f ", objectInfo.records.at(yy).at(1).c_str (), round (length*1000, 0));
								}
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

							} else if (objectInfo.keyDesc.at(xx).compare ("����������") == 0) {
								length = atof (objectInfo.records.at(yy).at(1).c_str ());
								sprintf (buffer, "%.0f ", round (length*1000, 0));
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

							} else if (objectInfo.keyDesc.at(xx).compare ("�ƿ��ڳʾޱ�") == 0) {
								length = atof (objectInfo.records.at(yy).at(1).c_str ());
								sprintf (buffer, "%.0f ", round (length*1000, 0));
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

							} else if (objectInfo.keyDesc.at(xx).compare ("������") == 0) {
								if (atoi (objectInfo.records.at(yy).at(2).c_str ()) > 0) {
									length = atof (objectInfo.records.at(yy).at(3).c_str ());
									length2 = atof (objectInfo.records.at(yy).at(4).c_str ());
									length3 = atof (objectInfo.records.at(yy).at(5).c_str ());
									sprintf (buffer, "%.0f / ����(%.0f X %.0f) ", round (atof (objectInfo.records.at(yy).at(1).c_str ())*1000, 0), round ((length - length2)*1000, 0), round (length3*1000, 0));
								} else {
									length = atof (objectInfo.records.at(yy).at(1).c_str ());
									sprintf (buffer, "%.0f ", round (length*1000, 0));
								}
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

							} else if (objectInfo.keyDesc.at(xx).compare ("�����ƿ��ڳ�") == 0) {
								sprintf (buffer, "Ÿ��(%s) %.0f ", objectInfo.records.at(yy).at(1).c_str (), round (atof (objectInfo.records.at(yy).at(2).c_str ())*1000, 0));
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
								if (atoi (objectInfo.records.at(yy).at(3).c_str ()) > 0) {
									sprintf (buffer, "����1(%.0f X %.0f) ", round (atof (objectInfo.records.at(yy).at(2).c_str ())*1000, 0), round (atof (objectInfo.records.at(yy).at(4).c_str ())*1000, 0));
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);
									sprintf (buffer, "����2(%.0f X %.0f) ", round (atof (objectInfo.records.at(yy).at(2).c_str ())*1000, 0), round (atof (objectInfo.records.at(yy).at(5).c_str ())*1000, 0));
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);
								}

							} else if (objectInfo.keyDesc.at(xx).compare ("�������ڳ�") == 0) {
								if (atoi (objectInfo.records.at(yy).at(3).c_str ()) > 0) {
									length = atof (objectInfo.records.at(yy).at(4).c_str ());
									length2 = atof (objectInfo.records.at(yy).at(5).c_str ());
									length3 = atof (objectInfo.records.at(yy).at(6).c_str ());
									sprintf (buffer, "%.0f / ����(%.0f X %.0f) ", round (atof (objectInfo.records.at(yy).at(2).c_str ())*1000, 0), round ((length - length2)*1000, 0), round (length3*1000, 0));
								} else {
									length = atof (objectInfo.records.at(yy).at(2).c_str ());
									sprintf (buffer, "%.0f ", round (length*1000, 0));
								}
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

							} else if (objectInfo.keyDesc.at(xx).compare ("���纸 ����� v2") == 0) {
								if (atoi (objectInfo.records.at(yy).at(1).c_str ()) > 0) {
									length = atof (objectInfo.records.at(yy).at(2).c_str ());
									sprintf (buffer, "����(%.0f) ", round (length*1000, 0));
								}
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

							} else if (objectInfo.keyDesc.at(xx).compare ("�ܿ���") == 0) {
								sprintf (buffer, "����ũ��: %.0f X %.0f / ����ũ��: %.0f X %.0f (���������� �ڸ�: %s)",
									round (atof (objectInfo.records.at(yy).at(2).c_str ())*1000, 0), round (atof (objectInfo.records.at(yy).at(3).c_str ())*1000, 0),
									round (atof (objectInfo.records.at(yy).at(4).c_str ())*1000, 0), round (atof (objectInfo.records.at(yy).at(5).c_str ())*1000, 0),
									(atoi (objectInfo.records.at(yy).at(5).c_str ()) ? "�ڸ�" : "�ڸ��� ����"));
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
						
							} else if (objectInfo.keyDesc.at(xx).compare ("PERI���ٸ� ������") == 0) {
								length = atof (objectInfo.records.at(yy).at(2).c_str ());
								if (atoi (objectInfo.records.at(yy).at(3).c_str ()) == 1) {
									sprintf (buffer, "�԰�(%s) ����(%.0f) ũ�ν����(%s) ", objectInfo.records.at(yy).at(1).c_str (), round (length*1000, 0), objectInfo.records.at(yy).at(4).c_str ());
								} else {
									sprintf (buffer, "�԰�(%s) ����(%.0f) ", objectInfo.records.at(yy).at(1).c_str (), round (length*1000, 0));
								}
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

							} else {
								for (zz = 0 ; zz < objectInfo.nInfo.at(xx) ; ++zz) {
									// ������ �� ���
									sprintf (buffer, "%s(%s) ", objectInfo.varDesc.at(xx).at(zz).c_str (), objectInfo.records.at(yy).at(zz+1).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);
								}
							}

							// ���� ���
							sprintf (buffer, ": %s EA\n", objectInfo.records.at(yy).at(objectInfo.records.at(yy).size ()-1).c_str ());
							fprintf (fp, buffer);
							fprintf (fp_unite, buffer);
						}
					}
				}
			} catch (exception& ex) {
				WriteReport ("��� �Լ����� ���� �߻�: %s", ex.what ());
			}

			// �Ϲ� ��� - ��
			//for (xx = 0 ; xx < objectInfo.nCountsBeam ; ++xx) {
			//	if (xx == 0) {
			//		fprintf (fp, "\n[��]\n");
			//	}
			//	sprintf (buffer, "%d : %d EA\n", objectInfo.beamLength [xx], objectInfo.beamCount [xx]);
			//	fprintf (fp, buffer);
			//	fprintf (fp_unite, buffer);
			//}

			// �� �� ���� ��ü
			if (objectInfo.nUnknownObjects > 0) {
				sprintf (buffer, "\n�� �� ���� ��ü : %d EA\n", objectInfo.nUnknownObjects);
				fprintf (fp, buffer);
				fprintf (fp_unite, buffer);
			}

			fclose (fp);

			/*
			// 3D ���� ���� ==================================
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
				// 3D ���� ���� ==================================
			}
			*/


			// ���̾� �����
			attrib.layer.head.flags |= APILay_Hidden;
			ACAPI_Attribute_Modify (&attrib, NULL);
		}

		// ���� ��Ȳ ǥ���ϴ� ��� - ����
		cur = mm;
		ACAPI_Interface (APIIo_SetProcessValueID, &cur, NULL);
		if (ACAPI_Interface (APIIo_IsProcessCanceledID, NULL, NULL) == APIERR_CANCEL)
			break;
	}

	// ���� ��Ȳ ǥ���ϴ� ��� - ������
	ACAPI_Interface (APIIo_CloseProcessWindowID, NULL, NULL);

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
	//ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	//ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

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
	bool		suspGrp;

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
	GS::Array<API_Guid>	objects;	long nObjects	= 0;
	GS::Array<API_Guid>	walls;		long nWalls		= 0;
	GS::Array<API_Guid>	columns;	long nColumns	= 0;
	GS::Array<API_Guid>	beams;		long nBeams		= 0;
	GS::Array<API_Guid>	slabs;		long nSlabs		= 0;
	GS::Array<API_Guid>	roofs;		long nRoofs		= 0;
	GS::Array<API_Guid>	meshes;		long nMeshes	= 0;
	GS::Array<API_Guid>	morphs;		long nMorphs	= 0;
	GS::Array<API_Guid>	shells;		long nShells	= 0;

	// ���ǿ� �´� ��ü���� GUID ����
	GS::Array<API_Guid> selection_known;
	GS::Array<API_Guid> selection_unknown;

	
	// �׷�ȭ �Ͻ����� ON
	ACAPI_Environment (APIEnv_IsSuspendGroupOnID, &suspGrp);
	if (suspGrp == false)	ACAPI_Element_Tool (NULL, NULL, APITool_SuspendGroups, NULL);

	ACAPI_Element_GetElemList (API_ObjectID, &objects, APIFilt_OnVisLayer);	nObjects = objects.GetSize ();	// ���̴� ���̾� ���� ��ü Ÿ�Ը� ��������
	ACAPI_Element_GetElemList (API_WallID, &walls, APIFilt_OnVisLayer);		nWalls = walls.GetSize ();		// ���̴� ���̾� ���� �� Ÿ�Ը� ��������
	ACAPI_Element_GetElemList (API_ColumnID, &columns, APIFilt_OnVisLayer);	nColumns = columns.GetSize ();	// ���̴� ���̾� ���� ��� Ÿ�Ը� ��������
	ACAPI_Element_GetElemList (API_BeamID, &beams, APIFilt_OnVisLayer);		nBeams = beams.GetSize ();		// ���̴� ���̾� ���� �� Ÿ�Ը� ��������
	ACAPI_Element_GetElemList (API_SlabID, &slabs, APIFilt_OnVisLayer);		nSlabs = slabs.GetSize ();		// ���̴� ���̾� ���� ������ Ÿ�Ը� ��������
	ACAPI_Element_GetElemList (API_RoofID, &roofs, APIFilt_OnVisLayer);		nRoofs = roofs.GetSize ();		// ���̴� ���̾� ���� ���� Ÿ�Ը� ��������
	ACAPI_Element_GetElemList (API_MeshID, &meshes, APIFilt_OnVisLayer);	nMeshes = meshes.GetSize ();	// ���̴� ���̾� ���� �޽� Ÿ�Ը� ��������
	ACAPI_Element_GetElemList (API_MorphID, &morphs, APIFilt_OnVisLayer);	nMorphs = morphs.GetSize ();	// ���̴� ���̾� ���� ���� Ÿ�Ը� ��������
	ACAPI_Element_GetElemList (API_ShellID, &shells, APIFilt_OnVisLayer);	nShells = shells.GetSize ();	// ���̴� ���̾� ���� �� Ÿ�Ը� ��������

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

		if (err == NoError && elem.header.hasMemo) {
			err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

			if (err == NoError) {
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
			}

			ACAPI_DisposeElemMemoHdls (&memo);
		}
	}
	
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

// �� ���̺��� ����ǥ �ۼ��� ���� Ŭ���� ������ (�ʱ�ȭ)
BeamTableformInfo::BeamTableformInfo ()
{
	this->init ();
}

// �ʱ�ȭ
void BeamTableformInfo::init ()
{
	this->iBeamDirection = 0;
	this->nCells = 0;

	for (short xx = 0 ; xx < 30 ; ++xx) {
		this->cells [xx].euroform_leftHeight = 0.0;
		this->cells [xx].euroform_rightHeight = 0.0;
		this->cells [xx].euroform_bottomWidth = 0.0;

		this->cells [xx].plywoodOnly_leftHeight = 0.0;
		this->cells [xx].plywoodOnly_rightHeight = 0.0;
		this->cells [xx].plywoodOnly_bottomWidth = 0.0;

		this->cells [xx].length = 0.0;
	}
}

// �� ���̺��� ���� ���� ��������
GSErrCode	exportBeamTableformInformation (void)
{
	GSErrCode	err = NoError;
	unsigned short		xx;
	short	mm;
	bool	regenerate = true;
	bool	suspGrp;

	GS::Array<API_Guid>		objects;
	long					nObjects = 0;

	API_Element			elem;
	API_ElementMemo		memo;

	BeamTableformInfo				tableformInfo;	// �� ���̺��� ����
	vector<objectInBeamTableform>	objectList;		// ��ü ����Ʈ
	objectInBeamTableform			newObject;

	double				xmin, xmax, ymin, ymax;
	int					ang_x, ang_y;
	bool				bValid, bFirst;
	double				xcenter, ycenter;

	// ���̾� ���� ����
	short			nLayers;
	API_Attribute	attrib;
	short			nVisibleLayers = 0;
	short			visLayerList [1024];
	char			fullLayerName [512];

	// ��Ÿ
	char			tempStr [512];
	char			buffer [512];
	char			filename [512];

	// ���� ���Ϸ� ��� ���� ��������
	// ���� ������ ���� ����
	API_SpecFolderID	specFolderID = API_ApplicationFolderID;
	IO::Location		location;
	GS::UniString		resultString;
	API_MiscAppInfo		miscAppInfo;
	FILE				*fp;


	// �׷�ȭ �Ͻ����� ON
	ACAPI_Environment (APIEnv_IsSuspendGroupOnID, &suspGrp);
	if (suspGrp == false)	ACAPI_Element_Tool (NULL, NULL, APITool_SuspendGroups, NULL);

	// ȭ�� ���ΰ�ħ
	ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

	ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);

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
	sprintf (filename, "%s - �� ���̺��� ����ǥ.csv", miscAppInfo.caption);
	fp = fopen (filename, "w+");

	if (fp == NULL) {
		WriteReport_Alert ("���������� ���� �� �����ϴ�.");
		return	NoError;
	}

	// ���̴� ���̾���� �ϳ��� ��ȸ�ϸ鼭 ��ü ��ҵ��� ������ �� "�� ���̺��� ����ǥ" ��ƾ ����
	for (mm = 1 ; mm <= nVisibleLayers ; ++mm) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = visLayerList [mm-1];
		err = ACAPI_Attribute_Get (&attrib);

		if (err == NoError) {
			// ���̾� ���̱�
			if ((attrib.layer.head.flags & APILay_Hidden) == true) {
				attrib.layer.head.flags ^= APILay_Hidden;
				ACAPI_Attribute_Modify (&attrib, NULL);
			}

			// �ʱ�ȭ
			tableformInfo.init ();
			objectList.clear ();

			// ��� ��� ��������
			ACAPI_Element_GetElemList (API_ObjectID, &objects, APIFilt_OnVisLayer);	// ���̴� ���̾ ����, ��ü Ÿ�Ը�
			nObjects = objects.GetSize ();

			if (nObjects == 0)
				continue;

			// ���̾� �̸� ������
			sprintf (fullLayerName, "%s", attrib.layer.head.name);
			fullLayerName [strlen (fullLayerName)] = '\0';

			for (xx = 0 ; xx < nObjects ; ++xx) {
				BNZeroMemory (&elem, sizeof (API_Element));
				BNZeroMemory (&memo, sizeof (API_ElementMemo));
				elem.header.guid = objects [xx];
				err = ACAPI_Element_Get (&elem);

				if (err == NoError && elem.header.hasMemo) {
					err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

					if (err == NoError) {
						bValid = false;

						// �ʱ�ȭ
						newObject.attachPosition = NO_POSITION;
						newObject.objType = NONE;
						newObject.length = 0.0;
						newObject.width = 0.0;
						newObject.length = 0.0;
						newObject.origin.x = 0.0;
						newObject.origin.y = 0.0;
						newObject.origin.z = 0.0;

						// ���� ��ǥ ����
						newObject.origin.x = elem.object.pos.x;
						newObject.origin.y = elem.object.pos.y;
						newObject.origin.z = elem.object.level;
						
						// ��ü�� Ÿ��, �ʺ�� ���̸� ����
						if (my_strcmp (getParameterStringByName (&memo, "u_comp"), "������") == 0) {
							if (my_strcmp (getParameterStringByName (&memo, "u_ins"), "��������") == 0) {
								newObject.objType = EUROFORM;

								sprintf (tempStr, "%s", getParameterStringByName (&memo, "eu_wid"));
								newObject.width = atof (tempStr) / 1000.0;
								sprintf (tempStr, "%s", getParameterStringByName (&memo, "eu_hei"));
								newObject.length = atof (tempStr) / 1000.0;

								ang_x = (int)round (RadToDegree (getParameterValueByName (&memo, "ang_x")), 0);
								ang_y = (int)round (RadToDegree (getParameterValueByName (&memo, "ang_y")), 0);

								if ( ((ang_x ==   0) && (ang_y ==  0)) ||
									 ((ang_x ==  90) && (ang_y ==  0)) ||
									 ((ang_x == 180) && (ang_y ==  0)) ||
									 ((ang_x ==  90) && (ang_y == 90)) )
									newObject.attachPosition = LEFT_SIDE;
								else if ( ((ang_x ==   0) && (ang_y == 90)) ||
										  ((ang_x == 180) && (ang_y == 90)) )
									newObject.attachPosition = BOTTOM_SIDE;

								bValid = true;
							}
						}

						else if (my_strcmp (getParameterStringByName (&memo, "g_comp"), "����") == 0) {
							if (abs (getParameterValueByName (&memo, "sogak") - 1.0) < EPS) {
								newObject.objType = PLYWOOD;

								ang_x = (int)round (RadToDegree (getParameterValueByName (&memo, "p_ang")), 0);

								if (my_strcmp (getParameterStringByName (&memo, "w_dir"), "��������") == 0) {
									newObject.width = getParameterValueByName (&memo, "p_wid");
									newObject.length = getParameterValueByName (&memo, "p_leng");
									if ( (ang_x == 0) || (ang_x == 180) )
										newObject.attachPosition = LEFT_SIDE;
									else
										newObject.attachPosition = BOTTOM_SIDE;
								} else if (my_strcmp (getParameterStringByName (&memo, "w_dir"), "�������") == 0) {
									newObject.width = getParameterValueByName (&memo, "p_leng");
									newObject.length = getParameterValueByName (&memo, "p_wid");
									if ( (ang_x == 0) || (ang_x == 180) )
										newObject.attachPosition = LEFT_SIDE;
									else
										newObject.attachPosition = BOTTOM_SIDE;
								} else if (my_strcmp (getParameterStringByName (&memo, "w_dir"), "�ٴڱ��") == 0) {
									newObject.width = getParameterValueByName (&memo, "p_wid");
									newObject.length = getParameterValueByName (&memo, "p_leng");
									if ( (ang_x == 90) || (ang_x == 270) )
										newObject.attachPosition = LEFT_SIDE;
									else
										newObject.attachPosition = BOTTOM_SIDE;
								} else if (my_strcmp (getParameterStringByName (&memo, "w_dir"), "�ٴڵ���") == 0) {
									newObject.width = getParameterValueByName (&memo, "p_wid");
									newObject.length = getParameterValueByName (&memo, "p_leng");
									if ( (ang_x == 90) || (ang_x == 270) )
										newObject.attachPosition = LEFT_SIDE;
									else
										newObject.attachPosition = BOTTOM_SIDE;
								}

								// ��, ��ü�� ���� ������ �ʺ� 200mm �̻��̾�� ��
								if (newObject.width > 0.200 - EPS)
									bValid = true;
								else
									bValid = false;
							}
						}

						if (bValid == true)
							objectList.push_back (newObject);
					}

					ACAPI_DisposeElemMemoHdls (&memo);
				}
			}

			nObjects = (long)objectList.size ();

			// �� ������ ã�Ƴ� (�����ΰ�, �����ΰ�?)
			bFirst = false;
			for (xx = 0 ; xx < nObjects ; ++xx) {
				if (objectList [xx].attachPosition != BOTTOM_SIDE) {
					if (bFirst == false) {
						xmin = xmax = objectList [xx].origin.x;
						ymin = ymax = objectList [xx].origin.y;
						bFirst = true;
					} else {
						if (xmin > objectList [xx].origin.x)	xmin = objectList [xx].origin.x;
						if (ymin > objectList [xx].origin.y)	ymin = objectList [xx].origin.y;
						if (xmax < objectList [xx].origin.x)	xmax = objectList [xx].origin.x;
						if (ymax < objectList [xx].origin.y)	ymax = objectList [xx].origin.y;
					}
				}
			}
			if (abs (xmax - xmin) > abs (ymax - ymin))
				tableformInfo.iBeamDirection = HORIZONTAL_DIRECTION;
			else
				tableformInfo.iBeamDirection = VERTICAL_DIRECTION;

			// ��ü �����ϱ�
			for (xx = 0 ; xx < nObjects ; ++xx) {
				if (tableformInfo.iBeamDirection == HORIZONTAL_DIRECTION)
					sort (objectList.begin (), objectList.end (), comparePosX);		// X �������� ����
				else
					sort (objectList.begin (), objectList.end (), comparePosY);		// Y �������� ����
			}

			// ���� ��ġ ã��
			xcenter = (xmax - xmin) / 2 + xmin;
			ycenter = (ymax - ymin) / 2 + ymin;

			// ������ ���� �з��ϱ�
			for (xx = 0 ; xx < nObjects ; ++xx) {
				// ����/�Ʒ���(�ּ�)�� ������/����(�ִ�) ���� ������ �߰����� �������� ����
				if (tableformInfo.iBeamDirection == HORIZONTAL_DIRECTION) {
					if (objectList [xx].origin.y > ycenter) {
						if (objectList [xx].attachPosition == LEFT_SIDE)
							objectList [xx].attachPosition = RIGHT_SIDE;	// ����
					}
				} else {
					if (objectList [xx].origin.x > xcenter) {
						if (objectList [xx].attachPosition == LEFT_SIDE)
							objectList [xx].attachPosition = RIGHT_SIDE;	// ������
					}
				}
			}

			// !!!
			// ���� 3���� ��ü Ÿ���� ��ġ���� �ʴ� ���, ���� ���� 3��° �׸�� ���� ���� 1��° �׸��� ��ȯ��
			objectInBeamTableform	tempObj;

			if ((objectList.size () % 3 == 0) && (objectList.size () > 2)) {
				for (xx = 0 ; xx < (objectList.size () / 3) - 1 ; ++xx) {
					// 3��° �׸� �ٸ� ���, (Mirrored ��ġ ������� ���)
					if ( (objectList [3*xx].objType == objectList [3*xx + 1].objType) && (objectList [3*xx + 1].objType != objectList [3*xx + 2].objType) ) {
						tempObj.objType			= objectList [3*xx + 2].objType;
						tempObj.attachPosition	= objectList [3*xx + 2].attachPosition;
						tempObj.origin			= objectList [3*xx + 2].origin;
						tempObj.width			= objectList [3*xx + 2].width;
						tempObj.length			= objectList [3*xx + 2].length;

						objectList [3*xx + 2].objType			= objectList [3*xx + 2 + 1].objType;
						objectList [3*xx + 2].attachPosition	= objectList [3*xx + 2 + 1].attachPosition;
						objectList [3*xx + 2].origin			= objectList [3*xx + 2 + 1].origin;
						objectList [3*xx + 2].width				= objectList [3*xx + 2 + 1].width;
						objectList [3*xx + 2].length			= objectList [3*xx + 2 + 1].length;

						objectList [3*xx + 2 + 1].objType			= tempObj.objType;
						objectList [3*xx + 2 + 1].attachPosition	= tempObj.attachPosition;
						objectList [3*xx + 2 + 1].origin			= tempObj.origin;
						objectList [3*xx + 2 + 1].width				= tempObj.width;
						objectList [3*xx + 2 + 1].length			= tempObj.length;
					}
					// ����-������-����-(������-����-������) ������� �Ǿ� �ִ� ���, (Mirrored ��ġ ����� ���)
					// �������� ���� �;� ��
					else if ( (objectList [3*xx].objType == PLYWOOD) && (objectList [3*xx + 1].objType == EUROFORM) && (objectList [3*xx + 2].objType == PLYWOOD) ) {
						if (objectList [3*xx + 3].objType == PLYWOOD) {
							// [3*xx]�� [3*xx + 4] ��ȯ
							tempObj.objType			= objectList [3*xx].objType;
							tempObj.attachPosition	= objectList [3*xx].attachPosition;
							tempObj.origin			= objectList [3*xx].origin;
							tempObj.width			= objectList [3*xx].width;
							tempObj.length			= objectList [3*xx].length;

							objectList [3*xx].objType			= objectList [3*xx + 4].objType;
							objectList [3*xx].attachPosition	= objectList [3*xx + 4].attachPosition;
							objectList [3*xx].origin			= objectList [3*xx + 4].origin;
							objectList [3*xx].width				= objectList [3*xx + 4].width;
							objectList [3*xx].length			= objectList [3*xx + 4].length;

							objectList [3*xx + 4].objType			= tempObj.objType;
							objectList [3*xx + 4].attachPosition	= tempObj.attachPosition;
							objectList [3*xx + 4].origin			= tempObj.origin;
							objectList [3*xx + 4].width				= tempObj.width;
							objectList [3*xx + 4].length			= tempObj.length;

							// [3*xx + 2]�� [3*xx + 5] ��ȯ
							tempObj.objType			= objectList [3*xx + 2].objType;
							tempObj.attachPosition	= objectList [3*xx + 2].attachPosition;
							tempObj.origin			= objectList [3*xx + 2].origin;
							tempObj.width			= objectList [3*xx + 2].width;
							tempObj.length			= objectList [3*xx + 2].length;

							objectList [3*xx + 2].objType			= objectList [3*xx + 5].objType;
							objectList [3*xx + 2].attachPosition	= objectList [3*xx + 5].attachPosition;
							objectList [3*xx + 2].origin			= objectList [3*xx + 5].origin;
							objectList [3*xx + 2].width				= objectList [3*xx + 5].width;
							objectList [3*xx + 2].length			= objectList [3*xx + 5].length;

							objectList [3*xx + 5].objType			= tempObj.objType;
							objectList [3*xx + 5].attachPosition	= tempObj.attachPosition;
							objectList [3*xx + 5].origin			= tempObj.origin;
							objectList [3*xx + 5].width				= tempObj.width;
							objectList [3*xx + 5].length			= tempObj.length;

						} else if (objectList [3*xx + 4].objType == PLYWOOD) {
							// [3*xx]�� [3*xx + 3] ��ȯ
							tempObj.objType			= objectList [3*xx].objType;
							tempObj.attachPosition	= objectList [3*xx].attachPosition;
							tempObj.origin			= objectList [3*xx].origin;
							tempObj.width			= objectList [3*xx].width;
							tempObj.length			= objectList [3*xx].length;

							objectList [3*xx].objType			= objectList [3*xx + 3].objType;
							objectList [3*xx].attachPosition	= objectList [3*xx + 3].attachPosition;
							objectList [3*xx].origin			= objectList [3*xx + 3].origin;
							objectList [3*xx].width				= objectList [3*xx + 3].width;
							objectList [3*xx].length			= objectList [3*xx + 3].length;

							objectList [3*xx + 3].objType			= tempObj.objType;
							objectList [3*xx + 3].attachPosition	= tempObj.attachPosition;
							objectList [3*xx + 3].origin			= tempObj.origin;
							objectList [3*xx + 3].width				= tempObj.width;
							objectList [3*xx + 3].length			= tempObj.length;

							// [3*xx + 2]�� [3*xx + 5] ��ȯ
							tempObj.objType			= objectList [3*xx + 2].objType;
							tempObj.attachPosition	= objectList [3*xx + 2].attachPosition;
							tempObj.origin			= objectList [3*xx + 2].origin;
							tempObj.width			= objectList [3*xx + 2].width;
							tempObj.length			= objectList [3*xx + 2].length;

							objectList [3*xx + 2].objType			= objectList [3*xx + 5].objType;
							objectList [3*xx + 2].attachPosition	= objectList [3*xx + 5].attachPosition;
							objectList [3*xx + 2].origin			= objectList [3*xx + 5].origin;
							objectList [3*xx + 2].width				= objectList [3*xx + 5].width;
							objectList [3*xx + 2].length			= objectList [3*xx + 5].length;

							objectList [3*xx + 5].objType			= tempObj.objType;
							objectList [3*xx + 5].attachPosition	= tempObj.attachPosition;
							objectList [3*xx + 5].origin			= tempObj.origin;
							objectList [3*xx + 5].width				= tempObj.width;
							objectList [3*xx + 5].length			= tempObj.length;

						} else if (objectList [3*xx + 5].objType == PLYWOOD) {
							// [3*xx]�� [3*xx + 3] ��ȯ
							tempObj.objType			= objectList [3*xx].objType;
							tempObj.attachPosition	= objectList [3*xx].attachPosition;
							tempObj.origin			= objectList [3*xx].origin;
							tempObj.width			= objectList [3*xx].width;
							tempObj.length			= objectList [3*xx].length;

							objectList [3*xx].objType			= objectList [3*xx + 3].objType;
							objectList [3*xx].attachPosition	= objectList [3*xx + 3].attachPosition;
							objectList [3*xx].origin			= objectList [3*xx + 3].origin;
							objectList [3*xx].width				= objectList [3*xx + 3].width;
							objectList [3*xx].length			= objectList [3*xx + 3].length;

							objectList [3*xx + 3].objType			= tempObj.objType;
							objectList [3*xx + 3].attachPosition	= tempObj.attachPosition;
							objectList [3*xx + 3].origin			= tempObj.origin;
							objectList [3*xx + 3].width				= tempObj.width;
							objectList [3*xx + 3].length			= tempObj.length;

							// [3*xx + 2]�� [3*xx + 4] ��ȯ
							tempObj.objType			= objectList [3*xx + 2].objType;
							tempObj.attachPosition	= objectList [3*xx + 2].attachPosition;
							tempObj.origin			= objectList [3*xx + 2].origin;
							tempObj.width			= objectList [3*xx + 2].width;
							tempObj.length			= objectList [3*xx + 2].length;

							objectList [3*xx + 2].objType			= objectList [3*xx + 4].objType;
							objectList [3*xx + 2].attachPosition	= objectList [3*xx + 4].attachPosition;
							objectList [3*xx + 2].origin			= objectList [3*xx + 4].origin;
							objectList [3*xx + 2].width				= objectList [3*xx + 4].width;
							objectList [3*xx + 2].length			= objectList [3*xx + 4].length;

							objectList [3*xx + 4].objType			= tempObj.objType;
							objectList [3*xx + 4].attachPosition	= tempObj.attachPosition;
							objectList [3*xx + 4].origin			= tempObj.origin;
							objectList [3*xx + 4].width				= tempObj.width;
							objectList [3*xx + 4].length			= tempObj.length;
						}
					}
					
					// ������-����-������-(����-������-����) ������� �Ǿ� �ִ� ���, (Mirrored ��ġ ����� ���)
					// �������� ���� �;� ��
					else if ( (objectList [3*xx].objType == EUROFORM) && (objectList [3*xx + 1].objType == PLYWOOD) && (objectList [3*xx + 2].objType == EUROFORM) ) {
						if (objectList [3*xx + 3].objType == EUROFORM) {
							// [3*xx + 1]�� [3*xx + 3] ��ȯ
							tempObj.objType			= objectList [3*xx + 1].objType;
							tempObj.attachPosition	= objectList [3*xx + 1].attachPosition;
							tempObj.origin			= objectList [3*xx + 1].origin;
							tempObj.width			= objectList [3*xx + 1].width;
							tempObj.length			= objectList [3*xx + 1].length;

							objectList [3*xx + 1].objType			= objectList [3*xx + 3].objType;
							objectList [3*xx + 1].attachPosition	= objectList [3*xx + 3].attachPosition;
							objectList [3*xx + 1].origin			= objectList [3*xx + 3].origin;
							objectList [3*xx + 1].width				= objectList [3*xx + 3].width;
							objectList [3*xx + 1].length			= objectList [3*xx + 3].length;

							objectList [3*xx + 3].objType			= tempObj.objType;
							objectList [3*xx + 3].attachPosition	= tempObj.attachPosition;
							objectList [3*xx + 3].origin			= tempObj.origin;
							objectList [3*xx + 3].width				= tempObj.width;
							objectList [3*xx + 3].length			= tempObj.length;

						} else if (objectList [3*xx + 4].objType == EUROFORM) {
							// [3*xx + 1]�� [3*xx + 4] ��ȯ
							tempObj.objType			= objectList [3*xx + 1].objType;
							tempObj.attachPosition	= objectList [3*xx + 1].attachPosition;
							tempObj.origin			= objectList [3*xx + 1].origin;
							tempObj.width			= objectList [3*xx + 1].width;
							tempObj.length			= objectList [3*xx + 1].length;

							objectList [3*xx + 1].objType			= objectList [3*xx + 4].objType;
							objectList [3*xx + 1].attachPosition	= objectList [3*xx + 4].attachPosition;
							objectList [3*xx + 1].origin			= objectList [3*xx + 4].origin;
							objectList [3*xx + 1].width				= objectList [3*xx + 4].width;
							objectList [3*xx + 1].length			= objectList [3*xx + 4].length;

							objectList [3*xx + 4].objType			= tempObj.objType;
							objectList [3*xx + 4].attachPosition	= tempObj.attachPosition;
							objectList [3*xx + 4].origin			= tempObj.origin;
							objectList [3*xx + 4].width				= tempObj.width;
							objectList [3*xx + 4].length			= tempObj.length;

						} else if (objectList [3*xx + 5].objType == EUROFORM) {
							// [3*xx + 1]�� [3*xx + 5] ��ȯ
							tempObj.objType			= objectList [3*xx + 1].objType;
							tempObj.attachPosition	= objectList [3*xx + 1].attachPosition;
							tempObj.origin			= objectList [3*xx + 1].origin;
							tempObj.width			= objectList [3*xx + 1].width;
							tempObj.length			= objectList [3*xx + 1].length;

							objectList [3*xx + 1].objType			= objectList [3*xx + 5].objType;
							objectList [3*xx + 1].attachPosition	= objectList [3*xx + 5].attachPosition;
							objectList [3*xx + 1].origin			= objectList [3*xx + 5].origin;
							objectList [3*xx + 1].width				= objectList [3*xx + 5].width;
							objectList [3*xx + 1].length			= objectList [3*xx + 5].length;

							objectList [3*xx + 5].objType			= tempObj.objType;
							objectList [3*xx + 5].attachPosition	= tempObj.attachPosition;
							objectList [3*xx + 5].origin			= tempObj.origin;
							objectList [3*xx + 5].width				= tempObj.width;
							objectList [3*xx + 5].length			= tempObj.length;
						}
					}
				}
			}

			// ����׿� �ڵ�
			//sprintf (buffer, "\n");
			//fprintf (fp, buffer);
			//for (xx = 0 ; xx < nObjects ; ++xx) {
			//	char temp1 [32];
			//	char temp2 [32];

			//	if (objectList [xx].objType == EUROFORM)		strcpy (temp1, "������");
			//	else if (objectList [xx].objType == PLYWOOD)	strcpy (temp1, "����");
			//	else											strcpy (temp1, "����");

			//	if (objectList [xx].attachPosition == LEFT_SIDE)		strcpy (temp2, "����");
			//	else if (objectList [xx].attachPosition == RIGHT_SIDE)	strcpy (temp2, "����");
			//	else if (objectList [xx].attachPosition == BOTTOM_SIDE)	strcpy (temp2, "�Ϻ�");

			//	sprintf (buffer, "%s(%s), (%.0f / %.0f / %.0f) �ʺ� (%.0f) ���� (%.0f)\n",
			//		temp1, temp2,
			//		objectList [xx].origin.x * 1000, objectList [xx].origin.y * 1000, objectList [xx].origin.z * 1000,
			//		objectList [xx].width * 1000, objectList [xx].length * 1000);
			//	fprintf (fp, buffer);
			//}

			// �� ������ ���� ���� ����
			short nCells_left = 0;
			short nCells_right = 0;
			short nCells_bottom = 0;

			// �� ���̺��� ����ǥ �ۼ��� ���� Ŭ���� - �ν��Ͻ� �ۼ��ϱ�
			for (xx = 0 ; xx < nObjects ; ++xx) {
				if (objectList [xx].attachPosition == LEFT_SIDE) {
					if (objectList [xx].objType == EUROFORM) {
						tableformInfo.cells [nCells_left].euroform_leftHeight = objectList [xx].width;
						tableformInfo.cells [nCells_left].length = objectList [xx].length;
						++ nCells_left;
					}
					if (objectList [xx].objType == PLYWOOD) {
						tableformInfo.cells [nCells_left].plywoodOnly_leftHeight = objectList [xx].width;
						tableformInfo.cells [nCells_left].length = objectList [xx].length;
						++ nCells_left;
					}
				}

				if (objectList [xx].attachPosition == RIGHT_SIDE) {
					if (objectList [xx].objType == EUROFORM) {
						tableformInfo.cells [nCells_right].euroform_rightHeight = objectList [xx].width;
						++ nCells_right;
					}
					if (objectList [xx].objType == PLYWOOD) {
						tableformInfo.cells [nCells_right].plywoodOnly_rightHeight = objectList [xx].width;
						++ nCells_right;
					}
				}

				if (objectList [xx].attachPosition == BOTTOM_SIDE) {
					if (objectList [xx].objType == EUROFORM) {
						tableformInfo.cells [nCells_bottom].euroform_bottomWidth = objectList [xx].width;
						++ nCells_bottom;
					}
					if (objectList [xx].objType == PLYWOOD) {
						tableformInfo.cells [nCells_bottom].plywoodOnly_bottomWidth = objectList [xx].width;
						++ nCells_bottom;
					}
				}
			}

			// �� ���� ����
			// !!! ������ ���� ���� ���¿��� ���� ���� ���·� �ٲ� ��
			if (nObjects != 0) {
				if ((nCells_left == nCells_right) && (nCells_left == nCells_bottom)) {
					// ������ ���
					tableformInfo.nCells = nCells_left;

					// ���� ����ϱ� (����ǥ �ۼ�)
					sprintf (buffer, "\n\n<< ���̾� : %s >>\n", fullLayerName);
					fprintf (fp, buffer);

					for (xx = 0 ; xx < tableformInfo.nCells ; ++xx) {
						if (tableformInfo.cells [xx].euroform_leftHeight > EPS) {
							sprintf (buffer, "\n������,����,%.0f,,�ظ�,%.0f,����1,%.0f,����2,%.0f", tableformInfo.cells [xx].length * 1000, tableformInfo.cells [xx].euroform_bottomWidth * 1000, tableformInfo.cells [xx].euroform_leftHeight * 1000, tableformInfo.cells [xx].euroform_rightHeight * 1000);
						} else {
							sprintf (buffer, "\n����,����,%.0f,,�ظ�,%.0f,����1,%.0f,����2,%.0f", tableformInfo.cells [xx].length * 1000, tableformInfo.cells [xx].plywoodOnly_bottomWidth * 1000, tableformInfo.cells [xx].plywoodOnly_leftHeight * 1000, tableformInfo.cells [xx].plywoodOnly_rightHeight * 1000);
						}
						fprintf (fp, buffer);
					}
				} else {
					// ������ ���
					tableformInfo.nCells = 0;

					// ���� ����ϱ� (����ǥ �ۼ�)
					sprintf (buffer, "\n\n<< ���̾� : %s >>\n", fullLayerName);
					fprintf (fp, buffer);

					sprintf (buffer, "\n����ȭ�� �� ���̺��� ���̾ �ƴմϴ�.\n");
					fprintf (fp, buffer);
				}
			}

			// ��ü ����
			if (!objects.IsEmpty ())
				objects.Clear ();
			if (!objectList.empty ())
				objectList.clear ();

			// ���̾� �����
			attrib.layer.head.flags |= APILay_Hidden;
			ACAPI_Attribute_Modify (&attrib, NULL);
		}
	}

	fclose (fp);

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

	ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
	location.ToDisplayText (&resultString);
	sprintf (buffer, "������� ���� ��ġ�� �����߽��ϴ�.\n\n%s\n�Ǵ� ������Ʈ ������ �ִ� ����", resultString.ToCStr ().Get ());
	WriteReport_Alert (buffer);

	return err;
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