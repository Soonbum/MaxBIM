#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "BeamEuroformPlacer.hpp"

using namespace beamPlacerDG;

static BeamPlacingZone	placingZone;				// �⺻ �� ���� ����
static InfoBeam			infoBeam;					// �� ��ü ����
static short			nInterfereBeams;			// ���� �� ����
static InfoBeam			infoOtherBeams [10];		// ���� �� ����
static bool				clickedOKButton;			// OK ��ư�� �������ϱ�?
static short			layerInd_Euroform;			// ���̾� ��ȣ: ������
static short			layerInd_Plywood;			// ���̾� ��ȣ: ����
static short			layerInd_Wood;				// ���̾� ��ȣ: ����
static short			layerInd_OutcornerAngle;	// ���̾� ��ȣ: �ƿ��ڳʾޱ�

// 3�� �޴�: ���� �������� ��ġ�ϴ� ���� ��ƾ
GSErrCode	placeEuroformOnBeam (void)
{
	GSErrCode		err = NoError;
	long			nSel;
	short			xx;
	double			dx, dy, ang;
	API_Coord3D		rotatedPoint, unrotatedPoint;
	short			result;

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

	// ������ ã��
	API_Coord p1, p2, p3, p4, pResult;

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
	placingZone.ang			= DegreeToRad (ang);
	placingZone.level		= infoBeam.level;
	placingZone.beamLength	= GetDistance (placingZone.begC, placingZone.endC);

	// ���� ������ ������/���� �� ���� ���� ����� �� ���� �˻� (���� ���� ���ʿ� �� �ָ� �ִٰ� ������)
	p1.x = infoBeam.begC.x;				p1.y = infoBeam.begC.y;
	p2.x = infoBeam.endC.x;				p2.y = infoBeam.endC.y;
	p3.x = infoOtherBeams [0].begC.x;	p3.y = infoOtherBeams [0].begC.y;
	p4.x = infoOtherBeams [0].endC.x;	p4.y = infoOtherBeams [0].endC.y;
	pResult = IntersectionPoint1 (&p1, &p2, &p3, &p4);	// ���� ���� ���� ���� ������ �˻�

	// ���� �� ���� ���� �Է�
	if (nInterfereBeams > 0) {
		placingZone.bInterfereBeam = true;
		placingZone.posInterfereBeamFromLeft = GetDistance (placingZone.begC.x, placingZone.begC.y, pResult.x, pResult.y);
		placingZone.interfereBeamWidth = infoOtherBeams [0].width;
		placingZone.interfereBeamHeight = infoOtherBeams [0].height;
	} else {
		placingZone.bInterfereBeam = false;
	}

	// ���� ������ ���� ���� �ʱ�ȭ
	placingZone.marginBeginAtSide = 0.0;
	placingZone.marginEndAtSide = 0.0;
	placingZone.marginBeginAtBottom = 0.0;
	placingZone.marginEndAtBottom = 0.0;

	// [DIALOG] 1��° ���̾�α׿��� ������ ���� �Է� ����
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32521, ACAPI_GetOwnResModule (), beamPlacerHandler1, 0);

	// �۾� �� ���� �ݿ�
	BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	workLevel_beam = 0.0;
	ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
	for (xx = 0 ; xx < (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
		if (storyInfo.data [0][xx].index == infoBeam.floorInd) {
			workLevel_beam = storyInfo.data [0][xx].level;
			break;
		}
	}
	BMKillHandle ((GSHandle *) &storyInfo.data);

	// ���� ������ �� ���� ����
	// ... placingZone.level = ???;

	// �� ���� �ʱ�ȭ
	placingZone.nCellsFromBeginAtSide = 0;
	placingZone.nCellsFromEndAtSide = 0;
	placingZone.nCellsFromBeginAtBottom = 0;
	placingZone.nCellsFromEndAtBottom = 0;

	// placingZone�� Cell ���� �ʱ�ȭ
	initCellsForBeam (&placingZone);

	// ��ġ�� ���� ���� �Է�
	firstPlacingSettingsForBeam (&placingZone);

	// !!! �׽�Ʈ
	for (xx = 0 ; xx < placingZone.nCellsFromBeginAtSide ; ++xx) {
		placeLibPartForBeam (placingZone.cellsFromBeginAtLSide [xx]);
		placeLibPartForBeam (placingZone.cellsFromBeginAtRSide [xx]);
	}

	// [DIALOG] 2��° ���̾�α׿��� ������ ��ġ�� �����մϴ�.
	clickedOKButton = false;
	//result = DGBlankModalDialog (185, 250, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, slabBottomPlacerHandler2, 0);

	// ������ ���� ä��� - ����, ����
	//err = fillRestAreasForSlabBottom ();

	/*
		���� ��ġ, �Ϻ� ��ġ ���� �����ֱ�
			������ ���� �����ֱ�
			���� ���� �����ֱ�
			��� ����/����� ������ ��ư���� ǥ���� ��
			*���� ���� ��� ��ư
			*�� �߰�/���� ��ư
			*��ġ ��ư -> ������(�԰�/��԰�) ��ġ
			*������ ä��� ��ư -> �ƿ��ڳʾޱ�, ����, ���� ��ġ
	*/

	return	err;
}

