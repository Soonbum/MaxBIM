#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "ColumnTableformPlacer.hpp"

using namespace columnTableformPlacerDG;

static ColumnTableformPlacingZone	placingZone;	// �⺻ ��� ���� ����
static InfoColumn			infoColumn;				// ��� ��ü ����

API_Guid	structuralObject_forTableformColumn;	// ���� ��ü�� GUID

static short						nInterfereBeams;		// ���� �� ����
static InfoBeamForColumnTableform	infoOtherBeams [10];	// ���� �� ����

static short			layerInd_Euroform;			// ���̾� ��ȣ: ������
static short			layerInd_Fillersp;			// ���̾� ��ȣ: �ٷ������̼�
static short			layerInd_OutPanel;			// ���̾� ��ȣ: �ƿ��ڳ��ǳ�
static short			layerInd_OutAngle;			// ���̾� ��ȣ: �ƿ��ڳʾޱ�
static short			layerInd_SqrPipe;			// ���̾� ��ȣ: ���������
static short			layerInd_Pinbolt;			// ���̾� ��ȣ: �ɺ�Ʈ��Ʈ
static short			layerInd_Hanger;			// ���̾� ��ȣ: �����������
static short			layerInd_Head;				// ���̾� ��ȣ: ������Ʈ�� Push-Pull Props ����ǽ�
static short			layerInd_ColumnBand;		// ���̾� ��ȣ: ��չ��
static short			layerInd_Plywood;			// ���̾� ��ȣ: ����
static short			layerInd_BlueClamp;			// ���̾� ��ȣ: ���Ŭ����
static short			clickedBtnItemIdx;			// �׸��� ��ư���� Ŭ���� ��ư�� �ε��� ��ȣ�� ����
static bool				clickedOKButton;			// OK ��ư�� �������ϱ�?
static bool				clickedPrevButton;			// ���� ��ư�� �������ϱ�?
static GS::Array<API_Guid>	elemList;				// �׷�ȭ�� ���� ������ ��������� GUID�� ���� ������

// ��-��� DG �׸� �ε��� ���� (���̸� ǥ���ϴ� Edit��Ʈ��)
static short	HLEN_LT, VLEN_LT;					// �»�� ����/���� ����
static short	HLEN_RT, VLEN_RT;					// ���� ����/���� ����
static short	HLEN_LB, VLEN_LB;					// ���ϴ� ����/���� ����
static short	HLEN_RB, VLEN_RB;					// ���ϴ� ����/���� ����
static short	LEN_T1, LEN_T2, LEN_B1, LEN_B2;		// T1, T2, B1, B2
static short	LEN_L1, LEN_L2, LEN_R1, LEN_R2;		// L1, L2, R1, R2

// �߰�/���� ��ư �ε��� ����
static short	EUROFORM_BUTTON_TOP;
static short	EUROFORM_BUTTON_BOTTOM;
static short	ADD_CELLS;
static short	DEL_CELLS;
static short	CHECKBOX_NORTH_MARGIN;
static short	CHECKBOX_SOUTH_MARGIN;
static short	CHECKBOX_WEST_MARGIN;
static short	CHECKBOX_EAST_MARGIN;
static short	EDITCONTROL_NORTH_MARGIN;
static short	EDITCONTROL_SOUTH_MARGIN;
static short	EDITCONTROL_WEST_MARGIN;
static short	EDITCONTROL_EAST_MARGIN;


// ��տ� �������� ��ġ�ϴ� ���� ��ƾ
GSErrCode	placeTableformOnColumn (void)
{
	GSErrCode		err = NoError;
	short			xx;
	short			result;
	double			lowestBeamBottomLevel;
	API_Coord		axisPoint, rotatedPoint, unrotatedPoint;

	GS::Array<API_Guid>		morphs;
	GS::Array<API_Guid>		columns;
	GS::Array<API_Guid>		beams;
	long					nMorphs = 0;
	long					nColumns = 0;
	long					nBeams = 0;

	// ��ü ���� ��������
	API_Element				elem;
	API_ElementMemo			memo;
	API_ElemInfo3D			info3D;

	// ���� ��ü ����
	InfoMorphForColumnTableform		infoMorph;

	// �۾� �� ����
	double					workLevel_column;
	double					workLevel_beam;


	// ������ ��� �������� (���� ��� 1�� �����ؾ� ��)
	err = getGuidsOfSelection (&columns, API_ColumnID, &nColumns);
	err = getGuidsOfSelection (&beams, API_BeamID, &nBeams);
	err = getGuidsOfSelection (&morphs, API_MorphID, &nMorphs);
	if (err == APIERR_NOPLAN) {
		DGAlert (DG_ERROR, L"����", L"���� ������Ʈ â�� �����ϴ�.", "", L"Ȯ��", "", "");
	}
	if (err == APIERR_NOSEL) {
		DGAlert (DG_ERROR, L"����", L"�ƹ� �͵� �������� �ʾҽ��ϴ�.\n�ʼ� ����: ��� (1��), ��� ������ ���� ���� (1��)\n�ɼ� ����: ��հ� �´�� �� (�ټ�)", "", L"Ȯ��", "", "");
	}

	// ����� 1���ΰ�?
	if (nColumns != 1) {
		DGAlert (DG_ERROR, L"����", L"����� 1�� �����ؾ� �մϴ�.", "", L"Ȯ��", "", "");
		err = APIERR_GENERAL;
		return err;
	}

	// ������ 1���ΰ�?
	if (nMorphs != 1) {
		DGAlert (DG_ERROR, L"����", L"��� ������ ���� ������ 1�� �����ϼž� �մϴ�.", "", L"Ȯ��", "", "");
		err = APIERR_GENERAL;
		return err;
	}

	// ��� ���� ����
	infoColumn.guid = columns.Pop ();

	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	elem.header.guid = infoColumn.guid;
	structuralObject_forTableformColumn = elem.header.guid;
	err = ACAPI_Element_Get (&elem);
	err = ACAPI_Element_GetMemo (elem.header.guid, &memo);
	
	infoColumn.floorInd		= elem.header.floorInd;			// �� �ε���
	infoColumn.bRectangle	= !elem.column.circleBased;		// ���簢���ΰ�?		// !elem.columnSegment.assemblySegmentData.circleBased;
	infoColumn.coreAnchor	= elem.column.coreAnchor;		// �ھ��� ��Ŀ ����Ʈ
	infoColumn.coreWidth	= elem.column.coreWidth;		// �ھ��� �ʺ� (X ����) // elem.columnSegment.assemblySegmentData.nominalWidth;
	infoColumn.coreDepth	= elem.column.coreDepth;		// �ھ��� ���� (Y ����)	// elem.columnSegment.assemblySegmentData.nominalHeight;
	infoColumn.venThick		= elem.column.venThick;			// ���Ͼ� �β�			// elem.columnSegment.venThick;
	infoColumn.height		= elem.column.height;			// ��� ����
	infoColumn.bottomOffset	= elem.column.bottomOffset;		// �ٴ� ������ ���� ��� ���̽� ����
	infoColumn.topOffset	= elem.column.topOffset;		// ���� ����� ������ ����Ǿ� �ִ� ��� �������κ����� ������
	infoColumn.angle		= elem.column.angle + elem.column.slantDirectionAngle;	// ��� ���� �߽����� �� ȸ�� ���� (����: Radian)	// elem.column.axisRotationAngle + elem.column.slantDirectionAngle;
	infoColumn.origoPos		= elem.column.origoPos;			// ��� �߽� ��ġ

	ACAPI_DisposeElemMemoHdls (&memo);

	// �۾� �� ���� �ݿ�
	workLevel_column = getWorkLevel (infoColumn.floorInd);

	// �� ���� ����
	nInterfereBeams = (short)nBeams;

	for (xx = 0 ; xx < nInterfereBeams ; ++xx) {
		infoOtherBeams [xx].guid = beams.Pop ();

		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		elem.header.guid = infoOtherBeams [xx].guid;
		err = ACAPI_Element_Get (&elem);
		err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

		infoOtherBeams [xx].valid		= true;						// ������ ��ȿ��
		infoOtherBeams [xx].floorInd	= elem.header.floorInd;		// �� �ε���
		infoOtherBeams [xx].height		= elem.beam.height;			// �� ����		// elem.beamSegment.assemblySegmentData.nominalHeight;
		infoOtherBeams [xx].width		= elem.beam.width;			// �� �ʺ�		// elem.beamSegment.assemblySegmentData.nominalWidth;
		infoOtherBeams [xx].offset		= elem.beam.offset;			// �� �߽����κ��� ���� ���۷��� ������ �������Դϴ�.
		infoOtherBeams [xx].level		= elem.beam.level;			// �ٴ� ������ ���� ���� ���ʸ� �����Դϴ�.
		infoOtherBeams [xx].begC		= elem.beam.begC;			// �� ���� ��ǥ
		infoOtherBeams [xx].endC		= elem.beam.endC;			// �� �� ��ǥ

		ACAPI_DisposeElemMemoHdls (&memo);

		// �۾� �� ���� �ݿ�
		workLevel_beam = getWorkLevel (infoOtherBeams [xx].floorInd);
	}

	// ���� ������ ������
	BNZeroMemory (&elem, sizeof (API_Element));
	elem.header.guid = morphs.Pop ();
	err = ACAPI_Element_Get (&elem);
	err = ACAPI_Element_Get3DInfo (elem.header, &info3D);

	// ������ ���� ����
	infoMorph.guid		= elem.header.guid;
	infoMorph.floorInd	= elem.header.floorInd;
	infoMorph.level		= info3D.bounds.zMin;
	infoMorph.height	= info3D.bounds.zMax - info3D.bounds.zMin;

	// ���� ���� ����
	GS::Array<API_Element>	elems;
	elems.Push (elem);
	deleteElements (elems);

	// ���� ���� ����
	placingZone.bRectangle		= infoColumn.bRectangle;	// ���簢���ΰ�?
	placingZone.coreAnchor		= infoColumn.coreAnchor;	// �ھ��� ��Ŀ ����Ʈ
	placingZone.coreWidth		= infoColumn.coreWidth;		// �ھ��� �ʺ� (X ����)
	placingZone.coreDepth		= infoColumn.coreDepth;		// �ھ��� ���� (Y ����)
	placingZone.venThick		= infoColumn.venThick;		// ���Ͼ� �β�
	placingZone.height			= infoColumn.height;		// ��� ����
	placingZone.bottomOffset	= infoColumn.bottomOffset;	// �ٴ� ������ ���� ��� ���̽� ����
	placingZone.topOffset		= infoColumn.topOffset;		// ���� ����� ������ ����Ǿ� �ִ� ��� �������κ����� ������
	placingZone.angle			= infoColumn.angle;			// ��� ���� �߽����� �� ȸ�� ���� (����: Radian)
	placingZone.origoPos		= infoColumn.origoPos;		// ��� �߽� ��ġ

	placingZone.areaHeight		= infoMorph.height;			// ���� ����

	// placingZone�� Cell ���� �ʱ�ȭ
	placingZone.initCells (&placingZone);

	// ���� �� ���� �ʱ�ȭ
	placingZone.bInterfereBeam = false;
	placingZone.nInterfereBeams = 0;

	for (xx = 0 ; xx < 4 ; ++xx) {
		placingZone.bottomLevelOfBeams [xx] = 0.0;
		placingZone.bExistBeams [xx] = false;
	}

	// ���� ���� �� ���� �� ���� ���� ������Ʈ
	if (nInterfereBeams > 0) {
		placingZone.bInterfereBeam = true;
		placingZone.nInterfereBeams = nInterfereBeams;
			
		for (xx = 0 ; xx < 4 ; ++xx) {
			axisPoint.x = placingZone.origoPos.x;
			axisPoint.y = placingZone.origoPos.y;

			if (infoOtherBeams [xx].valid == false)
				continue;

			rotatedPoint.x = infoOtherBeams [xx].begC.x;
			rotatedPoint.y = infoOtherBeams [xx].begC.y;
			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, -RadToDegree (placingZone.angle));

			// ����� ��/��/��/�� ���⿡ �ִ� ���� �ϴ� ������ ������
			if ( (unrotatedPoint.x <= (placingZone.origoPos.x + placingZone.coreWidth/2 + placingZone.venThick)) && (unrotatedPoint.x >= (placingZone.origoPos.x - placingZone.coreWidth/2 - placingZone.venThick)) && (unrotatedPoint.y >= (placingZone.origoPos.y + placingZone.coreDepth/2 + placingZone.venThick)) ) {
				placingZone.bottomLevelOfBeams [NORTH] = infoOtherBeams [xx].level - infoOtherBeams [xx].height;
				placingZone.bExistBeams [NORTH] = true;
				placingZone.beams [NORTH] = infoOtherBeams [xx];
			}
			if ( (unrotatedPoint.x <= (placingZone.origoPos.x + placingZone.coreWidth/2 + placingZone.venThick)) && (unrotatedPoint.x >= (placingZone.origoPos.x - placingZone.coreWidth/2 - placingZone.venThick)) && (unrotatedPoint.y <= (placingZone.origoPos.y - placingZone.coreDepth/2 - placingZone.venThick)) ) {
				placingZone.bottomLevelOfBeams [SOUTH] = infoOtherBeams [xx].level - infoOtherBeams [xx].height;
				placingZone.bExistBeams [SOUTH] = true;
				placingZone.beams [SOUTH] = infoOtherBeams [xx];
			}
			if ( (unrotatedPoint.y <= (placingZone.origoPos.y + placingZone.coreDepth/2 + placingZone.venThick)) && (unrotatedPoint.y >= (placingZone.origoPos.y - placingZone.coreDepth/2 - placingZone.venThick)) && (unrotatedPoint.x >= (placingZone.origoPos.x + placingZone.coreWidth/2 + placingZone.venThick)) ) {
				placingZone.bottomLevelOfBeams [EAST] = infoOtherBeams [xx].level - infoOtherBeams [xx].height;
				placingZone.bExistBeams [EAST] = true;
				placingZone.beams [EAST] = infoOtherBeams [xx];
			}
			if ( (unrotatedPoint.y <= (placingZone.origoPos.y + placingZone.coreDepth/2 + placingZone.venThick)) && (unrotatedPoint.y >= (placingZone.origoPos.y - placingZone.coreDepth/2 - placingZone.venThick)) && (unrotatedPoint.x <= (placingZone.origoPos.x - placingZone.coreWidth/2 - placingZone.venThick)) ) {
				placingZone.bottomLevelOfBeams [WEST] = infoOtherBeams [xx].level - infoOtherBeams [xx].height;
				placingZone.bExistBeams [WEST] = true;
				placingZone.beams [WEST] = infoOtherBeams [xx];
			}
		}
	}

	// ���� ���̿� �� �ϴ� ������ ����Ͽ� ���� ������ ����
	lowestBeamBottomLevel = placingZone.bottomOffset + placingZone.areaHeight;	// �⺻��: ���� ����
	if (nInterfereBeams > 0) {
		// ���� ���� �� �ϴ� ������ �������� �� ���� ����
		for (xx = 0 ; xx < 4 ; ++xx) {
			if (placingZone.bExistBeams [xx] == true)
				if (lowestBeamBottomLevel > placingZone.bottomLevelOfBeams [xx])
					lowestBeamBottomLevel = placingZone.bottomLevelOfBeams [xx];
		}
	}

	placingZone.nCells = static_cast<short>((lowestBeamBottomLevel + EPS) / 1.200);		// ���� 1200 �� �������� �� �� �ִ� �ִ� �� ���� ����

FIRST_SOLE_COLUMN:
	
	// [DIALOG] 1��° ���̾�α׿��� ������ ���� �Է� ����
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32508, ACAPI_GetOwnResModule (), columnTableformPlacerHandler_soleColumn_1, 0);

	if (result == DG_CANCEL)
		return err;

	// [DIALOG] 2��° ���̾�α׿��� ������ ��ġ�� �����մϴ�.
	clickedOKButton = false;
	clickedPrevButton = false;
	result = DGBlankModalDialog (700, 300, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, columnTableformPlacerHandler_soleColumn_2, 0);
	
	// ���� ��ư�� ������ 1��° ���̾�α� �ٽ� ����
	if (clickedPrevButton == true)
		goto FIRST_SOLE_COLUMN;

	// 2��° ���̾�α׿��� OK ��ư�� �����߸� ���� �ܰ�� �Ѿ
	if (clickedOKButton != true)
		return err;

	// 1, 2��° ���̾�α׸� ���� �Էµ� �����͸� ������� ��ü�� ��ġ
	err = placingZone.placeBasicObjects_soleColumn (&placingZone);

	// ���������, �ɺ�Ʈ��Ʈ/�����������, ����ǽ�, ��չ��/���� ��ġ
	if (placingZone.tableformType == 1)
		err = placingZone.placeRestObjectsA_soleColumn (&placingZone);
	else if (placingZone.tableformType == 2)
		err = placingZone.placeRestObjectsB_soleColumn (&placingZone);

	// ������ ���� ä��� - ����, ����
	err = placingZone.fillRestAreas_soleColumn (&placingZone);

	// �׷�ȭ�ϱ�
	groupElements (elemList);

	return	err;
}

// Cell �迭�� �ʱ�ȭ��
void	ColumnTableformPlacingZone::initCells (ColumnTableformPlacingZone* placingZone)
{
	short	xx;

	// ��� ���� ���� �ʱ�ȭ
	placingZone->marginTopAtNorth = 0.0;
	placingZone->marginTopAtWest = 0.0;
	placingZone->marginTopAtEast = 0.0;
	placingZone->marginTopAtSouth = 0.0;

	// ��� ���� ������ �⺻������ ä������ ����
	placingZone->bFillMarginTopAtNorth = true;
	placingZone->bFillMarginTopAtWest = true;
	placingZone->bFillMarginTopAtEast = true;
	placingZone->bFillMarginTopAtSouth = true;

	// �� ���� �ʱ�ȭ
	for (xx = 0 ; xx < 20 ; ++xx) {
		placingZone->cellsLT [xx].objType = NONE;
		placingZone->cellsLT [xx].leftBottomX = 0.0;
		placingZone->cellsLT [xx].leftBottomY = 0.0;
		placingZone->cellsLT [xx].leftBottomZ = 0.0;
		placingZone->cellsLT [xx].ang = 0.0;
		placingZone->cellsLT [xx].horLen = 0.0;
		placingZone->cellsLT [xx].verLen = 0.0;
		placingZone->cellsLT [xx].height = 0.0;

		placingZone->cellsRT [xx].objType = NONE;
		placingZone->cellsRT [xx].leftBottomX = 0.0;
		placingZone->cellsRT [xx].leftBottomY = 0.0;
		placingZone->cellsRT [xx].leftBottomZ = 0.0;
		placingZone->cellsRT [xx].ang = 0.0;
		placingZone->cellsRT [xx].horLen = 0.0;
		placingZone->cellsRT [xx].verLen = 0.0;
		placingZone->cellsRT [xx].height = 0.0;

		placingZone->cellsLB [xx].objType = NONE;
		placingZone->cellsLB [xx].leftBottomX = 0.0;
		placingZone->cellsLB [xx].leftBottomY = 0.0;
		placingZone->cellsLB [xx].leftBottomZ = 0.0;
		placingZone->cellsLB [xx].ang = 0.0;
		placingZone->cellsLB [xx].horLen = 0.0;
		placingZone->cellsLB [xx].verLen = 0.0;
		placingZone->cellsLB [xx].height = 0.0;

		placingZone->cellsRB [xx].objType = NONE;
		placingZone->cellsRB [xx].leftBottomX = 0.0;
		placingZone->cellsRB [xx].leftBottomY = 0.0;
		placingZone->cellsRB [xx].leftBottomZ = 0.0;
		placingZone->cellsRB [xx].ang = 0.0;
		placingZone->cellsRB [xx].horLen = 0.0;
		placingZone->cellsRB [xx].verLen = 0.0;
		placingZone->cellsRB [xx].height = 0.0;

		placingZone->cellsT1 [xx].objType = NONE;
		placingZone->cellsT1 [xx].leftBottomX = 0.0;
		placingZone->cellsT1 [xx].leftBottomY = 0.0;
		placingZone->cellsT1 [xx].leftBottomZ = 0.0;
		placingZone->cellsT1 [xx].ang = 0.0;
		placingZone->cellsT1 [xx].horLen = 0.0;
		placingZone->cellsT1 [xx].verLen = 0.0;
		placingZone->cellsT1 [xx].height = 0.0;

		placingZone->cellsT2 [xx].objType = NONE;
		placingZone->cellsT2 [xx].leftBottomX = 0.0;
		placingZone->cellsT2 [xx].leftBottomY = 0.0;
		placingZone->cellsT2 [xx].leftBottomZ = 0.0;
		placingZone->cellsT2 [xx].ang = 0.0;
		placingZone->cellsT2 [xx].horLen = 0.0;
		placingZone->cellsT2 [xx].verLen = 0.0;
		placingZone->cellsT2 [xx].height = 0.0;

		placingZone->cellsT3 [xx].objType = NONE;
		placingZone->cellsT3 [xx].leftBottomX = 0.0;
		placingZone->cellsT3 [xx].leftBottomY = 0.0;
		placingZone->cellsT3 [xx].leftBottomZ = 0.0;
		placingZone->cellsT3 [xx].ang = 0.0;
		placingZone->cellsT3 [xx].horLen = 0.0;
		placingZone->cellsT3 [xx].verLen = 0.0;
		placingZone->cellsT3 [xx].height = 0.0;

		placingZone->cellsL1 [xx].objType = NONE;
		placingZone->cellsL1 [xx].leftBottomX = 0.0;
		placingZone->cellsL1 [xx].leftBottomY = 0.0;
		placingZone->cellsL1 [xx].leftBottomZ = 0.0;
		placingZone->cellsL1 [xx].ang = 0.0;
		placingZone->cellsL1 [xx].horLen = 0.0;
		placingZone->cellsL1 [xx].verLen = 0.0;
		placingZone->cellsL1 [xx].height = 0.0;

		placingZone->cellsL2 [xx].objType = NONE;
		placingZone->cellsL2 [xx].leftBottomX = 0.0;
		placingZone->cellsL2 [xx].leftBottomY = 0.0;
		placingZone->cellsL2 [xx].leftBottomZ = 0.0;
		placingZone->cellsL2 [xx].ang = 0.0;
		placingZone->cellsL2 [xx].horLen = 0.0;
		placingZone->cellsL2 [xx].verLen = 0.0;
		placingZone->cellsL2 [xx].height = 0.0;

		placingZone->cellsL3 [xx].objType = NONE;
		placingZone->cellsL3 [xx].leftBottomX = 0.0;
		placingZone->cellsL3 [xx].leftBottomY = 0.0;
		placingZone->cellsL3 [xx].leftBottomZ = 0.0;
		placingZone->cellsL3 [xx].ang = 0.0;
		placingZone->cellsL3 [xx].horLen = 0.0;
		placingZone->cellsL3 [xx].verLen = 0.0;
		placingZone->cellsL3 [xx].height = 0.0;

		placingZone->cellsR1 [xx].objType = NONE;
		placingZone->cellsR1 [xx].leftBottomX = 0.0;
		placingZone->cellsR1 [xx].leftBottomY = 0.0;
		placingZone->cellsR1 [xx].leftBottomZ = 0.0;
		placingZone->cellsR1 [xx].ang = 0.0;
		placingZone->cellsR1 [xx].horLen = 0.0;
		placingZone->cellsR1 [xx].verLen = 0.0;
		placingZone->cellsR1 [xx].height = 0.0;

		placingZone->cellsR2 [xx].objType = NONE;
		placingZone->cellsR2 [xx].leftBottomX = 0.0;
		placingZone->cellsR2 [xx].leftBottomY = 0.0;
		placingZone->cellsR2 [xx].leftBottomZ = 0.0;
		placingZone->cellsR2 [xx].ang = 0.0;
		placingZone->cellsR2 [xx].horLen = 0.0;
		placingZone->cellsR2 [xx].verLen = 0.0;
		placingZone->cellsR2 [xx].height = 0.0;

		placingZone->cellsR3 [xx].objType = NONE;
		placingZone->cellsR3 [xx].leftBottomX = 0.0;
		placingZone->cellsR3 [xx].leftBottomY = 0.0;
		placingZone->cellsR3 [xx].leftBottomZ = 0.0;
		placingZone->cellsR3 [xx].ang = 0.0;
		placingZone->cellsR3 [xx].horLen = 0.0;
		placingZone->cellsR3 [xx].verLen = 0.0;
		placingZone->cellsR3 [xx].height = 0.0;

		placingZone->cellsB1 [xx].objType = NONE;
		placingZone->cellsB1 [xx].leftBottomX = 0.0;
		placingZone->cellsB1 [xx].leftBottomY = 0.0;
		placingZone->cellsB1 [xx].leftBottomZ = 0.0;
		placingZone->cellsB1 [xx].ang = 0.0;
		placingZone->cellsB1 [xx].horLen = 0.0;
		placingZone->cellsB1 [xx].verLen = 0.0;
		placingZone->cellsB1 [xx].height = 0.0;

		placingZone->cellsB2 [xx].objType = NONE;
		placingZone->cellsB2 [xx].leftBottomX = 0.0;
		placingZone->cellsB2 [xx].leftBottomY = 0.0;
		placingZone->cellsB2 [xx].leftBottomZ = 0.0;
		placingZone->cellsB2 [xx].ang = 0.0;
		placingZone->cellsB2 [xx].horLen = 0.0;
		placingZone->cellsB2 [xx].verLen = 0.0;
		placingZone->cellsB2 [xx].height = 0.0;

		placingZone->cellsB3 [xx].objType = NONE;
		placingZone->cellsB3 [xx].leftBottomX = 0.0;
		placingZone->cellsB3 [xx].leftBottomY = 0.0;
		placingZone->cellsB3 [xx].leftBottomZ = 0.0;
		placingZone->cellsB3 [xx].ang = 0.0;
		placingZone->cellsB3 [xx].horLen = 0.0;
		placingZone->cellsB3 [xx].verLen = 0.0;
		placingZone->cellsB3 [xx].height = 0.0;
	}

	// �� ���� �ʱ�ȭ
	placingZone->nCells = 0;
}

// ����⿡ �� �߰�
void	ColumnTableformPlacingZone::addTopCell (ColumnTableformPlacingZone* target_zone)
{
	if (target_zone->nCells >= 20) return;

	target_zone->cellsLT [target_zone->nCells] = target_zone->cellsLT [target_zone->nCells - 1];
	target_zone->cellsRT [target_zone->nCells] = target_zone->cellsRT [target_zone->nCells - 1];
	target_zone->cellsLB [target_zone->nCells] = target_zone->cellsLB [target_zone->nCells - 1];
	target_zone->cellsRB [target_zone->nCells] = target_zone->cellsRB [target_zone->nCells - 1];
	target_zone->cellsT1 [target_zone->nCells] = target_zone->cellsT1 [target_zone->nCells - 1];
	target_zone->cellsT2 [target_zone->nCells] = target_zone->cellsT2 [target_zone->nCells - 1];
	target_zone->cellsT3 [target_zone->nCells] = target_zone->cellsT3 [target_zone->nCells - 1];
	target_zone->cellsL1 [target_zone->nCells] = target_zone->cellsL1 [target_zone->nCells - 1];
	target_zone->cellsL2 [target_zone->nCells] = target_zone->cellsL2 [target_zone->nCells - 1];
	target_zone->cellsL3 [target_zone->nCells] = target_zone->cellsL3 [target_zone->nCells - 1];
	target_zone->cellsR1 [target_zone->nCells] = target_zone->cellsR1 [target_zone->nCells - 1];
	target_zone->cellsR2 [target_zone->nCells] = target_zone->cellsR2 [target_zone->nCells - 1];
	target_zone->cellsR3 [target_zone->nCells] = target_zone->cellsR3 [target_zone->nCells - 1];
	target_zone->cellsB1 [target_zone->nCells] = target_zone->cellsB1 [target_zone->nCells - 1];
	target_zone->cellsB2 [target_zone->nCells] = target_zone->cellsB2 [target_zone->nCells - 1];
	target_zone->cellsB3 [target_zone->nCells] = target_zone->cellsB3 [target_zone->nCells - 1];

	target_zone->nCells ++;
}

// ������� �� ����
void	ColumnTableformPlacingZone::delTopCell (ColumnTableformPlacingZone* target_zone)
{
	if (target_zone->nCells <= 1) return;

	target_zone->nCells --;
}

