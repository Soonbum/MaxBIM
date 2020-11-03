#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "BeamEuroformPlacer.hpp"
#include "SlabEuroformPlacer.hpp"

using namespace beamPlacerDG;

static BeamPlacingZone	placingZone;			// �⺻ �� ���� ����
static InfoBeam			infoBeam;				// �� ��ü ����
static short			nInterfereBeams;		// ���� �� ����
static InfoBeam			infoOtherBeams [10];	// ���� �� ����

// 3�� �޴�: ���� �������� ��ġ�ϴ� ���� ��ƾ
GSErrCode	placeEuroformOnBeam (void)
{
	GSErrCode		err = NoError;
	long			nSel;
	short			xx;
	double			dx, dy, ang;
	API_Coord3D		rotatedPoint, unrotatedPoint;

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
	API_ElemInfo3D			info3D;
	API_BeamRelation		relData;

	// ���� ��ü ����
	InfoMorphForBeam		infoMorph;

	// �� �Է�
	API_GetPointType		pointInfo;
	API_Coord3D				point1, point2;
	API_Coord3D				tempPoint, resultPoint;
	API_Coord3D				other_p1, other_p2;

	// ȸ������ 0�� ���� ���� ������, ������ ��ǥ�� ������
	API_Coord3D				nodes [2];

	// �۾� �� ����
	API_StoryInfo			storyInfo;
	double					workLevel_beam;


	// ������ ��� ��������
	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err == APIERR_NOPLAN) {
		ACAPI_WriteReport ("���� ������Ʈ â�� �����ϴ�.", true);
		return err;
	}
	if (err == APIERR_NOSEL) {
		ACAPI_WriteReport ("�ƹ� �͵� �������� �ʾҽ��ϴ�.\n�ʼ� ����: �� (1��), �� ������ ���� ���� (1��)", true);
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

			if (tElem.header.typeID == API_MorphID)		// �����ΰ�?
				morphs.Push (tElem.header.guid);

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

	// ������ 1���ΰ�?
	if (nMorphs != 1) {
		ACAPI_WriteReport ("�� ������ ���� ������ 1�� �����ϼž� �մϴ�.", true);
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

	// ���� ������ ������
	BNZeroMemory (&elem, sizeof (API_Element));
	elem.header.guid = morphs.Pop ();
	err = ACAPI_Element_Get (&elem);
	err = ACAPI_Element_Get3DInfo (elem.header, &info3D);

	// ������ ���� ����
	infoMorph.guid		= elem.header.guid;
	infoMorph.floorInd	= elem.header.floorInd;
	infoMorph.level		= info3D.bounds.zMin;

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

	// ���� ���� ����
	API_Elem_Head* headList = new API_Elem_Head [1];
	headList [0] = elem.header;
	err = ACAPI_Element_Delete (&headList, 1);
	delete headList;

	// ����ڰ� Ŭ���� �� ���� ���� ���� ������, ������ ã��
	other_p1.x = infoBeam.begC.x;
	other_p1.y = infoBeam.begC.y;
	other_p1.z = infoBeam.level - infoBeam.height;

	other_p2.x = infoBeam.endC.x;
	other_p2.y = infoBeam.endC.y;
	other_p2.z = infoBeam.level - infoBeam.height;

	// ���� ���� ���� ����
	placingZone.areaHeight = other_p2.z - other_p1.z;
	if (moreCloserPoint (point1, other_p1, other_p2) == 1) {
		placingZone.begC = other_p1;
		placingZone.endC = other_p2;
	} else {
		placingZone.begC = other_p2;
		placingZone.endC = other_p1;
	}

	// �� �� ���� ������ ����
	dx = placingZone.endC.x - placingZone.begC.x;
	dy = placingZone.endC.y - placingZone.begC.y;
	ang = RadToDegree (atan2 (dy, dx));

	// �������� ����, ������ ���������� ������ ȸ���� �� (ȸ������ 0�� ���� ��ǥ ���)
	tempPoint = placingZone.endC;
	resultPoint.x = point1.x + ((tempPoint.x - point1.x)*cos(DegreeToRad (-ang)) - (tempPoint.y - point1.y)*sin(DegreeToRad (-ang)));
	resultPoint.y = point1.y + ((tempPoint.x - point1.x)*sin(DegreeToRad (-ang)) + (tempPoint.y - point1.y)*cos(DegreeToRad (-ang)));
	resultPoint.z = tempPoint.z;

	nodes [0] = placingZone.begC;
	nodes [1] = resultPoint;

	// ������ �� ���� ������ ������
	placingZone.ang = ang;
	placingZone.level = infoBeam.level;

	// ���� ������ ������/���� �� ���� ���� ����� �� ���� ã�´�.
		// ���� ���� ���ʿ� �ѽ־��� �´ٰ� ����
		// IntersectionPoint1 �Լ��� �̿��Ͽ� ������ ã��
		// ���� �� �ϳ��� ã���� �� �̻� ã�� �ʰ� �ߴ�
	// ���� ����
		// ���� �� ��ü ���̸� ������� ������ ����� ��
	// ���� ���� �ִ� ���
		// ������ ���
			// �� ���� ���� ���̸� �Է��� �� - ������ ����/�������� ä��
			// ���� ���������� ������ ���� 1200/900/600 ������ �ڵ� ��ġ ������ (���� ��� ������ ���� �� ���� �ٷ� ��������, ��ġ�� ���� �� ���������� ����)
			// ���� �� ������ ���� ��ġ
		// �Ϻ��� ���
			// �� ���� ���� ���̸� �Է��� �� - ������ ����/�������� ä��
			// ���� ���������� ������ ���� 1200/900/600 ������ 2���� �ڵ� ��ġ ������
			// ��� ���� ������ ��԰��� ��ġ
	// ���� ���� ���� ���
		// ������ ���
		// �Ϻ��� ���
			// �� ���� ���� ���̸� �Է��� �� - ������ ����/�������� ä��
			// ���� ���������� ������ ���� 1200/900/600 ������ 2���� �ڵ� ��ġ ������
			// ��� ���� ������ ��԰��� ��ġ

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
				��� ����/����� ������ ��ư���� ǥ���� ��
				������ ���� (ó������ �߽�.. ������ �Ǵ� ���� ������ �̵� ������)
				*���� ���� ��� ��ư
				*�� �߰�/���� ��ư
				*��ġ ��ư -> ������(�԰�/��԰�) ��ġ
				*������ ä��� ��ư -> �ƿ��ڳʾޱ�, ����, ���� ��ġ
	*/

	return	err;
}