// Cell �迭�� �ʱ�ȭ��
void	initCellsForBeam (BeamPlacingZone* placingZone)
{
	short xx;

	for (xx = 0 ; xx < 20 ; ++xx) {
		placingZone->cellsFromBeginAtLSide [xx].objType = NONE;
		placingZone->cellsFromBeginAtLSide [xx].leftBottomX = 0.0;
		placingZone->cellsFromBeginAtLSide [xx].leftBottomY = 0.0;
		placingZone->cellsFromBeginAtLSide [xx].leftBottomZ = 0.0;
		placingZone->cellsFromBeginAtLSide [xx].ang = 0.0;
		placingZone->cellsFromBeginAtLSide [xx].dirLen = 0.0;
		placingZone->cellsFromBeginAtLSide [xx].perLen = 0.0;
		placingZone->cellsFromBeginAtLSide [xx].attached_side = LEFT_SIDE;

		placingZone->cellsFromBeginAtRSide [xx].objType = NONE;
		placingZone->cellsFromBeginAtRSide [xx].leftBottomX = 0.0;
		placingZone->cellsFromBeginAtRSide [xx].leftBottomY = 0.0;
		placingZone->cellsFromBeginAtRSide [xx].leftBottomZ = 0.0;
		placingZone->cellsFromBeginAtRSide [xx].ang = 0.0;
		placingZone->cellsFromBeginAtRSide [xx].dirLen = 0.0;
		placingZone->cellsFromBeginAtRSide [xx].perLen = 0.0;
		placingZone->cellsFromBeginAtLSide [xx].attached_side = RIGHT_SIDE;

		placingZone->cellsFromEndAtLSide [xx].objType = NONE;
		placingZone->cellsFromEndAtLSide [xx].leftBottomX = 0.0;
		placingZone->cellsFromEndAtLSide [xx].leftBottomY = 0.0;
		placingZone->cellsFromEndAtLSide [xx].leftBottomZ = 0.0;
		placingZone->cellsFromEndAtLSide [xx].ang = 0.0;
		placingZone->cellsFromEndAtLSide [xx].dirLen = 0.0;
		placingZone->cellsFromEndAtLSide [xx].perLen = 0.0;
		placingZone->cellsFromEndAtLSide [xx].attached_side = LEFT_SIDE;

		placingZone->cellsFromEndAtRSide [xx].objType = NONE;
		placingZone->cellsFromEndAtRSide [xx].leftBottomX = 0.0;
		placingZone->cellsFromEndAtRSide [xx].leftBottomY = 0.0;
		placingZone->cellsFromEndAtRSide [xx].leftBottomZ = 0.0;
		placingZone->cellsFromEndAtRSide [xx].ang = 0.0;
		placingZone->cellsFromEndAtRSide [xx].dirLen = 0.0;
		placingZone->cellsFromEndAtRSide [xx].perLen = 0.0;
		placingZone->cellsFromEndAtRSide [xx].attached_side = RIGHT_SIDE;

		placingZone->cellsFromBeginAtBottom [xx].objType = NONE;
		placingZone->cellsFromBeginAtBottom [xx].leftBottomX = 0.0;
		placingZone->cellsFromBeginAtBottom [xx].leftBottomY = 0.0;
		placingZone->cellsFromBeginAtBottom [xx].leftBottomZ = 0.0;
		placingZone->cellsFromBeginAtBottom [xx].ang = 0.0;
		placingZone->cellsFromBeginAtBottom [xx].dirLen = 0.0;
		placingZone->cellsFromBeginAtBottom [xx].perLen = 0.0;
		placingZone->cellsFromBeginAtBottom [xx].attached_side = BOTTOM_SIDE;

		placingZone->cellsFromEndAtBottom [xx].objType = NONE;
		placingZone->cellsFromEndAtBottom [xx].leftBottomX = 0.0;
		placingZone->cellsFromEndAtBottom [xx].leftBottomY = 0.0;
		placingZone->cellsFromEndAtBottom [xx].leftBottomZ = 0.0;
		placingZone->cellsFromEndAtBottom [xx].ang = 0.0;
		placingZone->cellsFromEndAtBottom [xx].dirLen = 0.0;
		placingZone->cellsFromEndAtBottom [xx].perLen = 0.0;
		placingZone->cellsFromEndAtBottom [xx].attached_side = BOTTOM_SIDE;
	}

	placingZone->cellCenterAtLSide.objType = NONE;
	placingZone->cellCenterAtLSide.leftBottomX = 0.0;
	placingZone->cellCenterAtLSide.leftBottomY = 0.0;
	placingZone->cellCenterAtLSide.leftBottomZ = 0.0;
	placingZone->cellCenterAtLSide.ang = 0.0;
	placingZone->cellCenterAtLSide.dirLen = 0.0;
	placingZone->cellCenterAtLSide.perLen = 0.0;
	placingZone->cellCenterAtLSide.attached_side = LEFT_SIDE;

	placingZone->cellCenterAtRSide.objType = NONE;
	placingZone->cellCenterAtRSide.leftBottomX = 0.0;
	placingZone->cellCenterAtRSide.leftBottomY = 0.0;
	placingZone->cellCenterAtRSide.leftBottomZ = 0.0;
	placingZone->cellCenterAtRSide.ang = 0.0;
	placingZone->cellCenterAtRSide.dirLen = 0.0;
	placingZone->cellCenterAtRSide.perLen = 0.0;
	placingZone->cellCenterAtRSide.attached_side = RIGHT_SIDE;

	placingZone->cellCenterAtBottom.objType = NONE;
	placingZone->cellCenterAtBottom.leftBottomX = 0.0;
	placingZone->cellCenterAtBottom.leftBottomY = 0.0;
	placingZone->cellCenterAtBottom.leftBottomZ = 0.0;
	placingZone->cellCenterAtBottom.ang = 0.0;
	placingZone->cellCenterAtBottom.dirLen = 0.0;
	placingZone->cellCenterAtBottom.perLen = 0.0;
	placingZone->cellCenterAtBottom.attached_side = BOTTOM_SIDE;
}