// Cell ������ ����ʿ� ���� ����ȭ�� ��ġ�� ��������
void	ColumnTableformPlacingZone::alignPlacingZone_soleColumn (ColumnTableformPlacingZone* placingZone)
{
	short	xx;
	double	formWidth, formHeight;
	
	// ���̸� �Ʒ����� �����ؼ� ������, �԰������� �ƴ��� ���θ� ����
	placingZone->cellsLT [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsRT [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsLB [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsRB [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsT1 [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsT2 [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsT3 [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsL1 [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsL2 [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsL3 [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsR1 [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsR2 [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsR3 [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsB1 [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsB2 [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsB3 [0].leftBottomZ = placingZone->bottomOffset;

	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {

		if (xx >= 1) {
			// �»��
			placingZone->cellsLT [xx].leftBottomZ = placingZone->cellsLT [xx-1].leftBottomZ + placingZone->cellsLT [xx-1].height;

			// ����
			placingZone->cellsRT [xx].leftBottomZ = placingZone->cellsRT [xx-1].leftBottomZ + placingZone->cellsRT [xx-1].height;

			// ���ϴ�
			placingZone->cellsLB [xx].leftBottomZ = placingZone->cellsLB [xx-1].leftBottomZ + placingZone->cellsLB [xx-1].height;

			// ���ϴ�
			placingZone->cellsRB [xx].leftBottomZ = placingZone->cellsRB [xx-1].leftBottomZ + placingZone->cellsRB [xx-1].height;

			// ���� 1
			placingZone->cellsT1 [xx].leftBottomZ = placingZone->cellsT1 [xx-1].leftBottomZ + placingZone->cellsT1 [xx-1].height;

			// ���� 2
			placingZone->cellsT2 [xx].leftBottomZ = placingZone->cellsT2 [xx-1].leftBottomZ + placingZone->cellsT2 [xx-1].height;

			// ���� 3
			placingZone->cellsT3 [xx].leftBottomZ = placingZone->cellsT3 [xx-1].leftBottomZ + placingZone->cellsT3 [xx-1].height;

			// ���� 1
			placingZone->cellsL1 [xx].leftBottomZ = placingZone->cellsL1 [xx-1].leftBottomZ + placingZone->cellsL1 [xx-1].height;

			// ���� 2
			placingZone->cellsL2 [xx].leftBottomZ = placingZone->cellsL2 [xx-1].leftBottomZ + placingZone->cellsL2 [xx-1].height;

			// ���� 3
			placingZone->cellsL3 [xx].leftBottomZ = placingZone->cellsL3 [xx-1].leftBottomZ + placingZone->cellsL3 [xx-1].height;

			// ���� 1
			placingZone->cellsR1 [xx].leftBottomZ = placingZone->cellsR1 [xx-1].leftBottomZ + placingZone->cellsR1 [xx-1].height;

			// ���� 2
			placingZone->cellsR2 [xx].leftBottomZ = placingZone->cellsR2 [xx-1].leftBottomZ + placingZone->cellsR2 [xx-1].height;

			// ���� 3
			placingZone->cellsR3 [xx].leftBottomZ = placingZone->cellsR3 [xx-1].leftBottomZ + placingZone->cellsR3 [xx-1].height;

			// �Ʒ��� 1
			placingZone->cellsB1 [xx].leftBottomZ = placingZone->cellsB1 [xx-1].leftBottomZ + placingZone->cellsB1 [xx-1].height;

			// �Ʒ��� 2
			placingZone->cellsB2 [xx].leftBottomZ = placingZone->cellsB2 [xx-1].leftBottomZ + placingZone->cellsB2 [xx-1].height;

			// �Ʒ��� 3
			placingZone->cellsB3 [xx].leftBottomZ = placingZone->cellsB3 [xx-1].leftBottomZ + placingZone->cellsB3 [xx-1].height;
		}

		// ���� 1
		formWidth = placingZone->cellsT1 [xx].horLen;
		formHeight = placingZone->cellsT1 [xx].height;
		placingZone->cellsT1 [xx].bStandardEuroform = false;
		if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
			if ( (abs (formHeight - 1.200) < EPS) || (abs (formHeight - 0.900) < EPS) || (abs (formHeight - 0.600) < EPS) )
				placingZone->cellsT1 [xx].bStandardEuroform = true;
		
		// ���� 2
		placingZone->cellsT2 [xx].bStandardEuroform = false;

		// ���� 3
		formWidth = placingZone->cellsT3 [xx].horLen;
		formHeight = placingZone->cellsT3 [xx].height;
		placingZone->cellsT3 [xx].bStandardEuroform = false;
		if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
			if ( (abs (formHeight - 1.200) < EPS) || (abs (formHeight - 0.900) < EPS) || (abs (formHeight - 0.600) < EPS) )
				placingZone->cellsT3 [xx].bStandardEuroform = true;

		// ���� 1
		formWidth = placingZone->cellsL1 [xx].verLen;
		formHeight = placingZone->cellsL1 [xx].height;
		placingZone->cellsL1 [xx].bStandardEuroform = false;
		if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
			if ( (abs (formHeight - 1.200) < EPS) || (abs (formHeight - 0.900) < EPS) || (abs (formHeight - 0.600) < EPS) )
				placingZone->cellsL1 [xx].bStandardEuroform = true;

		// ���� 2
		placingZone->cellsL2 [xx].bStandardEuroform = false;

		// ���� 3
		formWidth = placingZone->cellsL3 [xx].verLen;
		formHeight = placingZone->cellsL3 [xx].height;
		placingZone->cellsL3 [xx].bStandardEuroform = false;
		if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
			if ( (abs (formHeight - 1.200) < EPS) || (abs (formHeight - 0.900) < EPS) || (abs (formHeight - 0.600) < EPS) )
				placingZone->cellsL3 [xx].bStandardEuroform = true;

		// ���� 1
		formWidth = placingZone->cellsR1 [xx].verLen;
		formHeight = placingZone->cellsR1 [xx].height;
		placingZone->cellsR1 [xx].bStandardEuroform = false;
		if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
			if ( (abs (formHeight - 1.200) < EPS) || (abs (formHeight - 0.900) < EPS) || (abs (formHeight - 0.600) < EPS) )
				placingZone->cellsR1 [xx].bStandardEuroform = true;

		// ���� 2
		placingZone->cellsR2 [xx].bStandardEuroform = false;

		// ���� 3
		formWidth = placingZone->cellsR3 [xx].verLen;
		formHeight = placingZone->cellsR3 [xx].height;
		placingZone->cellsR3 [xx].bStandardEuroform = false;
		if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
			if ( (abs (formHeight - 1.200) < EPS) || (abs (formHeight - 0.900) < EPS) || (abs (formHeight - 0.600) < EPS) )
				placingZone->cellsR3 [xx].bStandardEuroform = true;

		// �Ʒ��� 1
		formWidth = placingZone->cellsB1 [xx].horLen;
		formHeight = placingZone->cellsB1 [xx].height;
		placingZone->cellsB1 [xx].bStandardEuroform = false;
		if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
			if ( (abs (formHeight - 1.200) < EPS) || (abs (formHeight - 0.900) < EPS) || (abs (formHeight - 0.600) < EPS) )
				placingZone->cellsB1 [xx].bStandardEuroform = true;

		// �Ʒ��� 2
		placingZone->cellsB2 [xx].bStandardEuroform = false;

		// �Ʒ��� 3
		formWidth = placingZone->cellsB3 [xx].horLen;
		formHeight = placingZone->cellsB3 [xx].height;
		placingZone->cellsB3 [xx].bStandardEuroform = false;
		if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
			if ( (abs (formHeight - 1.200) < EPS) || (abs (formHeight - 0.900) < EPS) || (abs (formHeight - 0.600) < EPS) )
				placingZone->cellsB3 [xx].bStandardEuroform = true;
	}
}

// ������/�ƿ��ڳ��ǳ�/�ƿ��ڳʾޱ�/�ٷ������̼�/���Ŭ���� ��ġ
GSErrCode	ColumnTableformPlacingZone::placeBasicObjects_soleColumn (ColumnTableformPlacingZone* placingZone)
{
	GSErrCode	err = NoError;
	short	xx;
	EasyObjectPlacement	euroform, outpanel, outangle, fillersp, blueClamp;

	double	accumDist;
	double	remainLength;
	double	length;

	// �ٷ������̼� ��ġ (���� 2)
	fillersp.init (L("�ٷ������̼�v1.0.gsm"), layerInd_Fillersp, infoColumn.floorInd, placingZone->cellsT2 [0].leftBottomX, placingZone->cellsT2 [0].leftBottomY, placingZone->cellsT2 [0].leftBottomZ, placingZone->cellsT2 [0].ang);
	if ((placingZone->cellsT2 [0].horLen > EPS) && (placingZone->cellsT2 [0].height > EPS)) {
		accumDist = 0.0;
		remainLength = 0.0;
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			remainLength += placingZone->cellsT2 [xx].height;
			accumDist += placingZone->cellsT2 [xx].height;
		}

		moveIn3D ('x', fillersp.radAng, placingZone->cellsT2 [0].horLen, &fillersp.posX, &fillersp.posY, &fillersp.posZ);
		while (remainLength > EPS) {
			if (remainLength >= 2.400)
				length = 2.400;
			else
				length = remainLength;

			elemList.Push (fillersp.placeObject (4,
				"f_thk", APIParT_Length, format_string ("%f", placingZone->cellsT2 [0].horLen),
				"f_leng", APIParT_Length, format_string ("%f", remainLength),
				"f_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
				"f_rota", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str ()));
			moveIn3D ('z', fillersp.radAng, length, &fillersp.posX, &fillersp.posY, &fillersp.posZ);

			remainLength -= 2.400;
		}
		moveIn3D ('x', fillersp.radAng, -placingZone->cellsT2 [0].horLen, &fillersp.posX, &fillersp.posY, &fillersp.posZ);
		moveIn3D ('z', fillersp.radAng, -accumDist, &fillersp.posX, &fillersp.posY, &fillersp.posZ);
	}

	// �ٷ������̼� ��ġ (�Ʒ��� 2)
	fillersp.init (L("�ٷ������̼�v1.0.gsm"), layerInd_Fillersp, infoColumn.floorInd, placingZone->cellsB2 [0].leftBottomX, placingZone->cellsB2 [0].leftBottomY, placingZone->cellsB2 [0].leftBottomZ, placingZone->cellsB2 [0].ang);
	if ((placingZone->cellsB2 [0].horLen > EPS) && (placingZone->cellsB2 [0].height > EPS)) {
		accumDist = 0.0;
		remainLength = 0.0;
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			remainLength += placingZone->cellsB2 [xx].height;
			accumDist += placingZone->cellsB2 [xx].height;
		}

		moveIn3D ('x', fillersp.radAng, placingZone->cellsB2 [0].horLen, &fillersp.posX, &fillersp.posY, &fillersp.posZ);
		while (remainLength > EPS) {
			if (remainLength >= 2.400)
				length = 2.400;
			else
				length = remainLength;

			elemList.Push (fillersp.placeObject (4,
				"f_thk", APIParT_Length, format_string ("%f", placingZone->cellsB2 [0].horLen),
				"f_leng", APIParT_Length, format_string ("%f", remainLength),
				"f_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
				"f_rota", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str ()));
			moveIn3D ('z', fillersp.radAng, length, &fillersp.posX, &fillersp.posY, &fillersp.posZ);

			remainLength -= 2.400;
		}
		moveIn3D ('x', fillersp.radAng, -placingZone->cellsB2 [0].horLen, &fillersp.posX, &fillersp.posY, &fillersp.posZ);
		moveIn3D ('z', fillersp.radAng, -accumDist, &fillersp.posX, &fillersp.posY, &fillersp.posZ);
	}

	// �ٷ������̼� ��ġ (���� 2)
	fillersp.init (L("�ٷ������̼�v1.0.gsm"), layerInd_Fillersp, infoColumn.floorInd, placingZone->cellsL2 [0].leftBottomX, placingZone->cellsL2 [0].leftBottomY, placingZone->cellsL2 [0].leftBottomZ, placingZone->cellsL2 [0].ang);
	if ((placingZone->cellsL2 [0].verLen > EPS) && (placingZone->cellsL2 [0].height > EPS)) {
		accumDist = 0.0;
		remainLength = 0.0;
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			remainLength += placingZone->cellsL2 [xx].height;
			accumDist += placingZone->cellsL2 [xx].height;
		}

		moveIn3D ('x', fillersp.radAng, placingZone->cellsL2 [0].verLen, &fillersp.posX, &fillersp.posY, &fillersp.posZ);
		while (remainLength > EPS) {
			if (remainLength >= 2.400)
				length = 2.400;
			else
				length = remainLength;

			elemList.Push (fillersp.placeObject (4,
				"f_thk", APIParT_Length, format_string ("%f", placingZone->cellsL2 [0].verLen),
				"f_leng", APIParT_Length, format_string ("%f", remainLength),
				"f_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
				"f_rota", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str ()));
			moveIn3D ('z', fillersp.radAng, length, &fillersp.posX, &fillersp.posY, &fillersp.posZ);

			remainLength -= 2.400;
		}
		moveIn3D ('x', fillersp.radAng, -placingZone->cellsL2 [0].verLen, &fillersp.posX, &fillersp.posY, &fillersp.posZ);
		moveIn3D ('z', fillersp.radAng, -accumDist, &fillersp.posX, &fillersp.posY, &fillersp.posZ);
	}

	// �ٷ������̼� ��ġ (���� 2)
	fillersp.init (L("�ٷ������̼�v1.0.gsm"), layerInd_Fillersp, infoColumn.floorInd, placingZone->cellsR2 [0].leftBottomX, placingZone->cellsR2 [0].leftBottomY, placingZone->cellsR2 [0].leftBottomZ, placingZone->cellsR2 [0].ang);
	if ((placingZone->cellsR2 [0].verLen > EPS) && (placingZone->cellsR2 [0].height > EPS)) {
		accumDist = 0.0;
		remainLength = 0.0;
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			remainLength += placingZone->cellsR2 [xx].height;
			accumDist += placingZone->cellsR2 [xx].height;
		}

		moveIn3D ('x', fillersp.radAng, placingZone->cellsR2 [0].verLen, &fillersp.posX, &fillersp.posY, &fillersp.posZ);
		while (remainLength > EPS) {
			if (remainLength >= 2.400)
				length = 2.400;
			else
				length = remainLength;

			elemList.Push (fillersp.placeObject (4,
				"f_thk", APIParT_Length, format_string ("%f", placingZone->cellsR2 [0].verLen),
				"f_leng", APIParT_Length, format_string ("%f", remainLength),
				"f_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
				"f_rota", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str ()));
			moveIn3D ('z', fillersp.radAng, length, &fillersp.posX, &fillersp.posY, &fillersp.posZ);

			remainLength -= 2.400;
		}
		moveIn3D ('x', fillersp.radAng, -placingZone->cellsR2 [0].verLen, &fillersp.posX, &fillersp.posY, &fillersp.posZ);
		moveIn3D ('z', fillersp.radAng, -accumDist, &fillersp.posX, &fillersp.posY, &fillersp.posZ);
	}

	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		// 1. ������ ��ġ
		// ���� 1
		euroform.init (L("������v2.0.gsm"), layerInd_Euroform, infoColumn.floorInd, placingZone->cellsT1 [xx].leftBottomX, placingZone->cellsT1 [xx].leftBottomY, placingZone->cellsT1 [xx].leftBottomZ, placingZone->cellsT1 [xx].ang);
		if ((placingZone->cellsT1 [xx].horLen > EPS) && (placingZone->cellsT1 [xx].height > EPS)) {
			if (placingZone->cellsT1 [xx].bStandardEuroform == true) {
				elemList.Push (euroform.placeObject (6,
					"eu_stan_onoff", APIParT_Boolean, "1.0",
					"eu_wid", APIParT_CString, format_string ("%.0f", placingZone->cellsT1 [xx].horLen * 1000),
					"eu_hei", APIParT_CString, format_string ("%.0f", placingZone->cellsT1 [xx].height * 1000),
					"u_ins", APIParT_CString, "�������",
					"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
					"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			} else {
				elemList.Push (euroform.placeObject (6,
					"eu_stan_onoff", APIParT_Boolean, "0.0",
					"eu_wid2", APIParT_Length, format_string ("%f", placingZone->cellsT1 [xx].horLen),
					"eu_hei2", APIParT_Length, format_string ("%f", placingZone->cellsT1 [xx].height),
					"u_ins", APIParT_CString, "�������",
					"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
					"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			}
		}

		// ���� 3
		euroform.init (L("������v2.0.gsm"), layerInd_Euroform, infoColumn.floorInd, placingZone->cellsT3 [xx].leftBottomX, placingZone->cellsT3 [xx].leftBottomY, placingZone->cellsT3 [xx].leftBottomZ, placingZone->cellsT3 [xx].ang);
		if ((placingZone->cellsT3 [xx].horLen > EPS) && (placingZone->cellsT3 [xx].height > EPS)) {
			if (placingZone->cellsT3 [xx].bStandardEuroform == true) {
				elemList.Push (euroform.placeObject (6,
					"eu_stan_onoff", APIParT_Boolean, "1.0",
					"eu_wid", APIParT_CString, format_string ("%.0f", placingZone->cellsT3 [xx].horLen * 1000),
					"eu_hei", APIParT_CString, format_string ("%.0f", placingZone->cellsT3 [xx].height * 1000),
					"u_ins", APIParT_CString, "�������",
					"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
					"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			} else {
				elemList.Push (euroform.placeObject (6,
					"eu_stan_onoff", APIParT_Boolean, "0.0",
					"eu_wid2", APIParT_Length, format_string ("%f", placingZone->cellsT3 [xx].horLen),
					"eu_hei2", APIParT_Length, format_string ("%f", placingZone->cellsT3 [xx].height),
					"u_ins", APIParT_CString, "�������",
					"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
					"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			}
		}

		// �Ʒ��� 1
		euroform.init (L("������v2.0.gsm"), layerInd_Euroform, infoColumn.floorInd, placingZone->cellsB1 [xx].leftBottomX, placingZone->cellsB1 [xx].leftBottomY, placingZone->cellsB1 [xx].leftBottomZ, placingZone->cellsB1 [xx].ang);
		if ((placingZone->cellsB1 [xx].horLen > EPS) && (placingZone->cellsB1 [xx].height > EPS)) {
			if (placingZone->cellsB1 [xx].bStandardEuroform == true) {
				elemList.Push (euroform.placeObject (6,
					"eu_stan_onoff", APIParT_Boolean, "1.0",
					"eu_wid", APIParT_CString, format_string ("%.0f", placingZone->cellsB1 [xx].horLen * 1000),
					"eu_hei", APIParT_CString, format_string ("%.0f", placingZone->cellsB1 [xx].height * 1000),
					"u_ins", APIParT_CString, "�������",
					"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
					"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			} else {
				elemList.Push (euroform.placeObject (6,
					"eu_stan_onoff", APIParT_Boolean, "0.0",
					"eu_wid2", APIParT_Length, format_string ("%f", placingZone->cellsB1 [xx].horLen),
					"eu_hei2", APIParT_Length, format_string ("%f", placingZone->cellsB1 [xx].height),
					"u_ins", APIParT_CString, "�������",
					"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
					"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			}
		}

		// �Ʒ��� 3
		euroform.init (L("������v2.0.gsm"), layerInd_Euroform, infoColumn.floorInd, placingZone->cellsB3 [xx].leftBottomX, placingZone->cellsB3 [xx].leftBottomY, placingZone->cellsB3 [xx].leftBottomZ, placingZone->cellsB3 [xx].ang);
		if ((placingZone->cellsB3 [xx].horLen > EPS) && (placingZone->cellsB3 [xx].height > EPS)) {
			if (placingZone->cellsB3 [xx].bStandardEuroform == true) {
				elemList.Push (euroform.placeObject (6,
					"eu_stan_onoff", APIParT_Boolean, "1.0",
					"eu_wid", APIParT_CString, format_string ("%.0f", placingZone->cellsB3 [xx].horLen * 1000),
					"eu_hei", APIParT_CString, format_string ("%.0f", placingZone->cellsB3 [xx].height * 1000),
					"u_ins", APIParT_CString, "�������",
					"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
					"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			} else {
				elemList.Push (euroform.placeObject (6,
					"eu_stan_onoff", APIParT_Boolean, "0.0",
					"eu_wid2", APIParT_Length, format_string ("%f", placingZone->cellsB3 [xx].horLen),
					"eu_hei2", APIParT_Length, format_string ("%f", placingZone->cellsB3 [xx].height),
					"u_ins", APIParT_CString, "�������",
					"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
					"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			}
		}

		// ���� 1
		euroform.init (L("������v2.0.gsm"), layerInd_Euroform, infoColumn.floorInd, placingZone->cellsL1 [xx].leftBottomX, placingZone->cellsL1 [xx].leftBottomY, placingZone->cellsL1 [xx].leftBottomZ, placingZone->cellsL1 [xx].ang);
		if ((placingZone->cellsL1 [xx].verLen > EPS) && (placingZone->cellsL1 [xx].height > EPS)) {
			if (placingZone->cellsL1 [xx].bStandardEuroform == true) {
				elemList.Push (euroform.placeObject (6,
					"eu_stan_onoff", APIParT_Boolean, "1.0",
					"eu_wid", APIParT_CString, format_string ("%.0f", placingZone->cellsL1 [xx].verLen * 1000),
					"eu_hei", APIParT_CString, format_string ("%.0f", placingZone->cellsL1 [xx].height * 1000),
					"u_ins", APIParT_CString, "�������",
					"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
					"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			} else {
				elemList.Push (euroform.placeObject (6,
					"eu_stan_onoff", APIParT_Boolean, "0.0",
					"eu_wid2", APIParT_Length, format_string ("%f", placingZone->cellsL1 [xx].verLen),
					"eu_hei2", APIParT_Length, format_string ("%f", placingZone->cellsL1 [xx].height),
					"u_ins", APIParT_CString, "�������",
					"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
					"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			}
		}

		// ���� 3
		euroform.init (L("������v2.0.gsm"), layerInd_Euroform, infoColumn.floorInd, placingZone->cellsL3 [xx].leftBottomX, placingZone->cellsL3 [xx].leftBottomY, placingZone->cellsL3 [xx].leftBottomZ, placingZone->cellsL3 [xx].ang);
		if ((placingZone->cellsL3 [xx].verLen > EPS) && (placingZone->cellsL3 [xx].height > EPS)) {
			if (placingZone->cellsL3 [xx].bStandardEuroform == true) {
				elemList.Push (euroform.placeObject (6,
					"eu_stan_onoff", APIParT_Boolean, "1.0",
					"eu_wid", APIParT_CString, format_string ("%.0f", placingZone->cellsL3 [xx].verLen * 1000),
					"eu_hei", APIParT_CString, format_string ("%.0f", placingZone->cellsL3 [xx].height * 1000),
					"u_ins", APIParT_CString, "�������",
					"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
					"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			} else {
				elemList.Push (euroform.placeObject (6,
					"eu_stan_onoff", APIParT_Boolean, "0.0",
					"eu_wid2", APIParT_Length, format_string ("%f", placingZone->cellsL3 [xx].verLen),
					"eu_hei2", APIParT_Length, format_string ("%f", placingZone->cellsL3 [xx].height),
					"u_ins", APIParT_CString, "�������",
					"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
					"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			}
		}

		// ���� 1
		euroform.init (L("������v2.0.gsm"), layerInd_Euroform, infoColumn.floorInd, placingZone->cellsR1 [xx].leftBottomX, placingZone->cellsR1 [xx].leftBottomY, placingZone->cellsR1 [xx].leftBottomZ, placingZone->cellsR1 [xx].ang);
		if ((placingZone->cellsR1 [xx].verLen > EPS) && (placingZone->cellsR1 [xx].height > EPS)) {
			if (placingZone->cellsR1 [xx].bStandardEuroform == true) {
				elemList.Push (euroform.placeObject (6,
					"eu_stan_onoff", APIParT_Boolean, "1.0",
					"eu_wid", APIParT_CString, format_string ("%.0f", placingZone->cellsR1 [xx].verLen * 1000),
					"eu_hei", APIParT_CString, format_string ("%.0f", placingZone->cellsR1 [xx].height * 1000),
					"u_ins", APIParT_CString, "�������",
					"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
					"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			} else {
				elemList.Push (euroform.placeObject (6,
					"eu_stan_onoff", APIParT_Boolean, "0.0",
					"eu_wid2", APIParT_Length, format_string ("%f", placingZone->cellsR1 [xx].verLen),
					"eu_hei2", APIParT_Length, format_string ("%f", placingZone->cellsR1 [xx].height),
					"u_ins", APIParT_CString, "�������",
					"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
					"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			}
		}

		// ���� 3
		euroform.init (L("������v2.0.gsm"), layerInd_Euroform, infoColumn.floorInd, placingZone->cellsR3 [xx].leftBottomX, placingZone->cellsR3 [xx].leftBottomY, placingZone->cellsR3 [xx].leftBottomZ, placingZone->cellsR3 [xx].ang);
		if ((placingZone->cellsR3 [xx].verLen > EPS) && (placingZone->cellsR3 [xx].height > EPS)) {
			if (placingZone->cellsR3 [xx].bStandardEuroform == true) {
				elemList.Push (euroform.placeObject (6,
					"eu_stan_onoff", APIParT_Boolean, "1.0",
					"eu_wid", APIParT_CString, format_string ("%.0f", placingZone->cellsR3 [xx].verLen * 1000),
					"eu_hei", APIParT_CString, format_string ("%.0f", placingZone->cellsR3 [xx].height * 1000),
					"u_ins", APIParT_CString, "�������",
					"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
					"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			} else {
				elemList.Push (euroform.placeObject (6,
					"eu_stan_onoff", APIParT_Boolean, "0.0",
					"eu_wid2", APIParT_Length, format_string ("%f", placingZone->cellsR3 [xx].verLen),
					"eu_hei2", APIParT_Length, format_string ("%f", placingZone->cellsR3 [xx].height),
					"u_ins", APIParT_CString, "�������",
					"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
					"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			}
		}


		// 2. �ƿ��ڳ��ǳ� ��ġ
		// �»��
		outpanel.init (L("�ƿ��ڳ��ǳ�v1.0.gsm"), layerInd_OutPanel, infoColumn.floorInd, placingZone->cellsLT [xx].leftBottomX, placingZone->cellsLT [xx].leftBottomY, placingZone->cellsLT [xx].leftBottomZ, placingZone->cellsLT [xx].ang);
		if ((placingZone->cellsLT [xx].objType == OUTPANEL) && (placingZone->cellsLT [xx].verLen > EPS) && (placingZone->cellsLT [xx].horLen > EPS) && (placingZone->cellsLT [xx].height > EPS))
			elemList.Push (outpanel.placeObject (4,
				"wid_s", APIParT_Length, format_string ("%f", placingZone->cellsLT [xx].verLen),
				"leng_s", APIParT_Length, format_string ("%f", placingZone->cellsLT [xx].horLen),
				"hei_s", APIParT_Length, format_string ("%f", placingZone->cellsLT [xx].height),
				"dir_s", APIParT_CString, "�����"));

		// ����
		outpanel.init (L("�ƿ��ڳ��ǳ�v1.0.gsm"), layerInd_OutPanel, infoColumn.floorInd, placingZone->cellsRT [xx].leftBottomX, placingZone->cellsRT [xx].leftBottomY, placingZone->cellsRT [xx].leftBottomZ, placingZone->cellsRT [xx].ang);
		if ((placingZone->cellsRT [xx].objType == OUTPANEL) && (placingZone->cellsRT [xx].verLen > EPS) && (placingZone->cellsRT [xx].horLen > EPS) && (placingZone->cellsRT [xx].height > EPS))
			elemList.Push (outpanel.placeObject (4,
				"wid_s", APIParT_Length, format_string ("%f", placingZone->cellsRT [xx].horLen),
				"leng_s", APIParT_Length, format_string ("%f", placingZone->cellsRT [xx].verLen),
				"hei_s", APIParT_Length, format_string ("%f", placingZone->cellsRT [xx].height),
				"dir_s", APIParT_CString, "�����"));

		// ���ϴ�
		outpanel.init (L("�ƿ��ڳ��ǳ�v1.0.gsm"), layerInd_OutPanel, infoColumn.floorInd, placingZone->cellsLB [xx].leftBottomX, placingZone->cellsLB [xx].leftBottomY, placingZone->cellsLB [xx].leftBottomZ, placingZone->cellsLB [xx].ang);
		if ((placingZone->cellsLB [xx].objType == OUTPANEL) && (placingZone->cellsLB [xx].verLen > EPS) && (placingZone->cellsLB [xx].horLen > EPS) && (placingZone->cellsLB [xx].height > EPS))
			elemList.Push (outpanel.placeObject (4,
				"wid_s", APIParT_Length, format_string ("%f", placingZone->cellsLB [xx].horLen),
				"leng_s", APIParT_Length, format_string ("%f", placingZone->cellsLB [xx].verLen),
				"hei_s", APIParT_Length, format_string ("%f", placingZone->cellsLB [xx].height),
				"dir_s", APIParT_CString, "�����"));

		// ���ϴ�
		outpanel.init (L("�ƿ��ڳ��ǳ�v1.0.gsm"), layerInd_OutPanel, infoColumn.floorInd, placingZone->cellsRB [xx].leftBottomX, placingZone->cellsRB [xx].leftBottomY, placingZone->cellsRB [xx].leftBottomZ, placingZone->cellsRB [xx].ang);
		if ((placingZone->cellsRB [xx].objType == OUTPANEL) && (placingZone->cellsRB [xx].verLen > EPS) && (placingZone->cellsRB [xx].horLen > EPS) && (placingZone->cellsRB [xx].height > EPS))
			elemList.Push (outpanel.placeObject (4,
				"wid_s", APIParT_Length, format_string ("%f", placingZone->cellsRB [xx].verLen),
				"leng_s", APIParT_Length, format_string ("%f", placingZone->cellsRB [xx].horLen),
				"hei_s", APIParT_Length, format_string ("%f", placingZone->cellsRB [xx].height),
				"dir_s", APIParT_CString, "�����"));


		// 3. �ƿ��ڳʾޱ� ��ġ
		// �»��
		outangle.init (L("�ƿ��ڳʾޱ�v1.0.gsm"), layerInd_OutAngle, infoColumn.floorInd, placingZone->cellsLT [xx].leftBottomX, placingZone->cellsLT [xx].leftBottomY, placingZone->cellsLT [xx].leftBottomZ, placingZone->cellsLT [xx].ang);
		if ((placingZone->cellsLT [xx].objType == OUTANGLE) && (placingZone->cellsLT [xx].height > EPS))
			elemList.Push (outangle.placeObject (2,
				"a_leng", APIParT_Length, format_string ("%f", placingZone->cellsLT [xx].height),
				"a_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));

		// ����
		outangle.init (L("�ƿ��ڳʾޱ�v1.0.gsm"), layerInd_OutAngle, infoColumn.floorInd, placingZone->cellsRT [xx].leftBottomX, placingZone->cellsRT [xx].leftBottomY, placingZone->cellsRT [xx].leftBottomZ, placingZone->cellsRT [xx].ang);
		if ((placingZone->cellsRT [xx].objType == OUTANGLE) && (placingZone->cellsRT [xx].height > EPS))
			elemList.Push (outangle.placeObject (2,
				"a_leng", APIParT_Length, format_string ("%f", placingZone->cellsRT [xx].height),
				"a_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));

		// ���ϴ�
		outangle.init (L("�ƿ��ڳʾޱ�v1.0.gsm"), layerInd_OutAngle, infoColumn.floorInd, placingZone->cellsLB [xx].leftBottomX, placingZone->cellsLB [xx].leftBottomY, placingZone->cellsLB [xx].leftBottomZ, placingZone->cellsLB [xx].ang);
		if ((placingZone->cellsLB [xx].objType == OUTANGLE) && (placingZone->cellsLB [xx].height > EPS))
			elemList.Push (outangle.placeObject (2,
				"a_leng", APIParT_Length, format_string ("%f", placingZone->cellsLB [xx].height),
				"a_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));

		// ���ϴ�
		outangle.init (L("�ƿ��ڳʾޱ�v1.0.gsm"), layerInd_OutAngle, infoColumn.floorInd, placingZone->cellsRB [xx].leftBottomX, placingZone->cellsRB [xx].leftBottomY, placingZone->cellsRB [xx].leftBottomZ, placingZone->cellsRB [xx].ang);
		if ((placingZone->cellsRB [xx].objType == OUTANGLE) && (placingZone->cellsRB [xx].height > EPS))
			elemList.Push (outangle.placeObject (2,
				"a_leng", APIParT_Length, format_string ("%f", placingZone->cellsRB [xx].height),
				"a_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
	}

	// ���Ŭ���� (���� 1)
	blueClamp.init (L("���Ŭ����v1.0.gsm"), layerInd_BlueClamp, infoColumn.floorInd, placingZone->cellsT1 [0].leftBottomX, placingZone->cellsT1 [0].leftBottomY, placingZone->cellsT1 [0].leftBottomZ, placingZone->cellsT1 [0].ang);
	if ((placingZone->cellsT1 [0].horLen > EPS) && (placingZone->cellsT1 [0].height > EPS)) {
		if (placingZone->marginTopAtNorth > EPS) {
			double formHeight = 0.0;
			for (xx = 0 ; xx < placingZone->nCells ; ++xx)
				formHeight += placingZone->cellsT1 [xx].height;

			moveIn3D ('z', blueClamp.radAng, formHeight + 0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('x', blueClamp.radAng, 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('y', blueClamp.radAng, -0.066, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			elemList.Push (blueClamp.placeObject (5,
				"type", APIParT_CString, "���θ���Ŭ����(����ǰv1)",
				"bReverseRotation", APIParT_Boolean, "1.0",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
				"openingWidth", APIParT_Length, "0.047"));
		}
	}

	// ���Ŭ���� (���� 3)
	blueClamp.init (L("���Ŭ����v1.0.gsm"), layerInd_BlueClamp, infoColumn.floorInd, placingZone->cellsT3 [0].leftBottomX, placingZone->cellsT3 [0].leftBottomY, placingZone->cellsT3 [0].leftBottomZ, placingZone->cellsT3 [0].ang);
	if ((placingZone->cellsT3 [0].horLen > EPS) && (placingZone->cellsT3 [0].height > EPS)) {
		if (placingZone->marginTopAtNorth > EPS) {
			double formHeight = 0.0;
			for (xx = 0 ; xx < placingZone->nCells ; ++xx)
				formHeight += placingZone->cellsT3 [xx].height;

			moveIn3D ('z', blueClamp.radAng, formHeight + 0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('x', blueClamp.radAng, placingZone->cellsT3 [0].horLen - 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('y', blueClamp.radAng, -0.066, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			elemList.Push (blueClamp.placeObject (5,
				"type", APIParT_CString, "���θ���Ŭ����(����ǰv1)",
				"bReverseRotation", APIParT_Boolean, "1.0",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
				"openingWidth", APIParT_Length, "0.047"));
		}
	}

	// ���Ŭ���� (�Ʒ��� 1)
	blueClamp.init (L("���Ŭ����v1.0.gsm"), layerInd_BlueClamp, infoColumn.floorInd, placingZone->cellsB1 [0].leftBottomX, placingZone->cellsB1 [0].leftBottomY, placingZone->cellsB1 [0].leftBottomZ, placingZone->cellsB1 [0].ang);
	if ((placingZone->cellsB1 [0].horLen > EPS) && (placingZone->cellsB1 [0].height > EPS)) {
		if (placingZone->marginTopAtSouth > EPS) {
			double formHeight = 0.0;
			for (xx = 0 ; xx < placingZone->nCells ; ++xx)
				formHeight += placingZone->cellsB1 [xx].height;

			moveIn3D ('z', blueClamp.radAng, formHeight + 0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('x', blueClamp.radAng, placingZone->cellsB1 [0].horLen - 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('y', blueClamp.radAng, -0.066, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			elemList.Push (blueClamp.placeObject (5,
				"type", APIParT_CString, "���θ���Ŭ����(����ǰv1)",
				"bReverseRotation", APIParT_Boolean, "1.0",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
				"openingWidth", APIParT_Length, "0.047"));
		}
	}

	// ���Ŭ���� (�Ʒ��� 3)
	blueClamp.init (L("���Ŭ����v1.0.gsm"), layerInd_BlueClamp, infoColumn.floorInd, placingZone->cellsB3 [0].leftBottomX, placingZone->cellsB3 [0].leftBottomY, placingZone->cellsB3 [0].leftBottomZ, placingZone->cellsB3 [0].ang);
	if ((placingZone->cellsB3 [0].horLen > EPS) && (placingZone->cellsB3 [0].height > EPS)) {
		if (placingZone->marginTopAtSouth > EPS) {
			double formHeight = 0.0;
			for (xx = 0 ; xx < placingZone->nCells ; ++xx)
				formHeight += placingZone->cellsB3 [xx].height;

			moveIn3D ('z', blueClamp.radAng, formHeight + 0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('x', blueClamp.radAng, 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('y', blueClamp.radAng, -0.066, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			elemList.Push (blueClamp.placeObject (5,
				"type", APIParT_CString, "���θ���Ŭ����(����ǰv1)",
				"bReverseRotation", APIParT_Boolean, "1.0",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
				"openingWidth", APIParT_Length, "0.047"));
		}
	}

	// ���Ŭ���� (���� 1)
	blueClamp.init (L("���Ŭ����v1.0.gsm"), layerInd_BlueClamp, infoColumn.floorInd, placingZone->cellsL1 [0].leftBottomX, placingZone->cellsL1 [0].leftBottomY, placingZone->cellsL1 [0].leftBottomZ, placingZone->cellsL1 [0].ang);
	if ((placingZone->cellsL1 [0].verLen > EPS) && (placingZone->cellsL1 [0].height > EPS)) {
		if (placingZone->marginTopAtWest > EPS) {
			double formHeight = 0.0;
			for (xx = 0 ; xx < placingZone->nCells ; ++xx)
				formHeight += placingZone->cellsL1 [xx].height;

			moveIn3D ('z', blueClamp.radAng, formHeight + 0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('x', blueClamp.radAng, placingZone->cellsL1 [0].verLen - 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('y', blueClamp.radAng, -0.066, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			elemList.Push (blueClamp.placeObject (5,
				"type", APIParT_CString, "���θ���Ŭ����(����ǰv1)",
				"bReverseRotation", APIParT_Boolean, "1.0",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
				"openingWidth", APIParT_Length, "0.047"));
		}
	}

	// ���Ŭ���� (���� 3)
	blueClamp.init (L("���Ŭ����v1.0.gsm"), layerInd_BlueClamp, infoColumn.floorInd, placingZone->cellsL3 [0].leftBottomX, placingZone->cellsL3 [0].leftBottomY, placingZone->cellsL3 [0].leftBottomZ, placingZone->cellsL3 [0].ang);
	if ((placingZone->cellsL3 [0].verLen > EPS) && (placingZone->cellsL3 [0].height > EPS)) {
		if (placingZone->marginTopAtWest > EPS) {
			double formHeight = 0.0;
			for (xx = 0 ; xx < placingZone->nCells ; ++xx)
				formHeight += placingZone->cellsL3 [xx].height;

			moveIn3D ('z', blueClamp.radAng, formHeight + 0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('x', blueClamp.radAng, 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('y', blueClamp.radAng, -0.066, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			elemList.Push (blueClamp.placeObject (5,
				"type", APIParT_CString, "���θ���Ŭ����(����ǰv1)",
				"bReverseRotation", APIParT_Boolean, "1.0",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
				"openingWidth", APIParT_Length, "0.047"));
		}
	}

	// ���Ŭ���� (���� 1)
	blueClamp.init (L("���Ŭ����v1.0.gsm"), layerInd_BlueClamp, infoColumn.floorInd, placingZone->cellsR1 [0].leftBottomX, placingZone->cellsR1 [0].leftBottomY, placingZone->cellsR1 [0].leftBottomZ, placingZone->cellsR1 [0].ang);
	if ((placingZone->cellsR1 [0].verLen > EPS) && (placingZone->cellsR1 [0].height > EPS)) {
		if (placingZone->marginTopAtEast > EPS) {
			double formHeight = 0.0;
			for (xx = 0 ; xx < placingZone->nCells ; ++xx)
				formHeight += placingZone->cellsR1 [xx].height;

			moveIn3D ('z', blueClamp.radAng, formHeight + 0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('x', blueClamp.radAng, 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('y', blueClamp.radAng, -0.066, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			elemList.Push (blueClamp.placeObject (5,
				"type", APIParT_CString, "���θ���Ŭ����(����ǰv1)",
				"bReverseRotation", APIParT_Boolean, "1.0",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
				"openingWidth", APIParT_Length, "0.047"));
		}
	}

	// ���Ŭ���� (���� 3)
	blueClamp.init (L("���Ŭ����v1.0.gsm"), layerInd_BlueClamp, infoColumn.floorInd, placingZone->cellsR3 [0].leftBottomX, placingZone->cellsR3 [0].leftBottomY, placingZone->cellsR3 [0].leftBottomZ, placingZone->cellsR3 [0].ang);
	if ((placingZone->cellsR3 [0].verLen > EPS) && (placingZone->cellsR3 [0].height > EPS)) {
		if (placingZone->marginTopAtEast > EPS) {
			double formHeight = 0.0;
			for (xx = 0 ; xx < placingZone->nCells ; ++xx)
				formHeight += placingZone->cellsR3 [xx].height;

			moveIn3D ('z', blueClamp.radAng, formHeight + 0.040, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('x', blueClamp.radAng, placingZone->cellsR3 [0].verLen - 0.150, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			moveIn3D ('y', blueClamp.radAng, -0.066, &blueClamp.posX, &blueClamp.posY, &blueClamp.posZ);
			elemList.Push (blueClamp.placeObject (5,
				"type", APIParT_CString, "���θ���Ŭ����(����ǰv1)",
				"bReverseRotation", APIParT_Boolean, "1.0",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
				"openingWidth", APIParT_Length, "0.047"));
		}
	}

	return	err;
}

// ���������, �ɺ�Ʈ��Ʈ/�����������, ����ǽ�, ��չ��/���� ��ġ (Ÿ��A)
GSErrCode	ColumnTableformPlacingZone::placeRestObjectsA_soleColumn (ColumnTableformPlacingZone* placingZone)
{
	GSErrCode	err = NoError;

	short	xx;
	double	xLen, yLen;
	double	heightOfFormArea = 0.0;
	double	elev_headpiece;

	EasyObjectPlacement	pipe, pinbolt, hanger, headpiece, columnBand1, columnBand2;

	// ����� ����, ���� ����
	xLen = (placingZone->coreWidth/2 + placingZone->venThick);
	yLen = (placingZone->coreDepth/2 + placingZone->venThick);

	// ���� ���� ���̸� ����
	for (xx = 0 ; xx < placingZone->nCells ; ++xx)
		heightOfFormArea += placingZone->cellsB1 [xx].height;

	// ����ǽ��� ���� ����
	if (heightOfFormArea >= 5.300) {
		elev_headpiece = 2.500;
	} else if (heightOfFormArea >= 4.600) {
		elev_headpiece = 2.500;
	} else if (heightOfFormArea >= 3.500) {
		elev_headpiece = 2.500;
	} else if (heightOfFormArea >= 3.000) {
		elev_headpiece = 2.200;
	} else if (heightOfFormArea >= 2.500) {
		elev_headpiece = 1.900;
	} else if (heightOfFormArea >= 2.000) {
		elev_headpiece = 1.500;
	} else if (heightOfFormArea >= 1.500) {
		elev_headpiece = 1.100;
	} else if (heightOfFormArea >= 1.000) {
		elev_headpiece = 0.800;
	} else {
		elev_headpiece = 0.150;
	}

	// 1. ��������� ��ġ
	pipe.init (L("���������v1.0.gsm"), layerInd_SqrPipe, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

	// ����
	if (placingZone->bUseOutcornerPanel == true) {
		moveIn3D ('x', pipe.radAng, -xLen + placingZone->cellsLT [0].horLen, &pipe.posX, &pipe.posY, &pipe.posZ);
		moveIn3D ('y', pipe.radAng, yLen + 0.0635 + 0.025, &pipe.posX, &pipe.posY, &pipe.posZ);
		moveIn3D ('z', pipe.radAng, 0.050, &pipe.posX, &pipe.posY, &pipe.posZ);
		
		moveIn3D ('x', pipe.radAng, -0.031, &pipe.posX, &pipe.posY, &pipe.posZ);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "0.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
		moveIn3D ('x', pipe.radAng, 0.062, &pipe.posX, &pipe.posY, &pipe.posZ);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "0.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
		moveIn3D ('x', pipe.radAng, -0.031, &pipe.posX, &pipe.posY, &pipe.posZ);

		moveIn3D ('x', pipe.radAng, -placingZone->cellsLT [0].horLen + xLen*2 - placingZone->cellsRT [0].horLen, &pipe.posX, &pipe.posY, &pipe.posZ);

		moveIn3D ('x', pipe.radAng, -0.031, &pipe.posX, &pipe.posY, &pipe.posZ);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "0.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
		moveIn3D ('x', pipe.radAng, 0.062, &pipe.posX, &pipe.posY, &pipe.posZ);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "0.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
		moveIn3D ('x', pipe.radAng, -0.031, &pipe.posX, &pipe.posY, &pipe.posZ);
	} else {
		moveIn3D ('x', pipe.radAng, -xLen, &pipe.posX, &pipe.posY, &pipe.posZ);
		moveIn3D ('y', pipe.radAng, yLen + 0.0635 + 0.025, &pipe.posX, &pipe.posY, &pipe.posZ);
		moveIn3D ('z', pipe.radAng, 0.050, &pipe.posX, &pipe.posY, &pipe.posZ);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "0.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
		moveIn3D ('x', pipe.radAng, xLen*2, &pipe.posX, &pipe.posY, &pipe.posZ);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "0.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
	}

	// �Ʒ���
	pipe.init (L("���������v1.0.gsm"), layerInd_SqrPipe, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

	if (placingZone->bUseOutcornerPanel == true) {
		moveIn3D ('x', pipe.radAng, -xLen + placingZone->cellsLT [0].horLen, &pipe.posX, &pipe.posY, &pipe.posZ);
		moveIn3D ('y', pipe.radAng, -(yLen + 0.0635 + 0.025), &pipe.posX, &pipe.posY, &pipe.posZ);
		moveIn3D ('z', pipe.radAng, 0.050, &pipe.posX, &pipe.posY, &pipe.posZ);

		moveIn3D ('x', pipe.radAng, -0.031, &pipe.posX, &pipe.posY, &pipe.posZ);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "0.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
		moveIn3D ('x', pipe.radAng, 0.062, &pipe.posX, &pipe.posY, &pipe.posZ);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "0.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
		moveIn3D ('x', pipe.radAng, -0.031, &pipe.posX, &pipe.posY, &pipe.posZ);

		moveIn3D ('x', pipe.radAng, -placingZone->cellsLT [0].horLen + xLen*2 - placingZone->cellsRT [0].horLen, &pipe.posX, &pipe.posY, &pipe.posZ);

		moveIn3D ('x', pipe.radAng, -0.031, &pipe.posX, &pipe.posY, &pipe.posZ);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "0.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
		moveIn3D ('x', pipe.radAng, 0.062, &pipe.posX, &pipe.posY, &pipe.posZ);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "0.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
		moveIn3D ('x', pipe.radAng, -0.031, &pipe.posX, &pipe.posY, &pipe.posZ);
	} else {
		moveIn3D ('x', pipe.radAng, -xLen, &pipe.posX, &pipe.posY, &pipe.posZ);
		moveIn3D ('y', pipe.radAng, -(yLen + 0.0635 + 0.025), &pipe.posX, &pipe.posY, &pipe.posZ);
		moveIn3D ('z', pipe.radAng, 0.050, &pipe.posX, &pipe.posY, &pipe.posZ);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "0.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
		moveIn3D ('x', pipe.radAng, xLen*2, &pipe.posX, &pipe.posY, &pipe.posZ);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "0.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
	}

	// ����
	pipe.init (L("���������v1.0.gsm"), layerInd_SqrPipe, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

	if (placingZone->bUseOutcornerPanel == true) {
		moveIn3D ('x', pipe.radAng, -(xLen + 0.0635 + 0.025), &pipe.posX, &pipe.posY, &pipe.posZ);
		moveIn3D ('y', pipe.radAng, yLen - placingZone->cellsLT [0].verLen, &pipe.posX, &pipe.posY, &pipe.posZ);
		moveIn3D ('z', pipe.radAng, 0.050, &pipe.posX, &pipe.posY, &pipe.posZ);
		pipe.radAng += DegreeToRad (90);

		moveIn3D ('x', pipe.radAng, -0.031, &pipe.posX, &pipe.posY, &pipe.posZ);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "0.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
		moveIn3D ('x', pipe.radAng, 0.062, &pipe.posX, &pipe.posY, &pipe.posZ);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "0.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
		moveIn3D ('x', pipe.radAng, -0.031, &pipe.posX, &pipe.posY, &pipe.posZ);

		pipe.radAng -= DegreeToRad (90);
		moveIn3D ('y', pipe.radAng, placingZone->cellsLT [0].verLen - yLen*2 + placingZone->cellsLB [0].verLen, &pipe.posX, &pipe.posY, &pipe.posZ);
		pipe.radAng += DegreeToRad (90);

		moveIn3D ('x', pipe.radAng, -0.031, &pipe.posX, &pipe.posY, &pipe.posZ);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "0.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
		moveIn3D ('x', pipe.radAng, 0.062, &pipe.posX, &pipe.posY, &pipe.posZ);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "0.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
		moveIn3D ('x', pipe.radAng, -0.031, &pipe.posX, &pipe.posY, &pipe.posZ);

		pipe.radAng -= DegreeToRad (90);
	} else {
		moveIn3D ('x', pipe.radAng, -(xLen + 0.0635 + 0.025), &pipe.posX, &pipe.posY, &pipe.posZ);
		moveIn3D ('y', pipe.radAng, yLen, &pipe.posX, &pipe.posY, &pipe.posZ);
		moveIn3D ('z', pipe.radAng, 0.050, &pipe.posX, &pipe.posY, &pipe.posZ);
		pipe.radAng += DegreeToRad (90);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "0.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
		pipe.radAng -= DegreeToRad (90);
		moveIn3D ('y', pipe.radAng, -yLen*2, &pipe.posX, &pipe.posY, &pipe.posZ);
		pipe.radAng += DegreeToRad (90);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "0.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
		pipe.radAng -= DegreeToRad (90);
	}

	// ������
	pipe.init (L("���������v1.0.gsm"), layerInd_SqrPipe, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

	if (placingZone->bUseOutcornerPanel == true) {
		moveIn3D ('x', pipe.radAng, (xLen + 0.0635 + 0.025), &pipe.posX, &pipe.posY, &pipe.posZ);
		moveIn3D ('y', pipe.radAng, yLen - placingZone->cellsLT [0].verLen, &pipe.posX, &pipe.posY, &pipe.posZ);
		moveIn3D ('z', pipe.radAng, 0.050, &pipe.posX, &pipe.posY, &pipe.posZ);
		pipe.radAng += DegreeToRad (90);

		moveIn3D ('x', pipe.radAng, -0.031, &pipe.posX, &pipe.posY, &pipe.posZ);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "0.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
		moveIn3D ('x', pipe.radAng, 0.062, &pipe.posX, &pipe.posY, &pipe.posZ);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "0.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
		moveIn3D ('x', pipe.radAng, -0.031, &pipe.posX, &pipe.posY, &pipe.posZ);

		pipe.radAng -= DegreeToRad (90);
		moveIn3D ('y', pipe.radAng, placingZone->cellsLT [0].verLen - yLen*2 + placingZone->cellsLB [0].verLen, &pipe.posX, &pipe.posY, &pipe.posZ);
		pipe.radAng += DegreeToRad (90);

		moveIn3D ('x', pipe.radAng, -0.031, &pipe.posX, &pipe.posY, &pipe.posZ);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "0.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
		moveIn3D ('x', pipe.radAng, 0.062, &pipe.posX, &pipe.posY, &pipe.posZ);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "0.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
		moveIn3D ('x', pipe.radAng, -0.031, &pipe.posX, &pipe.posY, &pipe.posZ);
		pipe.radAng -= DegreeToRad (90);
	} else {
		moveIn3D ('x', pipe.radAng, (xLen + 0.0635 + 0.025), &pipe.posX, &pipe.posY, &pipe.posZ);
		moveIn3D ('y', pipe.radAng, yLen, &pipe.posX, &pipe.posY, &pipe.posZ);
		moveIn3D ('z', pipe.radAng, 0.050, &pipe.posX, &pipe.posY, &pipe.posZ);
		pipe.radAng += DegreeToRad (90);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "0.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
		pipe.radAng -= DegreeToRad (90);
		moveIn3D ('y', pipe.radAng, -yLen*2, &pipe.posX, &pipe.posY, &pipe.posZ);
		pipe.radAng += DegreeToRad (90);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "0.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
		pipe.radAng -= DegreeToRad (90);
	}

	// 2. �ɺ�Ʈ��Ʈ �Ǵ� ����������� ��ġ
	if (placingZone->bUseOutcornerPanel == true) {
		// �ƿ��ڳ��ǳ��� ���, �ɺ�Ʈ��Ʈ ��ġ
		// ����
		pinbolt.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_Pinbolt, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

		moveIn3D ('x', pinbolt.radAng, -xLen + placingZone->cellsLT [0].horLen, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		moveIn3D ('y', pinbolt.radAng, yLen + 0.0635 + 0.050 + 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			pinbolt.radAng -= DegreeToRad (90);
			elemList.Push (pinbolt.placeObject (7,
				"bRotated", APIParT_Boolean, "0.0",
				"bolt_len", APIParT_Length, "0.100",
				"bolt_dia", APIParT_Length, "0.010",
				"washer_pos", APIParT_Length, "0.050",
				"washer_size", APIParT_Length, "0.100",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			pinbolt.radAng += DegreeToRad (90);
			moveIn3D ('z', pinbolt.radAng, -0.150 + placingZone->cellsT1 [xx].height - 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			pinbolt.radAng -= DegreeToRad (90);
			elemList.Push (pinbolt.placeObject (7,
				"bRotated", APIParT_Boolean, "0.0",
				"bolt_len", APIParT_Length, "0.100",
				"bolt_dia", APIParT_Length, "0.010",
				"washer_pos", APIParT_Length, "0.050",
				"washer_size", APIParT_Length, "0.100",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			pinbolt.radAng += DegreeToRad (90);
			moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		}
		moveIn3D ('z', pinbolt.radAng, -heightOfFormArea, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		moveIn3D ('x', pinbolt.radAng, -placingZone->cellsLT [0].horLen + xLen*2 - placingZone->cellsRT [0].horLen, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			pinbolt.radAng -= DegreeToRad (90);
			elemList.Push (pinbolt.placeObject (7,
				"bRotated", APIParT_Boolean, "0.0",
				"bolt_len", APIParT_Length, "0.100",
				"bolt_dia", APIParT_Length, "0.010",
				"washer_pos", APIParT_Length, "0.050",
				"washer_size", APIParT_Length, "0.100",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			pinbolt.radAng += DegreeToRad (90);
			moveIn3D ('z', pinbolt.radAng, -0.150 + placingZone->cellsT1 [xx].height - 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			pinbolt.radAng -= DegreeToRad (90);
			elemList.Push (pinbolt.placeObject (7,
				"bRotated", APIParT_Boolean, "0.0",
				"bolt_len", APIParT_Length, "0.100",
				"bolt_dia", APIParT_Length, "0.010",
				"washer_pos", APIParT_Length, "0.050",
				"washer_size", APIParT_Length, "0.100",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			pinbolt.radAng += DegreeToRad (90);
			moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		}

		// �Ʒ���
		pinbolt.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_Pinbolt, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

		moveIn3D ('x', pinbolt.radAng, -xLen + placingZone->cellsLT [0].horLen, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		moveIn3D ('y', pinbolt.radAng, -(yLen + 0.0635 + 0.050 + 0.050), &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			pinbolt.radAng -= DegreeToRad (270);
			elemList.Push (pinbolt.placeObject (7,
				"bRotated", APIParT_Boolean, "0.0",
				"bolt_len", APIParT_Length, "0.100",
				"bolt_dia", APIParT_Length, "0.010",
				"washer_pos", APIParT_Length, "0.050",
				"washer_size", APIParT_Length, "0.100",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			pinbolt.radAng += DegreeToRad (270);
			moveIn3D ('z', pinbolt.radAng, -0.150 + placingZone->cellsT1 [xx].height - 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			pinbolt.radAng -= DegreeToRad (270);
			elemList.Push (pinbolt.placeObject (7,
				"bRotated", APIParT_Boolean, "0.0",
				"bolt_len", APIParT_Length, "0.100",
				"bolt_dia", APIParT_Length, "0.010",
				"washer_pos", APIParT_Length, "0.050",
				"washer_size", APIParT_Length, "0.100",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			pinbolt.radAng += DegreeToRad (270);
			moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		}
		moveIn3D ('z', pinbolt.radAng, -heightOfFormArea, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		moveIn3D ('x', pinbolt.radAng, -placingZone->cellsLT [0].horLen + xLen*2 - placingZone->cellsRT [0].horLen, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			pinbolt.radAng -= DegreeToRad (270);
			elemList.Push (pinbolt.placeObject (7,
				"bRotated", APIParT_Boolean, "0.0",
				"bolt_len", APIParT_Length, "0.100",
				"bolt_dia", APIParT_Length, "0.010",
				"washer_pos", APIParT_Length, "0.050",
				"washer_size", APIParT_Length, "0.100",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			pinbolt.radAng += DegreeToRad (270);
			moveIn3D ('z', pinbolt.radAng, -0.150 + placingZone->cellsT1 [xx].height - 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			pinbolt.radAng -= DegreeToRad (270);
			elemList.Push (pinbolt.placeObject (7,
				"bRotated", APIParT_Boolean, "0.0",
				"bolt_len", APIParT_Length, "0.100",
				"bolt_dia", APIParT_Length, "0.010",
				"washer_pos", APIParT_Length, "0.050",
				"washer_size", APIParT_Length, "0.100",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			pinbolt.radAng += DegreeToRad (270);
			moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		}

		// ����
		pinbolt.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_Pinbolt, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

		moveIn3D ('x', pinbolt.radAng, -(xLen + 0.0635 + 0.050 + 0.050), &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		moveIn3D ('y', pinbolt.radAng, yLen - placingZone->cellsLT [0].verLen, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			elemList.Push (pinbolt.placeObject (7,
				"bRotated", APIParT_Boolean, "0.0",
				"bolt_len", APIParT_Length, "0.100",
				"bolt_dia", APIParT_Length, "0.010",
				"washer_pos", APIParT_Length, "0.050",
				"washer_size", APIParT_Length, "0.100",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			moveIn3D ('z', pinbolt.radAng, -0.150 + placingZone->cellsL1 [xx].height - 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			elemList.Push (pinbolt.placeObject (7,
				"bRotated", APIParT_Boolean, "0.0",
				"bolt_len", APIParT_Length, "0.100",
				"bolt_dia", APIParT_Length, "0.010",
				"washer_pos", APIParT_Length, "0.050",
				"washer_size", APIParT_Length, "0.100",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		}
		moveIn3D ('z', pinbolt.radAng, -heightOfFormArea, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		moveIn3D ('y', pinbolt.radAng, placingZone->cellsLT [0].verLen - xLen*2 + placingZone->cellsLB [0].verLen, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			elemList.Push (pinbolt.placeObject (7,
				"bRotated", APIParT_Boolean, "0.0",
				"bolt_len", APIParT_Length, "0.100",
				"bolt_dia", APIParT_Length, "0.010",
				"washer_pos", APIParT_Length, "0.050",
				"washer_size", APIParT_Length, "0.100",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			moveIn3D ('z', pinbolt.radAng, -0.150 + placingZone->cellsL1 [xx].height - 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			elemList.Push (pinbolt.placeObject (7,
				"bRotated", APIParT_Boolean, "0.0",
				"bolt_len", APIParT_Length, "0.100",
				"bolt_dia", APIParT_Length, "0.010",
				"washer_pos", APIParT_Length, "0.050",
				"washer_size", APIParT_Length, "0.100",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		}

		// ������
		pinbolt.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_Pinbolt, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

		moveIn3D ('x', pinbolt.radAng, (xLen + 0.0635 + 0.050 + 0.050), &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		moveIn3D ('y', pinbolt.radAng, yLen - placingZone->cellsLT [0].verLen, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			pinbolt.radAng += DegreeToRad (180);
			elemList.Push (pinbolt.placeObject (7,
				"bRotated", APIParT_Boolean, "0.0",
				"bolt_len", APIParT_Length, "0.100",
				"bolt_dia", APIParT_Length, "0.010",
				"washer_pos", APIParT_Length, "0.050",
				"washer_size", APIParT_Length, "0.100",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			pinbolt.radAng -= DegreeToRad (180);
			moveIn3D ('z', pinbolt.radAng, -0.150 + placingZone->cellsL1 [xx].height - 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			pinbolt.radAng += DegreeToRad (180);
			elemList.Push (pinbolt.placeObject (7,
				"bRotated", APIParT_Boolean, "0.0",
				"bolt_len", APIParT_Length, "0.100",
				"bolt_dia", APIParT_Length, "0.010",
				"washer_pos", APIParT_Length, "0.050",
				"washer_size", APIParT_Length, "0.100",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			pinbolt.radAng -= DegreeToRad (180);
			moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		}
		moveIn3D ('z', pinbolt.radAng, -heightOfFormArea, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		moveIn3D ('y', pinbolt.radAng, placingZone->cellsLT [0].verLen - xLen*2 + placingZone->cellsLB [0].verLen, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			pinbolt.radAng += DegreeToRad (180);
			elemList.Push (pinbolt.placeObject (7,
				"bRotated", APIParT_Boolean, "0.0",
				"bolt_len", APIParT_Length, "0.100",
				"bolt_dia", APIParT_Length, "0.010",
				"washer_pos", APIParT_Length, "0.050",
				"washer_size", APIParT_Length, "0.100",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			pinbolt.radAng -= DegreeToRad (180);
			moveIn3D ('z', pinbolt.radAng, -0.150 + placingZone->cellsL1 [xx].height - 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			pinbolt.radAng += DegreeToRad (180);
			elemList.Push (pinbolt.placeObject (7,
				"bRotated", APIParT_Boolean, "0.0",
				"bolt_len", APIParT_Length, "0.100",
				"bolt_dia", APIParT_Length, "0.010",
				"washer_pos", APIParT_Length, "0.050",
				"washer_size", APIParT_Length, "0.100",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			pinbolt.radAng -= DegreeToRad (180);
			moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		}
	} else {
		// �ƿ��ڳʾޱ��� ���, ����������� ��ġ
		// ����
		hanger.init (L("�����������.gsm"), layerInd_Hanger, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

		moveIn3D ('x', hanger.radAng, -xLen, &hanger.posX, &hanger.posY, &hanger.posZ);
		moveIn3D ('y', hanger.radAng, yLen + 0.0635, &hanger.posX, &hanger.posY, &hanger.posZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', hanger.radAng, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			elemList.Push (hanger.placeObject (4,
				"m_type", APIParT_CString, "�����������",
				"c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str ()));
			moveIn3D ('z', hanger.radAng, -0.150 + placingZone->cellsT1 [xx].height - 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			elemList.Push (hanger.placeObject (4,
				"m_type", APIParT_CString, "�����������",
				"c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str ()));
			moveIn3D ('z', hanger.radAng, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
		}
		moveIn3D ('z', hanger.radAng, -heightOfFormArea, &hanger.posX, &hanger.posY, &hanger.posZ);
		moveIn3D ('x', hanger.radAng, xLen*2, &hanger.posX, &hanger.posY, &hanger.posZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', hanger.radAng, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (180);
			elemList.Push (hanger.placeObject (4,
				"m_type", APIParT_CString, "�����������",
				"c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (-90.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str ()));
			hanger.radAng -= DegreeToRad (180);
			moveIn3D ('z', hanger.radAng, -0.150 + placingZone->cellsT1 [xx].height - 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (180);
			elemList.Push (hanger.placeObject (4,
				"m_type", APIParT_CString, "�����������",
				"c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (-90.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str ()));
			hanger.radAng -= DegreeToRad (180);
			moveIn3D ('z', hanger.radAng, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
		}

		// �Ʒ���
		hanger.init (L("�����������.gsm"), layerInd_Hanger, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

		moveIn3D ('x', hanger.radAng, -xLen, &hanger.posX, &hanger.posY, &hanger.posZ);
		moveIn3D ('y', hanger.radAng, -(yLen + 0.0635), &hanger.posX, &hanger.posY, &hanger.posZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', hanger.radAng, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			elemList.Push (hanger.placeObject (4,
				"m_type", APIParT_CString, "�����������",
				"c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str ()));
			moveIn3D ('z', hanger.radAng, -0.150 + placingZone->cellsT1 [xx].height - 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			elemList.Push (hanger.placeObject (4,
				"m_type", APIParT_CString, "�����������",
				"c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str ()));
			moveIn3D ('z', hanger.radAng, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
		}
		moveIn3D ('z', hanger.radAng, -heightOfFormArea, &hanger.posX, &hanger.posY, &hanger.posZ);
		moveIn3D ('x', hanger.radAng, xLen*2, &hanger.posX, &hanger.posY, &hanger.posZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', hanger.radAng, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (180);
			elemList.Push (hanger.placeObject (4,
				"m_type", APIParT_CString, "�����������",
				"c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (-270.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str ()));
			hanger.radAng -= DegreeToRad (180);
			moveIn3D ('z', hanger.radAng, -0.150 + placingZone->cellsT1 [xx].height - 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (180);
			elemList.Push (hanger.placeObject (4,
				"m_type", APIParT_CString, "�����������",
				"c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (-270.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str ()));
			hanger.radAng -= DegreeToRad (180);
			moveIn3D ('z', hanger.radAng, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
		}

		// ����
		hanger.init (L("�����������.gsm"), layerInd_Hanger, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

		moveIn3D ('x', hanger.radAng, -(xLen + 0.0635), &hanger.posX, &hanger.posY, &hanger.posZ);
		moveIn3D ('y', hanger.radAng, yLen, &hanger.posX, &hanger.posY, &hanger.posZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', hanger.radAng, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng -= DegreeToRad (90);
			elemList.Push (hanger.placeObject (4,
				"m_type", APIParT_CString, "�����������",
				"c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str ()));
			hanger.radAng += DegreeToRad (90);
			moveIn3D ('z', hanger.radAng, -0.150 + placingZone->cellsT1 [xx].height - 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng -= DegreeToRad (90);
			elemList.Push (hanger.placeObject (4,
				"m_type", APIParT_CString, "�����������",
				"c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str ()));
			hanger.radAng += DegreeToRad (90);
			moveIn3D ('z', hanger.radAng, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
		}
		moveIn3D ('z', hanger.radAng, -heightOfFormArea, &hanger.posX, &hanger.posY, &hanger.posZ);
		moveIn3D ('y', hanger.radAng, -yLen*2, &hanger.posX, &hanger.posY, &hanger.posZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', hanger.radAng, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (90);
			elemList.Push (hanger.placeObject (4,
				"m_type", APIParT_CString, "�����������",
				"c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str ()));
			hanger.radAng -= DegreeToRad (90);
			moveIn3D ('z', hanger.radAng, -0.150 + placingZone->cellsT1 [xx].height - 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (90);
			elemList.Push (hanger.placeObject (4,
				"m_type", APIParT_CString, "�����������",
				"c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str ()));
			hanger.radAng -= DegreeToRad (90);
			moveIn3D ('z', hanger.radAng, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
		}

		// ������
		hanger.init (L("�����������.gsm"), layerInd_Hanger, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

		moveIn3D ('x', hanger.radAng, (xLen + 0.0635), &hanger.posX, &hanger.posY, &hanger.posZ);
		moveIn3D ('y', hanger.radAng, yLen, &hanger.posX, &hanger.posY, &hanger.posZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', hanger.radAng, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng -= DegreeToRad (90);
			elemList.Push (hanger.placeObject (4,
				"m_type", APIParT_CString, "�����������",
				"c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str ()));
			hanger.radAng += DegreeToRad (90);
			moveIn3D ('z', hanger.radAng, -0.150 + placingZone->cellsT1 [xx].height - 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng -= DegreeToRad (90);
			elemList.Push (hanger.placeObject (4,
				"m_type", APIParT_CString, "�����������",
				"c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str ()));
			hanger.radAng += DegreeToRad (90);
			moveIn3D ('z', hanger.radAng, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
		}
		moveIn3D ('z', hanger.radAng, -heightOfFormArea, &hanger.posX, &hanger.posY, &hanger.posZ);
		moveIn3D ('y', hanger.radAng, -yLen*2, &hanger.posX, &hanger.posY, &hanger.posZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', hanger.radAng, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (90);
			elemList.Push (hanger.placeObject (4,
				"m_type", APIParT_CString, "�����������",
				"c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str ()));
			hanger.radAng -= DegreeToRad (90);
			moveIn3D ('z', hanger.radAng, -0.150 + placingZone->cellsT1 [xx].height - 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (90);
			elemList.Push (hanger.placeObject (4,
				"m_type", APIParT_CString, "�����������",
				"c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str ()));
			hanger.radAng -= DegreeToRad (90);
			moveIn3D ('z', hanger.radAng, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
		}
	}

	// 3. ����ǽ� ��ġ
	if (placingZone->bUseOutcornerPanel == true) {
		// �ƿ��ڳ��ǳ��� ���, �ƿ��ڳ��ǳ��� �ʺ� �����
		// ����
		headpiece.init (L("RS Push-Pull Props ����ǽ� v2.0 (�ξ�� ����).gsm"), layerInd_Head, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

		moveIn3D ('x', headpiece.radAng, -xLen + placingZone->cellsLT [0].horLen + 0.0475 + 0.050, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, yLen + 0.0635 + 0.155 - 0.096, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('z', headpiece.radAng, 0.300 - 0.100, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng += DegreeToRad (180);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng -= DegreeToRad (180);
		moveIn3D ('z', headpiece.radAng, -0.300 + elev_headpiece, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng += DegreeToRad (180);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng -= DegreeToRad (180);
		moveIn3D ('z', headpiece.radAng, -elev_headpiece + 0.300, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('x', headpiece.radAng, -placingZone->cellsLT [0].horLen + xLen*2 - placingZone->cellsRT [0].horLen, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng += DegreeToRad (180);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng -= DegreeToRad (180);
		moveIn3D ('z', headpiece.radAng, -0.300 + elev_headpiece, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng += DegreeToRad (180);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng -= DegreeToRad (180);

		// �Ʒ���
		headpiece.init (L("RS Push-Pull Props ����ǽ� v2.0 (�ξ�� ����).gsm"), layerInd_Head, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

		moveIn3D ('x', headpiece.radAng, -xLen + placingZone->cellsLT [0].horLen - 0.0475 - 0.050, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, -(yLen + 0.0635 + 0.155 - 0.096), &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('z', headpiece.radAng, 0.300 - 0.100, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('z', headpiece.radAng, -0.300 + elev_headpiece, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('z', headpiece.radAng, -elev_headpiece + 0.300, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('x', headpiece.radAng, -placingZone->cellsLT [0].horLen + xLen*2 - placingZone->cellsRT [0].horLen, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('z', headpiece.radAng, -0.300 + elev_headpiece, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

		// ����
		headpiece.init (L("RS Push-Pull Props ����ǽ� v2.0 (�ξ�� ����).gsm"), layerInd_Head, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

		moveIn3D ('x', headpiece.radAng, -(xLen + 0.0635 + 0.155 - 0.096), &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, yLen - placingZone->cellsLT [0].verLen + 0.0475 + 0.050, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('z', headpiece.radAng, 0.300 - 0.100, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng -= DegreeToRad (90);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng += DegreeToRad (90);
		moveIn3D ('z', headpiece.radAng, -0.300 + elev_headpiece, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng -= DegreeToRad (90);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng += DegreeToRad (90);
		moveIn3D ('z', headpiece.radAng, -elev_headpiece + 0.300, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, placingZone->cellsLT [0].verLen - yLen*2 + placingZone->cellsLB [0].verLen, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng -= DegreeToRad (90);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng += DegreeToRad (90);
		moveIn3D ('z', headpiece.radAng, -0.300 + elev_headpiece, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng -= DegreeToRad (90);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng += DegreeToRad (90);

		// ������
		headpiece.init (L("RS Push-Pull Props ����ǽ� v2.0 (�ξ�� ����).gsm"), layerInd_Head, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

		moveIn3D ('x', headpiece.radAng, (xLen + 0.0635 + 0.155 - 0.096), &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, yLen - placingZone->cellsLT [0].verLen - 0.0475 - 0.050, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('z', headpiece.radAng, 0.300 - 0.100, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng += DegreeToRad (90);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng -= DegreeToRad (90);
		moveIn3D ('z', headpiece.radAng, -0.300 + elev_headpiece, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng += DegreeToRad (90);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng -= DegreeToRad (90);
		moveIn3D ('z', headpiece.radAng, -elev_headpiece + 0.300, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, placingZone->cellsLT [0].verLen - yLen*2 + placingZone->cellsLB [0].verLen, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng += DegreeToRad (90);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng -= DegreeToRad (90);
		moveIn3D ('z', headpiece.radAng, -0.300 + elev_headpiece, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng += DegreeToRad (90);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng -= DegreeToRad (90);
	} else {
		// �ƿ��ڳʾޱ��� ���, �ƿ��ڳ��ǳ��� �ʺ� ������� ����
		// ����
		headpiece.init (L("RS Push-Pull Props ����ǽ� v2.0 (�ξ�� ����).gsm"), layerInd_Head, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

		moveIn3D ('x', headpiece.radAng, -xLen + 0.0475, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, yLen + 0.0635 + 0.155, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('z', headpiece.radAng, 0.300 - 0.100, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng += DegreeToRad (180);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng -= DegreeToRad (180);
		moveIn3D ('z', headpiece.radAng, -0.300 + elev_headpiece, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng += DegreeToRad (180);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng -= DegreeToRad (180);
		moveIn3D ('z', headpiece.radAng, -elev_headpiece + 0.300, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('x', headpiece.radAng, xLen*2, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng += DegreeToRad (180);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng -= DegreeToRad (180);
		moveIn3D ('z', headpiece.radAng, -0.300 + elev_headpiece, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng += DegreeToRad (180);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng -= DegreeToRad (180);

		// �Ʒ���
		headpiece.init (L("RS Push-Pull Props ����ǽ� v2.0 (�ξ�� ����).gsm"), layerInd_Head, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

		moveIn3D ('x', headpiece.radAng, -xLen- 0.0475, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, -(yLen + 0.0635 + 0.155), &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('z', headpiece.radAng, 0.300 - 0.100, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('z', headpiece.radAng, -0.300 + elev_headpiece, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('z', headpiece.radAng, -elev_headpiece + 0.300, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('x', headpiece.radAng, xLen*2, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('z', headpiece.radAng, -0.300 + elev_headpiece, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

		// ����
		headpiece.init (L("RS Push-Pull Props ����ǽ� v2.0 (�ξ�� ����).gsm"), layerInd_Head, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

		moveIn3D ('x', headpiece.radAng, -(xLen + 0.0635 + 0.155), &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, yLen + 0.0475, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('z', headpiece.radAng, 0.300 - 0.100, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng -= DegreeToRad (90);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng += DegreeToRad (90);
		moveIn3D ('z', headpiece.radAng, -0.300 + elev_headpiece, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng -= DegreeToRad (90);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng += DegreeToRad (90);
		moveIn3D ('z', headpiece.radAng, -elev_headpiece + 0.300, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, -yLen*2, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng -= DegreeToRad (90);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng += DegreeToRad (90);
		moveIn3D ('z', headpiece.radAng, -0.300 + elev_headpiece, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng -= DegreeToRad (90);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng += DegreeToRad (90);

		// ������
		headpiece.init (L("RS Push-Pull Props ����ǽ� v2.0 (�ξ�� ����).gsm"), layerInd_Head, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

		moveIn3D ('x', headpiece.radAng, (xLen + 0.0635 + 0.155), &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, yLen - 0.0475, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('z', headpiece.radAng, 0.300 - 0.100, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng += DegreeToRad (90);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng -= DegreeToRad (90);
		moveIn3D ('z', headpiece.radAng, -0.300 + elev_headpiece, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng += DegreeToRad (90);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng -= DegreeToRad (90);
		moveIn3D ('z', headpiece.radAng, -elev_headpiece + 0.300, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, -yLen*2, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng += DegreeToRad (90);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng -= DegreeToRad (90);
		moveIn3D ('z', headpiece.radAng, -0.300 + elev_headpiece, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng += DegreeToRad (90);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng -= DegreeToRad (90);
	}

	// 4. ��չ�� �Ǵ� ���� ��ġ
	if (placingZone->typeOfColumnBand == 1) {
		columnBand1.init (L("��չ��v2.0.gsm"), layerInd_ColumnBand, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

		moveIn3D ('x', columnBand1.radAng, -0.0035 + (placingZone->coreWidth + placingZone->venThick*2)/2, &columnBand1.posX, &columnBand1.posY, &columnBand1.posZ);
		moveIn3D ('y', columnBand1.radAng, -0.1535 - (placingZone->coreDepth + placingZone->venThick*2)/2, &columnBand1.posX, &columnBand1.posY, &columnBand1.posZ);
		moveIn3D ('z', columnBand1.radAng, 0.500, &columnBand1.posX, &columnBand1.posY, &columnBand1.posZ);
		elemList.Push (columnBand1.placeObject (4,
			"band_size", APIParT_CString, "80x40x1270",
			"c_w", APIParT_Length, format_string ("%f", placingZone->coreWidth + placingZone->venThick*2),
			"c_h", APIParT_Length, format_string ("%f", placingZone->coreDepth + placingZone->venThick*2),
			"addOffset", APIParT_Length, format_string ("%f", 0.050)));
		moveIn3D ('z', columnBand1.radAng, -0.500 + heightOfFormArea/2 - 0.900, &columnBand1.posX, &columnBand1.posY, &columnBand1.posZ);
		elemList.Push (columnBand1.placeObject (4,
			"band_size", APIParT_CString, "80x40x1270",
			"c_w", APIParT_Length, format_string ("%f", placingZone->coreWidth + placingZone->venThick*2),
			"c_h", APIParT_Length, format_string ("%f", placingZone->coreDepth + placingZone->venThick*2),
			"addOffset", APIParT_Length, format_string ("%f", 0.050)));
		moveIn3D ('z', columnBand1.radAng, 0.900, &columnBand1.posX, &columnBand1.posY, &columnBand1.posZ);
		elemList.Push (columnBand1.placeObject (4,
			"band_size", APIParT_CString, "80x40x1270",
			"c_w", APIParT_Length, format_string ("%f", placingZone->coreWidth + placingZone->venThick*2),
			"c_h", APIParT_Length, format_string ("%f", placingZone->coreDepth + placingZone->venThick*2),
			"addOffset", APIParT_Length, format_string ("%f", 0.050)));
		moveIn3D ('z', columnBand1.radAng, 0.900, &columnBand1.posX, &columnBand1.posY, &columnBand1.posZ);
		elemList.Push (columnBand1.placeObject (4,
			"band_size", APIParT_CString, "80x40x1270",
			"c_w", APIParT_Length, format_string ("%f", placingZone->coreWidth + placingZone->venThick*2),
			"c_h", APIParT_Length, format_string ("%f", placingZone->coreDepth + placingZone->venThick*2),
			"addOffset", APIParT_Length, format_string ("%f", 0.050)));
	} else if (placingZone->typeOfColumnBand == 2) {
		columnBand2.init (L("����v1.0.gsm"), layerInd_ColumnBand, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

		moveIn3D ('z', columnBand2.radAng, 0.500, &columnBand2.posX, &columnBand2.posY, &columnBand2.posZ);
		elemList.Push (columnBand2.placeObject (5,
			"gap", APIParT_Length, "0.100",
			"nutType", APIParT_CString, "����Ʈ",
			"c_w", APIParT_Length, format_string ("%f", placingZone->coreWidth + placingZone->venThick*2),
			"c_h", APIParT_Length, format_string ("%f", placingZone->coreDepth + placingZone->venThick*2),
			"addOffset", APIParT_Length, format_string ("%f", 0.050)));
		moveIn3D ('z', columnBand2.radAng, -0.500 + heightOfFormArea/2 - 0.900, &columnBand2.posX, &columnBand2.posY, &columnBand2.posZ);
		elemList.Push (columnBand2.placeObject (5,
			"gap", APIParT_Length, "0.100",
			"nutType", APIParT_CString, "����Ʈ",
			"c_w", APIParT_Length, format_string ("%f", placingZone->coreWidth + placingZone->venThick*2),
			"c_h", APIParT_Length, format_string ("%f", placingZone->coreDepth + placingZone->venThick*2),
			"addOffset", APIParT_Length, format_string ("%f", 0.050)));
		moveIn3D ('z', columnBand2.radAng, 0.900, &columnBand2.posX, &columnBand2.posY, &columnBand2.posZ);
		elemList.Push (columnBand2.placeObject (5,
			"gap", APIParT_Length, "0.100",
			"nutType", APIParT_CString, "����Ʈ",
			"c_w", APIParT_Length, format_string ("%f", placingZone->coreWidth + placingZone->venThick*2),
			"c_h", APIParT_Length, format_string ("%f", placingZone->coreDepth + placingZone->venThick*2),
			"addOffset", APIParT_Length, format_string ("%f", 0.050)));
		moveIn3D ('z', columnBand2.radAng, 0.900, &columnBand2.posX, &columnBand2.posY, &columnBand2.posZ);
		elemList.Push (columnBand2.placeObject (5,
			"gap", APIParT_Length, "0.100",
			"nutType", APIParT_CString, "����Ʈ",
			"c_w", APIParT_Length, format_string ("%f", placingZone->coreWidth + placingZone->venThick*2),
			"c_h", APIParT_Length, format_string ("%f", placingZone->coreDepth + placingZone->venThick*2),
			"addOffset", APIParT_Length, format_string ("%f", 0.050)));
	}

	return	err;
}

// ���������, �ɺ�Ʈ��Ʈ/�����������, ����ǽ�, ��չ��/���� ��ġ (Ÿ��B)
GSErrCode	ColumnTableformPlacingZone::placeRestObjectsB_soleColumn (ColumnTableformPlacingZone* placingZone)
{
	GSErrCode	err = NoError;

	short	xx;
	double	xLen, yLen;
	double	heightOfFormArea = 0.0;
	double	elev_headpiece;

	EasyObjectPlacement	pipe, pinbolt, hanger, headpiece, columnBand1, columnBand2;

	// ����� ����, ���� ����
	xLen = (placingZone->coreWidth/2 + placingZone->venThick);
	yLen = (placingZone->coreDepth/2 + placingZone->venThick);

	// ���� ���� ���̸� ����
	for (xx = 0 ; xx < placingZone->nCells ; ++xx)
		heightOfFormArea += placingZone->cellsB1 [xx].height;

	// ����ǽ��� ���� ����
	if (heightOfFormArea >= 5.300) {
		elev_headpiece = 2.500;
	} else if (heightOfFormArea >= 4.600) {
		elev_headpiece = 2.500;
	} else if (heightOfFormArea >= 3.500) {
		elev_headpiece = 2.500;
	} else if (heightOfFormArea >= 3.000) {
		elev_headpiece = 2.200;
	} else if (heightOfFormArea >= 2.500) {
		elev_headpiece = 1.900;
	} else if (heightOfFormArea >= 2.000) {
		elev_headpiece = 1.500;
	} else if (heightOfFormArea >= 1.500) {
		elev_headpiece = 1.100;
	} else if (heightOfFormArea >= 1.000) {
		elev_headpiece = 0.800;
	} else {
		elev_headpiece = 0.150;
	}

	// 1. ��������� ��ġ
	pipe.init (L("���������v1.0.gsm"), layerInd_SqrPipe, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

	// ����
	if (placingZone->bUseOutcornerPanel == true) {
		moveIn3D ('x', pipe.radAng, -xLen + placingZone->cellsLT [0].horLen, &pipe.posX, &pipe.posY, &pipe.posZ);
		moveIn3D ('y', pipe.radAng, yLen + 0.0635 + 0.025, &pipe.posX, &pipe.posY, &pipe.posZ);
		moveIn3D ('z', pipe.radAng, 0.050, &pipe.posX, &pipe.posY, &pipe.posZ);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "1.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
		moveIn3D ('x', pipe.radAng, -placingZone->cellsLT [0].horLen + xLen*2 - placingZone->cellsRT [0].horLen, &pipe.posX, &pipe.posY, &pipe.posZ);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "1.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
	} else {
		moveIn3D ('x', pipe.radAng, -xLen, &pipe.posX, &pipe.posY, &pipe.posZ);
		moveIn3D ('y', pipe.radAng, yLen + 0.0635 + 0.025, &pipe.posX, &pipe.posY, &pipe.posZ);
		moveIn3D ('z', pipe.radAng, 0.050, &pipe.posX, &pipe.posY, &pipe.posZ);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "1.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
		moveIn3D ('x', pipe.radAng, xLen*2, &pipe.posX, &pipe.posY, &pipe.posZ);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "1.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
	}

	// �Ʒ���
	pipe.init (L("���������v1.0.gsm"), layerInd_SqrPipe, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

	if (placingZone->bUseOutcornerPanel == true) {
		moveIn3D ('x', pipe.radAng, -xLen + placingZone->cellsLT [0].horLen, &pipe.posX, &pipe.posY, &pipe.posZ);
		moveIn3D ('y', pipe.radAng, -(yLen + 0.0635 + 0.025), &pipe.posX, &pipe.posY, &pipe.posZ);
		moveIn3D ('z', pipe.radAng, 0.050, &pipe.posX, &pipe.posY, &pipe.posZ);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "1.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
		moveIn3D ('x', pipe.radAng, -placingZone->cellsLT [0].horLen + xLen*2 - placingZone->cellsRT [0].horLen, &pipe.posX, &pipe.posY, &pipe.posZ);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "1.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
	} else {
		moveIn3D ('x', pipe.radAng, -xLen, &pipe.posX, &pipe.posY, &pipe.posZ);
		moveIn3D ('y', pipe.radAng, -(yLen + 0.0635 + 0.025), &pipe.posX, &pipe.posY, &pipe.posZ);
		moveIn3D ('z', pipe.radAng, 0.050, &pipe.posX, &pipe.posY, &pipe.posZ);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "1.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
		moveIn3D ('x', pipe.radAng, xLen*2, &pipe.posX, &pipe.posY, &pipe.posZ);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "1.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
	}

	// ����
	pipe.init (L("���������v1.0.gsm"), layerInd_SqrPipe, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

	if (placingZone->bUseOutcornerPanel == true) {
		moveIn3D ('x', pipe.radAng, -(xLen + 0.0635 + 0.025), &pipe.posX, &pipe.posY, &pipe.posZ);
		moveIn3D ('y', pipe.radAng, yLen - placingZone->cellsLT [0].verLen, &pipe.posX, &pipe.posY, &pipe.posZ);
		moveIn3D ('z', pipe.radAng, 0.050, &pipe.posX, &pipe.posY, &pipe.posZ);
		pipe.radAng += DegreeToRad (90);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "1.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
		pipe.radAng -= DegreeToRad (90);
		moveIn3D ('y', pipe.radAng, placingZone->cellsLT [0].verLen - yLen*2 + placingZone->cellsLB [0].verLen, &pipe.posX, &pipe.posY, &pipe.posZ);
		pipe.radAng += DegreeToRad (90);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "1.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
		pipe.radAng -= DegreeToRad (90);
	} else {
		moveIn3D ('x', pipe.radAng, -(xLen + 0.0635 + 0.025), &pipe.posX, &pipe.posY, &pipe.posZ);
		moveIn3D ('y', pipe.radAng, yLen, &pipe.posX, &pipe.posY, &pipe.posZ);
		moveIn3D ('z', pipe.radAng, 0.050, &pipe.posX, &pipe.posY, &pipe.posZ);
		pipe.radAng += DegreeToRad (90);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "1.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
		pipe.radAng -= DegreeToRad (90);
		moveIn3D ('y', pipe.radAng, -yLen*2, &pipe.posX, &pipe.posY, &pipe.posZ);
		pipe.radAng += DegreeToRad (90);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "1.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
		pipe.radAng -= DegreeToRad (90);
	}

	// ������
	pipe.init (L("���������v1.0.gsm"), layerInd_SqrPipe, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

	if (placingZone->bUseOutcornerPanel == true) {
		moveIn3D ('x', pipe.radAng, (xLen + 0.0635 + 0.025), &pipe.posX, &pipe.posY, &pipe.posZ);
		moveIn3D ('y', pipe.radAng, yLen - placingZone->cellsLT [0].verLen, &pipe.posX, &pipe.posY, &pipe.posZ);
		moveIn3D ('z', pipe.radAng, 0.050, &pipe.posX, &pipe.posY, &pipe.posZ);
		pipe.radAng += DegreeToRad (90);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "1.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
		pipe.radAng -= DegreeToRad (90);
		moveIn3D ('y', pipe.radAng, placingZone->cellsLT [0].verLen - yLen*2 + placingZone->cellsLB [0].verLen, &pipe.posX, &pipe.posY, &pipe.posZ);
		pipe.radAng += DegreeToRad (90);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "1.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
		pipe.radAng -= DegreeToRad (90);
	} else {
		moveIn3D ('x', pipe.radAng, (xLen + 0.0635 + 0.025), &pipe.posX, &pipe.posY, &pipe.posZ);
		moveIn3D ('y', pipe.radAng, yLen, &pipe.posX, &pipe.posY, &pipe.posZ);
		moveIn3D ('z', pipe.radAng, 0.050, &pipe.posX, &pipe.posY, &pipe.posZ);
		pipe.radAng += DegreeToRad (90);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "1.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
		pipe.radAng -= DegreeToRad (90);
		moveIn3D ('y', pipe.radAng, -yLen*2, &pipe.posX, &pipe.posY, &pipe.posZ);
		pipe.radAng += DegreeToRad (90);
		elemList.Push (pipe.placeObject (7,
			"p_comp", APIParT_CString, "�簢������",
			"p_leng", APIParT_Length, format_string ("%f", heightOfFormArea),
			"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
			"bPunching", APIParT_Boolean, "1.0",
			"holeDir", APIParT_CString, "����",
			"holeDia", APIParT_Length, "0.013",
			"holeDist", APIParT_Length, "0.050"));
		pipe.radAng -= DegreeToRad (90);
	}

	// 2. �ɺ�Ʈ��Ʈ �Ǵ� ����������� ��ġ
	if (placingZone->bUseOutcornerPanel == true) {
		// �ƿ��ڳ��ǳ��� ���, �ɺ�Ʈ��Ʈ ��ġ
		// ����
		pinbolt.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_Pinbolt, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

		moveIn3D ('x', pinbolt.radAng, -xLen + placingZone->cellsLT [0].horLen, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		moveIn3D ('y', pinbolt.radAng, yLen + 0.0635 + 0.050 + 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			pinbolt.radAng -= DegreeToRad (90);
			elemList.Push (pinbolt.placeObject (7,
				"bRotated", APIParT_Boolean, "0.0",
				"bolt_len", APIParT_Length, "0.100",
				"bolt_dia", APIParT_Length, "0.010",
				"washer_pos", APIParT_Length, "0.050",
				"washer_size", APIParT_Length, "0.100",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			pinbolt.radAng += DegreeToRad (90);
			moveIn3D ('z', pinbolt.radAng, -0.150 + placingZone->cellsT1 [xx].height - 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			pinbolt.radAng -= DegreeToRad (90);
			elemList.Push (pinbolt.placeObject (7,
				"bRotated", APIParT_Boolean, "0.0",
				"bolt_len", APIParT_Length, "0.100",
				"bolt_dia", APIParT_Length, "0.010",
				"washer_pos", APIParT_Length, "0.050",
				"washer_size", APIParT_Length, "0.100",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			pinbolt.radAng += DegreeToRad (90);
			moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		}
		moveIn3D ('z', pinbolt.radAng, -heightOfFormArea, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		moveIn3D ('x', pinbolt.radAng, -placingZone->cellsLT [0].horLen + xLen*2 - placingZone->cellsRT [0].horLen, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			pinbolt.radAng -= DegreeToRad (90);
			elemList.Push (pinbolt.placeObject (7,
				"bRotated", APIParT_Boolean, "0.0",
				"bolt_len", APIParT_Length, "0.100",
				"bolt_dia", APIParT_Length, "0.010",
				"washer_pos", APIParT_Length, "0.050",
				"washer_size", APIParT_Length, "0.100",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			pinbolt.radAng += DegreeToRad (90);
			moveIn3D ('z', pinbolt.radAng, -0.150 + placingZone->cellsT1 [xx].height - 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			pinbolt.radAng -= DegreeToRad (90);
			elemList.Push (pinbolt.placeObject (7,
				"bRotated", APIParT_Boolean, "0.0",
				"bolt_len", APIParT_Length, "0.100",
				"bolt_dia", APIParT_Length, "0.010",
				"washer_pos", APIParT_Length, "0.050",
				"washer_size", APIParT_Length, "0.100",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			pinbolt.radAng += DegreeToRad (90);
			moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		}

		// �Ʒ���
		pinbolt.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_Pinbolt, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

		moveIn3D ('x', pinbolt.radAng, -xLen + placingZone->cellsLT [0].horLen, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		moveIn3D ('y', pinbolt.radAng, -(yLen + 0.0635 + 0.050 + 0.050), &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			pinbolt.radAng -= DegreeToRad (270);
			elemList.Push (pinbolt.placeObject (7,
				"bRotated", APIParT_Boolean, "0.0",
				"bolt_len", APIParT_Length, "0.100",
				"bolt_dia", APIParT_Length, "0.010",
				"washer_pos", APIParT_Length, "0.050",
				"washer_size", APIParT_Length, "0.100",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			pinbolt.radAng += DegreeToRad (270);
			moveIn3D ('z', pinbolt.radAng, -0.150 + placingZone->cellsT1 [xx].height - 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			pinbolt.radAng -= DegreeToRad (270);
			elemList.Push (pinbolt.placeObject (7,
				"bRotated", APIParT_Boolean, "0.0",
				"bolt_len", APIParT_Length, "0.100",
				"bolt_dia", APIParT_Length, "0.010",
				"washer_pos", APIParT_Length, "0.050",
				"washer_size", APIParT_Length, "0.100",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			pinbolt.radAng += DegreeToRad (270);
			moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		}
		moveIn3D ('z', pinbolt.radAng, -heightOfFormArea, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		moveIn3D ('x', pinbolt.radAng, -placingZone->cellsLT [0].horLen + xLen*2 - placingZone->cellsRT [0].horLen, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			pinbolt.radAng -= DegreeToRad (270);
			elemList.Push (pinbolt.placeObject (7,
				"bRotated", APIParT_Boolean, "0.0",
				"bolt_len", APIParT_Length, "0.100",
				"bolt_dia", APIParT_Length, "0.010",
				"washer_pos", APIParT_Length, "0.050",
				"washer_size", APIParT_Length, "0.100",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			pinbolt.radAng += DegreeToRad (270);
			moveIn3D ('z', pinbolt.radAng, -0.150 + placingZone->cellsT1 [xx].height - 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			pinbolt.radAng -= DegreeToRad (270);
			elemList.Push (pinbolt.placeObject (7,
				"bRotated", APIParT_Boolean, "0.0",
				"bolt_len", APIParT_Length, "0.100",
				"bolt_dia", APIParT_Length, "0.010",
				"washer_pos", APIParT_Length, "0.050",
				"washer_size", APIParT_Length, "0.100",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			pinbolt.radAng += DegreeToRad (270);
			moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		}

		// ����
		pinbolt.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_Pinbolt, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

		moveIn3D ('x', pinbolt.radAng, -(xLen + 0.0635 + 0.050 + 0.050), &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		moveIn3D ('y', pinbolt.radAng, yLen - placingZone->cellsLT [0].verLen, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			elemList.Push (pinbolt.placeObject (7,
				"bRotated", APIParT_Boolean, "0.0",
				"bolt_len", APIParT_Length, "0.100",
				"bolt_dia", APIParT_Length, "0.010",
				"washer_pos", APIParT_Length, "0.050",
				"washer_size", APIParT_Length, "0.100",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			moveIn3D ('z', pinbolt.radAng, -0.150 + placingZone->cellsL1 [xx].height - 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			elemList.Push (pinbolt.placeObject (7,
				"bRotated", APIParT_Boolean, "0.0",
				"bolt_len", APIParT_Length, "0.100",
				"bolt_dia", APIParT_Length, "0.010",
				"washer_pos", APIParT_Length, "0.050",
				"washer_size", APIParT_Length, "0.100",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		}
		moveIn3D ('z', pinbolt.radAng, -heightOfFormArea, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		moveIn3D ('y', pinbolt.radAng, placingZone->cellsLT [0].verLen - xLen*2 + placingZone->cellsLB [0].verLen, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			elemList.Push (pinbolt.placeObject (7,
				"bRotated", APIParT_Boolean, "0.0",
				"bolt_len", APIParT_Length, "0.100",
				"bolt_dia", APIParT_Length, "0.010",
				"washer_pos", APIParT_Length, "0.050",
				"washer_size", APIParT_Length, "0.100",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			moveIn3D ('z', pinbolt.radAng, -0.150 + placingZone->cellsL1 [xx].height - 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			elemList.Push (pinbolt.placeObject (7,
				"bRotated", APIParT_Boolean, "0.0",
				"bolt_len", APIParT_Length, "0.100",
				"bolt_dia", APIParT_Length, "0.010",
				"washer_pos", APIParT_Length, "0.050",
				"washer_size", APIParT_Length, "0.100",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		}

		// ������
		pinbolt.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_Pinbolt, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

		moveIn3D ('x', pinbolt.radAng, (xLen + 0.0635 + 0.050 + 0.050), &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		moveIn3D ('y', pinbolt.radAng, yLen - placingZone->cellsLT [0].verLen, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			pinbolt.radAng += DegreeToRad (180);
			elemList.Push (pinbolt.placeObject (7,
				"bRotated", APIParT_Boolean, "0.0",
				"bolt_len", APIParT_Length, "0.100",
				"bolt_dia", APIParT_Length, "0.010",
				"washer_pos", APIParT_Length, "0.050",
				"washer_size", APIParT_Length, "0.100",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			pinbolt.radAng -= DegreeToRad (180);
			moveIn3D ('z', pinbolt.radAng, -0.150 + placingZone->cellsL1 [xx].height - 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			pinbolt.radAng += DegreeToRad (180);
			elemList.Push (pinbolt.placeObject (7,
				"bRotated", APIParT_Boolean, "0.0",
				"bolt_len", APIParT_Length, "0.100",
				"bolt_dia", APIParT_Length, "0.010",
				"washer_pos", APIParT_Length, "0.050",
				"washer_size", APIParT_Length, "0.100",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			pinbolt.radAng -= DegreeToRad (180);
			moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		}
		moveIn3D ('z', pinbolt.radAng, -heightOfFormArea, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		moveIn3D ('y', pinbolt.radAng, placingZone->cellsLT [0].verLen - xLen*2 + placingZone->cellsLB [0].verLen, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			pinbolt.radAng += DegreeToRad (180);
			elemList.Push (pinbolt.placeObject (7,
				"bRotated", APIParT_Boolean, "0.0",
				"bolt_len", APIParT_Length, "0.100",
				"bolt_dia", APIParT_Length, "0.010",
				"washer_pos", APIParT_Length, "0.050",
				"washer_size", APIParT_Length, "0.100",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			pinbolt.radAng -= DegreeToRad (180);
			moveIn3D ('z', pinbolt.radAng, -0.150 + placingZone->cellsL1 [xx].height - 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
			pinbolt.radAng += DegreeToRad (180);
			elemList.Push (pinbolt.placeObject (7,
				"bRotated", APIParT_Boolean, "0.0",
				"bolt_len", APIParT_Length, "0.100",
				"bolt_dia", APIParT_Length, "0.010",
				"washer_pos", APIParT_Length, "0.050",
				"washer_size", APIParT_Length, "0.100",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			pinbolt.radAng -= DegreeToRad (180);
			moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		}
	} else {
		// �ƿ��ڳʾޱ��� ���, ����������� ��ġ
		// ����
		hanger.init (L("�����������.gsm"), layerInd_Hanger, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

		moveIn3D ('x', hanger.radAng, -xLen, &hanger.posX, &hanger.posY, &hanger.posZ);
		moveIn3D ('y', hanger.radAng, yLen + 0.0635, &hanger.posX, &hanger.posY, &hanger.posZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', hanger.radAng, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			elemList.Push (hanger.placeObject (4,
				"m_type", APIParT_CString, "�����������",
				"c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str ()));
			moveIn3D ('z', hanger.radAng, -0.150 + placingZone->cellsT1 [xx].height - 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			elemList.Push (hanger.placeObject (4,
				"m_type", APIParT_CString, "�����������",
				"c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str ()));
			moveIn3D ('z', hanger.radAng, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
		}
		moveIn3D ('z', hanger.radAng, -heightOfFormArea, &hanger.posX, &hanger.posY, &hanger.posZ);
		moveIn3D ('x', hanger.radAng, xLen*2, &hanger.posX, &hanger.posY, &hanger.posZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', hanger.radAng, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (180);
			elemList.Push (hanger.placeObject (4,
				"m_type", APIParT_CString, "�����������",
				"c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (-90.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str ()));
			hanger.radAng -= DegreeToRad (180);
			moveIn3D ('z', hanger.radAng, -0.150 + placingZone->cellsT1 [xx].height - 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (180);
			elemList.Push (hanger.placeObject (4,
				"m_type", APIParT_CString, "�����������",
				"c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (-90.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str ()));
			hanger.radAng -= DegreeToRad (180);
			moveIn3D ('z', hanger.radAng, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
		}

		// �Ʒ���
		hanger.init (L("�����������.gsm"), layerInd_Hanger, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

		moveIn3D ('x', hanger.radAng, -xLen, &hanger.posX, &hanger.posY, &hanger.posZ);
		moveIn3D ('y', hanger.radAng, -(yLen + 0.0635), &hanger.posX, &hanger.posY, &hanger.posZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', hanger.radAng, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			elemList.Push (hanger.placeObject (4,
				"m_type", APIParT_CString, "�����������",
				"c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str ()));
			moveIn3D ('z', hanger.radAng, -0.150 + placingZone->cellsT1 [xx].height - 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			elemList.Push (hanger.placeObject (4,
				"m_type", APIParT_CString, "�����������",
				"c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str ()));
			moveIn3D ('z', hanger.radAng, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
		}
		moveIn3D ('z', hanger.radAng, -heightOfFormArea, &hanger.posX, &hanger.posY, &hanger.posZ);
		moveIn3D ('x', hanger.radAng, xLen*2, &hanger.posX, &hanger.posY, &hanger.posZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', hanger.radAng, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (180);
			elemList.Push (hanger.placeObject (4,
				"m_type", APIParT_CString, "�����������",
				"c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (-270.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str ()));
			hanger.radAng -= DegreeToRad (180);
			moveIn3D ('z', hanger.radAng, -0.150 + placingZone->cellsT1 [xx].height - 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (180);
			elemList.Push (hanger.placeObject (4,
				"m_type", APIParT_CString, "�����������",
				"c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (-270.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str ()));
			hanger.radAng -= DegreeToRad (180);
			moveIn3D ('z', hanger.radAng, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
		}

		// ����
		hanger.init (L("�����������.gsm"), layerInd_Hanger, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

		moveIn3D ('x', hanger.radAng, -(xLen + 0.0635), &hanger.posX, &hanger.posY, &hanger.posZ);
		moveIn3D ('y', hanger.radAng, yLen, &hanger.posX, &hanger.posY, &hanger.posZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', hanger.radAng, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng -= DegreeToRad (90);
			elemList.Push (hanger.placeObject (4,
				"m_type", APIParT_CString, "�����������",
				"c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str ()));
			hanger.radAng += DegreeToRad (90);
			moveIn3D ('z', hanger.radAng, -0.150 + placingZone->cellsT1 [xx].height - 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng -= DegreeToRad (90);
			elemList.Push (hanger.placeObject (4,
				"m_type", APIParT_CString, "�����������",
				"c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str ()));
			hanger.radAng += DegreeToRad (90);
			moveIn3D ('z', hanger.radAng, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
		}
		moveIn3D ('z', hanger.radAng, -heightOfFormArea, &hanger.posX, &hanger.posY, &hanger.posZ);
		moveIn3D ('y', hanger.radAng, -yLen*2, &hanger.posX, &hanger.posY, &hanger.posZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', hanger.radAng, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (90);
			elemList.Push (hanger.placeObject (4,
				"m_type", APIParT_CString, "�����������",
				"c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str ()));
			hanger.radAng -= DegreeToRad (90);
			moveIn3D ('z', hanger.radAng, -0.150 + placingZone->cellsT1 [xx].height - 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (90);
			elemList.Push (hanger.placeObject (4,
				"m_type", APIParT_CString, "�����������",
				"c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str ()));
			hanger.radAng -= DegreeToRad (90);
			moveIn3D ('z', hanger.radAng, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
		}

		// ������
		hanger.init (L("�����������.gsm"), layerInd_Hanger, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

		moveIn3D ('x', hanger.radAng, (xLen + 0.0635), &hanger.posX, &hanger.posY, &hanger.posZ);
		moveIn3D ('y', hanger.radAng, yLen, &hanger.posX, &hanger.posY, &hanger.posZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', hanger.radAng, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng -= DegreeToRad (90);
			elemList.Push (hanger.placeObject (4,
				"m_type", APIParT_CString, "�����������",
				"c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str ()));
			hanger.radAng += DegreeToRad (90);
			moveIn3D ('z', hanger.radAng, -0.150 + placingZone->cellsT1 [xx].height - 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng -= DegreeToRad (90);
			elemList.Push (hanger.placeObject (4,
				"m_type", APIParT_CString, "�����������",
				"c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str ()));
			hanger.radAng += DegreeToRad (90);
			moveIn3D ('z', hanger.radAng, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
		}
		moveIn3D ('z', hanger.radAng, -heightOfFormArea, &hanger.posX, &hanger.posY, &hanger.posZ);
		moveIn3D ('y', hanger.radAng, -yLen*2, &hanger.posX, &hanger.posY, &hanger.posZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', hanger.radAng, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (90);
			elemList.Push (hanger.placeObject (4,
				"m_type", APIParT_CString, "�����������",
				"c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str ()));
			hanger.radAng -= DegreeToRad (90);
			moveIn3D ('z', hanger.radAng, -0.150 + placingZone->cellsT1 [xx].height - 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
			hanger.radAng += DegreeToRad (90);
			elemList.Push (hanger.placeObject (4,
				"m_type", APIParT_CString, "�����������",
				"c_ag", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str (),
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)).c_str (),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)).c_str ()));
			hanger.radAng -= DegreeToRad (90);
			moveIn3D ('z', hanger.radAng, 0.150, &hanger.posX, &hanger.posY, &hanger.posZ);
		}
	}

	// 3. ����ǽ� ��ġ
	if (placingZone->bUseOutcornerPanel == true) {
		// �ƿ��ڳ��ǳ��� ���, �ƿ��ڳ��ǳ��� �ʺ� �����
		// ����
		headpiece.init (L("������Ʈ�� Push-Pull Props ����ǽ� v1.0.gsm"), layerInd_Head, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

		moveIn3D ('x', headpiece.radAng, -xLen + placingZone->cellsLT [0].horLen + 0.0475, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, yLen + 0.0635 + 0.155, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('z', headpiece.radAng, 0.300 - 0.040, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng += DegreeToRad (180);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng -= DegreeToRad (180);
		moveIn3D ('z', headpiece.radAng, -0.300 + elev_headpiece, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng += DegreeToRad (180);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng -= DegreeToRad (180);
		moveIn3D ('z', headpiece.radAng, -elev_headpiece + 0.300, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('x', headpiece.radAng, -placingZone->cellsLT [0].horLen + xLen*2 - placingZone->cellsRT [0].horLen, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng += DegreeToRad (180);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng -= DegreeToRad (180);
		moveIn3D ('z', headpiece.radAng, -0.300 + elev_headpiece, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng += DegreeToRad (180);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng -= DegreeToRad (180);

		// �Ʒ���
		headpiece.init (L("������Ʈ�� Push-Pull Props ����ǽ� v1.0.gsm"), layerInd_Head, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

		moveIn3D ('x', headpiece.radAng, -xLen + placingZone->cellsLT [0].horLen - 0.0475, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, -(yLen + 0.0635 + 0.155), &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('z', headpiece.radAng, 0.300 - 0.040, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('z', headpiece.radAng, -0.300 + elev_headpiece, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('z', headpiece.radAng, -elev_headpiece + 0.300, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('x', headpiece.radAng, -placingZone->cellsLT [0].horLen + xLen*2 - placingZone->cellsRT [0].horLen, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('z', headpiece.radAng, -0.300 + elev_headpiece, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

		// ����
		headpiece.init (L("������Ʈ�� Push-Pull Props ����ǽ� v1.0.gsm"), layerInd_Head, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

		moveIn3D ('x', headpiece.radAng, -(xLen + 0.0635 + 0.155), &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, yLen - placingZone->cellsLT [0].verLen + 0.0475, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('z', headpiece.radAng, 0.300 - 0.040, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng -= DegreeToRad (90);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng += DegreeToRad (90);
		moveIn3D ('z', headpiece.radAng, -0.300 + elev_headpiece, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng -= DegreeToRad (90);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng += DegreeToRad (90);
		moveIn3D ('z', headpiece.radAng, -elev_headpiece + 0.300, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, placingZone->cellsLT [0].verLen - yLen*2 + placingZone->cellsLB [0].verLen, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng -= DegreeToRad (90);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng += DegreeToRad (90);
		moveIn3D ('z', headpiece.radAng, -0.300 + elev_headpiece, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng -= DegreeToRad (90);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng += DegreeToRad (90);

		// ������
		headpiece.init (L("������Ʈ�� Push-Pull Props ����ǽ� v1.0.gsm"), layerInd_Head, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

		moveIn3D ('x', headpiece.radAng, (xLen + 0.0635 + 0.155), &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, yLen - placingZone->cellsLT [0].verLen - 0.0475, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('z', headpiece.radAng, 0.300 - 0.040, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng += DegreeToRad (90);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng -= DegreeToRad (90);
		moveIn3D ('z', headpiece.radAng, -0.300 + elev_headpiece, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng += DegreeToRad (90);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng -= DegreeToRad (90);
		moveIn3D ('z', headpiece.radAng, -elev_headpiece + 0.300, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, placingZone->cellsLT [0].verLen - yLen*2 + placingZone->cellsLB [0].verLen, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng += DegreeToRad (90);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng -= DegreeToRad (90);
		moveIn3D ('z', headpiece.radAng, -0.300 + elev_headpiece, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng += DegreeToRad (90);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng -= DegreeToRad (90);
	} else {
		// �ƿ��ڳʾޱ��� ���, �ƿ��ڳ��ǳ��� �ʺ� ������� ����
		// ����
		headpiece.init (L("������Ʈ�� Push-Pull Props ����ǽ� v1.0.gsm"), layerInd_Head, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

		moveIn3D ('x', headpiece.radAng, -xLen + 0.0475, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, yLen + 0.0635 + 0.155, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('z', headpiece.radAng, 0.300 - 0.040, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng += DegreeToRad (180);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng -= DegreeToRad (180);
		moveIn3D ('z', headpiece.radAng, -0.300 + elev_headpiece, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng += DegreeToRad (180);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng -= DegreeToRad (180);
		moveIn3D ('z', headpiece.radAng, -elev_headpiece + 0.300, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('x', headpiece.radAng, xLen*2, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng += DegreeToRad (180);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng -= DegreeToRad (180);
		moveIn3D ('z', headpiece.radAng, -0.300 + elev_headpiece, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng += DegreeToRad (180);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng -= DegreeToRad (180);

		// �Ʒ���
		headpiece.init (L("������Ʈ�� Push-Pull Props ����ǽ� v1.0.gsm"), layerInd_Head, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

		moveIn3D ('x', headpiece.radAng, -xLen- 0.0475, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, -(yLen + 0.0635 + 0.155), &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('z', headpiece.radAng, 0.300 - 0.040, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('z', headpiece.radAng, -0.300 + elev_headpiece, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('z', headpiece.radAng, -elev_headpiece + 0.300, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('x', headpiece.radAng, xLen*2, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('z', headpiece.radAng, -0.300 + elev_headpiece, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

		// ����
		headpiece.init (L("������Ʈ�� Push-Pull Props ����ǽ� v1.0.gsm"), layerInd_Head, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

		moveIn3D ('x', headpiece.radAng, -(xLen + 0.0635 + 0.155), &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, yLen + 0.0475, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('z', headpiece.radAng, 0.300 - 0.040, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng -= DegreeToRad (90);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng += DegreeToRad (90);
		moveIn3D ('z', headpiece.radAng, -0.300 + elev_headpiece, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng -= DegreeToRad (90);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng += DegreeToRad (90);
		moveIn3D ('z', headpiece.radAng, -elev_headpiece + 0.300, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, -yLen*2, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng -= DegreeToRad (90);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng += DegreeToRad (90);
		moveIn3D ('z', headpiece.radAng, -0.300 + elev_headpiece, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng -= DegreeToRad (90);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng += DegreeToRad (90);

		// ������
		headpiece.init (L("������Ʈ�� Push-Pull Props ����ǽ� v1.0.gsm"), layerInd_Head, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

		moveIn3D ('x', headpiece.radAng, (xLen + 0.0635 + 0.155), &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, yLen - 0.0475, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('z', headpiece.radAng, 0.300 - 0.040, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng += DegreeToRad (90);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng -= DegreeToRad (90);
		moveIn3D ('z', headpiece.radAng, -0.300 + elev_headpiece, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng += DegreeToRad (90);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng -= DegreeToRad (90);
		moveIn3D ('z', headpiece.radAng, -elev_headpiece + 0.300, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, -yLen*2, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng += DegreeToRad (90);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng -= DegreeToRad (90);
		moveIn3D ('z', headpiece.radAng, -0.300 + elev_headpiece, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		headpiece.radAng += DegreeToRad (90);
		elemList.Push (headpiece.placeObject (4,
			"type", APIParT_CString, "Ÿ�� A",
			"plateThk", APIParT_Length, format_string ("%f", 0.009),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		headpiece.radAng -= DegreeToRad (90);
	}

	// 4. ��չ�� �Ǵ� ���� ��ġ
	if (placingZone->typeOfColumnBand == 1) {
		columnBand1.init (L("��չ��v2.0.gsm"), layerInd_ColumnBand, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

		moveIn3D ('x', columnBand1.radAng, -0.0035 + (placingZone->coreWidth + placingZone->venThick*2)/2, &columnBand1.posX, &columnBand1.posY, &columnBand1.posZ);
		moveIn3D ('y', columnBand1.radAng, -0.1535 - (placingZone->coreDepth + placingZone->venThick*2)/2, &columnBand1.posX, &columnBand1.posY, &columnBand1.posZ);
		moveIn3D ('z', columnBand1.radAng, 0.500, &columnBand1.posX, &columnBand1.posY, &columnBand1.posZ);
		elemList.Push (columnBand1.placeObject (4,
			"band_size", APIParT_CString, "80x40x1270",
			"c_w", APIParT_Length, format_string ("%f", placingZone->coreWidth + placingZone->venThick*2),
			"c_h", APIParT_Length, format_string ("%f", placingZone->coreDepth + placingZone->venThick*2),
			"addOffset", APIParT_Length, format_string ("%f", 0.050)));
		moveIn3D ('z', columnBand1.radAng, -0.500 + heightOfFormArea/2 - 0.900, &columnBand1.posX, &columnBand1.posY, &columnBand1.posZ);
		elemList.Push (columnBand1.placeObject (4,
			"band_size", APIParT_CString, "80x40x1270",
			"c_w", APIParT_Length, format_string ("%f", placingZone->coreWidth + placingZone->venThick*2),
			"c_h", APIParT_Length, format_string ("%f", placingZone->coreDepth + placingZone->venThick*2),
			"addOffset", APIParT_Length, format_string ("%f", 0.050)));
		moveIn3D ('z', columnBand1.radAng, 0.900, &columnBand1.posX, &columnBand1.posY, &columnBand1.posZ);
		elemList.Push (columnBand1.placeObject (4,
			"band_size", APIParT_CString, "80x40x1270",
			"c_w", APIParT_Length, format_string ("%f", placingZone->coreWidth + placingZone->venThick*2),
			"c_h", APIParT_Length, format_string ("%f", placingZone->coreDepth + placingZone->venThick*2),
			"addOffset", APIParT_Length, format_string ("%f", 0.050)));
		moveIn3D ('z', columnBand1.radAng, 0.900, &columnBand1.posX, &columnBand1.posY, &columnBand1.posZ);
		elemList.Push (columnBand1.placeObject (4,
			"band_size", APIParT_CString, "80x40x1270",
			"c_w", APIParT_Length, format_string ("%f", placingZone->coreWidth + placingZone->venThick*2),
			"c_h", APIParT_Length, format_string ("%f", placingZone->coreDepth + placingZone->venThick*2),
			"addOffset", APIParT_Length, format_string ("%f", 0.050)));
	} else if (placingZone->typeOfColumnBand == 2) {
		columnBand2.init (L("����v1.0.gsm"), layerInd_ColumnBand, infoColumn.floorInd, placingZone->origoPos.x, placingZone->origoPos.y, placingZone->bottomOffset, placingZone->angle);

		moveIn3D ('z', columnBand2.radAng, 0.500, &columnBand2.posX, &columnBand2.posY, &columnBand2.posZ);
		elemList.Push (columnBand2.placeObject (5,
			"gap", APIParT_Length, "0.100",
			"nutType", APIParT_CString, "����Ʈ",
			"c_w", APIParT_Length, format_string ("%f", placingZone->coreWidth + placingZone->venThick*2),
			"c_h", APIParT_Length, format_string ("%f", placingZone->coreDepth + placingZone->venThick*2),
			"addOffset", APIParT_Length, format_string ("%f", 0.050)));
		moveIn3D ('z', columnBand2.radAng, -0.500 + heightOfFormArea/2 - 0.900, &columnBand2.posX, &columnBand2.posY, &columnBand2.posZ);
		elemList.Push (columnBand2.placeObject (5,
			"gap", APIParT_Length, "0.100",
			"nutType", APIParT_CString, "����Ʈ",
			"c_w", APIParT_Length, format_string ("%f", placingZone->coreWidth + placingZone->venThick*2),
			"c_h", APIParT_Length, format_string ("%f", placingZone->coreDepth + placingZone->venThick*2),
			"addOffset", APIParT_Length, format_string ("%f", 0.050)));
		moveIn3D ('z', columnBand2.radAng, 0.900, &columnBand2.posX, &columnBand2.posY, &columnBand2.posZ);
		elemList.Push (columnBand2.placeObject (5,
			"gap", APIParT_Length, "0.100",
			"nutType", APIParT_CString, "����Ʈ",
			"c_w", APIParT_Length, format_string ("%f", placingZone->coreWidth + placingZone->venThick*2),
			"c_h", APIParT_Length, format_string ("%f", placingZone->coreDepth + placingZone->venThick*2),
			"addOffset", APIParT_Length, format_string ("%f", 0.050)));
		moveIn3D ('z', columnBand2.radAng, 0.900, &columnBand2.posX, &columnBand2.posY, &columnBand2.posZ);
		elemList.Push (columnBand2.placeObject (5,
			"gap", APIParT_Length, "0.100",
			"nutType", APIParT_CString, "����Ʈ",
			"c_w", APIParT_Length, format_string ("%f", placingZone->coreWidth + placingZone->venThick*2),
			"c_h", APIParT_Length, format_string ("%f", placingZone->coreDepth + placingZone->venThick*2),
			"addOffset", APIParT_Length, format_string ("%f", 0.050)));
	}

	return	err;
}

// ������/�ƿ��ڳ��ǳ��� ä�� �� ������ ���� ä��� (�������� �������� ä��)
GSErrCode	ColumnTableformPlacingZone::fillRestAreas_soleColumn (ColumnTableformPlacingZone* placingZone)
{
	GSErrCode	err = NoError;

	short		xx;
	API_Coord3D	rotatedPoint;
	double		lineLen;
	double		xLen, yLen;
	double		dx, dy;
	double		leftLenOfBeam, rightLenOfBeam;

	double		heightOfFormArea = 0.0;
	double		columnWidth;
	double		marginHeight;

	EasyObjectPlacement		plywood;

	// ���� ���� ���̸� ����
	for (xx = 0 ; xx < placingZone->nCells ; ++xx)
		heightOfFormArea += placingZone->cellsB1 [xx].height;

	// ����
	if (placingZone->bFillMarginTopAtNorth == true) {
		xLen = (placingZone->coreWidth/2 + placingZone->venThick);
		yLen = (placingZone->coreDepth/2 + placingZone->venThick);
		lineLen = sqrt (xLen*xLen + yLen*yLen);
		rotatedPoint.x = placingZone->origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone->angle);
		rotatedPoint.y = placingZone->origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone->angle);
		rotatedPoint.z = placingZone->bottomOffset;

		columnWidth = placingZone->coreWidth + placingZone->venThick*2;
		marginHeight = placingZone->marginTopAtNorth;

		if (placingZone->bExistBeams [NORTH] == true) {
			dx = placingZone->beams [NORTH].endC.x - placingZone->beams [NORTH].begC.x;
			dy = placingZone->beams [NORTH].endC.y - placingZone->beams [NORTH].begC.y;
			if (RadToDegree (atan2 (dy, dx)) >= 0 && RadToDegree (atan2 (dy, dx)) < 180)
				leftLenOfBeam = distOfPointBetweenLine (rotatedPoint, placingZone->beams [NORTH].begC, placingZone->beams [NORTH].endC) - placingZone->beams [NORTH].width/2 + placingZone->beams [NORTH].offset;
			else
				leftLenOfBeam = distOfPointBetweenLine (rotatedPoint, placingZone->beams [NORTH].begC, placingZone->beams [NORTH].endC) - placingZone->beams [NORTH].width/2 - placingZone->beams [NORTH].offset;
			rightLenOfBeam = placingZone->coreWidth + placingZone->venThick*2 - placingZone->beams [NORTH].width - leftLenOfBeam;

			// ���� (����)
			plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoColumn.floorInd, rotatedPoint.x, rotatedPoint.y, rotatedPoint.z, placingZone->angle + DegreeToRad (180));
			moveIn3D ('z', placingZone->angle + DegreeToRad (180), heightOfFormArea, &plywood.posX, &plywood.posY, &plywood.posZ);
			elemList.Push (plywood.placeObject (13,
				"p_stan", APIParT_CString, "��԰�",
				"w_dir", APIParT_CString, "�������",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", leftLenOfBeam),
				"p_leng", APIParT_Length, format_string ("%f", placingZone->areaHeight - heightOfFormArea),
				"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "�Ұ�",
				"gap_a", APIParT_Length, format_string ("%f", 0.0),
				"gap_b", APIParT_Length, format_string ("%f", 0.0),
				"gap_c", APIParT_Length, format_string ("%f", 0.0),
				"gap_d", APIParT_Length, format_string ("%f", 0.0)));

			// ���� (�Ʒ���)
			plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoColumn.floorInd, rotatedPoint.x - leftLenOfBeam * cos(placingZone->angle), rotatedPoint.y - leftLenOfBeam * sin(placingZone->angle), rotatedPoint.z, placingZone->angle + DegreeToRad (180));
			moveIn3D ('z', placingZone->angle + DegreeToRad (180), heightOfFormArea, &plywood.posX, &plywood.posY, &plywood.posZ);
			elemList.Push (plywood.placeObject (13,
				"p_stan", APIParT_CString, "��԰�",
				"w_dir", APIParT_CString, "�������",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", placingZone->beams [NORTH].width),
				"p_leng", APIParT_Length, format_string ("%f", marginHeight),
				"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "�Ұ�",
				"gap_a", APIParT_Length, format_string ("%f", 0.0),
				"gap_b", APIParT_Length, format_string ("%f", 0.0),
				"gap_c", APIParT_Length, format_string ("%f", 0.0),
				"gap_d", APIParT_Length, format_string ("%f", 0.0)));

			// ���� (������)
			plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoColumn.floorInd, rotatedPoint.x - (leftLenOfBeam + placingZone->beams [NORTH].width) * cos(placingZone->angle), rotatedPoint.y - (leftLenOfBeam + placingZone->beams [NORTH].width) * sin(placingZone->angle), rotatedPoint.z, placingZone->angle + DegreeToRad (180));
			moveIn3D ('z', placingZone->angle + DegreeToRad (180), heightOfFormArea, &plywood.posX, &plywood.posY, &plywood.posZ);
			elemList.Push (plywood.placeObject (13,
				"p_stan", APIParT_CString, "��԰�",
				"w_dir", APIParT_CString, "�������",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", rightLenOfBeam + 0.0615),
				"p_leng", APIParT_Length, format_string ("%f", placingZone->areaHeight - heightOfFormArea),
				"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "�Ұ�",
				"gap_a", APIParT_Length, format_string ("%f", 0.0),
				"gap_b", APIParT_Length, format_string ("%f", 0.0),
				"gap_c", APIParT_Length, format_string ("%f", 0.0),
				"gap_d", APIParT_Length, format_string ("%f", 0.0)));
		} else {
			// ���� (��ü)
			plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoColumn.floorInd, rotatedPoint.x, rotatedPoint.y, rotatedPoint.z, placingZone->angle + DegreeToRad (180));
			moveIn3D ('z', placingZone->angle + DegreeToRad (180), heightOfFormArea, &plywood.posX, &plywood.posY, &plywood.posZ);
			elemList.Push (plywood.placeObject (13,
				"p_stan", APIParT_CString, "��԰�",
				"w_dir", APIParT_CString, "�������",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", columnWidth + 0.0615),
				"p_leng", APIParT_Length, format_string ("%f", marginHeight),
				"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "�Ұ�",
				"gap_a", APIParT_Length, format_string ("%f", 0.0),
				"gap_b", APIParT_Length, format_string ("%f", 0.0),
				"gap_c", APIParT_Length, format_string ("%f", 0.0),
				"gap_d", APIParT_Length, format_string ("%f", 0.0)));
		}
	}

	// ����
	if (placingZone->bFillMarginTopAtSouth == true) {
		xLen = -(placingZone->coreWidth/2 + placingZone->venThick);
		yLen = -(placingZone->coreDepth/2 + placingZone->venThick);
		lineLen = sqrt (xLen*xLen + yLen*yLen);
		rotatedPoint.x = placingZone->origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone->angle);
		rotatedPoint.y = placingZone->origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone->angle);
		rotatedPoint.z = placingZone->bottomOffset;

		columnWidth = placingZone->coreWidth + placingZone->venThick*2;
		marginHeight = placingZone->marginTopAtSouth;

		if (placingZone->bExistBeams [SOUTH] == true) {
			dx = placingZone->beams [SOUTH].endC.x - placingZone->beams [SOUTH].begC.x;
			dy = placingZone->beams [SOUTH].endC.y - placingZone->beams [SOUTH].begC.y;
			if (RadToDegree (atan2 (dy, dx)) >= 0 && RadToDegree (atan2 (dy, dx)) < 180)
				leftLenOfBeam = distOfPointBetweenLine (rotatedPoint, placingZone->beams [SOUTH].begC, placingZone->beams [SOUTH].endC) - placingZone->beams [SOUTH].width/2 - placingZone->beams [SOUTH].offset;
			else
				leftLenOfBeam = distOfPointBetweenLine (rotatedPoint, placingZone->beams [SOUTH].begC, placingZone->beams [SOUTH].endC) - placingZone->beams [SOUTH].width/2 + placingZone->beams [SOUTH].offset;
			rightLenOfBeam = placingZone->coreWidth + placingZone->venThick*2 - placingZone->beams [SOUTH].width - leftLenOfBeam;

			// ���� (����)
			plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoColumn.floorInd, rotatedPoint.x, rotatedPoint.y, rotatedPoint.z, placingZone->angle);
			moveIn3D ('z', placingZone->angle, heightOfFormArea, &plywood.posX, &plywood.posY, &plywood.posZ);
			elemList.Push (plywood.placeObject (13,
				"p_stan", APIParT_CString, "��԰�",
				"w_dir", APIParT_CString, "�������",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", leftLenOfBeam),
				"p_leng", APIParT_Length, format_string ("%f", placingZone->areaHeight - heightOfFormArea),
				"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "�Ұ�",
				"gap_a", APIParT_Length, format_string ("%f", 0.0),
				"gap_b", APIParT_Length, format_string ("%f", 0.0),
				"gap_c", APIParT_Length, format_string ("%f", 0.0),
				"gap_d", APIParT_Length, format_string ("%f", 0.0)));

			// ���� (�Ʒ���)
			plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoColumn.floorInd, rotatedPoint.x + leftLenOfBeam * cos(placingZone->angle), rotatedPoint.y + leftLenOfBeam * sin(placingZone->angle), rotatedPoint.z, placingZone->angle);
			moveIn3D ('z', placingZone->angle, heightOfFormArea, &plywood.posX, &plywood.posY, &plywood.posZ);
			elemList.Push (plywood.placeObject (13,
				"p_stan", APIParT_CString, "��԰�",
				"w_dir", APIParT_CString, "�������",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", placingZone->beams [SOUTH].width),
				"p_leng", APIParT_Length, format_string ("%f", marginHeight),
				"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "�Ұ�",
				"gap_a", APIParT_Length, format_string ("%f", 0.0),
				"gap_b", APIParT_Length, format_string ("%f", 0.0),
				"gap_c", APIParT_Length, format_string ("%f", 0.0),
				"gap_d", APIParT_Length, format_string ("%f", 0.0)));

			// ���� (������)
			plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoColumn.floorInd, rotatedPoint.x + (leftLenOfBeam + placingZone->beams [SOUTH].width) * cos(placingZone->angle), rotatedPoint.y + (leftLenOfBeam + placingZone->beams [SOUTH].width) * sin(placingZone->angle), rotatedPoint.z, placingZone->angle);
			moveIn3D ('z', placingZone->angle, heightOfFormArea, &plywood.posX, &plywood.posY, &plywood.posZ);
			elemList.Push (plywood.placeObject (13,
				"p_stan", APIParT_CString, "��԰�",
				"w_dir", APIParT_CString, "�������",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", rightLenOfBeam + 0.0615),
				"p_leng", APIParT_Length, format_string ("%f", placingZone->areaHeight - heightOfFormArea),
				"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "�Ұ�",
				"gap_a", APIParT_Length, format_string ("%f", 0.0),
				"gap_b", APIParT_Length, format_string ("%f", 0.0),
				"gap_c", APIParT_Length, format_string ("%f", 0.0),
				"gap_d", APIParT_Length, format_string ("%f", 0.0)));
		} else {
			// ���� (��ü)
			plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoColumn.floorInd, rotatedPoint.x, rotatedPoint.y, rotatedPoint.z, placingZone->angle);
			moveIn3D ('z', placingZone->angle, heightOfFormArea, &plywood.posX, &plywood.posY, &plywood.posZ);
			elemList.Push (plywood.placeObject (13,
				"p_stan", APIParT_CString, "��԰�",
				"w_dir", APIParT_CString, "�������",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", columnWidth + 0.0615),
				"p_leng", APIParT_Length, format_string ("%f", marginHeight),
				"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "�Ұ�",
				"gap_a", APIParT_Length, format_string ("%f", 0.0),
				"gap_b", APIParT_Length, format_string ("%f", 0.0),
				"gap_c", APIParT_Length, format_string ("%f", 0.0),
				"gap_d", APIParT_Length, format_string ("%f", 0.0)));
		}
	}

	// ����
	if (placingZone->bFillMarginTopAtWest == true) {
		xLen = -(placingZone->coreWidth/2 + placingZone->venThick);
		yLen = (placingZone->coreDepth/2 + placingZone->venThick);
		lineLen = sqrt (xLen*xLen + yLen*yLen);
		rotatedPoint.x = placingZone->origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone->angle);
		rotatedPoint.y = placingZone->origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone->angle);
		rotatedPoint.z = placingZone->bottomOffset;

		columnWidth = placingZone->coreDepth + placingZone->venThick*2;
		marginHeight = placingZone->marginTopAtWest;

		if (placingZone->bExistBeams [WEST] == true) {
			dx = placingZone->beams [WEST].endC.x - placingZone->beams [WEST].begC.x;
			dy = placingZone->beams [WEST].endC.y - placingZone->beams [WEST].begC.y;
			if (RadToDegree (atan2 (dy, dx)) >= 0 && RadToDegree (atan2 (dy, dx)) < 180)
				leftLenOfBeam = distOfPointBetweenLine (rotatedPoint, placingZone->beams [WEST].begC, placingZone->beams [WEST].endC) - placingZone->beams [WEST].width/2 - placingZone->beams [WEST].offset;
			else
				leftLenOfBeam = distOfPointBetweenLine (rotatedPoint, placingZone->beams [WEST].begC, placingZone->beams [WEST].endC) - placingZone->beams [WEST].width/2 + placingZone->beams [WEST].offset;
			rightLenOfBeam = placingZone->coreDepth + placingZone->venThick*2 - placingZone->beams [WEST].width - leftLenOfBeam;

			// ���� (����)
			plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoColumn.floorInd, rotatedPoint.x, rotatedPoint.y, rotatedPoint.z, placingZone->angle - DegreeToRad (90));
			moveIn3D ('z', placingZone->angle - DegreeToRad (90), heightOfFormArea, &plywood.posX, &plywood.posY, &plywood.posZ);
			elemList.Push (plywood.placeObject (13,
				"p_stan", APIParT_CString, "��԰�",
				"w_dir", APIParT_CString, "�������",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", leftLenOfBeam),
				"p_leng", APIParT_Length, format_string ("%f", placingZone->areaHeight - heightOfFormArea),
				"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "�Ұ�",
				"gap_a", APIParT_Length, format_string ("%f", 0.0),
				"gap_b", APIParT_Length, format_string ("%f", 0.0),
				"gap_c", APIParT_Length, format_string ("%f", 0.0),
				"gap_d", APIParT_Length, format_string ("%f", 0.0)));

			// ���� (�Ʒ���)
			plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoColumn.floorInd, rotatedPoint.x + leftLenOfBeam * sin(placingZone->angle), rotatedPoint.y - leftLenOfBeam * cos(placingZone->angle), rotatedPoint.z, placingZone->angle - DegreeToRad (90));
			moveIn3D ('z', placingZone->angle - DegreeToRad (90), heightOfFormArea, &plywood.posX, &plywood.posY, &plywood.posZ);
			elemList.Push (plywood.placeObject (13,
				"p_stan", APIParT_CString, "��԰�",
				"w_dir", APIParT_CString, "�������",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", placingZone->beams [WEST].width),
				"p_leng", APIParT_Length, format_string ("%f", marginHeight),
				"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "�Ұ�",
				"gap_a", APIParT_Length, format_string ("%f", 0.0),
				"gap_b", APIParT_Length, format_string ("%f", 0.0),
				"gap_c", APIParT_Length, format_string ("%f", 0.0),
				"gap_d", APIParT_Length, format_string ("%f", 0.0)));

			// ���� (������)
			plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoColumn.floorInd, rotatedPoint.x + (leftLenOfBeam + placingZone->beams [WEST].width) * sin(placingZone->angle), rotatedPoint.y - (leftLenOfBeam + placingZone->beams [WEST].width) * cos(placingZone->angle), rotatedPoint.z, placingZone->angle - DegreeToRad (90));
			moveIn3D ('z', placingZone->angle - DegreeToRad (90), heightOfFormArea, &plywood.posX, &plywood.posY, &plywood.posZ);
			elemList.Push (plywood.placeObject (13,
				"p_stan", APIParT_CString, "��԰�",
				"w_dir", APIParT_CString, "�������",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", rightLenOfBeam + 0.0615),
				"p_leng", APIParT_Length, format_string ("%f", placingZone->areaHeight - heightOfFormArea),
				"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "�Ұ�",
				"gap_a", APIParT_Length, format_string ("%f", 0.0),
				"gap_b", APIParT_Length, format_string ("%f", 0.0),
				"gap_c", APIParT_Length, format_string ("%f", 0.0),
				"gap_d", APIParT_Length, format_string ("%f", 0.0)));
		} else {
			// ���� (��ü)
			plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoColumn.floorInd, rotatedPoint.x, rotatedPoint.y, rotatedPoint.z, placingZone->angle - DegreeToRad (90));
			moveIn3D ('z', placingZone->angle - DegreeToRad (90), heightOfFormArea, &plywood.posX, &plywood.posY, &plywood.posZ);
			elemList.Push (plywood.placeObject (13,
				"p_stan", APIParT_CString, "��԰�",
				"w_dir", APIParT_CString, "�������",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", columnWidth + 0.0615),
				"p_leng", APIParT_Length, format_string ("%f", marginHeight),
				"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "�Ұ�",
				"gap_a", APIParT_Length, format_string ("%f", 0.0),
				"gap_b", APIParT_Length, format_string ("%f", 0.0),
				"gap_c", APIParT_Length, format_string ("%f", 0.0),
				"gap_d", APIParT_Length, format_string ("%f", 0.0)));
		}
	}

	// ����
	if (placingZone->bFillMarginTopAtEast == true) {
		xLen = (placingZone->coreWidth/2 + placingZone->venThick);
		yLen = -(placingZone->coreDepth/2 + placingZone->venThick);
		lineLen = sqrt (xLen*xLen + yLen*yLen);
		rotatedPoint.x = placingZone->origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone->angle);
		rotatedPoint.y = placingZone->origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone->angle);
		rotatedPoint.z = placingZone->bottomOffset;

		columnWidth = placingZone->coreDepth + placingZone->venThick*2;
		marginHeight = placingZone->marginTopAtEast;

		if (placingZone->bExistBeams [EAST] == true) {
			dx = placingZone->beams [EAST].endC.x - placingZone->beams [EAST].begC.x;
			dy = placingZone->beams [EAST].endC.y - placingZone->beams [EAST].begC.y;
			if (RadToDegree (atan2 (dy, dx)) >= 0 && RadToDegree (atan2 (dy, dx)) < 180)
				leftLenOfBeam = distOfPointBetweenLine (rotatedPoint, placingZone->beams [EAST].begC, placingZone->beams [EAST].endC) - placingZone->beams [EAST].width/2 + placingZone->beams [EAST].offset;
			else
				leftLenOfBeam = distOfPointBetweenLine (rotatedPoint, placingZone->beams [EAST].begC, placingZone->beams [EAST].endC) - placingZone->beams [EAST].width/2 - placingZone->beams [EAST].offset;
			rightLenOfBeam = placingZone->coreDepth + placingZone->venThick*2 - placingZone->beams [EAST].width - leftLenOfBeam;

			// ���� (����)
			plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoColumn.floorInd, rotatedPoint.x, rotatedPoint.y, rotatedPoint.z, placingZone->angle + DegreeToRad (90));
			moveIn3D ('z', placingZone->angle + DegreeToRad (90), heightOfFormArea, &plywood.posX, &plywood.posY, &plywood.posZ);
			elemList.Push (plywood.placeObject (13,
				"p_stan", APIParT_CString, "��԰�",
				"w_dir", APIParT_CString, "�������",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", leftLenOfBeam),
				"p_leng", APIParT_Length, format_string ("%f", placingZone->areaHeight - heightOfFormArea),
				"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "�Ұ�",
				"gap_a", APIParT_Length, format_string ("%f", 0.0),
				"gap_b", APIParT_Length, format_string ("%f", 0.0),
				"gap_c", APIParT_Length, format_string ("%f", 0.0),
				"gap_d", APIParT_Length, format_string ("%f", 0.0)));

			// ���� (�Ʒ���)
			plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoColumn.floorInd, rotatedPoint.x - leftLenOfBeam * sin(placingZone->angle), rotatedPoint.y + leftLenOfBeam * cos(placingZone->angle), rotatedPoint.z, placingZone->angle + DegreeToRad (90));
			moveIn3D ('z', placingZone->angle + DegreeToRad (90), heightOfFormArea, &plywood.posX, &plywood.posY, &plywood.posZ);
			elemList.Push (plywood.placeObject (13,
				"p_stan", APIParT_CString, "��԰�",
				"w_dir", APIParT_CString, "�������",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", placingZone->beams [EAST].width),
				"p_leng", APIParT_Length, format_string ("%f", marginHeight),
				"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "�Ұ�",
				"gap_a", APIParT_Length, format_string ("%f", 0.0),
				"gap_b", APIParT_Length, format_string ("%f", 0.0),
				"gap_c", APIParT_Length, format_string ("%f", 0.0),
				"gap_d", APIParT_Length, format_string ("%f", 0.0)));

			// ���� (������)
			plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoColumn.floorInd, rotatedPoint.x - (leftLenOfBeam + placingZone->beams [EAST].width) * sin(placingZone->angle), rotatedPoint.y + (leftLenOfBeam + placingZone->beams [EAST].width) * cos(placingZone->angle), rotatedPoint.z, placingZone->angle + DegreeToRad (90));
			moveIn3D ('z', placingZone->angle + DegreeToRad (90), heightOfFormArea, &plywood.posX, &plywood.posY, &plywood.posZ);
			elemList.Push (plywood.placeObject (13,
				"p_stan", APIParT_CString, "��԰�",
				"w_dir", APIParT_CString, "�������",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", rightLenOfBeam + 0.0615),
				"p_leng", APIParT_Length, format_string ("%f", placingZone->areaHeight - heightOfFormArea),
				"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "�Ұ�",
				"gap_a", APIParT_Length, format_string ("%f", 0.0),
				"gap_b", APIParT_Length, format_string ("%f", 0.0),
				"gap_c", APIParT_Length, format_string ("%f", 0.0),
				"gap_d", APIParT_Length, format_string ("%f", 0.0)));
		} else {
			// ���� (��ü)
			plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoColumn.floorInd, rotatedPoint.x, rotatedPoint.y, rotatedPoint.z, placingZone->angle + DegreeToRad (90));
			moveIn3D ('z', placingZone->angle + DegreeToRad (90), heightOfFormArea, &plywood.posX, &plywood.posY, &plywood.posZ);
			elemList.Push (plywood.placeObject (13,
				"p_stan", APIParT_CString, "��԰�",
				"w_dir", APIParT_CString, "�������",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", columnWidth + 0.0615),
				"p_leng", APIParT_Length, format_string ("%f", marginHeight),
				"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "�Ұ�",
				"gap_a", APIParT_Length, format_string ("%f", 0.0),
				"gap_b", APIParT_Length, format_string ("%f", 0.0),
				"gap_c", APIParT_Length, format_string ("%f", 0.0),
				"gap_d", APIParT_Length, format_string ("%f", 0.0)));
		}
	}

	return	err;
}

// 1�� ��ġ�� ���� ���Ǹ� ��û�ϴ� 1�� ���̾�α�
short DGCALLBACK columnTableformPlacerHandler_soleColumn_1 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short		result;
	short		xx;
	API_UCCallbackType	ucb;

	API_Coord	rotatedPoint;
	double		lineLen;
	double		xLen, yLen;
	double		formWidth;

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, L"��տ� ��ġ - ��� �ܸ�");

			//////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			// Ȯ�� ��ư
			DGSetItemText (dialogID, DG_OK, L"Ȯ ��");

			// ��� ��ư
			DGSetItemText (dialogID, DG_CANCEL, L"�� ��");
			
			//////////////////////////////////////////////////////////// ������ ��ġ (������)
			// ���� ��ư
			DGSetItemText (dialogID, RADIO_OUTCORNER_PANEL, L"�ƿ��ڳ�\n�ǳ�");
			DGSetItemText (dialogID, RADIO_OUTCORNER_ANGLE, L"�ƿ��ڳ�\n�ޱ�");
			DGSetItemText (dialogID, RADIO_COLUMN_BAND_1, L"��չ��");
			DGSetItemText (dialogID, RADIO_COLUMN_BAND_2, L"����");

			// �� �� üũ�ڽ�/�˾���Ʈ��
			DGSetItemText (dialogID, LABEL_COLUMN_SECTION, L"��� �ܸ�");
			DGSetItemText (dialogID, LABEL_COLUMN_DEPTH, L"����");
			DGSetItemText (dialogID, LABEL_COLUMN_WIDTH, L"����");
			DGSetItemText (dialogID, LABEL_OUTCORNER, L"�ƿ��ڳ� ó��");
			DGSetItemText (dialogID, LABEL_COLUMN_BAND_TYPE, L"��չ�� Ÿ��");
			DGSetItemText (dialogID, LABEL_TABLEFORM_TYPE, L"���̺��� Ÿ��");

			// ��: ���̾� ����
			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, L"���纰 ���̾� ����");
			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM, L"������");
			DGSetItemText (dialogID, LABEL_LAYER_FILLERSP, L"�ٷ������̼�");
			DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER_PANEL, L"�ƿ��ڳ��ǳ�");
			DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER_ANGLE, L"�ƿ��ڳʾޱ�");
			DGSetItemText (dialogID, LABEL_LAYER_SQUARE_PIPE, L"���������");
			DGSetItemText (dialogID, LABEL_LAYER_PINBOLT, L"�ɺ�Ʈ��Ʈ");
			DGSetItemText (dialogID, LABEL_LAYER_HANGER, L"�����������");
			DGSetItemText (dialogID, LABEL_LAYER_HEADPIECE, L"����ǽ�");
			DGSetItemText (dialogID, LABEL_LAYER_COLUMN_BAND, L"��չ��");
			DGSetItemText (dialogID, LABEL_LAYER_PLYWOOD, L"����");
			DGSetItemText (dialogID, LABEL_LAYER_BLUECLAMP, L"���Ŭ����");

			// �˾���Ʈ��: ���̺��� Ÿ��
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM, L"Ÿ��A");
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM, L"Ÿ��B");
			DGPopUpSelectItem (dialogID, POPUP_TABLEFORM_TYPE, DG_POPUP_TOP);

			// üũ�ڽ�: ���̾� ����
			DGSetItemText (dialogID, CHECKBOX_LAYER_COUPLING, L"���̾� ����");
			DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, TRUE);

			DGSetItemText (dialogID, BUTTON_AUTOSET, L"���̾� �ڵ� ����");

			// ���� ��Ʈ�� �ʱ�ȭ
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_LAYER_EUROFORM;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, 1);

			ucb.itemID	 = USERCONTROL_LAYER_FILLERSP;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, 1);

			ucb.itemID	 = USERCONTROL_LAYER_OUTCORNER_PANEL;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, 1);

			ucb.itemID	 = USERCONTROL_LAYER_OUTCORNER_ANGLE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, 1);

			ucb.itemID	 = USERCONTROL_LAYER_SQUARE_PIPE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_SQUARE_PIPE, 1);

			ucb.itemID	 = USERCONTROL_LAYER_PINBOLT;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, 1);

			ucb.itemID	 = USERCONTROL_LAYER_HANGER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_HANGER, 1);

			ucb.itemID	 = USERCONTROL_LAYER_HEADPIECE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, 1);

			ucb.itemID	 = USERCONTROL_LAYER_COLUMN_BAND;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_COLUMN_BAND, 1);

			ucb.itemID	 = USERCONTROL_LAYER_PLYWOOD;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, 1);

			ucb.itemID	 = USERCONTROL_LAYER_BLUECLAMP;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUECLAMP, 1);

			// �ƿ��ڳʾޱ� ���̾� ���� ��Ȱ��ȭ
			DGDisableItem (dialogID, LABEL_LAYER_OUTCORNER_ANGLE);
			DGDisableItem (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE);
			DGDisableItem (dialogID, LABEL_LAYER_HANGER);
			DGDisableItem (dialogID, USERCONTROL_LAYER_HANGER);

			// ó������ �ƿ��ڳ��ǳ� ���� �۵� (�׸� �� ���� ��ư)
			DGShowItem (dialogID, ICON_COLUMN_SECTION_OUTCORNER_PANEL);
			DGHideItem (dialogID, ICON_COLUMN_SECTION_OUTCORNER_ANGLE);
			DGSetItemValLong (dialogID, RADIO_OUTCORNER_PANEL, TRUE);

			// ó������ ��չ�� ����
			DGSetItemValLong (dialogID, RADIO_COLUMN_BAND_1, TRUE);

			// ��� ����/���� ���
			DGSetItemValDouble (dialogID, EDITCONTROL_COLUMN_WIDTH, placingZone.coreWidth + placingZone.venThick * 2);
			DGSetItemValDouble (dialogID, EDITCONTROL_COLUMN_DEPTH, placingZone.coreDepth + placingZone.venThick * 2);
			DGDisableItem (dialogID, EDITCONTROL_COLUMN_WIDTH);
			DGDisableItem (dialogID, EDITCONTROL_COLUMN_DEPTH);

			// ���纰 üũ�ڽ�-�԰� ����
			(DGGetItemValLong (dialogID, CHECKBOX_LEFT_ADDITIONAL_FORM) == TRUE) ?		DGEnableItem (dialogID, EDITCONTROL_LEFT_4)		: 	DGDisableItem (dialogID, EDITCONTROL_LEFT_4);
			(DGGetItemValLong (dialogID, CHECKBOX_BOTTOM_ADDITIONAL_FORM) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_BOTTOM_4)	: 	DGDisableItem (dialogID, EDITCONTROL_BOTTOM_4);

			// ���� �����ؼ��� �� �Ǵ� �׸� ��ױ�
			DGDisableItem (dialogID, EDITCONTROL_TOP_1);
			DGDisableItem (dialogID, EDITCONTROL_TOP_2);
			DGDisableItem (dialogID, EDITCONTROL_TOP_3);
			DGDisableItem (dialogID, EDITCONTROL_TOP_4);
			DGDisableItem (dialogID, EDITCONTROL_TOP_5);
			DGDisableItem (dialogID, CHECKBOX_TOP_ADDITIONAL_FORM);
			DGDisableItem (dialogID, EDITCONTROL_RIGHT_1);
			DGDisableItem (dialogID, EDITCONTROL_RIGHT_2);
			DGDisableItem (dialogID, EDITCONTROL_RIGHT_3);
			DGDisableItem (dialogID, EDITCONTROL_RIGHT_4);
			DGDisableItem (dialogID, EDITCONTROL_RIGHT_5);
			DGDisableItem (dialogID, CHECKBOX_RIGHT_ADDITIONAL_FORM);

			// �⺻�� �Է��� ����
			DGSetItemValDouble (dialogID, EDITCONTROL_TOP_1, 0.100);
			DGSetItemValDouble (dialogID, EDITCONTROL_TOP_2, 0.300);
			DGSetItemValDouble (dialogID, EDITCONTROL_TOP_5, 0.100);
			DGSetItemValDouble (dialogID, EDITCONTROL_LEFT_1, 0.100);
			DGSetItemValDouble (dialogID, EDITCONTROL_LEFT_2, 0.300);
			DGSetItemValDouble (dialogID, EDITCONTROL_LEFT_5, 0.100);
			DGSetItemValDouble (dialogID, EDITCONTROL_RIGHT_1, 0.100);
			DGSetItemValDouble (dialogID, EDITCONTROL_RIGHT_2, 0.300);
			DGSetItemValDouble (dialogID, EDITCONTROL_RIGHT_5, 0.100);
			DGSetItemValDouble (dialogID, EDITCONTROL_BOTTOM_1, 0.100);
			DGSetItemValDouble (dialogID, EDITCONTROL_BOTTOM_2, 0.300);
			DGSetItemValDouble (dialogID, EDITCONTROL_BOTTOM_5, 0.100);

			break;
		
		case DG_MSG_CHANGE:

			if (DGGetItemValLong (dialogID, RADIO_OUTCORNER_PANEL) == TRUE) {
				// �ƿ��ڳ��ǳ� ����� ��� (�׸� ����, Edit ��Ʈ�� ��� �����ֱ�)
				DGShowItem (dialogID, ICON_COLUMN_SECTION_OUTCORNER_PANEL);
				DGHideItem (dialogID, ICON_COLUMN_SECTION_OUTCORNER_ANGLE);
				DGShowItem (dialogID, EDITCONTROL_TOP_1);
				DGShowItem (dialogID, EDITCONTROL_TOP_5);
				DGShowItem (dialogID, EDITCONTROL_LEFT_1);
				DGShowItem (dialogID, EDITCONTROL_LEFT_5);
				DGShowItem (dialogID, EDITCONTROL_RIGHT_1);
				DGShowItem (dialogID, EDITCONTROL_RIGHT_5);
				DGShowItem (dialogID, EDITCONTROL_BOTTOM_1);
				DGShowItem (dialogID, EDITCONTROL_BOTTOM_5);

				DGEnableItem (dialogID, LABEL_LAYER_OUTCORNER_PANEL);
				DGEnableItem (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL);
				DGEnableItem (dialogID, LABEL_LAYER_PINBOLT);
				DGEnableItem (dialogID, USERCONTROL_LAYER_PINBOLT);

				DGDisableItem (dialogID, LABEL_LAYER_OUTCORNER_ANGLE);
				DGDisableItem (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE);
				DGDisableItem (dialogID, LABEL_LAYER_HANGER);
				DGDisableItem (dialogID, USERCONTROL_LAYER_HANGER);

				DGEnableItem (dialogID, POPUP_TABLEFORM_TYPE);

			} else {
				// �ƿ��ڳʾޱ� ����� ��� (�׸� ����, Edit ��Ʈ�� �Ϻ� �����)
				DGHideItem (dialogID, ICON_COLUMN_SECTION_OUTCORNER_PANEL);
				DGShowItem (dialogID, ICON_COLUMN_SECTION_OUTCORNER_ANGLE);
				DGHideItem (dialogID, EDITCONTROL_TOP_1);
				DGHideItem (dialogID, EDITCONTROL_TOP_5);
				DGHideItem (dialogID, EDITCONTROL_LEFT_1);
				DGHideItem (dialogID, EDITCONTROL_LEFT_5);
				DGHideItem (dialogID, EDITCONTROL_RIGHT_1);
				DGHideItem (dialogID, EDITCONTROL_RIGHT_5);
				DGHideItem (dialogID, EDITCONTROL_BOTTOM_1);
				DGHideItem (dialogID, EDITCONTROL_BOTTOM_5);

				DGDisableItem (dialogID, LABEL_LAYER_OUTCORNER_PANEL);
				DGDisableItem (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL);
				DGDisableItem (dialogID, LABEL_LAYER_PINBOLT);
				DGDisableItem (dialogID, USERCONTROL_LAYER_PINBOLT);

				DGEnableItem (dialogID, LABEL_LAYER_OUTCORNER_ANGLE);
				DGEnableItem (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE);
				DGEnableItem (dialogID, LABEL_LAYER_HANGER);
				DGEnableItem (dialogID, USERCONTROL_LAYER_HANGER);

				// �ƿ��ڳʾޱ��� ��� Ÿ��B�� ������
				DGPopUpSelectItem (dialogID, POPUP_TABLEFORM_TYPE, 2);
				DGDisableItem (dialogID, POPUP_TABLEFORM_TYPE);
			}

			// ���纰 üũ�ڽ�-�԰� ����
			if (DGGetItemValLong (dialogID, CHECKBOX_LEFT_ADDITIONAL_FORM) == TRUE) {
				DGEnableItem (dialogID, EDITCONTROL_LEFT_4);
				DGSetItemValLong (dialogID, CHECKBOX_RIGHT_ADDITIONAL_FORM, TRUE);
			} else {
				DGDisableItem (dialogID, EDITCONTROL_LEFT_4);
				DGSetItemValDouble (dialogID, EDITCONTROL_LEFT_4, 0.0);
				DGSetItemValDouble (dialogID, EDITCONTROL_RIGHT_4, 0.0);
				DGSetItemValLong (dialogID, CHECKBOX_RIGHT_ADDITIONAL_FORM, FALSE);
			}
			if (DGGetItemValLong (dialogID, CHECKBOX_BOTTOM_ADDITIONAL_FORM) == TRUE) {
				DGEnableItem (dialogID, EDITCONTROL_BOTTOM_4);
				DGSetItemValLong (dialogID, CHECKBOX_TOP_ADDITIONAL_FORM, TRUE);
			} else {
				DGDisableItem (dialogID, EDITCONTROL_BOTTOM_4);
				DGSetItemValDouble (dialogID, EDITCONTROL_BOTTOM_4, 0.0);
				DGSetItemValDouble (dialogID, EDITCONTROL_TOP_4, 0.0);
				DGSetItemValLong (dialogID, CHECKBOX_TOP_ADDITIONAL_FORM, FALSE);
			}

			DGSetItemValDouble (dialogID, EDITCONTROL_RIGHT_1, DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_1));
			DGSetItemValDouble (dialogID, EDITCONTROL_RIGHT_2, DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_2));
			DGSetItemValDouble (dialogID, EDITCONTROL_RIGHT_3, DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_3));
			DGSetItemValDouble (dialogID, EDITCONTROL_RIGHT_4, DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_4));
			DGSetItemValDouble (dialogID, EDITCONTROL_RIGHT_5, DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_5));

			DGSetItemValDouble (dialogID, EDITCONTROL_TOP_1, DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_1));
			DGSetItemValDouble (dialogID, EDITCONTROL_TOP_2, DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_2));
			DGSetItemValDouble (dialogID, EDITCONTROL_TOP_3, DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_3));
			DGSetItemValDouble (dialogID, EDITCONTROL_TOP_4, DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_4));
			DGSetItemValDouble (dialogID, EDITCONTROL_TOP_5, DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_5));

			// ���̾� ���� �ٲ�
			if ((item >= USERCONTROL_LAYER_EUROFORM) && (item <= USERCONTROL_LAYER_BLUECLAMP)) {
				if (DGGetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING) == 1) {
					long selectedLayer;

					selectedLayer = DGGetItemValLong (dialogID, item);

					for (xx = USERCONTROL_LAYER_EUROFORM ; xx <= USERCONTROL_LAYER_BLUECLAMP ; ++xx)
						DGSetItemValLong (dialogID, xx, selectedLayer);
				}
			}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					//////////////////////////////////////// ���̾�α� â ������ �Է� ����
					// �� ���� ����
					for (xx = 0 ; xx < 20 ; ++xx) {
						if (DGGetItemValLong (dialogID, RADIO_OUTCORNER_PANEL) == TRUE) {
							placingZone.bUseOutcornerPanel = true;

							// �»��
							xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
							yLen = (placingZone.coreDepth/2 + placingZone.venThick);
							lineLen = sqrt (xLen*xLen + yLen*yLen);
							rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
							rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

							placingZone.cellsLT [xx].objType = OUTPANEL;
							placingZone.cellsLT [xx].leftBottomX = rotatedPoint.x;
							placingZone.cellsLT [xx].leftBottomY = rotatedPoint.y;
							placingZone.cellsLT [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
							placingZone.cellsLT [xx].ang = placingZone.angle - DegreeToRad (90);
							placingZone.cellsLT [xx].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_TOP_1);
							placingZone.cellsLT [xx].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_1);
							placingZone.cellsLT [xx].height = 1.200;

							// ����
							xLen = (placingZone.coreWidth/2 + placingZone.venThick);
							yLen = (placingZone.coreDepth/2 + placingZone.venThick);
							lineLen = sqrt (xLen*xLen + yLen*yLen);
							rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
							rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

							placingZone.cellsRT [xx].objType = OUTPANEL;
							placingZone.cellsRT [xx].leftBottomX = rotatedPoint.x;
							placingZone.cellsRT [xx].leftBottomY = rotatedPoint.y;
							placingZone.cellsRT [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
							placingZone.cellsRT [xx].ang = placingZone.angle + DegreeToRad (180);
							placingZone.cellsRT [xx].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_TOP_5);
							placingZone.cellsRT [xx].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_1);
							placingZone.cellsRT [xx].height = 1.200;

							// ���ϴ�
							xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
							yLen = -(placingZone.coreDepth/2 + placingZone.venThick);
							lineLen = sqrt (xLen*xLen + yLen*yLen);
							rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
							rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

							placingZone.cellsLB [xx].objType = OUTPANEL;
							placingZone.cellsLB [xx].leftBottomX = rotatedPoint.x;
							placingZone.cellsLB [xx].leftBottomY = rotatedPoint.y;
							placingZone.cellsLB [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
							placingZone.cellsLB [xx].ang = placingZone.angle;
							placingZone.cellsLB [xx].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_1);
							placingZone.cellsLB [xx].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_5);
							placingZone.cellsLB [xx].height = 1.200;

							// ���ϴ�
							xLen = (placingZone.coreWidth/2 + placingZone.venThick);
							yLen = -(placingZone.coreDepth/2 + placingZone.venThick);
							lineLen = sqrt (xLen*xLen + yLen*yLen);
							rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
							rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

							placingZone.cellsRB [xx].objType = OUTPANEL;
							placingZone.cellsRB [xx].leftBottomX = rotatedPoint.x;
							placingZone.cellsRB [xx].leftBottomY = rotatedPoint.y;
							placingZone.cellsRB [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
							placingZone.cellsRB [xx].ang = placingZone.angle + DegreeToRad (90);
							placingZone.cellsRB [xx].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_5);
							placingZone.cellsRB [xx].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_5);
							placingZone.cellsRB [xx].height = 1.200;
						} else {
							placingZone.bUseOutcornerPanel = false;

							// �»��
							xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
							yLen = (placingZone.coreDepth/2 + placingZone.venThick);
							lineLen = sqrt (xLen*xLen + yLen*yLen);
							rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
							rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

							placingZone.cellsLT [xx].objType = OUTANGLE;
							placingZone.cellsLT [xx].leftBottomX = rotatedPoint.x;
							placingZone.cellsLT [xx].leftBottomY = rotatedPoint.y;
							placingZone.cellsLT [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
							placingZone.cellsLT [xx].ang = placingZone.angle + DegreeToRad (90);
							placingZone.cellsLT [xx].horLen = 0.0635;
							placingZone.cellsLT [xx].verLen = 0.0635;
							placingZone.cellsLT [xx].height = 1.200;

							// ����
							xLen = (placingZone.coreWidth/2 + placingZone.venThick);
							yLen = (placingZone.coreDepth/2 + placingZone.venThick);
							lineLen = sqrt (xLen*xLen + yLen*yLen);
							rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
							rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

							placingZone.cellsRT [xx].objType = OUTANGLE;
							placingZone.cellsRT [xx].leftBottomX = rotatedPoint.x;
							placingZone.cellsRT [xx].leftBottomY = rotatedPoint.y;
							placingZone.cellsRT [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
							placingZone.cellsRT [xx].ang = placingZone.angle;
							placingZone.cellsRT [xx].horLen = 0.0635;
							placingZone.cellsRT [xx].verLen = 0.0635;
							placingZone.cellsRT [xx].height = 1.200;

							// ���ϴ�
							xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
							yLen = -(placingZone.coreDepth/2 + placingZone.venThick);
							lineLen = sqrt (xLen*xLen + yLen*yLen);
							rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
							rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

							placingZone.cellsLB [xx].objType = OUTANGLE;
							placingZone.cellsLB [xx].leftBottomX = rotatedPoint.x;
							placingZone.cellsLB [xx].leftBottomY = rotatedPoint.y;
							placingZone.cellsLB [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
							placingZone.cellsLB [xx].ang = placingZone.angle + DegreeToRad (180);
							placingZone.cellsLB [xx].horLen = 0.0635;
							placingZone.cellsLB [xx].verLen = 0.0635;
							placingZone.cellsLB [xx].height = 1.200;

							// ���ϴ�
							xLen = (placingZone.coreWidth/2 + placingZone.venThick);
							yLen = -(placingZone.coreDepth/2 + placingZone.venThick);
							lineLen = sqrt (xLen*xLen + yLen*yLen);
							rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
							rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

							placingZone.cellsRB [xx].objType = OUTANGLE;
							placingZone.cellsRB [xx].leftBottomX = rotatedPoint.x;
							placingZone.cellsRB [xx].leftBottomY = rotatedPoint.y;
							placingZone.cellsRB [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
							placingZone.cellsRB [xx].ang = placingZone.angle + DegreeToRad (270);
							placingZone.cellsRB [xx].horLen = 0.0635;
							placingZone.cellsRB [xx].verLen = 0.0635;
							placingZone.cellsRB [xx].height = 1.200;
						}

						// ���� 1
						xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
						yLen = (placingZone.coreDepth/2 + placingZone.venThick);
						lineLen = sqrt (xLen*xLen + yLen*yLen);
						rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
						rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

						placingZone.cellsT1 [xx].objType = EUROFORM;
						if (DGGetItemValLong (dialogID, RADIO_OUTCORNER_PANEL) == TRUE) {
							placingZone.cellsT1 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, EDITCONTROL_TOP_1) + DGGetItemValDouble (dialogID, EDITCONTROL_TOP_2)) * cos(placingZone.angle);
							placingZone.cellsT1 [xx].leftBottomY = rotatedPoint.y + (DGGetItemValDouble (dialogID, EDITCONTROL_TOP_1) + DGGetItemValDouble (dialogID, EDITCONTROL_TOP_2)) * sin(placingZone.angle);
						} else {
							placingZone.cellsT1 [xx].leftBottomX = rotatedPoint.x + DGGetItemValDouble (dialogID, EDITCONTROL_TOP_2) * cos(placingZone.angle);
							placingZone.cellsT1 [xx].leftBottomY = rotatedPoint.y + DGGetItemValDouble (dialogID, EDITCONTROL_TOP_2) * sin(placingZone.angle);
						}
						placingZone.cellsT1 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
						placingZone.cellsT1 [xx].ang = placingZone.angle + DegreeToRad (180);
						placingZone.cellsT1 [xx].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_TOP_2);
						placingZone.cellsT1 [xx].verLen = 0.064;
						placingZone.cellsT1 [xx].height = 1.200;
						formWidth = DGGetItemValDouble (dialogID, EDITCONTROL_TOP_2);
						if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
							placingZone.cellsT1 [xx].bStandardEuroform = true;
						else
							placingZone.cellsT1 [xx].bStandardEuroform = false;

						// ���� 2
						xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
						yLen = (placingZone.coreDepth/2 + placingZone.venThick);
						lineLen = sqrt (xLen*xLen + yLen*yLen);
						rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
						rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

						placingZone.cellsT2 [xx].objType = FILLERSP;
						if (DGGetItemValLong (dialogID, RADIO_OUTCORNER_PANEL) == TRUE) {
							placingZone.cellsT2 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, EDITCONTROL_TOP_1) + DGGetItemValDouble (dialogID, EDITCONTROL_TOP_2) + DGGetItemValDouble (dialogID, EDITCONTROL_TOP_3)) * cos(placingZone.angle);
							placingZone.cellsT2 [xx].leftBottomY = rotatedPoint.y + (DGGetItemValDouble (dialogID, EDITCONTROL_TOP_1) + DGGetItemValDouble (dialogID, EDITCONTROL_TOP_2) + DGGetItemValDouble (dialogID, EDITCONTROL_TOP_3)) * sin(placingZone.angle);
						} else {
							placingZone.cellsT2 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, EDITCONTROL_TOP_2) + DGGetItemValDouble (dialogID, EDITCONTROL_TOP_3)) * cos(placingZone.angle);
							placingZone.cellsT2 [xx].leftBottomY = rotatedPoint.y + (DGGetItemValDouble (dialogID, EDITCONTROL_TOP_2) + DGGetItemValDouble (dialogID, EDITCONTROL_TOP_3)) * sin(placingZone.angle);
						}
						placingZone.cellsT2 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
						placingZone.cellsT2 [xx].ang = placingZone.angle + DegreeToRad (180);
						placingZone.cellsT2 [xx].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_TOP_3);
						placingZone.cellsT2 [xx].verLen = 0.064;
						placingZone.cellsT2 [xx].height = 1.200;
						formWidth = DGGetItemValDouble (dialogID, EDITCONTROL_TOP_3);
						placingZone.cellsT2 [xx].bStandardEuroform = false;

						// ���� 3
						if (DGGetItemValLong (dialogID, CHECKBOX_TOP_ADDITIONAL_FORM) == TRUE) {
							xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
							yLen = (placingZone.coreDepth/2 + placingZone.venThick);
							lineLen = sqrt (xLen*xLen + yLen*yLen);
							rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
							rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

							placingZone.cellsT3 [xx].objType = EUROFORM;
							if (DGGetItemValLong (dialogID, RADIO_OUTCORNER_PANEL) == TRUE) {
								placingZone.cellsT3 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, EDITCONTROL_TOP_1) + DGGetItemValDouble (dialogID, EDITCONTROL_TOP_2) + DGGetItemValDouble (dialogID, EDITCONTROL_TOP_3) + DGGetItemValDouble (dialogID, EDITCONTROL_TOP_4)) * cos(placingZone.angle);
								placingZone.cellsT3 [xx].leftBottomY = rotatedPoint.y + (DGGetItemValDouble (dialogID, EDITCONTROL_TOP_1) + DGGetItemValDouble (dialogID, EDITCONTROL_TOP_2) + DGGetItemValDouble (dialogID, EDITCONTROL_TOP_3) + DGGetItemValDouble (dialogID, EDITCONTROL_TOP_4)) * sin(placingZone.angle);
							} else {
								placingZone.cellsT3 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, EDITCONTROL_TOP_2) + DGGetItemValDouble (dialogID, EDITCONTROL_TOP_3) + DGGetItemValDouble (dialogID, EDITCONTROL_TOP_4)) * cos(placingZone.angle);
								placingZone.cellsT3 [xx].leftBottomY = rotatedPoint.y + (DGGetItemValDouble (dialogID, EDITCONTROL_TOP_2) + DGGetItemValDouble (dialogID, EDITCONTROL_TOP_3) + DGGetItemValDouble (dialogID, EDITCONTROL_TOP_4)) * sin(placingZone.angle);
							}
							placingZone.cellsT3 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
							placingZone.cellsT3 [xx].ang = placingZone.angle + DegreeToRad (180);
							placingZone.cellsT3 [xx].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_TOP_4);
							placingZone.cellsT3 [xx].verLen = 0.064;
							placingZone.cellsT3 [xx].height = 1.200;
							formWidth = DGGetItemValDouble (dialogID, EDITCONTROL_TOP_4);
							if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
								placingZone.cellsT3 [xx].bStandardEuroform = true;
							else
								placingZone.cellsT3 [xx].bStandardEuroform = false;
						}

						// ���� 1
						xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
						yLen = (placingZone.coreDepth/2 + placingZone.venThick);
						lineLen = sqrt (xLen*xLen + yLen*yLen);
						rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
						rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

						placingZone.cellsL1 [xx].objType = EUROFORM;
						if (DGGetItemValLong (dialogID, RADIO_OUTCORNER_PANEL) == TRUE) {
							placingZone.cellsL1 [xx].leftBottomX = rotatedPoint.x + DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_1) * sin(placingZone.angle);
							placingZone.cellsL1 [xx].leftBottomY = rotatedPoint.y - DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_1) * cos(placingZone.angle);
						} else {
							placingZone.cellsL1 [xx].leftBottomX = rotatedPoint.x;
							placingZone.cellsL1 [xx].leftBottomY = rotatedPoint.y;
						}
						placingZone.cellsL1 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
						placingZone.cellsL1 [xx].ang = placingZone.angle - DegreeToRad (90);
						placingZone.cellsL1 [xx].horLen = 0.064;
						placingZone.cellsL1 [xx].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_2);
						placingZone.cellsL1 [xx].height = 1.200;
						formWidth = DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_2);
						if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
							placingZone.cellsL1 [xx].bStandardEuroform = true;
						else
							placingZone.cellsL1 [xx].bStandardEuroform = false;

						// ���� 2
						xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
						yLen = (placingZone.coreDepth/2 + placingZone.venThick);
						lineLen = sqrt (xLen*xLen + yLen*yLen);
						rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
						rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

						placingZone.cellsL2 [xx].objType = FILLERSP;
						if (DGGetItemValLong (dialogID, RADIO_OUTCORNER_PANEL) == TRUE) {
							placingZone.cellsL2 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_1) + DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_2)) * sin(placingZone.angle);
							placingZone.cellsL2 [xx].leftBottomY = rotatedPoint.y - (DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_1) + DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_2)) * cos(placingZone.angle);
						} else {
							placingZone.cellsL2 [xx].leftBottomX = rotatedPoint.x + DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_2) * sin(placingZone.angle);
							placingZone.cellsL2 [xx].leftBottomY = rotatedPoint.y - DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_2) * cos(placingZone.angle);
						}
						placingZone.cellsL2 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
						placingZone.cellsL2 [xx].ang = placingZone.angle - DegreeToRad (90);
						placingZone.cellsL2 [xx].horLen = 0.064;
						placingZone.cellsL2 [xx].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_3);
						placingZone.cellsL2 [xx].height = 1.200;
						formWidth = DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_3);
						placingZone.cellsL2 [xx].bStandardEuroform = false;

						// ���� 3
						if (DGGetItemValLong (dialogID, CHECKBOX_LEFT_ADDITIONAL_FORM) == TRUE) {
							xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
							yLen = (placingZone.coreDepth/2 + placingZone.venThick);
							lineLen = sqrt (xLen*xLen + yLen*yLen);
							rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
							rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

							placingZone.cellsL3 [xx].objType = EUROFORM;
							if (DGGetItemValLong (dialogID, RADIO_OUTCORNER_PANEL) == TRUE) {
								placingZone.cellsL3 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_1) + DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_2) + DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_3)) * sin(placingZone.angle);
								placingZone.cellsL3 [xx].leftBottomY = rotatedPoint.y - (DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_1) + DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_2) + DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_3)) * cos(placingZone.angle);
							} else {
								placingZone.cellsL3 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_2) + DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_3)) * sin(placingZone.angle);
								placingZone.cellsL3 [xx].leftBottomY = rotatedPoint.y - (DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_2) + DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_3)) * cos(placingZone.angle);
							}
							placingZone.cellsL3 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
							placingZone.cellsL3 [xx].ang = placingZone.angle - DegreeToRad (90);
							placingZone.cellsL3 [xx].horLen = 0.064;
							placingZone.cellsL3 [xx].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_4);
							placingZone.cellsL3 [xx].height = 1.200;
							formWidth = DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_4);
							if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
								placingZone.cellsL3 [xx].bStandardEuroform = true;
							else
								placingZone.cellsL3 [xx].bStandardEuroform = false;
						}

						// ���� 1
						xLen = (placingZone.coreWidth/2 + placingZone.venThick);
						yLen = (placingZone.coreDepth/2 + placingZone.venThick);
						lineLen = sqrt (xLen*xLen + yLen*yLen);
						rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
						rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

						placingZone.cellsR1 [xx].objType = EUROFORM;
						if (DGGetItemValLong (dialogID, RADIO_OUTCORNER_PANEL) == TRUE) {
							placingZone.cellsR1 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_1) + DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_2)) * sin(placingZone.angle);
							placingZone.cellsR1 [xx].leftBottomY = rotatedPoint.y - (DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_1) + DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_2)) * cos(placingZone.angle);
						} else {
							placingZone.cellsR1 [xx].leftBottomX = rotatedPoint.x + DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_2) * sin(placingZone.angle);
							placingZone.cellsR1 [xx].leftBottomY = rotatedPoint.y - DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_2) * cos(placingZone.angle);
						}
						placingZone.cellsR1 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
						placingZone.cellsR1 [xx].ang = placingZone.angle + DegreeToRad (90);
						placingZone.cellsR1 [xx].horLen = 0.064;
						placingZone.cellsR1 [xx].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_2);
						placingZone.cellsR1 [xx].height = 1.200;
						formWidth = DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_2);
						if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
							placingZone.cellsR1 [xx].bStandardEuroform = true;
						else
							placingZone.cellsR1 [xx].bStandardEuroform = false;

						// ���� 2
						xLen = (placingZone.coreWidth/2 + placingZone.venThick);
						yLen = (placingZone.coreDepth/2 + placingZone.venThick);
						lineLen = sqrt (xLen*xLen + yLen*yLen);
						rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
						rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

						placingZone.cellsR2 [xx].objType = FILLERSP;
						if (DGGetItemValLong (dialogID, RADIO_OUTCORNER_PANEL) == TRUE) {
							placingZone.cellsR2 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_1) + DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_2) + DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_3)) * sin(placingZone.angle);
							placingZone.cellsR2 [xx].leftBottomY = rotatedPoint.y - (DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_1) + DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_2) + DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_3)) * cos(placingZone.angle);
						} else {
							placingZone.cellsR2 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_2) + DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_3)) * sin(placingZone.angle);
							placingZone.cellsR2 [xx].leftBottomY = rotatedPoint.y - (DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_2) + DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_3)) * cos(placingZone.angle);
						}
						placingZone.cellsR2 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
						placingZone.cellsR2 [xx].ang = placingZone.angle + DegreeToRad (90);
						placingZone.cellsR2 [xx].horLen = 0.064;
						placingZone.cellsR2 [xx].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_3);
						placingZone.cellsR2 [xx].height = 1.200;
						formWidth = DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_3);
						placingZone.cellsR2 [xx].bStandardEuroform = false;

						// ���� 3
						if (DGGetItemValLong (dialogID, CHECKBOX_RIGHT_ADDITIONAL_FORM) == TRUE) { 
							xLen = (placingZone.coreWidth/2 + placingZone.venThick);
							yLen = (placingZone.coreDepth/2 + placingZone.venThick);
							lineLen = sqrt (xLen*xLen + yLen*yLen);
							rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
							rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

							placingZone.cellsR3 [xx].objType = EUROFORM;
							if (DGGetItemValLong (dialogID, RADIO_OUTCORNER_PANEL) == TRUE) {
								placingZone.cellsR3 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_1) + DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_2) + DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_3) + DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_4)) * sin(placingZone.angle);
								placingZone.cellsR3 [xx].leftBottomY = rotatedPoint.y - (DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_1) + DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_2) + DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_3) + DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_4)) * cos(placingZone.angle);
							} else {
								placingZone.cellsR3 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_2) + DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_3) + DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_4)) * sin(placingZone.angle);
								placingZone.cellsR3 [xx].leftBottomY = rotatedPoint.y - (DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_2) + DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_3) + DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_4)) * cos(placingZone.angle);
							}
							placingZone.cellsR3 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
							placingZone.cellsR3 [xx].ang = placingZone.angle + DegreeToRad (90);
							placingZone.cellsR3 [xx].horLen = 0.064;
							placingZone.cellsR3 [xx].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_4);
							placingZone.cellsR3 [xx].height = 1.200;
							formWidth = DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_4);
							if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
								placingZone.cellsR3 [xx].bStandardEuroform = true;
							else
								placingZone.cellsR3 [xx].bStandardEuroform = false;
						}

						// �Ʒ��� 1
						xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
						yLen = -(placingZone.coreDepth/2 + placingZone.venThick);
						lineLen = sqrt (xLen*xLen + yLen*yLen);
						rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
						rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

						placingZone.cellsB1 [xx].objType = EUROFORM;
						if (DGGetItemValLong (dialogID, RADIO_OUTCORNER_PANEL) == TRUE) {
							placingZone.cellsB1 [xx].leftBottomX = rotatedPoint.x + DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_1) * cos(placingZone.angle);
							placingZone.cellsB1 [xx].leftBottomY = rotatedPoint.y + DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_1) * sin(placingZone.angle);
						} else {
							placingZone.cellsB1 [xx].leftBottomX = rotatedPoint.x;
							placingZone.cellsB1 [xx].leftBottomY = rotatedPoint.y;
						}
						placingZone.cellsB1 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
						placingZone.cellsB1 [xx].ang = placingZone.angle;
						placingZone.cellsB1 [xx].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_2);
						placingZone.cellsB1 [xx].verLen = 0.064;
						placingZone.cellsB1 [xx].height = 1.200;
						formWidth = DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_2);
						if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
							placingZone.cellsB1 [xx].bStandardEuroform = true;
						else
							placingZone.cellsB1 [xx].bStandardEuroform = false;

						// �Ʒ��� 2
						xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
						yLen = -(placingZone.coreDepth/2 + placingZone.venThick);
						lineLen = sqrt (xLen*xLen + yLen*yLen);
						rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
						rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

						placingZone.cellsB2 [xx].objType = FILLERSP;
						if (DGGetItemValLong (dialogID, RADIO_OUTCORNER_PANEL) == TRUE) {
							placingZone.cellsB2 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_1) + DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_2)) * cos(placingZone.angle);
							placingZone.cellsB2 [xx].leftBottomY = rotatedPoint.y + (DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_1) + DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_2)) * sin(placingZone.angle);
						} else {
							placingZone.cellsB2 [xx].leftBottomX = rotatedPoint.x + DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_2) * cos(placingZone.angle);
							placingZone.cellsB2 [xx].leftBottomY = rotatedPoint.y + DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_2) * sin(placingZone.angle);
						}
						placingZone.cellsB2 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
						placingZone.cellsB2 [xx].ang = placingZone.angle;
						placingZone.cellsB2 [xx].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_3);
						placingZone.cellsB2 [xx].verLen = 0.064;
						placingZone.cellsB2 [xx].height = 1.200;
						formWidth = DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_3);
						placingZone.cellsB2 [xx].bStandardEuroform = false;

						// �Ʒ��� 3
						if (DGGetItemValLong (dialogID, CHECKBOX_BOTTOM_ADDITIONAL_FORM) == TRUE) {
							xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
							yLen = -(placingZone.coreDepth/2 + placingZone.venThick);
							lineLen = sqrt (xLen*xLen + yLen*yLen);
							rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
							rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

							placingZone.cellsB3 [xx].objType = EUROFORM;
							if (DGGetItemValLong (dialogID, RADIO_OUTCORNER_PANEL) == TRUE) {
								placingZone.cellsB3 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_1) + DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_2) + DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_3)) * cos(placingZone.angle);
								placingZone.cellsB3 [xx].leftBottomY = rotatedPoint.y + (DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_1) + DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_2) + DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_3)) * sin(placingZone.angle);
							} else {
								placingZone.cellsB3 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_2) + DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_3)) * cos(placingZone.angle);
								placingZone.cellsB3 [xx].leftBottomY = rotatedPoint.y + (DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_2) + DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_3)) * sin(placingZone.angle);
							}
							placingZone.cellsB3 [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
							placingZone.cellsB3 [xx].ang = placingZone.angle;
							placingZone.cellsB3 [xx].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_4);
							placingZone.cellsB3 [xx].verLen = 0.064;
							placingZone.cellsB3 [xx].height = 1.200;
							formWidth = DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_4);
							if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
								placingZone.cellsB3 [xx].bStandardEuroform = true;
							else
								placingZone.cellsB3 [xx].bStandardEuroform = false;
						}
					}

					// ��չ�� Ÿ��
					if (DGGetItemValLong (dialogID, RADIO_COLUMN_BAND_1) == TRUE) {
						placingZone.typeOfColumnBand = 1;	// ��չ��
					} else {
						placingZone.typeOfColumnBand = 2;	// ����
					}

					// ���̺��� Ÿ��
					placingZone.tableformType = DGPopUpGetSelected (dialogID, POPUP_TABLEFORM_TYPE);

					// ���̾� ��ȣ ����
					layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM);
					layerInd_Fillersp		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP);
					layerInd_OutPanel		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL);
					layerInd_OutAngle		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE);
					layerInd_SqrPipe		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_SQUARE_PIPE);
					layerInd_Pinbolt		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT);
					layerInd_Hanger			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_HANGER);
					layerInd_Head			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE);
					layerInd_ColumnBand		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_COLUMN_BAND);
					layerInd_Plywood		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD);
					layerInd_BlueClamp		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_BLUECLAMP);

					break;

				case BUTTON_AUTOSET:
					item = 0;

					DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, FALSE);

					layerInd_Euroform	= makeTemporaryLayer (structuralObject_forTableformColumn, "UFOM", NULL);
					layerInd_Fillersp	= makeTemporaryLayer (structuralObject_forTableformColumn, "FISP", NULL);
					layerInd_OutPanel	= makeTemporaryLayer (structuralObject_forTableformColumn, "OUTP", NULL);
					layerInd_OutAngle	= makeTemporaryLayer (structuralObject_forTableformColumn, "OUTA", NULL);
					layerInd_SqrPipe	= makeTemporaryLayer (structuralObject_forTableformColumn, "SPIP", NULL);
					layerInd_Pinbolt	= makeTemporaryLayer (structuralObject_forTableformColumn, "PINB", NULL);
					layerInd_Hanger		= makeTemporaryLayer (structuralObject_forTableformColumn, "JOIB", NULL);
					layerInd_Head		= makeTemporaryLayer (structuralObject_forTableformColumn, "HEAD", NULL);
					layerInd_ColumnBand	= makeTemporaryLayer (structuralObject_forTableformColumn, "BDCM", NULL);
					layerInd_Plywood	= makeTemporaryLayer (structuralObject_forTableformColumn, "PLYW", NULL);
					layerInd_BlueClamp	= makeTemporaryLayer (structuralObject_forTableformColumn, "UFCL", NULL);

					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, layerInd_Euroform);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, layerInd_Fillersp);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, layerInd_OutPanel);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, layerInd_OutAngle);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_SQUARE_PIPE, layerInd_SqrPipe);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, layerInd_Pinbolt);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_HANGER, layerInd_Hanger);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, layerInd_Head);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_COLUMN_BAND, layerInd_ColumnBand);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, layerInd_Plywood);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUECLAMP, layerInd_BlueClamp);

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
short DGCALLBACK columnTableformPlacerHandler_soleColumn_2 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;
	short	btnSizeX = 50, btnSizeY = 50;
	short	dialogSizeX, dialogSizeY;
	short	btnPosX, btnPosY;
	short	xx;
	std::string		txtButton = "";

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, L"��տ� ��ġ - ��� ����");

			//////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			// Ȯ�� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 30, 100, 100, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, L"Ȯ��");
			DGShowItem (dialogID, DG_OK);

			// ��� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 30, 140, 100, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, L"���");
			DGShowItem (dialogID, DG_CANCEL);

			// ������Ʈ ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 30, 60, 100, 25);
			DGSetItemFont (dialogID, DG_UPDATE_BUTTON, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_UPDATE_BUTTON, L"������Ʈ");
			DGShowItem (dialogID, DG_UPDATE_BUTTON);
			DGDisableItem (dialogID, DG_UPDATE_BUTTON);

			// ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 30, 180, 100, 25);
			DGSetItemFont (dialogID, DG_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_PREV, L"����");
			DGShowItem (dialogID, DG_PREV);

			// ��: ��� ����
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 10, 10, 100, 23);
			DGSetItemFont (dialogID, LABEL_COLUMN_SIDE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_COLUMN_SIDE, L"��� ����");
			DGShowItem (dialogID, LABEL_COLUMN_SIDE);

			// ���� �ǹ��ϴ� ���簢��
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 220, 30, 70, 175 + (btnSizeY * (placingZone.nCells-1)));
			DGShowItem (dialogID, itmIdx);

			// �� �ܸ�
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 230, 40, btnSizeX, btnSizeY);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, itmIdx);
			DGDisableItem (dialogID, itmIdx);
			if (placingZone.bInterfereBeam == true)
				DGSetItemText (dialogID, itmIdx, L"��\n����");
			else
				DGSetItemText (dialogID, itmIdx, L"��\n����");

			// ���� ��ġ
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 235, 110, 40, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, L"����");
			DGShowItem (dialogID, itmIdx);

			// ������ ��ư ����
			btnPosX = 230;
			btnPosY = 140 + (btnSizeY * (placingZone.nCells-1));
			for (xx = 0 ; xx < placingZone.nCells ; ++xx) {

				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				txtButton = "";
				if (placingZone.cellsB1 [xx].objType == NONE) {
					txtButton = "NONE";
				} else if (placingZone.cellsB1 [xx].objType == EUROFORM) {
					txtButton = format_string ("������\n��%.0f", placingZone.cellsB1 [xx].height * 1000);
				}
				DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// �׸��� ��ư �ؽ�Ʈ ����
				DGShowItem (dialogID, itmIdx);
				btnPosY -= 50;

				if (xx == 0)
					EUROFORM_BUTTON_BOTTOM = itmIdx;
				if (xx == placingZone.nCells-1)
					EUROFORM_BUTTON_TOP = itmIdx;
			}

			// �߰�/���� ��ư
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 310, 113, 50, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, L"�߰�");
			DGShowItem (dialogID, itmIdx);
			ADD_CELLS = itmIdx;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 310, 142, 50, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, L"����");
			DGShowItem (dialogID, itmIdx);
			DEL_CELLS = itmIdx;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 290, 134, 20, 20);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, L"��");
			DGShowItem (dialogID, itmIdx);

			// �� �ܸ��� �ǹ��ϴ� ���簢��
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 500, 100, 80, 80);
			DGShowItem (dialogID, itmIdx);

			// ��: ��������
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 530, 105, 25, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, L"��");
			DGShowItem (dialogID, itmIdx);
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 530, 155, 25, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, L"��");
			DGShowItem (dialogID, itmIdx);
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 505, 130, 25, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, L"��");
			DGShowItem (dialogID, itmIdx);
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 550, 130, 25, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, L"��");
			DGShowItem (dialogID, itmIdx);

			// üũ�ڽ�: �������� ���� ä�� ����
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 505, 50, 80, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, L"���� ä��");
			DGSetItemValLong (dialogID, itmIdx, TRUE);
			DGShowItem (dialogID, itmIdx);	// ��
			CHECKBOX_NORTH_MARGIN = itmIdx;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 505, 210, 80, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, L"���� ä��");
			DGSetItemValLong (dialogID, itmIdx, TRUE);
			DGShowItem (dialogID, itmIdx);	// ��
			CHECKBOX_SOUTH_MARGIN = itmIdx;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 420, 115, 80, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, L"���� ä��");
			DGSetItemValLong (dialogID, itmIdx, TRUE);
			DGShowItem (dialogID, itmIdx);	// ��
			CHECKBOX_WEST_MARGIN = itmIdx;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 590, 115, 80, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, L"���� ä��");
			DGSetItemValLong (dialogID, itmIdx, TRUE);
			DGShowItem (dialogID, itmIdx);	// ��
			CHECKBOX_EAST_MARGIN = itmIdx;

			// ���� ��� (��)
			if (placingZone.bExistBeams [NORTH] == true)
				placingZone.marginTopAtNorth = placingZone.bottomLevelOfBeams [NORTH];
			else
				placingZone.marginTopAtNorth = placingZone.areaHeight;
			for (xx = 0 ; xx < placingZone.nCells ; ++xx)
				placingZone.marginTopAtNorth -= placingZone.cellsB1 [xx].height;

			// ���� ��� (��)
			if (placingZone.bExistBeams [SOUTH] == true)
				placingZone.marginTopAtSouth = placingZone.bottomLevelOfBeams [SOUTH];
			else
				placingZone.marginTopAtSouth = placingZone.areaHeight;
			for (xx = 0 ; xx < placingZone.nCells ; ++xx)
				placingZone.marginTopAtSouth -= placingZone.cellsB1 [xx].height;

			// ���� ��� (��)
			if (placingZone.bExistBeams [WEST] == true)
				placingZone.marginTopAtWest = placingZone.bottomLevelOfBeams [WEST];
			else
				placingZone.marginTopAtWest = placingZone.areaHeight;
			for (xx = 0 ; xx < placingZone.nCells ; ++xx)
				placingZone.marginTopAtWest -= placingZone.cellsB1 [xx].height;

			// ���� ��� (��)
			if (placingZone.bExistBeams [EAST] == true)
				placingZone.marginTopAtEast = placingZone.bottomLevelOfBeams [EAST];
			else
				placingZone.marginTopAtEast = placingZone.areaHeight;
			for (xx = 0 ; xx < placingZone.nCells ; ++xx)
				placingZone.marginTopAtEast -= placingZone.cellsB1 [xx].height;

			// Edit ��Ʈ��: �������� ���� ġ�� ǥ��
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 520, 75, 40, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemValDouble (dialogID, itmIdx, placingZone.marginTopAtNorth);
			DGShowItem (dialogID, itmIdx);	// ��
			DGDisableItem (dialogID, itmIdx);
			EDITCONTROL_NORTH_MARGIN = itmIdx;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 520, 182, 40, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemValDouble (dialogID, itmIdx, placingZone.marginTopAtSouth);
			DGShowItem (dialogID, itmIdx);	// ��
			DGDisableItem (dialogID, itmIdx);
			EDITCONTROL_SOUTH_MARGIN = itmIdx;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 440, 142, 40, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemValDouble (dialogID, itmIdx, placingZone.marginTopAtWest);
			DGShowItem (dialogID, itmIdx);	// ��
			DGDisableItem (dialogID, itmIdx);
			EDITCONTROL_WEST_MARGIN = itmIdx;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 600, 142, 40, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemValDouble (dialogID, itmIdx, placingZone.marginTopAtEast);
			DGShowItem (dialogID, itmIdx);	// ��
			DGDisableItem (dialogID, itmIdx);
			EDITCONTROL_EAST_MARGIN = itmIdx;

			// ���� â ũ�⸦ ����
			dialogSizeX = 700;
			dialogSizeY = max<short>(300, 300 + (btnSizeY * (placingZone.nCells - 1)));
			DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);

			break;

		case DG_MSG_CHANGE:

			break;

		case DG_MSG_CLICK:

			// ������Ʈ ��ư
			if (item == DG_UPDATE_BUTTON) {
				item = 0;

				// ����� ���� ä�� ���� ����
				if (DGGetItemValLong (dialogID, CHECKBOX_NORTH_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtNorth = true;
				else
					placingZone.bFillMarginTopAtNorth = false;

				if (DGGetItemValLong (dialogID, CHECKBOX_SOUTH_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtSouth = true;
				else
					placingZone.bFillMarginTopAtSouth = false;

				if (DGGetItemValLong (dialogID, CHECKBOX_WEST_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtWest = true;
				else
					placingZone.bFillMarginTopAtWest = false;

				if (DGGetItemValLong (dialogID, CHECKBOX_EAST_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtEast = true;
				else
					placingZone.bFillMarginTopAtEast = false;

				// �� ���� ���� �߻�, ��� ���� ��ġ ���� ������Ʈ
				placingZone.alignPlacingZone_soleColumn (&placingZone);

				// ���� ���ɼ��� �ִ� DG �׸� ��� ����
				DGRemoveDialogItems (dialogID, AFTER_ALL);

				// ���� �ǹ��ϴ� ���簢��
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 220, 30, 70, 175 + (btnSizeY * (placingZone.nCells-1)));
				DGShowItem (dialogID, itmIdx);

				// �� �ܸ�
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 230, 40, btnSizeX, btnSizeY);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem (dialogID, itmIdx);
				DGDisableItem (dialogID, itmIdx);
				if (placingZone.bInterfereBeam == true)
					DGSetItemText (dialogID, itmIdx, L"��\n����");
				else
					DGSetItemText (dialogID, itmIdx, L"��\n����");

				// ���� ��ġ
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 235, 110, 40, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"����");
				DGShowItem (dialogID, itmIdx);

				// ������ ��ư ����
				btnPosX = 230;
				btnPosY = 140 + (btnSizeY * (placingZone.nCells-1));
				for (xx = 0 ; xx < placingZone.nCells ; ++xx) {

					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					txtButton = "";
					if (placingZone.cellsB1 [xx].objType == NONE) {
						txtButton = "NONE";
					} else if (placingZone.cellsB1 [xx].objType == EUROFORM) {
						txtButton = format_string ("������\n��%.0f", placingZone.cellsB1 [xx].height * 1000);
					}
					DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// �׸��� ��ư �ؽ�Ʈ ����
					DGShowItem (dialogID, itmIdx);
					btnPosY -= 50;

					if (xx == 0)
						EUROFORM_BUTTON_BOTTOM = itmIdx;
					if (xx == placingZone.nCells-1)
						EUROFORM_BUTTON_TOP = itmIdx;
				}

				// �߰�/���� ��ư
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 310, 113, 50, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"�߰�");
				DGShowItem (dialogID, itmIdx);
				ADD_CELLS = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 310, 142, 50, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"����");
				DGShowItem (dialogID, itmIdx);
				DEL_CELLS = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 290, 134, 20, 20);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"��");
				DGShowItem (dialogID, itmIdx);

				// �� �ܸ��� �ǹ��ϴ� ���簢��
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 500, 100, 80, 80);
				DGShowItem (dialogID, itmIdx);

				// ��: ��������
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 530, 105, 25, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"��");
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 530, 155, 25, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"��");
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 505, 130, 25, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"��");
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 550, 130, 25, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"��");
				DGShowItem (dialogID, itmIdx);

				// üũ�ڽ�: �������� ���� ä�� ����
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 505, 50, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"���� ä��");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtNorth == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// ��
				CHECKBOX_NORTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 505, 210, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"���� ä��");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtSouth == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// ��
				CHECKBOX_SOUTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 420, 115, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"���� ä��");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtWest == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// ��
				CHECKBOX_WEST_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 590, 115, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"���� ä��");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtEast == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// ��
				CHECKBOX_EAST_MARGIN = itmIdx;

				// ���� ��� (��)
				if (placingZone.bExistBeams [NORTH] == true)
					placingZone.marginTopAtNorth = placingZone.bottomLevelOfBeams [NORTH];
				else
					placingZone.marginTopAtNorth = placingZone.areaHeight;
				for (xx = 0 ; xx < placingZone.nCells ; ++xx)
					placingZone.marginTopAtNorth -= placingZone.cellsB1 [xx].height;

				// ���� ��� (��)
				if (placingZone.bExistBeams [SOUTH] == true)
					placingZone.marginTopAtSouth = placingZone.bottomLevelOfBeams [SOUTH];
				else
					placingZone.marginTopAtSouth = placingZone.areaHeight;
				for (xx = 0 ; xx < placingZone.nCells ; ++xx)
					placingZone.marginTopAtSouth -= placingZone.cellsB1 [xx].height;

				// ���� ��� (��)
				if (placingZone.bExistBeams [WEST] == true)
					placingZone.marginTopAtWest = placingZone.bottomLevelOfBeams [WEST];
				else
					placingZone.marginTopAtWest = placingZone.areaHeight;
				for (xx = 0 ; xx < placingZone.nCells ; ++xx)
					placingZone.marginTopAtWest -= placingZone.cellsB1 [xx].height;

				// ���� ��� (��)
				if (placingZone.bExistBeams [EAST] == true)
					placingZone.marginTopAtEast = placingZone.bottomLevelOfBeams [EAST];
				else
					placingZone.marginTopAtEast = placingZone.areaHeight;
				for (xx = 0 ; xx < placingZone.nCells ; ++xx)
					placingZone.marginTopAtEast -= placingZone.cellsB1 [xx].height;

				// Edit ��Ʈ��: �������� ���� ġ�� ǥ��
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 520, 75, 40, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemValDouble (dialogID, itmIdx, placingZone.marginTopAtNorth);
				DGShowItem (dialogID, itmIdx);	// ��
				DGDisableItem (dialogID, itmIdx);
				EDITCONTROL_NORTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 520, 182, 40, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemValDouble (dialogID, itmIdx, placingZone.marginTopAtSouth);
				DGShowItem (dialogID, itmIdx);	// ��
				DGDisableItem (dialogID, itmIdx);
				EDITCONTROL_SOUTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 440, 142, 40, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemValDouble (dialogID, itmIdx, placingZone.marginTopAtWest);
				DGShowItem (dialogID, itmIdx);	// ��
				DGDisableItem (dialogID, itmIdx);
				EDITCONTROL_WEST_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 600, 142, 40, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemValDouble (dialogID, itmIdx, placingZone.marginTopAtEast);
				DGShowItem (dialogID, itmIdx);	// ��
				DGDisableItem (dialogID, itmIdx);
				EDITCONTROL_EAST_MARGIN = itmIdx;

				// ���� â ũ�⸦ ����
				dialogSizeX = 700;
				dialogSizeY = max<short>(300, 300 + (btnSizeY * (placingZone.nCells - 1)));
				DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);
			}

			// ���� ��ư
			if (item == DG_PREV) {
				clickedPrevButton = true;
			}

			// Ȯ�� ��ư
			if (item == DG_OK) {
				clickedOKButton = true;

				// ����� ���� ä�� ���� ����
				if (DGGetItemValLong (dialogID, CHECKBOX_NORTH_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtNorth = true;
				else
					placingZone.bFillMarginTopAtNorth = false;

				if (DGGetItemValLong (dialogID, CHECKBOX_SOUTH_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtSouth = true;
				else
					placingZone.bFillMarginTopAtSouth = false;

				if (DGGetItemValLong (dialogID, CHECKBOX_WEST_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtWest = true;
				else
					placingZone.bFillMarginTopAtWest = false;

				if (DGGetItemValLong (dialogID, CHECKBOX_EAST_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtEast = true;
				else
					placingZone.bFillMarginTopAtEast = false;

				// �� ���� ���� �߻�, ��� ���� ��ġ ���� ������Ʈ
				placingZone.alignPlacingZone_soleColumn (&placingZone);

				// ���� ���ɼ��� �ִ� DG �׸� ��� ����
				DGRemoveDialogItems (dialogID, AFTER_ALL);

				// ���� �ǹ��ϴ� ���簢��
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 220, 30, 70, 175 + (btnSizeY * (placingZone.nCells-1)));
				DGShowItem (dialogID, itmIdx);

				// �� �ܸ�
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 230, 40, btnSizeX, btnSizeY);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem (dialogID, itmIdx);
				DGDisableItem (dialogID, itmIdx);
				if (placingZone.bInterfereBeam == true)
					DGSetItemText (dialogID, itmIdx, L"��\n����");
				else
					DGSetItemText (dialogID, itmIdx, L"��\n����");

				// ���� ��ġ
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 235, 110, 40, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"����");
				DGShowItem (dialogID, itmIdx);

				// ������ ��ư ����
				btnPosX = 230;
				btnPosY = 140 + (btnSizeY * (placingZone.nCells-1));
				for (xx = 0 ; xx < placingZone.nCells ; ++xx) {

					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					txtButton = "";
					if (placingZone.cellsB1 [xx].objType == NONE) {
						txtButton = "NONE";
					} else if (placingZone.cellsB1 [xx].objType == EUROFORM) {
						txtButton = format_string ("������\n��%.0f", placingZone.cellsB1 [xx].height * 1000);
					}
					DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// �׸��� ��ư �ؽ�Ʈ ����
					DGShowItem (dialogID, itmIdx);
					btnPosY -= 50;

					if (xx == 0)
						EUROFORM_BUTTON_BOTTOM = itmIdx;
					if (xx == placingZone.nCells-1)
						EUROFORM_BUTTON_TOP = itmIdx;
				}

				// �߰�/���� ��ư
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 310, 113, 50, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"�߰�");
				DGShowItem (dialogID, itmIdx);
				ADD_CELLS = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 310, 142, 50, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"����");
				DGShowItem (dialogID, itmIdx);
				DEL_CELLS = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 290, 134, 20, 20);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"��");
				DGShowItem (dialogID, itmIdx);

				// �� �ܸ��� �ǹ��ϴ� ���簢��
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 500, 100, 80, 80);
				DGShowItem (dialogID, itmIdx);

				// ��: ��������
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 530, 105, 25, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"��");
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 530, 155, 25, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"��");
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 505, 130, 25, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"��");
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 550, 130, 25, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"��");
				DGShowItem (dialogID, itmIdx);

				// üũ�ڽ�: �������� ���� ä�� ����
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 505, 50, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"���� ä��");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtNorth == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// ��
				CHECKBOX_NORTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 505, 210, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"���� ä��");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtSouth == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// ��
				CHECKBOX_SOUTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 420, 115, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"���� ä��");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtWest == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// ��
				CHECKBOX_WEST_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 590, 115, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"���� ä��");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtEast == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// ��
				CHECKBOX_EAST_MARGIN = itmIdx;

				// ���� ��� (��)
				if (placingZone.bExistBeams [NORTH] == true)
					placingZone.marginTopAtNorth = placingZone.bottomLevelOfBeams [NORTH];
				else
					placingZone.marginTopAtNorth = placingZone.areaHeight;
				for (xx = 0 ; xx < placingZone.nCells ; ++xx)
					placingZone.marginTopAtNorth -= placingZone.cellsB1 [xx].height;

				// ���� ��� (��)
				if (placingZone.bExistBeams [SOUTH] == true)
					placingZone.marginTopAtSouth = placingZone.bottomLevelOfBeams [SOUTH];
				else
					placingZone.marginTopAtSouth = placingZone.areaHeight;
				for (xx = 0 ; xx < placingZone.nCells ; ++xx)
					placingZone.marginTopAtSouth -= placingZone.cellsB1 [xx].height;

				// ���� ��� (��)
				if (placingZone.bExistBeams [WEST] == true)
					placingZone.marginTopAtWest = placingZone.bottomLevelOfBeams [WEST];
				else
					placingZone.marginTopAtWest = placingZone.areaHeight;
				for (xx = 0 ; xx < placingZone.nCells ; ++xx)
					placingZone.marginTopAtWest -= placingZone.cellsB1 [xx].height;

				// ���� ��� (��)
				if (placingZone.bExistBeams [EAST] == true)
					placingZone.marginTopAtEast = placingZone.bottomLevelOfBeams [EAST];
				else
					placingZone.marginTopAtEast = placingZone.areaHeight;
				for (xx = 0 ; xx < placingZone.nCells ; ++xx)
					placingZone.marginTopAtEast -= placingZone.cellsB1 [xx].height;

				// Edit ��Ʈ��: �������� ���� ġ�� ǥ��
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 520, 75, 40, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemValDouble (dialogID, itmIdx, placingZone.marginTopAtNorth);
				DGShowItem (dialogID, itmIdx);	// ��
				DGDisableItem (dialogID, itmIdx);
				EDITCONTROL_NORTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 520, 182, 40, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemValDouble (dialogID, itmIdx, placingZone.marginTopAtSouth);
				DGShowItem (dialogID, itmIdx);	// ��
				DGDisableItem (dialogID, itmIdx);
				EDITCONTROL_SOUTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 440, 142, 40, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemValDouble (dialogID, itmIdx, placingZone.marginTopAtWest);
				DGShowItem (dialogID, itmIdx);	// ��
				DGDisableItem (dialogID, itmIdx);
				EDITCONTROL_WEST_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 600, 142, 40, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemValDouble (dialogID, itmIdx, placingZone.marginTopAtEast);
				DGShowItem (dialogID, itmIdx);	// ��
				DGDisableItem (dialogID, itmIdx);
				EDITCONTROL_EAST_MARGIN = itmIdx;

				// ���� â ũ�⸦ ����
				dialogSizeX = 700;
				dialogSizeY = max<short>(300, 300 + (btnSizeY * (placingZone.nCells - 1)));
				DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);
			}

			// ��� ��ư
			if (item == DG_CANCEL) {
			}

			// �� �߰�/���� ��ư
			if (item == ADD_CELLS) {
				placingZone.addTopCell (&placingZone);
			}
			if (item == DEL_CELLS) {
				placingZone.delTopCell (&placingZone);
			}

			if ((item == ADD_CELLS) || (item == DEL_CELLS)) {
				item = 0;

				// ����� ���� ä�� ���� ����
				if (DGGetItemValLong (dialogID, CHECKBOX_NORTH_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtNorth = true;
				else
					placingZone.bFillMarginTopAtNorth = false;

				if (DGGetItemValLong (dialogID, CHECKBOX_SOUTH_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtSouth = true;
				else
					placingZone.bFillMarginTopAtSouth = false;

				if (DGGetItemValLong (dialogID, CHECKBOX_WEST_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtWest = true;
				else
					placingZone.bFillMarginTopAtWest = false;

				if (DGGetItemValLong (dialogID, CHECKBOX_EAST_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtEast = true;
				else
					placingZone.bFillMarginTopAtEast = false;

				// �� ���� ���� �߻�, ��� ���� ��ġ ���� ������Ʈ
				placingZone.alignPlacingZone_soleColumn (&placingZone);

				// ���� ���ɼ��� �ִ� DG �׸� ��� ����
				DGRemoveDialogItems (dialogID, AFTER_ALL);

				// ���� �ǹ��ϴ� ���簢��
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 220, 30, 70, 175 + (btnSizeY * (placingZone.nCells-1)));
				DGShowItem (dialogID, itmIdx);

				// �� �ܸ�
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 230, 40, btnSizeX, btnSizeY);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem (dialogID, itmIdx);
				DGDisableItem (dialogID, itmIdx);
				if (placingZone.bInterfereBeam == true)
					DGSetItemText (dialogID, itmIdx, L"��\n����");
				else
					DGSetItemText (dialogID, itmIdx, L"��\n����");

				// ���� ��ġ
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 235, 110, 40, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"����");
				DGShowItem (dialogID, itmIdx);

				// ������ ��ư ����
				btnPosX = 230;
				btnPosY = 140 + (btnSizeY * (placingZone.nCells-1));
				for (xx = 0 ; xx < placingZone.nCells ; ++xx) {

					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					txtButton = "";
					if (placingZone.cellsB1 [xx].objType == NONE) {
						txtButton = "NONE";
					} else if (placingZone.cellsB1 [xx].objType == EUROFORM) {
						txtButton = format_string ("������\n��%.0f", placingZone.cellsB1 [xx].height * 1000);
					}
					DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// �׸��� ��ư �ؽ�Ʈ ����
					DGShowItem (dialogID, itmIdx);
					btnPosY -= 50;

					if (xx == 0)
						EUROFORM_BUTTON_BOTTOM = itmIdx;
					if (xx == placingZone.nCells-1)
						EUROFORM_BUTTON_TOP = itmIdx;
				}

				// �߰�/���� ��ư
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 310, 113, 50, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"�߰�");
				DGShowItem (dialogID, itmIdx);
				ADD_CELLS = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 310, 142, 50, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"����");
				DGShowItem (dialogID, itmIdx);
				DEL_CELLS = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 290, 134, 20, 20);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"��");
				DGShowItem (dialogID, itmIdx);

				// �� �ܸ��� �ǹ��ϴ� ���簢��
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 500, 100, 80, 80);
				DGShowItem (dialogID, itmIdx);

				// ��: ��������
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 530, 105, 25, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"��");
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 530, 155, 25, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"��");
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 505, 130, 25, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"��");
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 550, 130, 25, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"��");
				DGShowItem (dialogID, itmIdx);

				// üũ�ڽ�: �������� ���� ä�� ����
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 505, 50, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"���� ä��");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtNorth == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// ��
				CHECKBOX_NORTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 505, 210, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"���� ä��");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtSouth == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// ��
				CHECKBOX_SOUTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 420, 115, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"���� ä��");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtWest == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// ��
				CHECKBOX_WEST_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 590, 115, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"���� ä��");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtEast == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// ��
				CHECKBOX_EAST_MARGIN = itmIdx;

				// ���� ��� (��)
				if (placingZone.bExistBeams [NORTH] == true)
					placingZone.marginTopAtNorth = placingZone.bottomLevelOfBeams [NORTH];
				else
					placingZone.marginTopAtNorth = placingZone.areaHeight;
				for (xx = 0 ; xx < placingZone.nCells ; ++xx)
					placingZone.marginTopAtNorth -= placingZone.cellsB1 [xx].height;

				// ���� ��� (��)
				if (placingZone.bExistBeams [SOUTH] == true)
					placingZone.marginTopAtSouth = placingZone.bottomLevelOfBeams [SOUTH];
				else
					placingZone.marginTopAtSouth = placingZone.areaHeight;
				for (xx = 0 ; xx < placingZone.nCells ; ++xx)
					placingZone.marginTopAtSouth -= placingZone.cellsB1 [xx].height;

				// ���� ��� (��)
				if (placingZone.bExistBeams [WEST] == true)
					placingZone.marginTopAtWest = placingZone.bottomLevelOfBeams [WEST];
				else
					placingZone.marginTopAtWest = placingZone.areaHeight;
				for (xx = 0 ; xx < placingZone.nCells ; ++xx)
					placingZone.marginTopAtWest -= placingZone.cellsB1 [xx].height;

				// ���� ��� (��)
				if (placingZone.bExistBeams [EAST] == true)
					placingZone.marginTopAtEast = placingZone.bottomLevelOfBeams [EAST];
				else
					placingZone.marginTopAtEast = placingZone.areaHeight;
				for (xx = 0 ; xx < placingZone.nCells ; ++xx)
					placingZone.marginTopAtEast -= placingZone.cellsB1 [xx].height;

				// Edit ��Ʈ��: �������� ���� ġ�� ǥ��
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 520, 75, 40, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemValDouble (dialogID, itmIdx, placingZone.marginTopAtNorth);
				DGShowItem (dialogID, itmIdx);	// ��
				DGDisableItem (dialogID, itmIdx);
				EDITCONTROL_NORTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 520, 182, 40, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemValDouble (dialogID, itmIdx, placingZone.marginTopAtSouth);
				DGShowItem (dialogID, itmIdx);	// ��
				DGDisableItem (dialogID, itmIdx);
				EDITCONTROL_SOUTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 440, 142, 40, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemValDouble (dialogID, itmIdx, placingZone.marginTopAtWest);
				DGShowItem (dialogID, itmIdx);	// ��
				DGDisableItem (dialogID, itmIdx);
				EDITCONTROL_WEST_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 600, 142, 40, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemValDouble (dialogID, itmIdx, placingZone.marginTopAtEast);
				DGShowItem (dialogID, itmIdx);	// ��
				DGDisableItem (dialogID, itmIdx);
				EDITCONTROL_EAST_MARGIN = itmIdx;

				// ���� â ũ�⸦ ����
				dialogSizeX = 700;
				dialogSizeY = max<short>(300, 300 + (btnSizeY * (placingZone.nCells - 1)));
				DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);
			}

			// ������ ��ư
			if ((item >= EUROFORM_BUTTON_BOTTOM) && (item <= EUROFORM_BUTTON_TOP)) {
				// [DIALOG] �׸��� ��ư�� ������ Cell�� �����ϱ� ���� ���� â(3��° ���̾�α�)�� ����
				clickedBtnItemIdx = item;
				result = DGBlankModalDialog (200, 200, DG_DLG_NOGROW, 0, DG_DLG_NORMALFRAME, columnTableformPlacerHandler_soleColumn_3, 0);
				item = 0;

				// ����� ���� ä�� ���� ����
				if (DGGetItemValLong (dialogID, CHECKBOX_NORTH_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtNorth = true;
				else
					placingZone.bFillMarginTopAtNorth = false;

				if (DGGetItemValLong (dialogID, CHECKBOX_SOUTH_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtSouth = true;
				else
					placingZone.bFillMarginTopAtSouth = false;

				if (DGGetItemValLong (dialogID, CHECKBOX_WEST_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtWest = true;
				else
					placingZone.bFillMarginTopAtWest = false;

				if (DGGetItemValLong (dialogID, CHECKBOX_EAST_MARGIN) == TRUE)
					placingZone.bFillMarginTopAtEast = true;
				else
					placingZone.bFillMarginTopAtEast = false;

				// �� ���� ���� �߻�, ��� ���� ��ġ ���� ������Ʈ
				placingZone.alignPlacingZone_soleColumn (&placingZone);

				// ���� ���ɼ��� �ִ� DG �׸� ��� ����
				DGRemoveDialogItems (dialogID, AFTER_ALL);

				// ���� �ǹ��ϴ� ���簢��
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 220, 30, 70, 175 + (btnSizeY * (placingZone.nCells-1)));
				DGShowItem (dialogID, itmIdx);

				// �� �ܸ�
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 230, 40, btnSizeX, btnSizeY);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem (dialogID, itmIdx);
				DGDisableItem (dialogID, itmIdx);
				if (placingZone.bInterfereBeam == true)
					DGSetItemText (dialogID, itmIdx, L"��\n����");
				else
					DGSetItemText (dialogID, itmIdx, L"��\n����");

				// ���� ��ġ
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 235, 110, 40, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"����");
				DGShowItem (dialogID, itmIdx);

				// ������ ��ư ����
				btnPosX = 230;
				btnPosY = 140 + (btnSizeY * (placingZone.nCells-1));
				for (xx = 0 ; xx < placingZone.nCells ; ++xx) {

					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					txtButton = "";
					if (placingZone.cellsB1 [xx].objType == NONE) {
						txtButton = "NONE";
					} else if (placingZone.cellsB1 [xx].objType == EUROFORM) {
						txtButton = format_string ("������\n��%.0f", placingZone.cellsB1 [xx].height * 1000);
					}
					DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// �׸��� ��ư �ؽ�Ʈ ����
					DGShowItem (dialogID, itmIdx);
					btnPosY -= 50;

					if (xx == 0)
						EUROFORM_BUTTON_BOTTOM = itmIdx;
					if (xx == placingZone.nCells-1)
						EUROFORM_BUTTON_TOP = itmIdx;
				}

				// �߰�/���� ��ư
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 310, 113, 50, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"�߰�");
				DGShowItem (dialogID, itmIdx);
				ADD_CELLS = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 310, 142, 50, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"����");
				DGShowItem (dialogID, itmIdx);
				DEL_CELLS = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 290, 134, 20, 20);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"��");
				DGShowItem (dialogID, itmIdx);

				// �� �ܸ��� �ǹ��ϴ� ���簢��
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 500, 100, 80, 80);
				DGShowItem (dialogID, itmIdx);

				// ��: ��������
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 530, 105, 25, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"��");
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 530, 155, 25, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"��");
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 505, 130, 25, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"��");
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 550, 130, 25, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"��");
				DGShowItem (dialogID, itmIdx);

				// üũ�ڽ�: �������� ���� ä�� ����
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 505, 50, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"���� ä��");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtNorth == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// ��
				CHECKBOX_NORTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 505, 210, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"���� ä��");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtSouth == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// ��
				CHECKBOX_SOUTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 420, 115, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"���� ä��");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtWest == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// ��
				CHECKBOX_WEST_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 590, 115, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"���� ä��");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtEast == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// ��
				CHECKBOX_EAST_MARGIN = itmIdx;

				// ���� ��� (��)
				if (placingZone.bExistBeams [NORTH] == true)
					placingZone.marginTopAtNorth = placingZone.bottomLevelOfBeams [NORTH];
				else
					placingZone.marginTopAtNorth = placingZone.areaHeight;
				for (xx = 0 ; xx < placingZone.nCells ; ++xx)
					placingZone.marginTopAtNorth -= placingZone.cellsB1 [xx].height;

				// ���� ��� (��)
				if (placingZone.bExistBeams [SOUTH] == true)
					placingZone.marginTopAtSouth = placingZone.bottomLevelOfBeams [SOUTH];
				else
					placingZone.marginTopAtSouth = placingZone.areaHeight;
				for (xx = 0 ; xx < placingZone.nCells ; ++xx)
					placingZone.marginTopAtSouth -= placingZone.cellsB1 [xx].height;

				// ���� ��� (��)
				if (placingZone.bExistBeams [WEST] == true)
					placingZone.marginTopAtWest = placingZone.bottomLevelOfBeams [WEST];
				else
					placingZone.marginTopAtWest = placingZone.areaHeight;
				for (xx = 0 ; xx < placingZone.nCells ; ++xx)
					placingZone.marginTopAtWest -= placingZone.cellsB1 [xx].height;

				// ���� ��� (��)
				if (placingZone.bExistBeams [EAST] == true)
					placingZone.marginTopAtEast = placingZone.bottomLevelOfBeams [EAST];
				else
					placingZone.marginTopAtEast = placingZone.areaHeight;
				for (xx = 0 ; xx < placingZone.nCells ; ++xx)
					placingZone.marginTopAtEast -= placingZone.cellsB1 [xx].height;

				// Edit ��Ʈ��: �������� ���� ġ�� ǥ��
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 520, 75, 40, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemValDouble (dialogID, itmIdx, placingZone.marginTopAtNorth);
				DGShowItem (dialogID, itmIdx);	// ��
				DGDisableItem (dialogID, itmIdx);
				EDITCONTROL_NORTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 520, 182, 40, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemValDouble (dialogID, itmIdx, placingZone.marginTopAtSouth);
				DGShowItem (dialogID, itmIdx);	// ��
				DGDisableItem (dialogID, itmIdx);
				EDITCONTROL_SOUTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 440, 142, 40, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemValDouble (dialogID, itmIdx, placingZone.marginTopAtWest);
				DGShowItem (dialogID, itmIdx);	// ��
				DGDisableItem (dialogID, itmIdx);
				EDITCONTROL_WEST_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 600, 142, 40, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemValDouble (dialogID, itmIdx, placingZone.marginTopAtEast);
				DGShowItem (dialogID, itmIdx);	// ��
				DGDisableItem (dialogID, itmIdx);
				EDITCONTROL_EAST_MARGIN = itmIdx;

				// ���� â ũ�⸦ ����
				dialogSizeX = 700;
				dialogSizeY = max<short>(300, 300 + (btnSizeY * (placingZone.nCells - 1)));
				DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);
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
short DGCALLBACK columnTableformPlacerHandler_soleColumn_3 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	idx = -1;
	short	popupSelectedIdx = -1;
	double	length;

	switch (message) {
		case DG_MSG_INIT:

			// ��ġ ��ư
			if ((clickedBtnItemIdx >= EUROFORM_BUTTON_BOTTOM) && (clickedBtnItemIdx <= EUROFORM_BUTTON_TOP))
				idx = clickedBtnItemIdx - EUROFORM_BUTTON_BOTTOM;

			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, L"�� ����");

			//////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			// ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 30, 160, 60, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, L"����");
			DGShowItem (dialogID, DG_OK);

			// ��� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 110, 160, 60, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, L"���");
			DGShowItem (dialogID, DG_CANCEL);

			//////////////////////////////////////////////////////////// �ʵ� ���� (Ŭ���� ��)
			// ��: ��ü Ÿ��
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 10, 20, 70, 23);
			DGSetItemFont (dialogID, LABEL_OBJ_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_OBJ_TYPE, L"��ü Ÿ��");
			DGShowItem (dialogID, LABEL_OBJ_TYPE);

			// �˾���Ʈ��: ��ü Ÿ���� �ٲ� �� �ִ� �޺��ڽ��� �� ���� ����
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 90, 20-7, 100, 25);
			DGSetItemFont (dialogID, POPUP_OBJ_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, L"����");
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, L"������");
			DGShowItem (dialogID, POPUP_OBJ_TYPE);

			// üũ�ڽ�: �԰���
			DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 20, 60, 70, 25-5);
			DGSetItemFont (dialogID, CHECKBOX_SET_STANDARD, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, CHECKBOX_SET_STANDARD, L"�԰���");

			// ��: ����
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 20, 90, 50, 23);
			DGSetItemFont (dialogID, LABEL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_LENGTH, L"����");

			// Edit ��Ʈ��: ����
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 80, 90-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);

			// �˾� ��Ʈ��: ����
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 80, 90-6, 80, 25);
			DGSetItemFont (dialogID, POPUP_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_LENGTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_LENGTH, DG_POPUP_BOTTOM, "1200");
			DGPopUpInsertItem (dialogID, POPUP_LENGTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_LENGTH, DG_POPUP_BOTTOM, "900");
			DGPopUpInsertItem (dialogID, POPUP_LENGTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_LENGTH, DG_POPUP_BOTTOM, "600");

			// �ʱ� �Է� �ʵ� ǥ��
			if (placingZone.cellsB1 [idx].objType == NONE) {
				DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, NONE + 1);

			} else if (placingZone.cellsB1 [idx].objType == EUROFORM) {
				DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, EUROFORM + 1);

				DGShowItem (dialogID, CHECKBOX_SET_STANDARD);
				DGShowItem (dialogID, LABEL_LENGTH);
				if (placingZone.cellsB1 [idx].bStandardEuroform == true) {
					DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD, TRUE);
					DGShowItem (dialogID, POPUP_LENGTH);
					if (abs (placingZone.cellsB1 [idx].height - 1.200) < EPS)		popupSelectedIdx = 1;
					if (abs (placingZone.cellsB1 [idx].height - 0.900) < EPS)		popupSelectedIdx = 2;
					if (abs (placingZone.cellsB1 [idx].height - 0.600) < EPS)		popupSelectedIdx = 3;
					DGPopUpSelectItem (dialogID, POPUP_LENGTH, popupSelectedIdx);
				} else {
					DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD, FALSE);
					DGShowItem (dialogID, EDITCONTROL_LENGTH);
					DGSetItemValDouble (dialogID, EDITCONTROL_LENGTH, placingZone.cellsB1 [idx].height);
					DGSetItemMinDouble (dialogID, EDITCONTROL_LENGTH, 0.050);
					DGSetItemMaxDouble (dialogID, EDITCONTROL_LENGTH, 1.500);
				}
			}

			break;

		case DG_MSG_CHANGE:
			switch (item) {
				case POPUP_OBJ_TYPE:
					if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == NONE + 1) {
						DGHideItem (dialogID, CHECKBOX_SET_STANDARD);
						DGHideItem (dialogID, LABEL_LENGTH);
						DGHideItem (dialogID, EDITCONTROL_LENGTH);
						DGHideItem (dialogID, POPUP_LENGTH);
					} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == EUROFORM + 1) {
						DGShowItem (dialogID, CHECKBOX_SET_STANDARD);
						DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD, TRUE);
						DGShowItem (dialogID, LABEL_LENGTH);
						DGShowItem (dialogID, POPUP_LENGTH);
						DGHideItem (dialogID, EDITCONTROL_LENGTH);
					}
					break;

				case CHECKBOX_SET_STANDARD:
					if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == TRUE) {
						DGShowItem (dialogID, POPUP_LENGTH);
						DGHideItem (dialogID, EDITCONTROL_LENGTH);
					} else {
						DGHideItem (dialogID, POPUP_LENGTH);
						DGShowItem (dialogID, EDITCONTROL_LENGTH);
					}
					break;
			}
		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					idx = -1;

					// ��ġ ��ư
					if ((clickedBtnItemIdx >= EUROFORM_BUTTON_BOTTOM) && (clickedBtnItemIdx <= EUROFORM_BUTTON_TOP))
						idx = clickedBtnItemIdx - EUROFORM_BUTTON_BOTTOM;

					// �Է��� ���̸� �ش� ���� ��� ��ü�鿡�� ������
					if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == NONE + 1) {

						placingZone.cellsLT [idx].objType = NONE;
						placingZone.cellsRT [idx].objType = NONE;
						placingZone.cellsLB [idx].objType = NONE;
						placingZone.cellsRB [idx].objType = NONE;
						placingZone.cellsT1 [idx].objType = NONE;
						placingZone.cellsT2 [idx].objType = NONE;
						placingZone.cellsT3 [idx].objType = NONE;
						placingZone.cellsL1 [idx].objType = NONE;
						placingZone.cellsL2 [idx].objType = NONE;
						placingZone.cellsL3 [idx].objType = NONE;
						placingZone.cellsR1 [idx].objType = NONE;
						placingZone.cellsR2 [idx].objType = NONE;
						placingZone.cellsR3 [idx].objType = NONE;
						placingZone.cellsB1 [idx].objType = NONE;
						placingZone.cellsB2 [idx].objType = NONE;
						placingZone.cellsB3 [idx].objType = NONE;

					} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == EUROFORM + 1) {

						// �԰������� ������ ���
						if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == TRUE)
							length = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
						// ��԰������� ������ ���
						else
							length = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);

						if (placingZone.bUseOutcornerPanel == true) {
							placingZone.cellsLT [idx].objType = OUTPANEL;
							placingZone.cellsRT [idx].objType = OUTPANEL;
							placingZone.cellsLB [idx].objType = OUTPANEL;
							placingZone.cellsRB [idx].objType = OUTPANEL;
						} else {
							placingZone.cellsLT [idx].objType = OUTANGLE;
							placingZone.cellsRT [idx].objType = OUTANGLE;
							placingZone.cellsLB [idx].objType = OUTANGLE;
							placingZone.cellsRB [idx].objType = OUTANGLE;
						}
							
						placingZone.cellsLT [idx].height = length;
						placingZone.cellsRT [idx].height = length;
						placingZone.cellsLB [idx].height = length;
						placingZone.cellsRB [idx].height = length;

						placingZone.cellsT1 [idx].objType = EUROFORM;
						placingZone.cellsT1 [idx].height = length;
						
						placingZone.cellsT2 [idx].objType = FILLERSP;
						placingZone.cellsT2 [idx].height = length;
						
						placingZone.cellsT3 [idx].objType = EUROFORM;
						placingZone.cellsT3 [idx].height = length;

						placingZone.cellsL1 [idx].objType = EUROFORM;
						placingZone.cellsL1 [idx].height = length;

						placingZone.cellsL2 [idx].objType = FILLERSP;
						placingZone.cellsL2 [idx].height = length;

						placingZone.cellsL3 [idx].objType = EUROFORM;
						placingZone.cellsL3 [idx].height = length;

						placingZone.cellsR1 [idx].objType = EUROFORM;
						placingZone.cellsR1 [idx].height = length;

						placingZone.cellsR2 [idx].objType = FILLERSP;
						placingZone.cellsR2 [idx].height = length;

						placingZone.cellsR3 [idx].objType = EUROFORM;
						placingZone.cellsR3 [idx].height = length;

						placingZone.cellsB1 [idx].objType = EUROFORM;
						placingZone.cellsB1 [idx].height = length;

						placingZone.cellsB2 [idx].objType = FILLERSP;
						placingZone.cellsB2 [idx].height = length;

						placingZone.cellsB3 [idx].objType = EUROFORM;
						placingZone.cellsB3 [idx].height = length;
					}

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
