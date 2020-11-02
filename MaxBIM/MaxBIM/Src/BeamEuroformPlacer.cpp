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
static short			nInterfereBeams;		// ���� �� ����
static InfoBeam			infoOtherBeams [10];	// ���� �� ����

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
	GS::Array<API_Guid>&	morphs = GS::Array<API_Guid> ();
	GS::Array<API_Guid>&	beams = GS::Array<API_Guid> ();
	long					nMorphs = 0;
	long					nBeams = 0;

	// ��ü ���� ��������
	API_Element				elem;
	API_ElementMemo			memo;
	//API_ElemInfo3D			info3D;
	API_BeamRelation		relData;

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

	// ���� �� 1�� �����ؾ� ��
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
	nMorphs = morphs.GetSize ();
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
	elem.header.guid = infoBeam.guid;
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

	// ���� ���� ���� �� �ִ� ���� ������ ������ ������
	ACAPI_Element_GetRelations (infoBeam.guid, API_BeamID, (void*) &relData);
	nInterfereBeams = 0;

	// ���� ���� �߰��� �پ� �ִ� ���� ��
	if (relData.con != NULL) {
		for (xx = 0; xx < relData.nCon; xx++) {
			BNZeroMemory (&elem, sizeof (API_Element));
			elem.header.guid = (*(relData.con))[xx].guid;
			ACAPI_Element_Get (&elem);
			
			infoOtherBeams [nInterfereBeams].guid		= (*(relData.con))[xx].guid;
			infoOtherBeams [nInterfereBeams].floorInd	= elem.header.floorInd;
			infoOtherBeams [nInterfereBeams].height		= elem.beam.height;
			infoOtherBeams [nInterfereBeams].width		= elem.beam.width;
			infoOtherBeams [nInterfereBeams].offset		= elem.beam.offset;
			infoOtherBeams [nInterfereBeams].level		= elem.beam.level;
			infoOtherBeams [nInterfereBeams].begC		= elem.beam.begC;
			infoOtherBeams [nInterfereBeams].endC		= elem.beam.endC;

			++nInterfereBeams;
		}
	}

    ACAPI_DisposeBeamRelationHdls (&relData);

	// ���� �κ� �ϴ� �� Ŭ��, �� �κ� ��� �� Ŭ��
	BNZeroMemory (&pointInfo, sizeof (API_GetPointType));
	CHCopyC ("���� ���� �κ� �ϴ� ���� Ŭ���Ͻʽÿ�.", pointInfo.prompt);
	pointInfo.enableQuickSelection = true;
	err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
	point1 = pointInfo.pos;

	BNZeroMemory (&pointInfo, sizeof (API_GetPointType));
	CHCopyC ("���� �� �κ� ��� ���� Ŭ���Ͻʽÿ�.", pointInfo.prompt);
	pointInfo.enableQuickSelection = true;
	err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
	point2 = pointInfo.pos;

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
		1. ��� ����: ������, ����, ����, �ƿ��ڳʾޱ�
		2. UI
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