// 1�� ��ġ
void	firstPlacingSettingsForBeam (BeamPlacingZone* placingZone)
{
	short			xx;
	API_Coord3D		axisPoint, rotatedPoint, unrotatedPoint;

	double			centerPos;		// �߽� ��ġ
	double			width;			// ���� ���� �����ϴ� �ʺ�
	double			remainLength;	// ���� ���̸� ����ϱ� ���� �ӽ� ����
	double			xPos;			// ��ġ Ŀ��
	double			accumDist;		// �̵� �Ÿ�
	
	double			formLength1 = 1.200;
	double			formLength2 = 0.900;
	double			formLength3 = 0.600;
	double			formLength;
	
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

	// ���鿡���� �߽� ��ġ ã��
	if (placingZone->bInterfereBeam == true) {
		centerPos = placingZone->posInterfereBeamFromLeft;	// ���� ���� �߽� ��ġ
		width = placingZone->interfereBeamWidth;
	} else {
		centerPos = placingZone->beamLength / 2;			// ���� ���� ������ �߽��� �������� ��
		width = 0.0;
	}

	// ���� ���̿� ���� ������ ���̸� �ڵ����� ����
	if ((placingZone->areaHeight - 0.050) >= 0.600)
		placingZone->eu_wid_numeric = 0.600;
	else if ((placingZone->areaHeight - 0.050) >= 0.500)
		placingZone->eu_wid_numeric = 0.500;
	else if ((placingZone->areaHeight - 0.050) >= 0.450)
		placingZone->eu_wid_numeric = 0.450;
	else if ((placingZone->areaHeight - 0.050) >= 0.400)
		placingZone->eu_wid_numeric = 0.400;
	else if ((placingZone->areaHeight - 0.050) >= 0.300)
		placingZone->eu_wid_numeric = 0.300;
	else
		placingZone->eu_wid_numeric = 0.200;

	// (1-1) ���� ���� �κ�
	remainLength = centerPos - width/2;
	while (remainLength < EPS) {
		if (remainLength > formLength1) {
			formLength = formLength1;
			remainLength -= formLength1;
		} else if (remainLength > formLength2) {
			formLength = formLength2;
			remainLength -= formLength2;
		} else if (remainLength > formLength3) {
			formLength = formLength3;
			remainLength -= formLength3;
		}

		placingZone->cellsFromBeginAtLSide [placingZone->nCellsFromBeginAtSide].objType = EUROFORM;
		placingZone->cellsFromBeginAtLSide [placingZone->nCellsFromBeginAtSide].ang = placingZone->ang;
		placingZone->cellsFromBeginAtLSide [placingZone->nCellsFromBeginAtSide].dirLen = formLength;
		placingZone->cellsFromBeginAtLSide [placingZone->nCellsFromBeginAtSide].perLen = placingZone->eu_wid_numeric;
		placingZone->cellsFromBeginAtLSide [placingZone->nCellsFromBeginAtSide].attached_side = LEFT_SIDE;
		placingZone->cellsFromBeginAtLSide [placingZone->nCellsFromBeginAtSide].libPart.form.eu_stan_onoff = true;
		placingZone->cellsFromBeginAtLSide [placingZone->nCellsFromBeginAtSide].libPart.form.u_ins_wall = false;
		placingZone->cellsFromBeginAtLSide [placingZone->nCellsFromBeginAtSide].libPart.form.eu_hei = formLength;
		placingZone->cellsFromBeginAtLSide [placingZone->nCellsFromBeginAtSide].libPart.form.eu_wid = placingZone->eu_wid_numeric;

		placingZone->cellsFromBeginAtRSide [placingZone->nCellsFromBeginAtSide].objType = EUROFORM;
		placingZone->cellsFromBeginAtRSide [placingZone->nCellsFromBeginAtSide].ang = placingZone->ang;
		placingZone->cellsFromBeginAtRSide [placingZone->nCellsFromBeginAtSide].dirLen = formLength;
		placingZone->cellsFromBeginAtRSide [placingZone->nCellsFromBeginAtSide].perLen = placingZone->eu_wid_numeric;
		placingZone->cellsFromBeginAtRSide [placingZone->nCellsFromBeginAtSide].attached_side = LEFT_SIDE;
		placingZone->cellsFromBeginAtRSide [placingZone->nCellsFromBeginAtSide].libPart.form.eu_stan_onoff = true;
		placingZone->cellsFromBeginAtRSide [placingZone->nCellsFromBeginAtSide].libPart.form.u_ins_wall = false;
		placingZone->cellsFromBeginAtRSide [placingZone->nCellsFromBeginAtSide].libPart.form.eu_hei = formLength;
		placingZone->cellsFromBeginAtRSide [placingZone->nCellsFromBeginAtSide].libPart.form.eu_wid = placingZone->eu_wid_numeric;

		placingZone->nCellsFromBeginAtSide ++;
	}

	// �߽ɺ��� ������ �̵��ؾ� ��
	accumDist = 0.0;
	for (xx = 0 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)
		accumDist += placingZone->cellsFromBeginAtLSide [placingZone->nCellsFromBeginAtSide].dirLen;

	// ��ġ ����
	xPos = centerPos - width/2 - accumDist;
	for (xx = 0 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx) {
		placingZone->cellsFromBeginAtLSide [xx].leftBottomX = xPos;
		placingZone->cellsFromBeginAtLSide [xx].leftBottomY = infoBeam.width/2;
		placingZone->cellsFromBeginAtLSide [xx].leftBottomZ = placingZone->level - infoBeam.height;
		
		placingZone->cellsFromBeginAtRSide [xx].leftBottomX = xPos;
		placingZone->cellsFromBeginAtRSide [xx].leftBottomY = -infoBeam.width/2;
		placingZone->cellsFromBeginAtRSide [xx].leftBottomZ = placingZone->level - infoBeam.height;

		xPos += placingZone->cellsFromBeginAtLSide [xx].dirLen;
	}

	// (1-2) ���� �� �κ�
	//remainLength = placingZone->beamLength - centerPos - width/2;
	// ...

	// (1-3) ���� ��� �κ�
	// ...

	// (2-1) �Ϻ� ���� �κ�
	// ...

	// (2-2) �Ϻ� �� �κ�
	// ...

	// (2-3) �Ϻ� ��� �κ�
	// ...


	//		// ������ʹ� �� ��ġ ���� ��ƾ
	//		placingZone->cells [xx][yy].leftBottomX = placingZone->leftBottomX + (placingZone->cells [xx][yy].horLen * yy);
	//		placingZone->cells [xx][yy].leftBottomY = placingZone->leftBottomY - (placingZone->cells [xx][yy].verLen * xx);
	//		placingZone->cells [xx][yy].leftBottomZ = placingZone->leftBottomZ;

	//		axisPoint.x = placingZone->leftBottomX;
	//		axisPoint.y = placingZone->leftBottomY;
	//		axisPoint.z = placingZone->leftBottomZ;

	//		rotatedPoint.x = placingZone->cells [xx][yy].leftBottomX;
	//		rotatedPoint.y = placingZone->cells [xx][yy].leftBottomY;
	//		rotatedPoint.z = placingZone->cells [xx][yy].leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		placingZone->cells [xx][yy].leftBottomX = unrotatedPoint.x;
	//		placingZone->cells [xx][yy].leftBottomY = unrotatedPoint.y;
	//		placingZone->cells [xx][yy].leftBottomZ = unrotatedPoint.z;
	//	}
	//}

	// ���� �� ������Ʈ
	// ...
}

// �ش� ���� ������ �࿡ �ִ� �ٸ� ������ Ÿ�� �� ���̸� ������
void		adjustOtherCellsInSameRow (BeamPlacingZone* target_zone, short row, short col)
{
}

