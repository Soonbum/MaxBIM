#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "BeamEuroformPlacer.hpp"

using namespace beamPlacerDG;

static BeamPlacingZone	placingZone;			// �⺻ �� ���� ����
static InfoBeam			infoBeam;				// �� ��ü ����

// 3�� �޴�: ���� �������� ��ġ�ϴ� ���� ��ƾ
GSErrCode	placeEuroformOnBeam (void)
{
	GSErrCode	err = NoError;
	long		nSel;
	short		xx;
	//double		dx, dy, ang;
	//API_Coord3D	rotatedPoint, unrotatedPoint;

	// Selection Manager ���� ����
	API_SelectionInfo		selectionInfo;
	API_Element				tElem;
	API_Neig				**selNeigs;
	GS::Array<API_Guid>&	beams = GS::Array<API_Guid> ();
	long					nBeams = 0;

	// ��ü ���� ��������
	API_Element				elem;
	API_ElementMemo			memo;

	// �� �Է�
	API_GetPointType		pointInfo;
	API_Coord3D				point1, point2;

	// �۾� �� ����
	API_StoryInfo	storyInfo;
	double			workLevel_beam;


	// ������ ��� ��������
	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err == APIERR_NOPLAN) {
		ACAPI_WriteReport ("���� ������Ʈ â�� �����ϴ�.", true);
		return err;
	}
	if (err == APIERR_NOSEL) {
		ACAPI_WriteReport ("�ƹ� �͵� �������� �ʾҽ��ϴ�.\n�ʼ� ����: �� (1��)", true);
		return err;
	}
	if (err != NoError) {
		BMKillHandle ((GSHandle *) &selNeigs);
		return err;
	}

	// ���� �� ������ ������
	if (selectionInfo.typeID != API_SelEmpty) {
		nSel = BMGetHandleSize ((GSHandle) selNeigs) / sizeof (API_Neig);
		for (xx = 0 ; xx < nSel && err == NoError ; ++xx) {
			tElem.header.typeID = Neig_To_ElemID ((*selNeigs)[xx].neigID);

			tElem.header.guid = (*selNeigs)[xx].guid;
			if (ACAPI_Element_Get (&tElem) != NoError)	// ������ �� �ִ� ����ΰ�?
				continue;

			if (tElem.header.typeID == API_BeamID)		// ���ΰ�?
				beams.Push (tElem.header.guid);
		}
	}
	BMKillHandle ((GSHandle *) &selNeigs);
	nBeams = beams.GetSize ();

	// ���� 1���ΰ�?
	if (nBeams != 1) {
		ACAPI_WriteReport ("���� 1�� �����ؾ� �մϴ�.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// �� ���� ����
	infoBeam.guid = beams.Pop ();

	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	err = ACAPI_Element_Get (&elem);
	err = ACAPI_Element_GetMemo (elem.header.guid, &memo);
	
	infoBeam.floorInd	= elem.header.floorInd;
	infoBeam.height		= elem.beam.height;
	infoBeam.width		= elem.beam.width;
	infoBeam.offset		= elem.beam.offset;
	infoBeam.level		= elem.beam.level;
	infoBeam.begC		= elem.beam.begC;
	infoBeam.endC		= elem.beam.endC;

	ACAPI_DisposeElemMemoHdls (&memo);

	// !!! ���� ���� ���� �� �ִ� ���� ������ ������ ������
	API_BeamRelation	relData;
	Int32				ind;
	char				msgStr[256], numStr[32];

	ACAPI_Element_GetRelations (infoBeam.guid, API_BeamID, (void*) &relData);

	if (relData.con != NULL) {
		sprintf (msgStr, "���� ���� �߰��� �پ� �ִ� ���� ��:");
		for (ind = 0; ind < relData.nCon; ind ++) {
			sprintf (numStr, " #%ld", (*(relData.con)) [ind]);
			strcat (msgStr, numStr);
		}
		ACAPI_WriteReport (msgStr, true);
	}

	if (relData.conX != NULL) {
		sprintf (msgStr, "���� ���� �����ϴ� ���� ��:");
		for (ind = 0; ind < relData.nConX; ind ++) {
			sprintf (numStr, " #%ld", (*(relData.conX)) [ind]);
			strcat (msgStr, numStr);
		}
		ACAPI_WriteReport (msgStr, true);
	}

	// !!! ���� ���� ����� ������ ���� ���� �����ϴ� �ʺ�/���̴�?

    ACAPI_DisposeBeamRelationHdls (&relData);



	// ���� �κ� �ϴ� �� Ŭ��, �� �κ� ��� �� Ŭ��
	//BNZeroMemory (&pointInfo, sizeof (API_GetPointType));
	//CHCopyC ("���� ���� �κ� �ϴ� ���� Ŭ���Ͻʽÿ�.", pointInfo.prompt);
	//pointInfo.enableQuickSelection = true;
	//err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
	//point1 = pointInfo.pos;

	//BNZeroMemory (&pointInfo, sizeof (API_GetPointType));
	//CHCopyC ("���� �� �κ� ��� ���� Ŭ���Ͻʽÿ�.", pointInfo.prompt);
	//pointInfo.enableQuickSelection = true;
	//err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
	//point2 = pointInfo.pos;

	// ȸ������ 0�� ���� ��ǥ�� ����ϰ�, �� ���� ������ ������

	// �۾� �� ���� �ݿ�

	// ���� ������ �� ���� ����

	// [DIALOG] 1��° ���̾�α׿��� ������ ���� �Է� ����

	// ���ڿ��� �� �������� �ʺ�/���̸� �Ǽ������ε� ����

	// ���� ���� �ʱ�ȭ

	// ������ ���� ����

	// ������ ���� ��ǥ ����

	// ���� ���� unrotated ��ġ�� ������Ʈ

	// placingZone�� Cell ���� �ʱ�ȭ

	// ��ġ�� ���� ���� �Է�

	// [DIALOG] 2��° ���̾�α׿��� ������ ��ġ�� �����մϴ�.

	/*
		1. ����: ���� ��, ���� ��(�ټ�)
			- 1) BeamRelation �̿� --> ���� �� ����
			- 2) ��Ÿ --> ���� ��, ���� �� ����
				���� ���� ���Ʒ��� ������ ��:
					(1) ���� ���� �糡 Y �� ���� �ȿ� ������ �糡�� Y���� ����
					(2) ���� ���� ���� X=0�̸�, ���� ������ X���� + �Ǵ� -��
		2. ���� ���� ������ �ϴܰ� ���� ��� Ŭ��
		3. ��� ����: ������, ����, ����, �ƿ��ڳʾޱ�
		4. UI
			(1) ������: �ʺ�, ����
				���� ������ ���� ���� ���� ���� ���̸� ������
			(2) ���� ��ġ, �Ϻ� ��ġ ���� �����ֱ�
				������ ���� �����ֱ�
				���� ���� �����ֱ�
				������ ���� (ó������ �߽�.. ������ �Ǵ� ���� ������ �̵� ������)
				*���� ���� ��� ��ư
				*�� �߰�/���� ��ư
				*��ġ ��ư -> ������(�԰�/��԰�) ��ġ
				*������ ä��� ��ư -> �ƿ��ڳʾޱ�, ����, ���� ��ġ
	*/

	return	err;
}