// ���� ���� �κ� - ���ο� ���� �߰��� (�� �ϳ��� �ø��� �߰��� ���� ������ �� ���� ����)
void		addNewColFromBeginAtSide (BeamPlacingZone* target_zone)
{
}

// ���� ���� �κ� - ������ ���� ������
void		delLastColFromBeginAtSide (BeamPlacingZone* target_zone)
{
}

// ���� �� �κ� - ���ο� ���� �߰��� (�� �ϳ��� �ø��� �߰��� ���� ������ �� ���� ����)
void		addNewColFromEndAtSide (BeamPlacingZone* target_zone)
{
}

// ���� �� �κ� - ������ ���� ������
void		delLastColFromEndAtSide (BeamPlacingZone* target_zone)
{
}

// �Ϻ� ���� �κ� - ���ο� ���� �߰��� (�� �ϳ��� �ø��� �߰��� ���� ������ �� ���� ����)
void		addNewColFromBeginAtBottom (BeamPlacingZone* target_zone)
{
}

// �Ϻ� ���� �κ� - ������ ���� ������
void		delLastColFromBeginAtBottom (BeamPlacingZone* target_zone)
{
}

// �Ϻ� �� �κ� - ���ο� ���� �߰��� (�� �ϳ��� �ø��� �߰��� ���� ������ �� ���� ����)
void		addNewColFromEndAtBottom (BeamPlacingZone* target_zone)
{
}

// �Ϻ� �� �κ� - ������ ���� ������
void		delLastColFromEndAtBottom (BeamPlacingZone* target_zone)
{
}

// Cell ������ ����ʿ� ���� ����ȭ�� ��ġ�� ��������
void		alignPlacingZoneForBeam (BeamPlacingZone* target_zone)
{
}

// �ش� �� ������ ������� ���̺귯�� ��ġ
API_Guid	placeLibPartForBeam (CellForBeam objInfo)
{
	GSErrCode	err = NoError;

	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const	GS::uchar_t* gsmName = NULL;
	double	aParam;
	double	bParam;
	Int32	addParNum;

	std::string		tempString;

	// GUID ���� �ʱ�ȭ
	element.header.guid.clock_seq_hi_and_reserved = 0;
	element.header.guid.clock_seq_low = 0;
	element.header.guid.node[0] = 0;
	element.header.guid.node[1] = 0;
	element.header.guid.node[2] = 0;
	element.header.guid.node[3] = 0;
	element.header.guid.node[4] = 0;
	element.header.guid.node[5] = 0;
	element.header.guid.time_hi_and_version = 0;
	element.header.guid.time_low = 0;
	element.header.guid.time_mid = 0;

	// ���̺귯�� �̸� ����
	if (objInfo.objType == NONE)			return element.header.guid;
	if (objInfo.objType == EUROFORM)		gsmName = L("������v2.0.gsm");
	if (objInfo.objType == PLYWOOD)			gsmName = L("����v1.0.gsm");
	if (objInfo.objType == WOOD)			gsmName = L("����v1.0.gsm");
	if (objInfo.objType == OUTCORNER_ANGLE)	gsmName = L("�ƿ��ڳʾޱ�v1.0.gsm");

	// ��ü �ε�
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return element.header.guid;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

	element.header.typeID = API_ObjectID;
	element.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&element, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	// ���̺귯���� �Ķ���� �� �Է�
	element.object.libInd = libPart.index;
	element.object.pos.x = objInfo.leftBottomX;
	element.object.pos.y = objInfo.leftBottomY;
	element.object.level = objInfo.leftBottomZ;
	element.object.xRatio = aParam;
	element.object.yRatio = bParam;
	element.object.angle = objInfo.ang;
	element.header.floorInd = infoBeam.floorInd;

	if (objInfo.objType == EUROFORM) {
		element.header.layer = layerInd_Euroform;

		// �԰�ǰ�� ���,
		if (objInfo.libPart.form.eu_stan_onoff == true) {
			// �԰��� On/Off
			memo.params [0][27].value.real = TRUE;

			// �ʺ�
			tempString = format_string ("%.0f", objInfo.libPart.form.eu_wid * 1000);
			GS::ucscpy (memo.params [0][28].value.uStr, GS::UniString (tempString.c_str ()).ToUStr ().Get ());

			// ����
			tempString = format_string ("%.0f", objInfo.libPart.form.eu_hei * 1000);
			GS::ucscpy (memo.params [0][29].value.uStr, GS::UniString (tempString.c_str ()).ToUStr ().Get ());

		// ��԰�ǰ�� ���,
		} else {
			// �԰��� On/Off
			memo.params [0][27].value.real = FALSE;

			// �ʺ�
			memo.params [0][30].value.real = objInfo.libPart.form.eu_wid2;

			// ����
			memo.params [0][31].value.real = objInfo.libPart.form.eu_hei2;
		}

		// ��ġ����
		if (objInfo.libPart.form.u_ins_wall == true) {
			tempString = "�������";
		} else {
			tempString = "��������";
			if (objInfo.libPart.form.eu_stan_onoff == true) {
				element.object.pos.x += ( objInfo.libPart.form.eu_hei * cos(objInfo.ang) );
				element.object.pos.y += ( objInfo.libPart.form.eu_hei * sin(objInfo.ang) );
			} else {
				element.object.pos.x += ( objInfo.libPart.form.eu_hei2 * cos(objInfo.ang) );
				element.object.pos.y += ( objInfo.libPart.form.eu_hei2 * sin(objInfo.ang) );
			}
		}
		GS::ucscpy (memo.params [0][32].value.uStr, GS::UniString (tempString.c_str ()).ToUStr ().Get ());
		
		// ȸ��X
		memo.params [0][33].value.real = DegreeToRad (0.0);

	} else if (objInfo.objType == PLYWOOD) {
		element.header.layer = layerInd_Plywood;
		GS::ucscpy (memo.params [0][32].value.uStr, L("��԰�"));
		GS::ucscpy (memo.params [0][33].value.uStr, L("�ٴڱ��"));
		GS::ucscpy (memo.params [0][34].value.uStr, L("11.5T"));
		memo.params [0][35].value.real = objInfo.libPart.plywood.p_wid;		// ����
		memo.params [0][36].value.real = objInfo.libPart.plywood.p_leng;	// ����
		memo.params [0][38].value.real = FALSE;		// ����Ʋ OFF
		
	} else if (objInfo.objType == WOOD) {
		element.header.layer = layerInd_Wood;
		GS::ucscpy (memo.params [0][27].value.uStr, L("�ٴڴ�����"));	// ��ġ����
		memo.params [0][28].value.real = objInfo.libPart.wood.w_w;		// �β�
		memo.params [0][29].value.real = objInfo.libPart.wood.w_h;		// �ʺ�
		memo.params [0][30].value.real = objInfo.libPart.wood.w_leng;	// ����
		memo.params [0][31].value.real = objInfo.libPart.wood.w_ang;	// ����
	
	} else if (objInfo.objType == OUTCORNER_ANGLE) {
		element.header.layer = layerInd_OutcornerAngle;
		// ...
	}

	// ��ü ��ġ
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return element.header.guid;
}

// �������� ä�� �� ������ ���� ä���
GSErrCode	fillRestAreasForBeam (void)
{
	// ...

	return	NoError;
}

// 1�� ��ġ�� ���� ���Ǹ� ��û�ϴ� 1�� ���̾�α�
short DGCALLBACK beamPlacerHandler1 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short		result;
	API_UCCallbackType	ucb;

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, "���� ��ġ");

			//////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			// ���� ��ư
			DGSetItemText (dialogID, DG_OK, "Ȯ ��");

			// ���� ��ư
			DGSetItemText (dialogID, DG_CANCEL, "�� ��");

			//////////////////////////////////////////////////////////// ������ ��ġ (������)
			// ��: ������ ��ġ ����
			DGSetItemText (dialogID, LABEL_PLACING_EUROFORM, "������ ��ġ ����");

			// ��: �ʺ�
			DGSetItemText (dialogID, LABEL_EUROFORM_WIDTH, "�ʺ�");

			// ��: ����
			DGSetItemText (dialogID, LABEL_EUROFORM_HEIGHT, "����");

			// ��: ������ ����
			DGSetItemText (dialogID, LABEL_GAP_LENGTH, "������ ����");

			// ��: ���̾� ����
			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, "���纰 ���̾� ����");

			// ��: ���̾� - ������
			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM, "������");

			// ��: ���̾� - ����
			DGSetItemText (dialogID, LABEL_LAYER_PLYWOOD, "����");

			// ��: ���̾� - ����
			DGSetItemText (dialogID, LABEL_LAYER_WOOD, "����");

			// ��: ���̾� - �ƿ��ڳʾޱ�
			DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER_ANGLE, "�ƿ��ڳʾޱ�");

			// ���� ��Ʈ�� �ʱ�ȭ
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_LAYER_EUROFORM;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, 1);

			ucb.itemID	 = USERCONTROL_LAYER_PLYWOOD;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, 1);

			ucb.itemID	 = USERCONTROL_LAYER_WOOD;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, 1);

			ucb.itemID	 = USERCONTROL_LAYER_OUTCORNER_ANGLE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, 1);

			// ������ �ʺ�� ���̴� �ڵ����� ���ǹǷ� �Է� ���� ����
			DGDisableItem (dialogID, POPUP_EUROFORM_WIDTH);
			DGDisableItem (dialogID, POPUP_EUROFORM_HEIGHT);

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					//////////////////////////////////////// ���̾�α� â ������ �Է� ����
					// ������ ����
					placingZone.gap = DGGetItemValDouble (dialogID, EDITCONTROL_GAP_LENGTH);

					// ���̾� ��ȣ ����
					layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM);
					layerInd_Plywood		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD);
					layerInd_Wood			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD);
					layerInd_OutcornerAngle	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE);

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

// 1�� ��ġ �� ������ ��û�ϴ� 2�� ���̾�α�
short DGCALLBACK beamPlacerHandler2 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	btnSizeX = 50, btnSizeY = 50;
	short	dialogSizeX, dialogSizeY;
	short	btnInitPosX = 220 + 25;
	short	btnPosX, btnPosY;
	short	xx, yy;
	short	idxBtn;
	short	lastIdxBtn = 0;
	std::string		txtButton = "";
	API_Element		elem;
	GSErrCode		err;

	switch (message) {
		case DG_MSG_INIT:
			//// ���̾�α� Ÿ��Ʋ
			//DGSetDialogTitle (dialogID, "������ �Ϻο� ��ġ - ������ ��ġ ����");

			////////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			//// ������Ʈ ��ư
			//DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 210, 130, 25);
			//DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, DG_OK, "2. ��  ġ");
			//DGShowItem (dialogID, DG_OK);

			//// ���� ��ư
			//DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 250, 130, 25);
			//DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, DG_CANCEL, "3. ������ ä���");
			//DGShowItem (dialogID, DG_CANCEL);

			//// ��: ���� ���� ����
			//DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 30, 20, 90, 23);
			//if ( ((placingZone.remain_hor_updated / 2) >= 0.150) && ((placingZone.remain_hor_updated / 2) <= 0.300) )
			//	DGSetItemFont (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH, DG_IS_LARGE | DG_IS_BOLD);
			//else
			//	DGSetItemFont (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH, "�¿� ����");
			//DGShowItem (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH);

			//// ��: ���� ���� ����
			//DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 30, 50, 90, 23);
			//if ( ((placingZone.remain_ver_updated / 2) >= 0.150) && ((placingZone.remain_ver_updated / 2) <= 0.300) )
			//	DGSetItemFont (dialogID, LABEL_REMAIN_VERTICAL_LENGTH, DG_IS_LARGE | DG_IS_BOLD);
			//else
			//	DGSetItemFont (dialogID, LABEL_REMAIN_VERTICAL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, LABEL_REMAIN_VERTICAL_LENGTH, "���� ����");
			//DGShowItem (dialogID, LABEL_REMAIN_VERTICAL_LENGTH);

			//// Edit ��Ʈ��: ���� ���� ����
			//DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 130, 20-7, 50, 25);
			//DGSetItemFont (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			//DGShowItem (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH);
			//DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH, placingZone.remain_hor_updated / 2);

			//// Edit ��Ʈ��: ���� ���� ����
			//DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 130, 50-7, 50, 25);
			//DGSetItemFont (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			//DGShowItem (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH);
			//DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH, placingZone.remain_ver_updated / 2);

			//// ��: ������/�ٷ������̼� ��ġ ����
			//DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 200, 10, 200, 23);
			//DGSetItemFont (dialogID, LABEL_GRID_EUROFORM_WOOD, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, LABEL_GRID_EUROFORM_WOOD, "������/���� ���� ��ġ ����");
			//DGShowItem (dialogID, LABEL_GRID_EUROFORM_WOOD);

			//// ���� �Ÿ� Ȯ�� ��ư
			//DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 90, 130, 25);
			//DGSetItemFont (dialogID, PUSHBUTTON_CONFIRM_REMAIN_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, PUSHBUTTON_CONFIRM_REMAIN_LENGTH, "1. ���� ���� Ȯ��");
			//DGShowItem (dialogID, PUSHBUTTON_CONFIRM_REMAIN_LENGTH);

			//// �� �߰� ��ư
			//DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 130, 65, 25);
			//DGSetItemFont (dialogID, PUSHBUTTON_ADD_ROW, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, PUSHBUTTON_ADD_ROW, "�� �߰�");
			//DGShowItem (dialogID, PUSHBUTTON_ADD_ROW);

			//// �� ���� ��ư
			//DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 105, 130, 65, 25);
			//DGSetItemFont (dialogID, PUSHBUTTON_DEL_ROW, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, PUSHBUTTON_DEL_ROW, "�� ����");
			//DGShowItem (dialogID, PUSHBUTTON_DEL_ROW);

			//// �� �߰� ��ư
			//DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 170, 65, 25);
			//DGSetItemFont (dialogID, PUSHBUTTON_ADD_COL, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, PUSHBUTTON_ADD_COL, "�� �߰�");
			//DGShowItem (dialogID, PUSHBUTTON_ADD_COL);
		
			//// �� ���� ��ư
			//DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 105, 170, 65, 25);
			//DGSetItemFont (dialogID, PUSHBUTTON_DEL_COL, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, PUSHBUTTON_DEL_COL, "�� ����");
			//DGShowItem (dialogID, PUSHBUTTON_DEL_COL);

			//// ���� â ũ�⸦ ����
			//dialogSizeX = 270 + (btnSizeX * placingZone.eu_count_hor) + 50;
			//dialogSizeY = max<short>(300, 150 + (btnSizeY * placingZone.eu_count_ver) + 50);
			//DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);

			//// �׸��� ����ü�� ���� ��ư�� �������� ��ġ��
			//btnPosX = 220 + 25, btnPosY = (btnSizeY * placingZone.eu_count_ver) + 25;
			//for (xx = 0 ; xx < placingZone.eu_count_ver ; ++xx) {
			//	for (yy = 0 ; yy < placingZone.eu_count_hor ; ++yy) {
			//		idxBtn = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
			//		lastIdxBtn = idxBtn;
			//		DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL);

			//		txtButton = "";
			//		if (placingZone.cells [xx][yy].objType == NONE) {
			//			txtButton = "NONE";
			//		} else if (placingZone.cells [xx][yy].objType == EUROFORM) {
			//			if (placingZone.cells [xx][yy].libPart.form.u_ins_wall)
			//				txtButton = format_string ("������\n(����)\n��%.0f\n��%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
			//			else
			//				txtButton = format_string ("������\n(����)\n��%.0f\n��%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
			//		}
			//		DGSetItemText (dialogID, idxBtn, txtButton.c_str ());		// �׸��� ��ư �ؽ�Ʈ ����
			//		DGShowItem (dialogID, idxBtn);
			//		btnPosX += btnSizeX;
			//	}
			//	btnPosX = btnInitPosX;
			//	btnPosY -= btnSizeY;
			//}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				//case PUSHBUTTON_CONFIRM_REMAIN_LENGTH:
				//	// �������� �ʰ� ���� ���� �Ÿ��� �׸��� ��ư �Ӽ��� ������
				//	item = 0;

				//	break;

				case DG_OK:
					// �������� �ʰ� ��ġ�� ��ü�� ���� �� ���ġ�ϰ� �׸��� ��ư �Ӽ��� ������
					item = 0;

					//clickedOKButton = true;

					break;

				case DG_CANCEL:
					break;

				//case PUSHBUTTON_ADD_ROW:
				//	// �������� �ʰ� ���� ���� �Ÿ��� �׸��� ��ư �Ӽ��� ������
				//	item = 0;

				//	break;

				//case PUSHBUTTON_DEL_ROW:
				//	// �������� �ʰ� ���� ���� �Ÿ��� �׸��� ��ư �Ӽ��� ������
				//	item = 0;

				//	break;

				//case PUSHBUTTON_ADD_COL:
				//	// �������� �ʰ� ���� ���� �Ÿ��� �׸��� ��ư �Ӽ��� ������
				//	item = 0;

				//	break;

				//case PUSHBUTTON_DEL_COL:
				//	// �������� �ʰ� ���� ���� �Ÿ��� �׸��� ��ư �Ӽ��� ������
				//	item = 0;

				//	break;

				default:
					// [DIALOG] �׸��� ��ư�� ������ Cell�� �����ϱ� ���� ���� â(3��° ���̾�α�)�� ����
					//clickedBtnItemIdx = item;
					//result = DGBlankModalDialog (240, 260, DG_DLG_NOGROW, 0, DG_DLG_NORMALFRAME, beamPlacerHandler3, 0);

					item = 0;	// �׸��� ��ư�� ������ �� â�� ������ �ʰ� ��

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

// 2�� ���̾�α׿��� �� ���� ��ü Ÿ���� �����ϱ� ���� 3�� ���̾�α�
short DGCALLBACK beamPlacerHandler3 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	idxItem;
	short	idxCell;
	short	popupSelectedIdx = 0;
	double	temp;
	short	rIdx, cIdx;		// �� ��ȣ, �� ��ȣ

	switch (message) {
		case DG_MSG_INIT:

			//// beamPlacerHandler2 ���� Ŭ���� �׸��� ��ư�� �ε��� ���� �̿��Ͽ� �� �ε��� �� �ε�
			//idxCell = (clickedBtnItemIdx - itemInitIdx);
			//rIdx = 0;
			//while (idxCell >= (placingZone.eu_count_hor)) {
			//	idxCell -= ((placingZone.eu_count_hor));
			//	++rIdx;
			//}
			//cIdx = idxCell;
			//
			//// ���̾�α� Ÿ��Ʋ
			//DGSetDialogTitle (dialogID, "Cell �� ����");

			////////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			//// ���� ��ư
			//DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 215, 70, 25);
			//DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, DG_OK, "����");
			//DGShowItem (dialogID, DG_OK);

			//// ���� ��ư
			//DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 130, 215, 70, 25);
			//DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, DG_CANCEL, "���");
			//DGShowItem (dialogID, DG_CANCEL);

			////////////////////////////////////////////////////////////// �ʵ� ���� (Ŭ���� ��)
			//// ��: ��ü Ÿ��
			//idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 20, 70, 23);
			//DGSetItemFont (dialogID, LABEL_OBJ_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, LABEL_OBJ_TYPE, "��ü Ÿ��");
			//DGShowItem (dialogID, LABEL_OBJ_TYPE);
			//DGDisableItem (dialogID, LABEL_OBJ_TYPE);

			//// �˾���Ʈ��: ��ü Ÿ���� �ٲ� �� �ִ� �޺��ڽ��� �� ���� ����
			//DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100, 20-7, 120, 25);
			//DGSetItemFont (dialogID, POPUP_OBJ_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			//DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			//DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, "����");
			//DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			//DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, "������");
			//DGShowItem (dialogID, POPUP_OBJ_TYPE);
			//DGDisableItem (dialogID, POPUP_OBJ_TYPE);

			//// ��: �ʺ�
			//idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 50, 70, 23);
			//DGSetItemFont (dialogID, LABEL_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, LABEL_WIDTH, "�ʺ�");

			//// Edit ��Ʈ��: �ʺ�
			//DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100, 50-6, 50, 25);
			//DGSetItemFont (dialogID, EDITCONTROL_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);

			//// ��: ����
			//idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 80, 70, 23);
			//DGSetItemFont (dialogID, LABEL_HEIGHT, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, LABEL_HEIGHT, "����");

			//// Edit ��Ʈ��: ����
			//DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100, 80-6, 50, 25);
			//DGSetItemFont (dialogID, EDITCONTROL_HEIGHT, DG_IS_LARGE | DG_IS_PLAIN);

			//// ��: ��ġ����
			//idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 140, 70, 23);
			//DGSetItemFont (dialogID, LABEL_ORIENTATION, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, LABEL_ORIENTATION, "��ġ����");
			//	
			//// ���� ��ư: ��ġ���� (�������)
			//idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 777, 100, 140-6, 70, 25);
			//DGSetItemFont (dialogID, RADIO_ORIENTATION_1_PLYWOOD, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, RADIO_ORIENTATION_1_PLYWOOD, "�������");
			//// ���� ��ư: ��ġ���� (��������)
			//idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 777, 100, 170-6, 70, 25);
			//DGSetItemFont (dialogID, RADIO_ORIENTATION_2_PLYWOOD, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, RADIO_ORIENTATION_2_PLYWOOD, "��������");

			//// üũ�ڽ�: �԰���
			//DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 20, 50, 70, 25-5);
			//DGSetItemFont (dialogID, CHECKBOX_SET_STANDARD, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, CHECKBOX_SET_STANDARD, "�԰���");

			//// ��: �ʺ�
			//idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 80, 70, 23);
			//DGSetItemFont (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS, "�ʺ�");

			//// �˾� ��Ʈ��: �ʺ�
			//DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100, 80-7, 100, 25);
			//DGSetItemFont (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			//DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM);
			//DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM, "600");
			//DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM);
			//DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM, "500");
			//DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM);
			//DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM, "450");
			//DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM);
			//DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM, "400");
			//DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM);
			//DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM, "300");
			//DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM);
			//DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM, "200");

			//// Edit ��Ʈ��: �ʺ�
			//DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100, 80-6, 50, 25);
			//DGSetItemFont (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);

			//// ��: ����
			//idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 110, 70, 23);
			//DGSetItemFont (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS, "����");

			//// �˾� ��Ʈ��: ����
			//DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100, 110-7, 100, 25);
			//DGSetItemFont (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			//DGPopUpInsertItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM);
			//DGPopUpSetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM, "1200");
			//DGPopUpInsertItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM);
			//DGPopUpSetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM, "900");
			//DGPopUpInsertItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM);
			//DGPopUpSetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM, "600");

			//// Edit ��Ʈ��: ����
			//DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100, 110-6, 50, 25);
			//DGSetItemFont (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			//
			//// ��: ��ġ����
			//idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 140, 70, 23);
			//DGSetItemFont (dialogID, LABEL_EUROFORM_ORIENTATION_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, LABEL_EUROFORM_ORIENTATION_OPTIONS, "��ġ����");
			//
			//// ���� ��ư: ��ġ���� (�������)
			//idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 778, 100, 140-6, 70, 25);
			//DGSetItemFont (dialogID, RADIO_ORIENTATION_1_EUROFORM, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, RADIO_ORIENTATION_1_EUROFORM, "�������");
			//// ���� ��ư: ��ġ���� (��������)
			//idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 778, 100, 170-6, 70, 25);
			//DGSetItemFont (dialogID, RADIO_ORIENTATION_2_EUROFORM, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, RADIO_ORIENTATION_2_EUROFORM, "��������");

			//// �ʱ� �Է� �ʵ� ǥ��
			//if (placingZone.cells [rIdx][cIdx].objType == EUROFORM) {
			//	DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, EUROFORM + 1);

			//	// üũ�ڽ�: �԰���
			//	DGShowItem (dialogID, CHECKBOX_SET_STANDARD);
			//	DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD, placingZone.cells [rIdx][cIdx].libPart.form.eu_stan_onoff);

			//	if (placingZone.cells [rIdx][cIdx].libPart.form.eu_stan_onoff == true) {
			//		// ��: �ʺ�
			//		DGShowItem (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS);

			//		// �˾� ��Ʈ��: �ʺ�
			//		DGShowItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS);
			//		if (abs(placingZone.cells [rIdx][cIdx].libPart.form.eu_wid - 0.600) < EPS)		popupSelectedIdx = 1;
			//		if (abs(placingZone.cells [rIdx][cIdx].libPart.form.eu_wid - 0.500) < EPS)		popupSelectedIdx = 2;
			//		if (abs(placingZone.cells [rIdx][cIdx].libPart.form.eu_wid - 0.450) < EPS)		popupSelectedIdx = 3;
			//		if (abs(placingZone.cells [rIdx][cIdx].libPart.form.eu_wid - 0.400) < EPS)		popupSelectedIdx = 4;
			//		if (abs(placingZone.cells [rIdx][cIdx].libPart.form.eu_wid - 0.300) < EPS)		popupSelectedIdx = 5;
			//		if (abs(placingZone.cells [rIdx][cIdx].libPart.form.eu_wid - 0.200) < EPS)		popupSelectedIdx = 6;
			//		DGPopUpSelectItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, popupSelectedIdx);

			//		// ��: ����
			//		DGShowItem (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS);

			//		// �˾� ��Ʈ��: ����
			//		DGShowItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS);
			//		if (abs(placingZone.cells [rIdx][cIdx].libPart.form.eu_hei - 1.200) < EPS)		popupSelectedIdx = 1;
			//		if (abs(placingZone.cells [rIdx][cIdx].libPart.form.eu_hei - 0.900) < EPS)		popupSelectedIdx = 2;
			//		if (abs(placingZone.cells [rIdx][cIdx].libPart.form.eu_hei - 0.600) < EPS)		popupSelectedIdx = 3;
			//		DGPopUpSelectItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, popupSelectedIdx);
			//	} else if (placingZone.cells [rIdx][cIdx].libPart.form.eu_stan_onoff == false) {
			//		// ��: �ʺ�
			//		DGShowItem (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS);

			//		// Edit ��Ʈ��: �ʺ�
			//		DGShowItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS);
			//		DGSetItemValDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS, placingZone.cells [rIdx][cIdx].libPart.form.eu_wid2);
			//		DGSetItemMinDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS, 0.050);
			//		DGSetItemMaxDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS, 0.900);

			//		// ��: ����
			//		DGShowItem (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS);

			//		// Edit ��Ʈ��: ����
			//		DGShowItem (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS);
			//		DGSetItemValDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS, placingZone.cells[rIdx][cIdx].libPart.form.eu_hei2);
			//		DGSetItemMinDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS, 0.050);
			//		DGSetItemMaxDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS, 1.500);
			//	}

			//	// ��: ��ġ����
			//	DGShowItem (dialogID, LABEL_EUROFORM_ORIENTATION_OPTIONS);
			//	
			//	// ���� ��ư: ��ġ���� (�������)
			//	DGShowItem (dialogID, RADIO_ORIENTATION_1_EUROFORM);
			//	// ���� ��ư: ��ġ���� (��������)
			//	DGShowItem (dialogID, RADIO_ORIENTATION_2_EUROFORM);

			//	if (placingZone.cells [rIdx][cIdx].libPart.form.u_ins_wall == true) {
			//		DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM, true);
			//		DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_EUROFORM, false);
			//	} else if (placingZone.cells [rIdx][cIdx].libPart.form.u_ins_wall == false) {
			//		DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM, false);
			//		DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_EUROFORM, true);
			//	}

			//	DGDisableItem (dialogID, RADIO_ORIENTATION_1_EUROFORM);
			//	DGDisableItem (dialogID, RADIO_ORIENTATION_2_EUROFORM);
			//}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:

					//// beamPlacerHandler2 ���� Ŭ���� �׸��� ��ư�� �ε��� ���� �̿��Ͽ� �� �ε��� �� �ε�
					//idxCell = (clickedBtnItemIdx - itemInitIdx);
					//rIdx = 0;
					//while (idxCell >= (placingZone.eu_count_hor)) {
					//	idxCell -= ((placingZone.eu_count_hor));
					//	++rIdx;
					//}
					//cIdx = idxCell;

					////////////////////////////////////////////////////////////// �ʵ� ���� (Ŭ���� ��)
					//// �Է��� ���� �ٽ� ���� ����
					//if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == NONE + 1) {
					//	placingZone.cells [rIdx][cIdx].objType = NONE;
					//	adjustOtherCellsInSameRow (&placingZone, rIdx, cIdx);
					//	adjustOtherCellsInSameCol (&placingZone, rIdx, cIdx);

					//} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == EUROFORM + 1) {
					//	placingZone.cells [rIdx][cIdx].objType = EUROFORM;

					//	// �԰���
					//	if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == TRUE)
					//		placingZone.cells [rIdx][cIdx].libPart.form.eu_stan_onoff = true;
					//	else if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == FALSE)
					//		placingZone.cells [rIdx][cIdx].libPart.form.eu_stan_onoff = false;

					//	if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == TRUE) {
					//		// �ʺ�
					//		placingZone.cells [rIdx][cIdx].libPart.form.eu_wid = atof (DGPopUpGetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DGPopUpGetSelected (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS)).ToCStr ()) / 1000.0;
					//		placingZone.cells [rIdx][cIdx].horLen = placingZone.cells [rIdx][cIdx].libPart.form.eu_wid;
					//		// ����
					//		placingZone.cells [rIdx][cIdx].libPart.form.eu_hei = atof (DGPopUpGetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DGPopUpGetSelected (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS)).ToCStr ()) / 1000.0;
					//		placingZone.cells [rIdx][cIdx].verLen = placingZone.cells [rIdx][cIdx].libPart.form.eu_hei;
					//	} else if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == FALSE) {
					//		// �ʺ�
					//		placingZone.cells [rIdx][cIdx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS);
					//		placingZone.cells [rIdx][cIdx].horLen = placingZone.cells [rIdx][cIdx].libPart.form.eu_wid2;
					//		// ����
					//		placingZone.cells [rIdx][cIdx].libPart.form.eu_hei2 = DGGetItemValDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS);
					//		placingZone.cells [rIdx][cIdx].verLen = placingZone.cells [rIdx][cIdx].libPart.form.eu_hei2;
					//	}

					//	// ��ġ����
					//	if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM) == TRUE)
					//		placingZone.cells [rIdx][cIdx].libPart.form.u_ins_wall = true;
					//	else if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM) == FALSE) {
					//		placingZone.cells [rIdx][cIdx].libPart.form.u_ins_wall = false;
					//		// ����, ���� ���� ��ȯ
					//		temp = placingZone.cells [rIdx][cIdx].horLen;
					//		placingZone.cells [rIdx][cIdx].horLen = placingZone.cells [rIdx][cIdx].verLen;
					//		placingZone.cells [rIdx][cIdx].verLen = temp;
					//	}

					//	adjustOtherCellsInSameRow (&placingZone, rIdx, cIdx);
					//	adjustOtherCellsInSameCol (&placingZone, rIdx, cIdx);
					//}

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
