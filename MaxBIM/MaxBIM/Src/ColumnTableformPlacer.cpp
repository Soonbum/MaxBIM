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
static short			layerInd_OutPanel;			// ���̾� ��ȣ: �ƿ��ڳ��ǳ�
static short			layerInd_OutAngle;			// ���̾� ��ȣ: �ƿ��ڳʾޱ�
static short			layerInd_SqrPipe;			// ���̾� ��ȣ: ���������
static short			layerInd_Pinbolt;			// ���̾� ��ȣ: �ɺ�Ʈ��Ʈ
static short			layerInd_Hanger;			// ���̾� ��ȣ: �����������
static short			layerInd_Head;				// ���̾� ��ȣ: ������Ʈ�� Push-Pull Props ����ǽ�
static short			layerInd_ColumnBand;		// ���̾� ��ȣ: ��չ��
static short			layerInd_Plywood;			// ���̾� ��ȣ: ����
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
	long			nSel;
	short			xx, yy;
	short			result;
	double			lowestBeamBottomLevel;
	API_Coord		axisPoint, rotatedPoint, unrotatedPoint;

	// Selection Manager ���� ����
	API_SelectionInfo		selectionInfo;
	API_Element				tElem;
	API_Neig				**selNeigs;
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
	API_StoryInfo			storyInfo;
	double					workLevel_column;
	double					workLevel_beam;


	// ������ ��� ��������
	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err == APIERR_NOPLAN) {
		ACAPI_WriteReport ("���� ������Ʈ â�� �����ϴ�.", true);
	}
	if (err == APIERR_NOSEL) {
		ACAPI_WriteReport ("�ƹ� �͵� �������� �ʾҽ��ϴ�.\n�ʼ� ����: ��� (1��), ��� ������ ���� ���� (1��)\n�ɼ� ����: ��հ� �´�� �� (�ټ�)", true);
	}
	if (err != NoError) {
		BMKillHandle ((GSHandle *) &selNeigs);
		return err;
	}

	// ���� ��� 1�� �����ؾ� ��
	if (selectionInfo.typeID != API_SelEmpty) {
		nSel = BMGetHandleSize ((GSHandle) selNeigs) / sizeof (API_Neig);
		for (xx = 0 ; xx < nSel && err == NoError ; ++xx) {
			tElem.header.typeID = Neig_To_ElemID ((*selNeigs)[xx].neigID);

			tElem.header.guid = (*selNeigs)[xx].guid;
			if (ACAPI_Element_Get (&tElem) != NoError)	// ������ �� �ִ� ����ΰ�?
				continue;

			if (tElem.header.typeID == API_ColumnID)	// ����ΰ�?
				columns.Push (tElem.header.guid);

			if (tElem.header.typeID == API_MorphID)		// �����ΰ�?
				morphs.Push (tElem.header.guid);

			if (tElem.header.typeID == API_BeamID)		// ���ΰ�?
				beams.Push (tElem.header.guid);
		}
	}
	BMKillHandle ((GSHandle *) &selNeigs);
	nColumns = columns.GetSize ();
	nMorphs = morphs.GetSize ();
	nBeams = beams.GetSize ();

	// ����� 1���ΰ�?
	if (nColumns != 1) {
		ACAPI_WriteReport ("����� 1�� �����ؾ� �մϴ�.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// ������ 1���ΰ�?
	if (nMorphs != 1) {
		ACAPI_WriteReport ("��� ������ ���� ������ 1�� �����ϼž� �մϴ�.", true);
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
	infoColumn.bRectangle	= !elem.column.circleBased;		// ���簢���ΰ�?
	infoColumn.coreAnchor	= elem.column.coreAnchor;		// �ھ��� ��Ŀ ����Ʈ
	infoColumn.coreWidth	= elem.column.coreWidth;		// �ھ��� �ʺ� (X ����)
	infoColumn.coreDepth	= elem.column.coreDepth;		// �ھ��� ���� (Y ����)
	infoColumn.venThick		= elem.column.venThick;			// ���Ͼ� �β�
	infoColumn.height		= elem.column.height;			// ��� ����
	infoColumn.bottomOffset	= elem.column.bottomOffset;		// �ٴ� ������ ���� ��� ���̽� ����
	infoColumn.topOffset	= elem.column.topOffset;		// ���� ����� ������ ����Ǿ� �ִ� ��� �������κ����� ������
	infoColumn.angle		= elem.column.angle + elem.column.slantDirectionAngle;	// ��� ���� �߽����� �� ȸ�� ���� (����: Radian)
	infoColumn.origoPos		= elem.column.origoPos;			// ��� �߽� ��ġ

	ACAPI_DisposeElemMemoHdls (&memo);

	// �۾� �� ���� �ݿ�
	BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	workLevel_column = 0.0;
	ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
	for (yy = 0 ; yy < (storyInfo.lastStory - storyInfo.firstStory) ; ++yy) {
		if (storyInfo.data [0][yy].index == infoColumn.floorInd) {
			workLevel_column = storyInfo.data [0][yy].level;
			break;
		}
	}
	BMKillHandle ((GSHandle *) &storyInfo.data);

	infoColumn.bottomOffset += workLevel_column;
	infoColumn.topOffset += workLevel_column;

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
		infoOtherBeams [xx].height		= elem.beam.height;			// �� ����
		infoOtherBeams [xx].width		= elem.beam.width;			// �� �ʺ�
		infoOtherBeams [xx].offset		= elem.beam.offset;			// �� �߽����κ��� ���� ���۷��� ������ �������Դϴ�.
		infoOtherBeams [xx].level		= elem.beam.level;			// �ٴ� ������ ���� ���� ���ʸ� �����Դϴ�.
		infoOtherBeams [xx].begC		= elem.beam.begC;			// �� ���� ��ǥ
		infoOtherBeams [xx].endC		= elem.beam.endC;			// �� �� ��ǥ

		ACAPI_DisposeElemMemoHdls (&memo);

		// �۾� �� ���� �ݿ�
		BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
		workLevel_beam = 0.0;
		ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
		for (yy = 0 ; yy < (storyInfo.lastStory - storyInfo.firstStory) ; ++yy) {
			if (storyInfo.data [0][yy].index == infoOtherBeams [xx].floorInd) {
				workLevel_beam = storyInfo.data [0][yy].level;
				break;
			}
		}
		BMKillHandle ((GSHandle *) &storyInfo.data);

		infoOtherBeams [xx].level += workLevel_beam;
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
	API_Elem_Head* headList = new API_Elem_Head [1];
	headList [0] = elem.header;
	err = ACAPI_Element_Delete (&headList, 1);
	delete headList;

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
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32524, ACAPI_GetOwnResModule (), columnTableformPlacerHandler_soleColumn_1, 0);

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
	for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
		placingZone.cellsLT [xx].guid = placingZone.placeLibPart (placingZone.cellsLT [xx]);
		elemList.Push (placingZone.cellsLT [xx].guid);
		placingZone.cellsRT [xx].guid = placingZone.placeLibPart (placingZone.cellsRT [xx]);
		elemList.Push (placingZone.cellsRT [xx].guid);
		placingZone.cellsLB [xx].guid = placingZone.placeLibPart (placingZone.cellsLB [xx]);
		elemList.Push (placingZone.cellsLB [xx].guid);
		placingZone.cellsRB [xx].guid = placingZone.placeLibPart (placingZone.cellsRB [xx]);
		elemList.Push (placingZone.cellsRB [xx].guid);

		placingZone.cellsT1 [xx].guid = placingZone.placeLibPart (placingZone.cellsT1 [xx]);
		elemList.Push (placingZone.cellsT1 [xx].guid);
		placingZone.cellsT2 [xx].guid = placingZone.placeLibPart (placingZone.cellsT2 [xx]);
		elemList.Push (placingZone.cellsT2 [xx].guid);
		placingZone.cellsL1 [xx].guid = placingZone.placeLibPart (placingZone.cellsL1 [xx]);
		elemList.Push (placingZone.cellsL1 [xx].guid);
		placingZone.cellsL2 [xx].guid = placingZone.placeLibPart (placingZone.cellsL2 [xx]);
		elemList.Push (placingZone.cellsL2 [xx].guid);
		placingZone.cellsR1 [xx].guid = placingZone.placeLibPart  (placingZone.cellsR1 [xx]);
		elemList.Push (placingZone.cellsR1 [xx].guid);
		placingZone.cellsR2 [xx].guid = placingZone.placeLibPart (placingZone.cellsR2 [xx]);
		elemList.Push (placingZone.cellsR2 [xx].guid);
		placingZone.cellsB1 [xx].guid = placingZone.placeLibPart (placingZone.cellsB1 [xx]);
		elemList.Push (placingZone.cellsB1 [xx].guid);
		placingZone.cellsB2 [xx].guid = placingZone.placeLibPart (placingZone.cellsB2 [xx]);
		elemList.Push (placingZone.cellsB2 [xx].guid);
	}

	// ���������, �ɺ�Ʈ��Ʈ/�����������, ����ǽ� ��ġ
	err = placingZone.placeRestObjects_soleColumn (&placingZone);

	// ������ ���� ä��� - ����, ����
	err = placingZone.fillRestAreas_soleColumn (&placingZone);

	// �׷�ȭ�ϱ�
	if (!elemList.IsEmpty ()) {
		GSSize nElems = elemList.GetSize ();
		API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
		if (elemHead != NULL) {
			for (GSIndex i = 0; i < nElems; i++)
				(*elemHead)[i].guid = elemList[i];

			ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

			BMKillHandle ((GSHandle *) &elemHead);
		}
	}
	elemList.Clear (false);

	// ȭ�� ���ΰ�ħ
	ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	bool	regenerate = true;
	ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

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
	target_zone->cellsL1 [target_zone->nCells] = target_zone->cellsL1 [target_zone->nCells - 1];
	target_zone->cellsL2 [target_zone->nCells] = target_zone->cellsL2 [target_zone->nCells - 1];
	target_zone->cellsR1 [target_zone->nCells] = target_zone->cellsR1 [target_zone->nCells - 1];
	target_zone->cellsR2 [target_zone->nCells] = target_zone->cellsR2 [target_zone->nCells - 1];
	target_zone->cellsB1 [target_zone->nCells] = target_zone->cellsB1 [target_zone->nCells - 1];
	target_zone->cellsB2 [target_zone->nCells] = target_zone->cellsB2 [target_zone->nCells - 1];

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
	placingZone->cellsL1 [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsL2 [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsR1 [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsR2 [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsB1 [0].leftBottomZ = placingZone->bottomOffset;
	placingZone->cellsB2 [0].leftBottomZ = placingZone->bottomOffset;

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

			// ���� 1
			placingZone->cellsL1 [xx].leftBottomZ = placingZone->cellsL1 [xx-1].leftBottomZ + placingZone->cellsL1 [xx-1].height;

			// ���� 2
			placingZone->cellsL2 [xx].leftBottomZ = placingZone->cellsL2 [xx-1].leftBottomZ + placingZone->cellsL2 [xx-1].height;

			// ���� 1
			placingZone->cellsR1 [xx].leftBottomZ = placingZone->cellsR1 [xx-1].leftBottomZ + placingZone->cellsR1 [xx-1].height;

			// ���� 2
			placingZone->cellsR2 [xx].leftBottomZ = placingZone->cellsR2 [xx-1].leftBottomZ + placingZone->cellsR2 [xx-1].height;

			// �Ʒ��� 1
			placingZone->cellsB1 [xx].leftBottomZ = placingZone->cellsB1 [xx-1].leftBottomZ + placingZone->cellsB1 [xx-1].height;

			// �Ʒ��� 2
			placingZone->cellsB2 [xx].leftBottomZ = placingZone->cellsB2 [xx-1].leftBottomZ + placingZone->cellsB2 [xx-1].height;
		}

		// ���� 1
		formWidth = placingZone->cellsT1 [xx].libPart.form.eu_wid;
		formHeight = placingZone->cellsT1 [xx].libPart.form.eu_hei;
		placingZone->cellsT1 [xx].libPart.form.eu_stan_onoff = false;
		if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
			if ( (abs (formHeight - 1.200) < EPS) || (abs (formHeight - 0.900) < EPS) || (abs (formHeight - 0.600) < EPS) )
				placingZone->cellsT1 [xx].libPart.form.eu_stan_onoff = true;
		
		// ���� 2
		formWidth = placingZone->cellsT2 [xx].libPart.form.eu_wid;
		formHeight = placingZone->cellsT2 [xx].libPart.form.eu_hei;
		placingZone->cellsT2 [xx].libPart.form.eu_stan_onoff = false;
		if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
			if ( (abs (formHeight - 1.200) < EPS) || (abs (formHeight - 0.900) < EPS) || (abs (formHeight - 0.600) < EPS) )
				placingZone->cellsT2 [xx].libPart.form.eu_stan_onoff = true;

		// ���� 1
		formWidth = placingZone->cellsL1 [xx].libPart.form.eu_wid;
		formHeight = placingZone->cellsL1 [xx].libPart.form.eu_hei;
		placingZone->cellsL1 [xx].libPart.form.eu_stan_onoff = false;
		if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
			if ( (abs (formHeight - 1.200) < EPS) || (abs (formHeight - 0.900) < EPS) || (abs (formHeight - 0.600) < EPS) )
				placingZone->cellsL1 [xx].libPart.form.eu_stan_onoff = true;

		// ���� 2
		formWidth = placingZone->cellsL2 [xx].libPart.form.eu_wid;
		formHeight = placingZone->cellsL2 [xx].libPart.form.eu_hei;
		placingZone->cellsL2 [xx].libPart.form.eu_stan_onoff = false;
		if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
			if ( (abs (formHeight - 1.200) < EPS) || (abs (formHeight - 0.900) < EPS) || (abs (formHeight - 0.600) < EPS) )
				placingZone->cellsL2 [xx].libPart.form.eu_stan_onoff = true;

		// ���� 1
		formWidth = placingZone->cellsR1 [xx].libPart.form.eu_wid;
		formHeight = placingZone->cellsR1 [xx].libPart.form.eu_hei;
		placingZone->cellsR1 [xx].libPart.form.eu_stan_onoff = false;
		if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
			if ( (abs (formHeight - 1.200) < EPS) || (abs (formHeight - 0.900) < EPS) || (abs (formHeight - 0.600) < EPS) )
				placingZone->cellsR1 [xx].libPart.form.eu_stan_onoff = true;

		// ���� 2
		formWidth = placingZone->cellsR2 [xx].libPart.form.eu_wid;
		formHeight = placingZone->cellsR2 [xx].libPart.form.eu_hei;
		placingZone->cellsR2 [xx].libPart.form.eu_stan_onoff = false;
		if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
			if ( (abs (formHeight - 1.200) < EPS) || (abs (formHeight - 0.900) < EPS) || (abs (formHeight - 0.600) < EPS) )
				placingZone->cellsR2 [xx].libPart.form.eu_stan_onoff = true;

		// �Ʒ��� 1
		formWidth = placingZone->cellsB1 [xx].libPart.form.eu_wid;
		formHeight = placingZone->cellsB1 [xx].libPart.form.eu_hei;
		placingZone->cellsB1 [xx].libPart.form.eu_stan_onoff = false;
		if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
			if ( (abs (formHeight - 1.200) < EPS) || (abs (formHeight - 0.900) < EPS) || (abs (formHeight - 0.600) < EPS) )
				placingZone->cellsB1 [xx].libPart.form.eu_stan_onoff = true;

		// �Ʒ��� 2
		formWidth = placingZone->cellsB2 [xx].libPart.form.eu_wid;
		formHeight = placingZone->cellsB2 [xx].libPart.form.eu_hei;
		placingZone->cellsB2 [xx].libPart.form.eu_stan_onoff = false;
		if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
			if ( (abs (formHeight - 1.200) < EPS) || (abs (formHeight - 0.900) < EPS) || (abs (formHeight - 0.600) < EPS) )
				placingZone->cellsB2 [xx].libPart.form.eu_stan_onoff = true;
	}
}

// �ش� �� ������ ������� ���̺귯�� ��ġ
API_Guid	ColumnTableformPlacingZone::placeLibPart (CellForColumnTableform objInfo)
{
	GSErrCode	err = NoError;

	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const	GS::uchar_t* gsmName = NULL;
	double	aParam;
	double	bParam;
	Int32	addParNum;

	double	validLength = 0.0;	// ��ȿ�� �����ΰ�?
	double	validWidth = 0.0;	// ��ȿ�� �ʺ��ΰ�?

	char	tempString [20];

	short		xx;
	API_StoryInfo	storyInfo;

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
	if (objInfo.objType == OUTPANEL)		gsmName = L("�ƿ��ڳ��ǳ�v1.0.gsm");
	if (objInfo.objType == OUTANGLE)		gsmName = L("�ƿ��ڳʾޱ�v1.0.gsm");
	if (objInfo.objType == SQUARE_PIPE)		gsmName = L("���������v1.0.gsm");
	if (objInfo.objType == PINBOLT)			gsmName = L("�ɺ�Ʈ��Ʈv1.0.gsm");
	if (objInfo.objType == HANGER)			gsmName = L("�����������.gsm");
	if (objInfo.objType == HEAD)			gsmName = L("������Ʈ�� Push-Pull Props ����ǽ� v1.0.gsm");
	if (objInfo.objType == COLUMN_BAND1)	gsmName = L("��չ��v2.0.gsm");
	if (objInfo.objType == COLUMN_BAND2)	gsmName = L("����v1.0.gsm");
	if (objInfo.objType == PLYWOOD)			gsmName = L("����v1.0.gsm");

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
	element.object.reflected = false;
	element.object.pos.x = objInfo.leftBottomX;
	element.object.pos.y = objInfo.leftBottomY;
	element.object.level = objInfo.leftBottomZ;
	element.object.xRatio = aParam;
	element.object.yRatio = bParam;
	element.object.angle = objInfo.ang;
	element.header.floorInd = infoColumn.floorInd;

	// �۾� �� ���� �ݿ�
	BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
	for (xx = 0 ; xx < (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
		if (storyInfo.data [0][xx].index == infoColumn.floorInd) {
			element.object.level -= storyInfo.data [0][xx].level;
			break;
		}
	}
	BMKillHandle ((GSHandle *) &storyInfo.data);

	if (objInfo.objType == EUROFORM) {
		element.header.layer = layerInd_Euroform;

		// �԰�ǰ�� ���,
		if (objInfo.libPart.form.eu_stan_onoff == true) {
			// �԰��� On/Off
			setParameterByName (&memo, "eu_stan_onoff", 1.0);	// �԰��� On/Off

			// �ʺ�
			sprintf (tempString, "%.0f", objInfo.libPart.form.eu_wid * 1000);
			setParameterByName (&memo, "eu_wid", tempString);

			// ����
			sprintf (tempString, "%.0f", objInfo.libPart.form.eu_hei * 1000);
			setParameterByName (&memo, "eu_hei", tempString);

		// ��԰�ǰ�� ���,
		} else {
			setParameterByName (&memo, "eu_stan_onoff", 0.0);	// �԰��� On/Off
			setParameterByName (&memo, "eu_wid2", objInfo.libPart.form.eu_wid2);	// �ʺ�
			setParameterByName (&memo, "eu_hei2", objInfo.libPart.form.eu_hei2);	// ����
		}

		// ��ġ����
		if (objInfo.libPart.form.u_ins_wall == true) {
			strcpy (tempString, "�������");
			if (objInfo.libPart.form.eu_stan_onoff == true) {
				validWidth = objInfo.libPart.form.eu_wid;
				validLength = objInfo.libPart.form.eu_hei;
			} else {
				validWidth = objInfo.libPart.form.eu_wid2;
				validLength = objInfo.libPart.form.eu_hei2;
			}
		} else {
			strcpy (tempString, "��������");
			if (objInfo.libPart.form.eu_stan_onoff == true) {
				element.object.pos.x += ( objInfo.libPart.form.eu_hei * cos(objInfo.ang) );
				element.object.pos.y += ( objInfo.libPart.form.eu_hei * sin(objInfo.ang) );
				validWidth = objInfo.libPart.form.eu_hei;
				validLength = objInfo.libPart.form.eu_wid;
			} else {
				element.object.pos.x += ( objInfo.libPart.form.eu_hei2 * cos(objInfo.ang) );
				element.object.pos.y += ( objInfo.libPart.form.eu_hei2 * sin(objInfo.ang) );
				validWidth = objInfo.libPart.form.eu_hei2;
				validLength = objInfo.libPart.form.eu_wid2;
			}
		}
		setParameterByName (&memo, "u_ins", tempString);

		// ȸ��X
		setParameterByName (&memo, "ang_x", DegreeToRad (90.0));	// ȸ��X

	} else if (objInfo.objType == OUTPANEL) {
		element.header.layer = layerInd_OutPanel;
		setParameterByName (&memo, "wid_s", objInfo.libPart.outpanel.wid_s);	// ����(����)
		setParameterByName (&memo, "leng_s", objInfo.libPart.outpanel.leng_s);	// ����(�Ķ�)
		setParameterByName (&memo, "hei_s", objInfo.libPart.outpanel.hei_s);	// ����
		setParameterByName (&memo, "dir_s", "�����");							// ��ġ����

		validWidth = objInfo.libPart.outpanel.leng_s + objInfo.libPart.outpanel.wid_s;
		validLength = objInfo.libPart.outpanel.hei_s;

	} else if (objInfo.objType == OUTANGLE) {
		element.header.layer = layerInd_OutAngle;
		setParameterByName (&memo, "a_leng", objInfo.libPart.outangle.a_leng);	// ����
		setParameterByName (&memo, "a_ang", objInfo.libPart.outangle.a_ang);	// ����

		validWidth = 0.063;
		validLength = objInfo.libPart.outangle.a_leng;

	} else if (objInfo.objType == SQUARE_PIPE) {
		element.header.layer = layerInd_SqrPipe;
		setParameterByName (&memo, "p_comp", "�簢������");						// ǰ��
		setParameterByName (&memo, "p_leng", objInfo.libPart.sqrPipe.length);	// ������ ����
		setParameterByName (&memo, "p_ang", objInfo.libPart.sqrPipe.pipeAng);	// ���� (����: 0, ����: 90)
		
		// Ÿ��
		if (objInfo.libPart.sqrPipe.bPunching == true)
			setParameterByName (&memo, "bPunching", 1.0);
		else
			setParameterByName (&memo, "bPunching", 0.0);

		if (objInfo.libPart.sqrPipe.bHoleDir == true)
			setParameterByName (&memo, "holeDir", "����");
		else
			setParameterByName (&memo, "holeDir", "����");

		validWidth = 0.050;
		validLength = objInfo.libPart.sqrPipe.length;

	} else if (objInfo.objType == PINBOLT) {
		element.header.layer = layerInd_Pinbolt;
		// �ɺ�Ʈ 90�� ȸ��
		if (objInfo.libPart.pinbolt.bPinBoltRot90 == true)
			setParameterByName (&memo, "bPinBoltRot90", 1.0);
		else
			setParameterByName (&memo, "bPinBoltRot90", 0.0);

		setParameterByName (&memo, "bolt_len", objInfo.libPart.pinbolt.boltLen);		// ��Ʈ ����
		setParameterByName (&memo, "washer_pos", objInfo.libPart.pinbolt.washerPos);	// �ͼ� ��ġ
		setParameterByName (&memo, "angX", objInfo.libPart.pinbolt.angX);				// X�� ȸ��
		setParameterByName (&memo, "angY", objInfo.libPart.pinbolt.angY);				// Y�� ȸ��

		validWidth = 0.100;
		validLength = objInfo.libPart.pinbolt.boltLen;

	} else if (objInfo.objType == HANGER) {
		element.header.layer = layerInd_Hanger;
		setParameterByName (&memo, "m_type", "�����������");					// ǰ��
		setParameterByName (&memo, "angX", objInfo.libPart.hanger.angX);		// X�� ȸ��
		setParameterByName (&memo, "angY", objInfo.libPart.hanger.angY);		// Y�� ȸ��

		validWidth = 1.0;
		validLength = 1.0;

	} else if (objInfo.objType == HEAD) {
		element.header.layer = layerInd_Head;

		validWidth = 1.0;
		validLength = 1.0;

	} else if (objInfo.objType == COLUMN_BAND1) {
		element.header.layer = layerInd_ColumnBand;

		// �԰�
		if (objInfo.libPart.columnBand.band_size == 1)
			setParameterByName (&memo, "band_size", "80x40x1270");
		else if (objInfo.libPart.columnBand.band_size == 2)
			setParameterByName (&memo, "band_size", "80x40x1620");
		else if (objInfo.libPart.columnBand.band_size == 3)
			setParameterByName (&memo, "band_size", "80x40x2020");
		else if (objInfo.libPart.columnBand.band_size == 4)
			setParameterByName (&memo, "band_size", "80x40x2420");

		setParameterByName (&memo, "c_w", objInfo.libPart.columnBand.c_w);				// ���� (����)
		setParameterByName (&memo, "c_h", objInfo.libPart.columnBand.c_h);				// ���� (����)
		setParameterByName (&memo, "addOffset", objInfo.libPart.columnBand.addOffset);	// �β� �߰� (�������� ��) - ��, ������ �β��� �ڵ����� ���ԵǹǷ� ���⿡���� ����

		validWidth = objInfo.libPart.columnBand.c_w;
		validLength = objInfo.libPart.columnBand.c_w;

	} else if (objInfo.objType == COLUMN_BAND2) {
		element.header.layer = layerInd_ColumnBand;

		setParameterByName (&memo, "c_w", objInfo.libPart.wella.c_w);				// ���� (����)
		setParameterByName (&memo, "c_h", objInfo.libPart.wella.c_h);				// ���� (����)
		setParameterByName (&memo, "addOffset", objInfo.libPart.wella.addOffset);	// �β� �߰� (�������� ��) - ��, ������ �β��� �ڵ����� ���ԵǹǷ� ���⿡���� ����

		validWidth = objInfo.libPart.wella.c_w;
		validLength = objInfo.libPart.wella.c_w;

	} else if (objInfo.objType == PLYWOOD) {
		element.header.layer = layerInd_Plywood;
		setParameterByName (&memo, "p_stan", "��԰�");							// �԰�
		setParameterByName (&memo, "w_dir", "�������");						// ��ġ����
		setParameterByName (&memo, "p_thk", "11.5T");							// �β�
		setParameterByName (&memo, "p_wid", objInfo.libPart.plywood.p_wid);		// ����
		setParameterByName (&memo, "p_leng", objInfo.libPart.plywood.p_leng);	// ����
		setParameterByName (&memo, "sogak", 0.0);								// ����Ʋ OFF
		setParameterByName (&memo, "p_ang", DegreeToRad (0.0));					// ����

		validLength = objInfo.libPart.plywood.p_leng;
		validWidth = objInfo.libPart.plywood.p_wid;
	}

	// ��ü ��ġ
	if ((objInfo.objType != NONE) && (validLength > EPS) && (validWidth > EPS))
		ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return element.header.guid;
}

// ���������, �ɺ�Ʈ��Ʈ/�����������, ����ǽ� ��ġ
GSErrCode	ColumnTableformPlacingZone::placeRestObjects_soleColumn (ColumnTableformPlacingZone* placingZone)
{
	GSErrCode	err = NoError;

	short	xx;
	double	xLen, yLen;
	double	heightOfFormArea = 0.0;
	double	elev_headpiece;

	CellForColumnTableform	insCell;

	// ����� ����, ���� ����
	xLen = (placingZone->coreWidth/2 + placingZone->venThick);
	yLen = (placingZone->coreDepth/2 + placingZone->venThick);

	// ���� ���� ���̸� ����
	for (xx = 0 ; xx < placingZone->nCells ; ++xx)
		heightOfFormArea += placingZone->cellsB1 [xx].height;

	// ����ǽ��� ���� ����
	if (heightOfFormArea >= 5.300) {
		elev_headpiece = 4.200;
	} else if ((heightOfFormArea >= 4.600) && (heightOfFormArea < 5.300)) {
		elev_headpiece = 3.800;
	} else if ((heightOfFormArea >= 3.500) && (heightOfFormArea < 4.600)) {
		elev_headpiece = 2.800;
	} else if ((heightOfFormArea >= 3.000) && (heightOfFormArea < 3.500)) {
		elev_headpiece = 2.200;
	} else if ((heightOfFormArea >= 2.500) && (heightOfFormArea < 3.000)) {
		elev_headpiece = 1.900;
	} else if ((heightOfFormArea >= 2.000) && (heightOfFormArea < 2.500)) {
		elev_headpiece = 1.500;
	} else if ((heightOfFormArea >= 1.500) && (heightOfFormArea < 2.000)) {
		elev_headpiece = 1.100;
	} else if ((heightOfFormArea >= 1.000) && (heightOfFormArea < 1.500)) {
		elev_headpiece = 0.800;
	} else {
		elev_headpiece = heightOfFormArea - 0.150;
	}

	// 1. ��������� ��ġ
	insCell.objType = SQUARE_PIPE;
	insCell.libPart.sqrPipe.length = heightOfFormArea;
	insCell.libPart.sqrPipe.pipeAng = DegreeToRad (90);
	insCell.libPart.sqrPipe.bPunching = true;
	insCell.libPart.sqrPipe.bHoleDir = true;

	// ����
	insCell.leftBottomX = placingZone->origoPos.x;
	insCell.leftBottomY = placingZone->origoPos.y;
	insCell.leftBottomZ = placingZone->bottomOffset;
	insCell.ang = placingZone->angle;

	if (placingZone->bUseOutcornerPanel == true) {
		moveIn3D ('x', insCell.ang, -xLen + placingZone->cellsLT [0].horLen, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('y', insCell.ang, yLen + 0.0635 + 0.025, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('z', insCell.ang, 0.050, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		elemList.Push (placeLibPart (insCell));
		moveIn3D ('x', insCell.ang, -placingZone->cellsLT [0].horLen + xLen*2 - placingZone->cellsRT [0].horLen, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		elemList.Push (placeLibPart (insCell));
	} else {
		moveIn3D ('x', insCell.ang, -xLen, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('y', insCell.ang, yLen + 0.0635 + 0.025, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('z', insCell.ang, 0.050, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		elemList.Push (placeLibPart (insCell));
		moveIn3D ('x', insCell.ang, xLen*2, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		elemList.Push (placeLibPart (insCell));
	}

	// �Ʒ���
	insCell.leftBottomX = placingZone->origoPos.x;
	insCell.leftBottomY = placingZone->origoPos.y;
	insCell.leftBottomZ = placingZone->bottomOffset;
	insCell.ang = placingZone->angle;

	if (placingZone->bUseOutcornerPanel == true) {
		moveIn3D ('x', insCell.ang, -xLen + placingZone->cellsLT [0].horLen, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('y', insCell.ang, -(yLen + 0.0635 + 0.025), &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('z', insCell.ang, 0.050, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		elemList.Push (placeLibPart (insCell));
		moveIn3D ('x', insCell.ang, -placingZone->cellsLT [0].horLen + xLen*2 - placingZone->cellsRT [0].horLen, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		elemList.Push (placeLibPart (insCell));
	} else {
		moveIn3D ('x', insCell.ang, -xLen, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('y', insCell.ang, -(yLen + 0.0635 + 0.025), &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('z', insCell.ang, 0.050, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		elemList.Push (placeLibPart (insCell));
		moveIn3D ('x', insCell.ang, xLen*2, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		elemList.Push (placeLibPart (insCell));
	}

	// ����
	insCell.leftBottomX = placingZone->origoPos.x;
	insCell.leftBottomY = placingZone->origoPos.y;
	insCell.leftBottomZ = placingZone->bottomOffset;
	insCell.ang = placingZone->angle;

	if (placingZone->bUseOutcornerPanel == true) {
		moveIn3D ('x', insCell.ang, -(xLen + 0.0635 + 0.025), &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('y', insCell.ang, yLen - placingZone->cellsLT [0].verLen, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('z', insCell.ang, 0.050, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		insCell.ang += DegreeToRad (90);
		elemList.Push (placeLibPart (insCell));
		insCell.ang -= DegreeToRad (90);
		moveIn3D ('y', insCell.ang, placingZone->cellsLT [0].verLen - yLen*2 + placingZone->cellsLB [0].verLen, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		insCell.ang += DegreeToRad (90);
		elemList.Push (placeLibPart (insCell));
		insCell.ang -= DegreeToRad (90);
	} else {
		moveIn3D ('x', insCell.ang, -(xLen + 0.0635 + 0.025), &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('y', insCell.ang, yLen, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('z', insCell.ang, 0.050, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		insCell.ang += DegreeToRad (90);
		elemList.Push (placeLibPart (insCell));
		insCell.ang -= DegreeToRad (90);
		moveIn3D ('y', insCell.ang, -yLen*2, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		insCell.ang += DegreeToRad (90);
		elemList.Push (placeLibPart (insCell));
		insCell.ang -= DegreeToRad (90);
	}

	// ������
	insCell.leftBottomX = placingZone->origoPos.x;
	insCell.leftBottomY = placingZone->origoPos.y;
	insCell.leftBottomZ = placingZone->bottomOffset;
	insCell.ang = placingZone->angle;

	if (placingZone->bUseOutcornerPanel == true) {
		moveIn3D ('x', insCell.ang, (xLen + 0.0635 + 0.025), &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('y', insCell.ang, yLen - placingZone->cellsLT [0].verLen, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('z', insCell.ang, 0.050, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		insCell.ang += DegreeToRad (90);
		elemList.Push (placeLibPart (insCell));
		insCell.ang -= DegreeToRad (90);
		moveIn3D ('y', insCell.ang, placingZone->cellsLT [0].verLen - yLen*2 + placingZone->cellsLB [0].verLen, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		insCell.ang += DegreeToRad (90);
		elemList.Push (placeLibPart (insCell));
		insCell.ang -= DegreeToRad (90);
	} else {
		moveIn3D ('x', insCell.ang, (xLen + 0.0635 + 0.025), &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('y', insCell.ang, yLen, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('z', insCell.ang, 0.050, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		insCell.ang += DegreeToRad (90);
		elemList.Push (placeLibPart (insCell));
		insCell.ang -= DegreeToRad (90);
		moveIn3D ('y', insCell.ang, -yLen*2, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		insCell.ang += DegreeToRad (90);
		elemList.Push (placeLibPart (insCell));
		insCell.ang -= DegreeToRad (90);
	}

	// 2. �ɺ�Ʈ��Ʈ �Ǵ� ����������� ��ġ
	if (placingZone->bUseOutcornerPanel == true) {
		// �ƿ��ڳ��ǳ��� ���, �ɺ�Ʈ��Ʈ ��ġ
		insCell.objType = PINBOLT;
		insCell.libPart.pinbolt.bPinBoltRot90 = false;
		insCell.libPart.pinbolt.boltLen = 0.100;
		insCell.libPart.pinbolt.washerPos = 0.050;
		insCell.libPart.pinbolt.angX = DegreeToRad (0);
		insCell.libPart.pinbolt.angY = DegreeToRad (90);

		// ����
		insCell.leftBottomX = placingZone->origoPos.x;
		insCell.leftBottomY = placingZone->origoPos.y;
		insCell.leftBottomZ = placingZone->bottomOffset;
		insCell.ang = placingZone->angle;

		moveIn3D ('x', insCell.ang, -xLen + placingZone->cellsLT [0].horLen, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('y', insCell.ang, yLen + 0.0635 + 0.050 + 0.050, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', insCell.ang, 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
			insCell.ang -= DegreeToRad (90);
			elemList.Push (placeLibPart (insCell));
			insCell.ang += DegreeToRad (90);
			moveIn3D ('z', insCell.ang, -0.150 + placingZone->cellsT1 [xx].height - 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
			insCell.ang -= DegreeToRad (90);
			elemList.Push (placeLibPart (insCell));
			insCell.ang += DegreeToRad (90);
			moveIn3D ('z', insCell.ang, 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		}
		moveIn3D ('z', insCell.ang, -heightOfFormArea, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('x', insCell.ang, -placingZone->cellsLT [0].horLen + xLen*2 - placingZone->cellsRT [0].horLen, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', insCell.ang, 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
			insCell.ang -= DegreeToRad (90);
			elemList.Push (placeLibPart (insCell));
			insCell.ang += DegreeToRad (90);
			moveIn3D ('z', insCell.ang, -0.150 + placingZone->cellsT1 [xx].height - 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
			insCell.ang -= DegreeToRad (90);
			elemList.Push (placeLibPart (insCell));
			insCell.ang += DegreeToRad (90);
			moveIn3D ('z', insCell.ang, 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		}

		// �Ʒ���
		insCell.leftBottomX = placingZone->origoPos.x;
		insCell.leftBottomY = placingZone->origoPos.y;
		insCell.leftBottomZ = placingZone->bottomOffset;
		insCell.ang = placingZone->angle;

		moveIn3D ('x', insCell.ang, -xLen + placingZone->cellsLT [0].horLen, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('y', insCell.ang, -(yLen + 0.0635 + 0.050 + 0.050), &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', insCell.ang, 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
			insCell.ang -= DegreeToRad (270);
			elemList.Push (placeLibPart (insCell));
			insCell.ang += DegreeToRad (270);
			moveIn3D ('z', insCell.ang, -0.150 + placingZone->cellsT1 [xx].height - 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
			insCell.ang -= DegreeToRad (270);
			elemList.Push (placeLibPart (insCell));
			insCell.ang += DegreeToRad (270);
			moveIn3D ('z', insCell.ang, 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		}
		moveIn3D ('z', insCell.ang, -heightOfFormArea, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('x', insCell.ang, -placingZone->cellsLT [0].horLen + xLen*2 - placingZone->cellsRT [0].horLen, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', insCell.ang, 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
			insCell.ang -= DegreeToRad (270);
			elemList.Push (placeLibPart (insCell));
			insCell.ang += DegreeToRad (270);
			moveIn3D ('z', insCell.ang, -0.150 + placingZone->cellsT1 [xx].height - 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
			insCell.ang -= DegreeToRad (270);
			elemList.Push (placeLibPart (insCell));
			insCell.ang += DegreeToRad (270);
			moveIn3D ('z', insCell.ang, 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		}

		// ����
		insCell.leftBottomX = placingZone->origoPos.x;
		insCell.leftBottomY = placingZone->origoPos.y;
		insCell.leftBottomZ = placingZone->bottomOffset;
		insCell.ang = placingZone->angle;

		moveIn3D ('x', insCell.ang, -(xLen + 0.0635 + 0.050 + 0.050), &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('y', insCell.ang, yLen - placingZone->cellsLT [0].verLen, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', insCell.ang, 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
			elemList.Push (placeLibPart (insCell));
			moveIn3D ('z', insCell.ang, -0.150 + placingZone->cellsL1 [xx].height - 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
			elemList.Push (placeLibPart (insCell));
			moveIn3D ('z', insCell.ang, 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		}
		moveIn3D ('z', insCell.ang, -heightOfFormArea, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('y', insCell.ang, placingZone->cellsLT [0].verLen - xLen*2 + placingZone->cellsLB [0].verLen, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', insCell.ang, 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
			elemList.Push (placeLibPart (insCell));
			moveIn3D ('z', insCell.ang, -0.150 + placingZone->cellsL1 [xx].height - 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
			elemList.Push (placeLibPart (insCell));
			moveIn3D ('z', insCell.ang, 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		}

		// ������
		insCell.leftBottomX = placingZone->origoPos.x;
		insCell.leftBottomY = placingZone->origoPos.y;
		insCell.leftBottomZ = placingZone->bottomOffset;
		insCell.ang = placingZone->angle;

		moveIn3D ('x', insCell.ang, (xLen + 0.0635 + 0.050 + 0.050), &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('y', insCell.ang, yLen - placingZone->cellsLT [0].verLen, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', insCell.ang, 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
			insCell.ang += DegreeToRad (180);
			elemList.Push (placeLibPart (insCell));
			insCell.ang -= DegreeToRad (180);
			moveIn3D ('z', insCell.ang, -0.150 + placingZone->cellsL1 [xx].height - 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
			insCell.ang += DegreeToRad (180);
			elemList.Push (placeLibPart (insCell));
			insCell.ang -= DegreeToRad (180);
			moveIn3D ('z', insCell.ang, 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		}
		moveIn3D ('z', insCell.ang, -heightOfFormArea, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('y', insCell.ang, placingZone->cellsLT [0].verLen - xLen*2 + placingZone->cellsLB [0].verLen, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', insCell.ang, 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
			insCell.ang += DegreeToRad (180);
			elemList.Push (placeLibPart (insCell));
			insCell.ang -= DegreeToRad (180);
			moveIn3D ('z', insCell.ang, -0.150 + placingZone->cellsL1 [xx].height - 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
			insCell.ang += DegreeToRad (180);
			elemList.Push (placeLibPart (insCell));
			insCell.ang -= DegreeToRad (180);
			moveIn3D ('z', insCell.ang, 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		}
	} else {
		// �ƿ��ڳʾޱ��� ���, ����������� ��ġ
		insCell.objType = HANGER;
		insCell.libPart.hanger.angX = DegreeToRad (0);
		insCell.libPart.hanger.angY = DegreeToRad (0);

		// ����
		insCell.leftBottomX = placingZone->origoPos.x;
		insCell.leftBottomY = placingZone->origoPos.y;
		insCell.leftBottomZ = placingZone->bottomOffset;
		insCell.ang = placingZone->angle;

		insCell.libPart.hanger.angX = DegreeToRad (90);
		moveIn3D ('x', insCell.ang, -xLen, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('y', insCell.ang, yLen + 0.0635, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', insCell.ang, 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
			elemList.Push (placeLibPart (insCell));
			moveIn3D ('z', insCell.ang, -0.150 + placingZone->cellsT1 [xx].height - 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
			elemList.Push (placeLibPart (insCell));
			moveIn3D ('z', insCell.ang, 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		}
		insCell.libPart.hanger.angX = DegreeToRad (-90);
		moveIn3D ('z', insCell.ang, -heightOfFormArea, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('x', insCell.ang, xLen*2, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', insCell.ang, 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
			insCell.ang += DegreeToRad (180);
			elemList.Push (placeLibPart (insCell));
			insCell.ang -= DegreeToRad (180);
			moveIn3D ('z', insCell.ang, -0.150 + placingZone->cellsT1 [xx].height - 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
			insCell.ang += DegreeToRad (180);
			elemList.Push (placeLibPart (insCell));
			insCell.ang -= DegreeToRad (180);
			moveIn3D ('z', insCell.ang, 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		}

		// �Ʒ���
		insCell.leftBottomX = placingZone->origoPos.x;
		insCell.leftBottomY = placingZone->origoPos.y;
		insCell.leftBottomZ = placingZone->bottomOffset;
		insCell.ang = placingZone->angle;

		insCell.libPart.hanger.angX = DegreeToRad (270);
		moveIn3D ('x', insCell.ang, -xLen, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('y', insCell.ang, -(yLen + 0.0635), &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', insCell.ang, 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
			elemList.Push (placeLibPart (insCell));
			moveIn3D ('z', insCell.ang, -0.150 + placingZone->cellsT1 [xx].height - 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
			elemList.Push (placeLibPart (insCell));
			moveIn3D ('z', insCell.ang, 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		}
		insCell.libPart.hanger.angX = DegreeToRad (-270);
		moveIn3D ('z', insCell.ang, -heightOfFormArea, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('x', insCell.ang, xLen*2, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', insCell.ang, 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
			insCell.ang += DegreeToRad (180);
			elemList.Push (placeLibPart (insCell));
			insCell.ang -= DegreeToRad (180);
			moveIn3D ('z', insCell.ang, -0.150 + placingZone->cellsT1 [xx].height - 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
			insCell.ang += DegreeToRad (180);
			elemList.Push (placeLibPart (insCell));
			insCell.ang -= DegreeToRad (180);
			moveIn3D ('z', insCell.ang, 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		}

		// ����
		insCell.leftBottomX = placingZone->origoPos.x;
		insCell.leftBottomY = placingZone->origoPos.y;
		insCell.leftBottomZ = placingZone->bottomOffset;
		insCell.ang = placingZone->angle;

		insCell.libPart.hanger.angX = DegreeToRad (270);
		moveIn3D ('x', insCell.ang, -(xLen + 0.0635), &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('y', insCell.ang, yLen, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', insCell.ang, 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
			insCell.ang -= DegreeToRad (90);
			elemList.Push (placeLibPart (insCell));
			insCell.ang += DegreeToRad (90);
			moveIn3D ('z', insCell.ang, -0.150 + placingZone->cellsT1 [xx].height - 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
			insCell.ang -= DegreeToRad (90);
			elemList.Push (placeLibPart (insCell));
			insCell.ang += DegreeToRad (90);
			moveIn3D ('z', insCell.ang, 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		}
		insCell.libPart.hanger.angX = DegreeToRad (90);
		moveIn3D ('z', insCell.ang, -heightOfFormArea, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('y', insCell.ang, -yLen*2, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', insCell.ang, 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
			insCell.ang += DegreeToRad (90);
			elemList.Push (placeLibPart (insCell));
			insCell.ang -= DegreeToRad (90);
			moveIn3D ('z', insCell.ang, -0.150 + placingZone->cellsT1 [xx].height - 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
			insCell.ang += DegreeToRad (90);
			elemList.Push (placeLibPart (insCell));
			insCell.ang -= DegreeToRad (90);
			moveIn3D ('z', insCell.ang, 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		}

		// ������
		insCell.leftBottomX = placingZone->origoPos.x;
		insCell.leftBottomY = placingZone->origoPos.y;
		insCell.leftBottomZ = placingZone->bottomOffset;
		insCell.ang = placingZone->angle;

		insCell.libPart.hanger.angX = DegreeToRad (90);
		moveIn3D ('x', insCell.ang, (xLen + 0.0635), &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('y', insCell.ang, yLen, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', insCell.ang, 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
			insCell.ang -= DegreeToRad (90);
			elemList.Push (placeLibPart (insCell));
			insCell.ang += DegreeToRad (90);
			moveIn3D ('z', insCell.ang, -0.150 + placingZone->cellsT1 [xx].height - 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
			insCell.ang -= DegreeToRad (90);
			elemList.Push (placeLibPart (insCell));
			insCell.ang += DegreeToRad (90);
			moveIn3D ('z', insCell.ang, 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		}
		insCell.libPart.hanger.angX = DegreeToRad (270);
		moveIn3D ('z', insCell.ang, -heightOfFormArea, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('y', insCell.ang, -yLen*2, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
			moveIn3D ('z', insCell.ang, 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
			insCell.ang += DegreeToRad (90);
			elemList.Push (placeLibPart (insCell));
			insCell.ang -= DegreeToRad (90);
			moveIn3D ('z', insCell.ang, -0.150 + placingZone->cellsT1 [xx].height - 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
			insCell.ang += DegreeToRad (90);
			elemList.Push (placeLibPart (insCell));
			insCell.ang -= DegreeToRad (90);
			moveIn3D ('z', insCell.ang, 0.150, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		}
	}

	// 3. ����ǽ� ��ġ
	insCell.objType = HEAD;

	if (placingZone->bUseOutcornerPanel == true) {
		// �ƿ��ڳ��ǳ��� ���, �ƿ��ڳ��ǳ��� �ʺ� �����

		// ����
		insCell.leftBottomX = placingZone->origoPos.x;
		insCell.leftBottomY = placingZone->origoPos.y;
		insCell.leftBottomZ = placingZone->bottomOffset;
		insCell.ang = placingZone->angle;

		moveIn3D ('x', insCell.ang, -xLen + placingZone->cellsLT [0].horLen + 0.0475, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('y', insCell.ang, yLen + 0.0635 + 0.155, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('z', insCell.ang, 0.300, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		insCell.ang += DegreeToRad (180);
		elemList.Push (placeLibPart (insCell));
		insCell.ang -= DegreeToRad (180);
		moveIn3D ('z', insCell.ang, -0.300 + elev_headpiece, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		insCell.ang += DegreeToRad (180);
		elemList.Push (placeLibPart (insCell));
		insCell.ang -= DegreeToRad (180);
		moveIn3D ('z', insCell.ang, -elev_headpiece + 0.300, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('x', insCell.ang, -placingZone->cellsLT [0].horLen + xLen*2 - placingZone->cellsRT [0].horLen, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		insCell.ang += DegreeToRad (180);
		elemList.Push (placeLibPart (insCell));
		insCell.ang -= DegreeToRad (180);
		moveIn3D ('z', insCell.ang, -0.300 + elev_headpiece, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		insCell.ang += DegreeToRad (180);
		elemList.Push (placeLibPart (insCell));
		insCell.ang -= DegreeToRad (180);

		// �Ʒ���
		insCell.leftBottomX = placingZone->origoPos.x;
		insCell.leftBottomY = placingZone->origoPos.y;
		insCell.leftBottomZ = placingZone->bottomOffset;
		insCell.ang = placingZone->angle;

		moveIn3D ('x', insCell.ang, -xLen + placingZone->cellsLT [0].horLen - 0.0475, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('y', insCell.ang, -(yLen + 0.0635 + 0.155), &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('z', insCell.ang, 0.300, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		elemList.Push (placeLibPart (insCell));
		moveIn3D ('z', insCell.ang, -0.300 + elev_headpiece, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		elemList.Push (placeLibPart (insCell));
		moveIn3D ('z', insCell.ang, -elev_headpiece + 0.300, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('x', insCell.ang, -placingZone->cellsLT [0].horLen + xLen*2 - placingZone->cellsRT [0].horLen, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		elemList.Push (placeLibPart (insCell));
		moveIn3D ('z', insCell.ang, -0.300 + elev_headpiece, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		elemList.Push (placeLibPart (insCell));

		// ����
		insCell.leftBottomX = placingZone->origoPos.x;
		insCell.leftBottomY = placingZone->origoPos.y;
		insCell.leftBottomZ = placingZone->bottomOffset;
		insCell.ang = placingZone->angle;

		moveIn3D ('x', insCell.ang, -(xLen + 0.0635 + 0.155), &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('y', insCell.ang, yLen - placingZone->cellsLT [0].verLen + 0.0475, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('z', insCell.ang, 0.300, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		insCell.ang -= DegreeToRad (90);
		elemList.Push (placeLibPart (insCell));
		insCell.ang += DegreeToRad (90);
		moveIn3D ('z', insCell.ang, -0.300 + elev_headpiece, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		insCell.ang -= DegreeToRad (90);
		elemList.Push (placeLibPart (insCell));
		insCell.ang += DegreeToRad (90);
		moveIn3D ('z', insCell.ang, -elev_headpiece + 0.300, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('y', insCell.ang, placingZone->cellsLT [0].verLen - yLen*2 + placingZone->cellsLB [0].verLen, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		insCell.ang -= DegreeToRad (90);
		elemList.Push (placeLibPart (insCell));
		insCell.ang += DegreeToRad (90);
		moveIn3D ('z', insCell.ang, -0.300 + elev_headpiece, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		insCell.ang -= DegreeToRad (90);
		elemList.Push (placeLibPart (insCell));
		insCell.ang += DegreeToRad (90);

		// ������
		insCell.leftBottomX = placingZone->origoPos.x;
		insCell.leftBottomY = placingZone->origoPos.y;
		insCell.leftBottomZ = placingZone->bottomOffset;
		insCell.ang = placingZone->angle;

		moveIn3D ('x', insCell.ang, (xLen + 0.0635 + 0.155), &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('y', insCell.ang, yLen - placingZone->cellsLT [0].verLen - 0.0475, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('z', insCell.ang, 0.300, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		insCell.ang += DegreeToRad (90);
		elemList.Push (placeLibPart (insCell));
		insCell.ang -= DegreeToRad (90);
		moveIn3D ('z', insCell.ang, -0.300 + elev_headpiece, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		insCell.ang += DegreeToRad (90);
		elemList.Push (placeLibPart (insCell));
		insCell.ang -= DegreeToRad (90);
		moveIn3D ('z', insCell.ang, -elev_headpiece + 0.300, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('y', insCell.ang, placingZone->cellsLT [0].verLen - yLen*2 + placingZone->cellsLB [0].verLen, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		insCell.ang += DegreeToRad (90);
		elemList.Push (placeLibPart (insCell));
		insCell.ang -= DegreeToRad (90);
		moveIn3D ('z', insCell.ang, -0.300 + elev_headpiece, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		insCell.ang += DegreeToRad (90);
		elemList.Push (placeLibPart (insCell));
		insCell.ang -= DegreeToRad (90);
	} else {
		// �ƿ��ڳʾޱ��� ���, �ƿ��ڳ��ǳ��� �ʺ� ������� ����

		// ����
		insCell.leftBottomX = placingZone->origoPos.x;
		insCell.leftBottomY = placingZone->origoPos.y;
		insCell.leftBottomZ = placingZone->bottomOffset;
		insCell.ang = placingZone->angle;

		moveIn3D ('x', insCell.ang, -xLen + 0.0475, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('y', insCell.ang, yLen + 0.0635 + 0.155, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('z', insCell.ang, 0.300, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		insCell.ang += DegreeToRad (180);
		elemList.Push (placeLibPart (insCell));
		insCell.ang -= DegreeToRad (180);
		moveIn3D ('z', insCell.ang, -0.300 + elev_headpiece, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		insCell.ang += DegreeToRad (180);
		elemList.Push (placeLibPart (insCell));
		insCell.ang -= DegreeToRad (180);
		moveIn3D ('z', insCell.ang, -elev_headpiece + 0.300, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('x', insCell.ang, xLen*2, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		insCell.ang += DegreeToRad (180);
		elemList.Push (placeLibPart (insCell));
		insCell.ang -= DegreeToRad (180);
		moveIn3D ('z', insCell.ang, -0.300 + elev_headpiece, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		insCell.ang += DegreeToRad (180);
		elemList.Push (placeLibPart (insCell));
		insCell.ang -= DegreeToRad (180);

		// �Ʒ���
		insCell.leftBottomX = placingZone->origoPos.x;
		insCell.leftBottomY = placingZone->origoPos.y;
		insCell.leftBottomZ = placingZone->bottomOffset;
		insCell.ang = placingZone->angle;

		moveIn3D ('x', insCell.ang, -xLen- 0.0475, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('y', insCell.ang, -(yLen + 0.0635 + 0.155), &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('z', insCell.ang, 0.300, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		elemList.Push (placeLibPart (insCell));
		moveIn3D ('z', insCell.ang, -0.300 + elev_headpiece, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		elemList.Push (placeLibPart (insCell));
		moveIn3D ('z', insCell.ang, -elev_headpiece + 0.300, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('x', insCell.ang, xLen*2, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		elemList.Push (placeLibPart (insCell));
		moveIn3D ('z', insCell.ang, -0.300 + elev_headpiece, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		elemList.Push (placeLibPart (insCell));

		// ����
		insCell.leftBottomX = placingZone->origoPos.x;
		insCell.leftBottomY = placingZone->origoPos.y;
		insCell.leftBottomZ = placingZone->bottomOffset;
		insCell.ang = placingZone->angle;

		moveIn3D ('x', insCell.ang, -(xLen + 0.0635 + 0.155), &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('y', insCell.ang, yLen + 0.0475, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('z', insCell.ang, 0.300, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		insCell.ang -= DegreeToRad (90);
		elemList.Push (placeLibPart (insCell));
		insCell.ang += DegreeToRad (90);
		moveIn3D ('z', insCell.ang, -0.300 + elev_headpiece, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		insCell.ang -= DegreeToRad (90);
		elemList.Push (placeLibPart (insCell));
		insCell.ang += DegreeToRad (90);
		moveIn3D ('z', insCell.ang, -elev_headpiece + 0.300, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('y', insCell.ang, -yLen*2, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		insCell.ang -= DegreeToRad (90);
		elemList.Push (placeLibPart (insCell));
		insCell.ang += DegreeToRad (90);
		moveIn3D ('z', insCell.ang, -0.300 + elev_headpiece, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		insCell.ang -= DegreeToRad (90);
		elemList.Push (placeLibPart (insCell));
		insCell.ang += DegreeToRad (90);

		// ������
		insCell.leftBottomX = placingZone->origoPos.x;
		insCell.leftBottomY = placingZone->origoPos.y;
		insCell.leftBottomZ = placingZone->bottomOffset;
		insCell.ang = placingZone->angle;

		moveIn3D ('x', insCell.ang, (xLen + 0.0635 + 0.155), &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('y', insCell.ang, yLen - 0.0475, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('z', insCell.ang, 0.300, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		insCell.ang += DegreeToRad (90);
		elemList.Push (placeLibPart (insCell));
		insCell.ang -= DegreeToRad (90);
		moveIn3D ('z', insCell.ang, -0.300 + elev_headpiece, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		insCell.ang += DegreeToRad (90);
		elemList.Push (placeLibPart (insCell));
		insCell.ang -= DegreeToRad (90);
		moveIn3D ('z', insCell.ang, -elev_headpiece + 0.300, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('y', insCell.ang, -yLen*2, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		insCell.ang += DegreeToRad (90);
		elemList.Push (placeLibPart (insCell));
		insCell.ang -= DegreeToRad (90);
		moveIn3D ('z', insCell.ang, -0.300 + elev_headpiece, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		insCell.ang += DegreeToRad (90);
		elemList.Push (placeLibPart (insCell));
		insCell.ang -= DegreeToRad (90);
	}

	// 4. ��չ�� �Ǵ� ���� ��ġ
	if (placingZone->typeOfColumnBand == 1) {
		insCell.objType = COLUMN_BAND1;
		insCell.libPart.columnBand.band_size = 1;
		insCell.libPart.columnBand.c_w = placingZone->coreWidth + placingZone->venThick*2;
		insCell.libPart.columnBand.c_h = placingZone->coreDepth + placingZone->venThick*2;
		insCell.libPart.columnBand.addOffset = 0.050;

		insCell.leftBottomX = placingZone->origoPos.x;
		insCell.leftBottomY = placingZone->origoPos.y;
		insCell.leftBottomZ = placingZone->bottomOffset;
		insCell.ang = placingZone->angle;

		moveIn3D ('x', insCell.ang, -0.0035 + insCell.libPart.columnBand.c_w/2, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('y', insCell.ang, -0.1535 - insCell.libPart.columnBand.c_h/2, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		moveIn3D ('z', insCell.ang, heightOfFormArea/2 - 0.900, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		elemList.Push (placeLibPart (insCell));
		moveIn3D ('z', insCell.ang, 0.900, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		elemList.Push (placeLibPart (insCell));
		moveIn3D ('z', insCell.ang, 0.900, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		elemList.Push (placeLibPart (insCell));
	} else if (placingZone->typeOfColumnBand == 2) {
		insCell.objType = COLUMN_BAND2;
		insCell.libPart.wella.c_w = placingZone->coreWidth + placingZone->venThick*2;
		insCell.libPart.wella.c_h = placingZone->coreDepth + placingZone->venThick*2;
		insCell.libPart.wella.addOffset = 0.050;

		insCell.leftBottomX = placingZone->origoPos.x;
		insCell.leftBottomY = placingZone->origoPos.y;
		insCell.leftBottomZ = placingZone->bottomOffset;
		insCell.ang = placingZone->angle;

		moveIn3D ('z', insCell.ang, heightOfFormArea/2 - 0.900, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		elemList.Push (placeLibPart (insCell));
		moveIn3D ('z', insCell.ang, 0.900, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		elemList.Push (placeLibPart (insCell));
		moveIn3D ('z', insCell.ang, 0.900, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
		elemList.Push (placeLibPart (insCell));
	}

	return	err;
}

// ������/�ƿ��ڳ��ǳ��� ä�� �� ������ ���� ä��� (�������� �������� ä��)
GSErrCode	ColumnTableformPlacingZone::fillRestAreas_soleColumn (ColumnTableformPlacingZone* placingZone)
{
	GSErrCode	err = NoError;

	short		xx;
	API_Coord	rotatedPoint;
	double		lineLen;
	double		xLen, yLen;
	double		dx, dy;
	double		leftLenOfBeam, rightLenOfBeam;

	double		heightOfFormArea = 0.0;
	double		columnWidth;
	double		marginHeight;

	CellForColumnTableform	insCell;

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
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle + DegreeToRad (180);
			insCell.leftBottomX = rotatedPoint.x;
			insCell.leftBottomY = rotatedPoint.y;
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = leftLenOfBeam;
			insCell.libPart.plywood.p_leng = placingZone->areaHeight - heightOfFormArea;

			elemList.Push (placingZone->placeLibPart (insCell));

			// ���� (�Ʒ���)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle + DegreeToRad (180);
			insCell.leftBottomX = rotatedPoint.x - leftLenOfBeam * cos(placingZone->angle);
			insCell.leftBottomY = rotatedPoint.y - leftLenOfBeam * sin(placingZone->angle);
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = placingZone->beams [NORTH].width;
			insCell.libPart.plywood.p_leng = marginHeight;

			elemList.Push (placingZone->placeLibPart (insCell));

			// ���� (������)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle + DegreeToRad (180);
			insCell.leftBottomX = rotatedPoint.x - (leftLenOfBeam + placingZone->beams [NORTH].width) * cos(placingZone->angle);
			insCell.leftBottomY = rotatedPoint.y - (leftLenOfBeam + placingZone->beams [NORTH].width) * sin(placingZone->angle);
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = rightLenOfBeam;
			insCell.libPart.plywood.p_leng = placingZone->areaHeight - heightOfFormArea;

			elemList.Push (placingZone->placeLibPart (insCell));
		} else {
			// ���� (��ü)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle + DegreeToRad (180);
			insCell.leftBottomX = rotatedPoint.x;
			insCell.leftBottomY = rotatedPoint.y;
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = columnWidth;
			insCell.libPart.plywood.p_leng = marginHeight;

			elemList.Push (placingZone->placeLibPart (insCell));
		}
	}

	// ����
	if (placingZone->bFillMarginTopAtSouth == true) {
		xLen = -(placingZone->coreWidth/2 + placingZone->venThick);
		yLen = -(placingZone->coreDepth/2 + placingZone->venThick);
		lineLen = sqrt (xLen*xLen + yLen*yLen);
		rotatedPoint.x = placingZone->origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone->angle);
		rotatedPoint.y = placingZone->origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone->angle);

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
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle;
			insCell.leftBottomX = rotatedPoint.x;
			insCell.leftBottomY = rotatedPoint.y;
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = leftLenOfBeam;
			insCell.libPart.plywood.p_leng = placingZone->areaHeight - heightOfFormArea;

			elemList.Push (placingZone->placeLibPart (insCell));

			// ���� (�Ʒ���)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle;
			insCell.leftBottomX = rotatedPoint.x + leftLenOfBeam * cos(placingZone->angle);
			insCell.leftBottomY = rotatedPoint.y + leftLenOfBeam * sin(placingZone->angle);
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = placingZone->beams [SOUTH].width;
			insCell.libPart.plywood.p_leng = marginHeight;

			elemList.Push (placingZone->placeLibPart (insCell));

			// ���� (������)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle;
			insCell.leftBottomX = rotatedPoint.x + (leftLenOfBeam + placingZone->beams [SOUTH].width) * cos(placingZone->angle);
			insCell.leftBottomY = rotatedPoint.y + (leftLenOfBeam + placingZone->beams [SOUTH].width) * sin(placingZone->angle);
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = rightLenOfBeam;
			insCell.libPart.plywood.p_leng = placingZone->areaHeight - heightOfFormArea;

			elemList.Push (placingZone->placeLibPart (insCell));
		} else {
			// ���� (��ü)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle;
			insCell.leftBottomX = rotatedPoint.x;
			insCell.leftBottomY = rotatedPoint.y;
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = columnWidth;
			insCell.libPart.plywood.p_leng = marginHeight;

			elemList.Push (placingZone->placeLibPart (insCell));
		}
	}

	// ����
	if (placingZone->bFillMarginTopAtWest == true) {
		xLen = -(placingZone->coreWidth/2 + placingZone->venThick);
		yLen = (placingZone->coreDepth/2 + placingZone->venThick);
		lineLen = sqrt (xLen*xLen + yLen*yLen);
		rotatedPoint.x = placingZone->origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone->angle);
		rotatedPoint.y = placingZone->origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone->angle);

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
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle - DegreeToRad (90);
			insCell.leftBottomX = rotatedPoint.x;
			insCell.leftBottomY = rotatedPoint.y;
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = leftLenOfBeam;
			insCell.libPart.plywood.p_leng = placingZone->areaHeight - heightOfFormArea;

			elemList.Push (placingZone->placeLibPart (insCell));

			// ���� (�Ʒ���)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle - DegreeToRad (90);
			insCell.leftBottomX = rotatedPoint.x + leftLenOfBeam * sin(placingZone->angle);
			insCell.leftBottomY = rotatedPoint.y - leftLenOfBeam * cos(placingZone->angle);
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = placingZone->beams [WEST].width;
			insCell.libPart.plywood.p_leng = marginHeight;

			elemList.Push (placingZone->placeLibPart (insCell));

			// ���� (������)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle - DegreeToRad (90);
			insCell.leftBottomX = rotatedPoint.x + (leftLenOfBeam + placingZone->beams [WEST].width) * sin(placingZone->angle);
			insCell.leftBottomY = rotatedPoint.y - (leftLenOfBeam + placingZone->beams [WEST].width) * cos(placingZone->angle);
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = rightLenOfBeam;
			insCell.libPart.plywood.p_leng = placingZone->areaHeight - heightOfFormArea;

			elemList.Push (placingZone->placeLibPart (insCell));
		} else {
			// ���� (��ü)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle - DegreeToRad (90);
			insCell.leftBottomX = rotatedPoint.x;
			insCell.leftBottomY = rotatedPoint.y;
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = columnWidth;
			insCell.libPart.plywood.p_leng = marginHeight;

			elemList.Push (placingZone->placeLibPart (insCell));
		}
	}

	// ����
	if (placingZone->bFillMarginTopAtEast == true) {
		xLen = (placingZone->coreWidth/2 + placingZone->venThick);
		yLen = -(placingZone->coreDepth/2 + placingZone->venThick);
		lineLen = sqrt (xLen*xLen + yLen*yLen);
		rotatedPoint.x = placingZone->origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone->angle);
		rotatedPoint.y = placingZone->origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone->angle);

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
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle + DegreeToRad (90);
			insCell.leftBottomX = rotatedPoint.x;
			insCell.leftBottomY = rotatedPoint.y;
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = leftLenOfBeam;
			insCell.libPart.plywood.p_leng = placingZone->areaHeight - heightOfFormArea;

			elemList.Push (placingZone->placeLibPart (insCell));

			// ���� (�Ʒ���)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle + DegreeToRad (90);
			insCell.leftBottomX = rotatedPoint.x - leftLenOfBeam * sin(placingZone->angle);
			insCell.leftBottomY = rotatedPoint.y + leftLenOfBeam * cos(placingZone->angle);
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = placingZone->beams [EAST].width;
			insCell.libPart.plywood.p_leng = marginHeight;

			elemList.Push (placingZone->placeLibPart (insCell));

			// ���� (������)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle + DegreeToRad (90);
			insCell.leftBottomX = rotatedPoint.x - (leftLenOfBeam + placingZone->beams [EAST].width) * sin(placingZone->angle);
			insCell.leftBottomY = rotatedPoint.y + (leftLenOfBeam + placingZone->beams [EAST].width) * cos(placingZone->angle);
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = rightLenOfBeam;
			insCell.libPart.plywood.p_leng = placingZone->areaHeight - heightOfFormArea;

			elemList.Push (placingZone->placeLibPart (insCell));
		} else {
			// ���� (��ü)
			insCell.objType = PLYWOOD;
			insCell.ang = placingZone->angle + DegreeToRad (90);
			insCell.leftBottomX = rotatedPoint.x;
			insCell.leftBottomY = rotatedPoint.y;
			insCell.leftBottomZ = heightOfFormArea;
			insCell.libPart.plywood.p_wid = columnWidth;
			insCell.libPart.plywood.p_leng = marginHeight;

			elemList.Push (placingZone->placeLibPart (insCell));
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
			DGSetDialogTitle (dialogID, "��տ� ��ġ - ��� �ܸ�");

			//////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			// Ȯ�� ��ư
			DGSetItemText (dialogID, DG_OK, "Ȯ ��");

			// ��� ��ư
			DGSetItemText (dialogID, DG_CANCEL, "�� ��");

			//////////////////////////////////////////////////////////// ������ ��ġ (������)
			// ���� ��ư
			DGSetItemText (dialogID, RADIO_OUTCORNER_PANEL, "�ƿ��ڳ�\n�ǳ�");
			DGSetItemText (dialogID, RADIO_OUTCORNER_ANGLE, "�ƿ��ڳ�\n�ޱ�");
			DGSetItemText (dialogID, RADIO_COLUMN_BAND_1, "��չ��");
			DGSetItemText (dialogID, RADIO_COLUMN_BAND_2, "����");

			// �� �� üũ�ڽ�
			DGSetItemText (dialogID, LABEL_COLUMN_SECTION, "��� �ܸ�");
			DGSetItemText (dialogID, LABEL_COLUMN_DEPTH, "����");
			DGSetItemText (dialogID, LABEL_COLUMN_WIDTH, "����");
			DGSetItemText (dialogID, LABEL_OUTCORNER, "�ƿ��ڳ� ó��");
			DGSetItemText (dialogID, LABEL_COLUMN_BAND_TYPE, "��չ�� Ÿ��");

			// ��: ���̾� ����
			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, "���纰 ���̾� ����");
			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM, "������");
			DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER_PANEL, "�ƿ��ڳ��ǳ�");
			DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER_ANGLE, "�ƿ��ڳʾޱ�");
			DGSetItemText (dialogID, LABEL_LAYER_SQUARE_PIPE, "���������");
			DGSetItemText (dialogID, LABEL_LAYER_PINBOLT, "�ɺ�Ʈ��Ʈ");
			DGSetItemText (dialogID, LABEL_LAYER_HANGER, "�����������");
			DGSetItemText (dialogID, LABEL_LAYER_HEADPIECE, "����ǽ�");
			DGSetItemText (dialogID, LABEL_LAYER_COLUMN_BAND, "��չ��");
			DGSetItemText (dialogID, LABEL_LAYER_PLYWOOD, "����");

			// üũ�ڽ�: ���̾� ����
			DGSetItemText (dialogID, CHECKBOX_LAYER_COUPLING, "���̾� ����");
			DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, TRUE);

			DGSetItemText (dialogID, BUTTON_AUTOSET, "���̾� �ڵ� ����");

			// ���� ��Ʈ�� �ʱ�ȭ
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_LAYER_EUROFORM;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, 1);

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
			(DGGetItemValLong (dialogID, CHECKBOX_LEFT_ADDITIONAL_FORM) == TRUE) ?		DGEnableItem (dialogID, EDITCONTROL_LEFT_3)		: 	DGDisableItem (dialogID, EDITCONTROL_LEFT_3);
			(DGGetItemValLong (dialogID, CHECKBOX_BOTTOM_ADDITIONAL_FORM) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_BOTTOM_3)	: 	DGDisableItem (dialogID, EDITCONTROL_BOTTOM_3);

			// ���� �����ؼ��� �� �Ǵ� �׸� ��ױ�
			DGDisableItem (dialogID, EDITCONTROL_TOP_1);
			DGDisableItem (dialogID, EDITCONTROL_TOP_2);
			DGDisableItem (dialogID, EDITCONTROL_TOP_3);
			DGDisableItem (dialogID, EDITCONTROL_TOP_4);
			DGDisableItem (dialogID, CHECKBOX_TOP_ADDITIONAL_FORM);
			DGDisableItem (dialogID, EDITCONTROL_RIGHT_1);
			DGDisableItem (dialogID, EDITCONTROL_RIGHT_2);
			DGDisableItem (dialogID, EDITCONTROL_RIGHT_3);
			DGDisableItem (dialogID, EDITCONTROL_RIGHT_4);
			DGDisableItem (dialogID, CHECKBOX_RIGHT_ADDITIONAL_FORM);

			// �⺻�� �Է��� ����
			DGSetItemValDouble (dialogID, EDITCONTROL_TOP_1, 0.100);
			DGSetItemValDouble (dialogID, EDITCONTROL_TOP_2, 0.300);
			DGSetItemValDouble (dialogID, EDITCONTROL_TOP_4, 0.100);
			DGSetItemValDouble (dialogID, EDITCONTROL_LEFT_1, 0.100);
			DGSetItemValDouble (dialogID, EDITCONTROL_LEFT_2, 0.300);
			DGSetItemValDouble (dialogID, EDITCONTROL_LEFT_4, 0.100);
			DGSetItemValDouble (dialogID, EDITCONTROL_RIGHT_1, 0.100);
			DGSetItemValDouble (dialogID, EDITCONTROL_RIGHT_2, 0.300);
			DGSetItemValDouble (dialogID, EDITCONTROL_RIGHT_4, 0.100);
			DGSetItemValDouble (dialogID, EDITCONTROL_BOTTOM_1, 0.100);
			DGSetItemValDouble (dialogID, EDITCONTROL_BOTTOM_2, 0.300);
			DGSetItemValDouble (dialogID, EDITCONTROL_BOTTOM_4, 0.100);

			break;
		
		case DG_MSG_CHANGE:

			if (DGGetItemValLong (dialogID, RADIO_OUTCORNER_PANEL) == TRUE) {
				// �ƿ��ڳ��ǳ� ����� ��� (�׸� ����, Edit ��Ʈ�� ��� �����ֱ�)
				DGShowItem (dialogID, ICON_COLUMN_SECTION_OUTCORNER_PANEL);
				DGHideItem (dialogID, ICON_COLUMN_SECTION_OUTCORNER_ANGLE);
				DGShowItem (dialogID, EDITCONTROL_TOP_1);
				DGShowItem (dialogID, EDITCONTROL_TOP_4);
				DGShowItem (dialogID, EDITCONTROL_LEFT_1);
				DGShowItem (dialogID, EDITCONTROL_LEFT_4);
				DGShowItem (dialogID, EDITCONTROL_RIGHT_1);
				DGShowItem (dialogID, EDITCONTROL_RIGHT_4);
				DGShowItem (dialogID, EDITCONTROL_BOTTOM_1);
				DGShowItem (dialogID, EDITCONTROL_BOTTOM_4);

				DGEnableItem (dialogID, LABEL_LAYER_OUTCORNER_PANEL);
				DGEnableItem (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL);
				DGEnableItem (dialogID, LABEL_LAYER_PINBOLT);
				DGEnableItem (dialogID, USERCONTROL_LAYER_PINBOLT);

				DGDisableItem (dialogID, LABEL_LAYER_OUTCORNER_ANGLE);
				DGDisableItem (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE);
				DGDisableItem (dialogID, LABEL_LAYER_HANGER);
				DGDisableItem (dialogID, USERCONTROL_LAYER_HANGER);

			} else {
				// �ƿ��ڳʾޱ� ����� ��� (�׸� ����, Edit ��Ʈ�� �Ϻ� �����)
				DGHideItem (dialogID, ICON_COLUMN_SECTION_OUTCORNER_PANEL);
				DGShowItem (dialogID, ICON_COLUMN_SECTION_OUTCORNER_ANGLE);
				DGHideItem (dialogID, EDITCONTROL_TOP_1);
				DGHideItem (dialogID, EDITCONTROL_TOP_4);
				DGHideItem (dialogID, EDITCONTROL_LEFT_1);
				DGHideItem (dialogID, EDITCONTROL_LEFT_4);
				DGHideItem (dialogID, EDITCONTROL_RIGHT_1);
				DGHideItem (dialogID, EDITCONTROL_RIGHT_4);
				DGHideItem (dialogID, EDITCONTROL_BOTTOM_1);
				DGHideItem (dialogID, EDITCONTROL_BOTTOM_4);

				DGDisableItem (dialogID, LABEL_LAYER_OUTCORNER_PANEL);
				DGDisableItem (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL);
				DGDisableItem (dialogID, LABEL_LAYER_PINBOLT);
				DGDisableItem (dialogID, USERCONTROL_LAYER_PINBOLT);

				DGEnableItem (dialogID, LABEL_LAYER_OUTCORNER_ANGLE);
				DGEnableItem (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE);
				DGEnableItem (dialogID, LABEL_LAYER_HANGER);
				DGEnableItem (dialogID, USERCONTROL_LAYER_HANGER);
			}

			// ���纰 üũ�ڽ�-�԰� ����
			if (DGGetItemValLong (dialogID, CHECKBOX_LEFT_ADDITIONAL_FORM) == TRUE) {
				DGEnableItem (dialogID, EDITCONTROL_LEFT_3);
				DGSetItemValLong (dialogID, CHECKBOX_RIGHT_ADDITIONAL_FORM, TRUE);
			} else {
				DGDisableItem (dialogID, EDITCONTROL_LEFT_3);
				DGSetItemValDouble (dialogID, EDITCONTROL_LEFT_3, 0.0);
				DGSetItemValDouble (dialogID, EDITCONTROL_RIGHT_3, 0.0);
				DGSetItemValLong (dialogID, CHECKBOX_RIGHT_ADDITIONAL_FORM, FALSE);
			}
			if (DGGetItemValLong (dialogID, CHECKBOX_BOTTOM_ADDITIONAL_FORM) == TRUE) {
				DGEnableItem (dialogID, EDITCONTROL_BOTTOM_3);
				DGSetItemValLong (dialogID, CHECKBOX_TOP_ADDITIONAL_FORM, TRUE);
			} else {
				DGDisableItem (dialogID, EDITCONTROL_BOTTOM_3);
				DGSetItemValDouble (dialogID, EDITCONTROL_BOTTOM_3, 0.0);
				DGSetItemValDouble (dialogID, EDITCONTROL_TOP_3, 0.0);
				DGSetItemValLong (dialogID, CHECKBOX_TOP_ADDITIONAL_FORM, FALSE);
			}

			DGSetItemValDouble (dialogID, EDITCONTROL_RIGHT_1, DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_1));
			DGSetItemValDouble (dialogID, EDITCONTROL_RIGHT_2, DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_2));
			DGSetItemValDouble (dialogID, EDITCONTROL_RIGHT_3, DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_3));
			DGSetItemValDouble (dialogID, EDITCONTROL_RIGHT_4, DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_4));

			DGSetItemValDouble (dialogID, EDITCONTROL_TOP_1, DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_1));
			DGSetItemValDouble (dialogID, EDITCONTROL_TOP_2, DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_2));
			DGSetItemValDouble (dialogID, EDITCONTROL_TOP_3, DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_3));
			DGSetItemValDouble (dialogID, EDITCONTROL_TOP_4, DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_4));

			// ���̾� ���� �ٲ�
			if (DGGetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING) == 1) {
				switch (item) {
					case USERCONTROL_LAYER_EUROFORM:
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SQUARE_PIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_COLUMN_BAND, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						break;
					case USERCONTROL_LAYER_OUTCORNER_PANEL:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SQUARE_PIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_COLUMN_BAND, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						break;
					case USERCONTROL_LAYER_OUTCORNER_ANGLE:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SQUARE_PIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_COLUMN_BAND, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						break;
					case USERCONTROL_LAYER_SQUARE_PIPE:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SQUARE_PIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SQUARE_PIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SQUARE_PIPE));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_SQUARE_PIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SQUARE_PIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SQUARE_PIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SQUARE_PIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SQUARE_PIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_COLUMN_BAND, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SQUARE_PIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SQUARE_PIPE));
						break;
					case USERCONTROL_LAYER_PINBOLT:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SQUARE_PIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_COLUMN_BAND, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						break;
					case USERCONTROL_LAYER_HANGER:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SQUARE_PIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HANGER));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_COLUMN_BAND, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HANGER));
						break;
					case USERCONTROL_LAYER_HEADPIECE:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SQUARE_PIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_COLUMN_BAND, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						break;
					case USERCONTROL_LAYER_COLUMN_BAND:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_COLUMN_BAND));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_COLUMN_BAND));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_COLUMN_BAND));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SQUARE_PIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_COLUMN_BAND));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_COLUMN_BAND));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_COLUMN_BAND));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_COLUMN_BAND));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_COLUMN_BAND, DGGetItemValLong (dialogID, USERCONTROL_LAYER_COLUMN_BAND));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_COLUMN_BAND));
						break;
					case USERCONTROL_LAYER_PLYWOOD:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SQUARE_PIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_COLUMN_BAND, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						break;
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
							placingZone.cellsLT [xx].libPart.outpanel.leng_s = DGGetItemValDouble (dialogID, EDITCONTROL_TOP_1);
							placingZone.cellsLT [xx].libPart.outpanel.wid_s = DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_1);
							placingZone.cellsLT [xx].libPart.outpanel.hei_s = 1.200;

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
							placingZone.cellsRT [xx].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_TOP_4);
							placingZone.cellsRT [xx].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_1);
							placingZone.cellsRT [xx].height = 1.200;
							placingZone.cellsRT [xx].libPart.outpanel.leng_s = DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_1);
							placingZone.cellsRT [xx].libPart.outpanel.wid_s = DGGetItemValDouble (dialogID, EDITCONTROL_TOP_4);
							placingZone.cellsRT [xx].libPart.outpanel.hei_s = 1.200;

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
							placingZone.cellsLB [xx].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_4);
							placingZone.cellsLB [xx].height = 1.200;
							placingZone.cellsLB [xx].libPart.outpanel.leng_s = DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_4);
							placingZone.cellsLB [xx].libPart.outpanel.wid_s = DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_1);
							placingZone.cellsLB [xx].libPart.outpanel.hei_s = 1.200;

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
							placingZone.cellsRB [xx].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_4);
							placingZone.cellsRB [xx].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_4);
							placingZone.cellsRB [xx].height = 1.200;
							placingZone.cellsRB [xx].libPart.outpanel.leng_s = DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_4);
							placingZone.cellsRB [xx].libPart.outpanel.wid_s = DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_4);
							placingZone.cellsRB [xx].libPart.outpanel.hei_s = 1.200;
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
							placingZone.cellsLT [xx].libPart.outangle.a_leng = 1.200;
							placingZone.cellsLT [xx].libPart.outangle.a_ang = DegreeToRad (90);

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
							placingZone.cellsRT [xx].libPart.outangle.a_leng = 1.200;
							placingZone.cellsRT [xx].libPart.outangle.a_ang = DegreeToRad (90);

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
							placingZone.cellsLB [xx].libPart.outangle.a_leng = 1.200;
							placingZone.cellsLB [xx].libPart.outangle.a_ang = DegreeToRad (90);

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
							placingZone.cellsRB [xx].libPart.outangle.a_leng = 1.200;
							placingZone.cellsRB [xx].libPart.outangle.a_ang = DegreeToRad (90);
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
						placingZone.cellsT1 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, EDITCONTROL_TOP_2);
						placingZone.cellsT1 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, EDITCONTROL_TOP_2);
						placingZone.cellsT1 [xx].libPart.form.eu_hei = 1.200;
						placingZone.cellsT1 [xx].libPart.form.eu_hei2 = 1.200;
						placingZone.cellsT1 [xx].libPart.form.u_ins_wall = true;
						formWidth = placingZone.cellsT1 [xx].libPart.form.eu_wid;
						if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
							placingZone.cellsT1 [xx].libPart.form.eu_stan_onoff = true;
						else
							placingZone.cellsT1 [xx].libPart.form.eu_stan_onoff = false;

						// ���� 2
						if (DGGetItemValLong (dialogID, CHECKBOX_TOP_ADDITIONAL_FORM) == TRUE) {
							xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
							yLen = (placingZone.coreDepth/2 + placingZone.venThick);
							lineLen = sqrt (xLen*xLen + yLen*yLen);
							rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
							rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

							placingZone.cellsT2 [xx].objType = EUROFORM;
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
							placingZone.cellsT2 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, EDITCONTROL_TOP_3);
							placingZone.cellsT2 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, EDITCONTROL_TOP_3);
							placingZone.cellsT2 [xx].libPart.form.eu_hei = 1.200;
							placingZone.cellsT2 [xx].libPart.form.eu_hei2 = 1.200;
							placingZone.cellsT2 [xx].libPart.form.u_ins_wall = true;
							formWidth = placingZone.cellsT2 [xx].libPart.form.eu_wid;
							if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
								placingZone.cellsT2 [xx].libPart.form.eu_stan_onoff = true;
							else
								placingZone.cellsT2 [xx].libPart.form.eu_stan_onoff = false;
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
						placingZone.cellsL1 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_2);
						placingZone.cellsL1 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_2);
						placingZone.cellsL1 [xx].libPart.form.eu_hei = 1.200;
						placingZone.cellsL1 [xx].libPart.form.eu_hei2 = 1.200;
						placingZone.cellsL1 [xx].libPart.form.u_ins_wall = true;
						formWidth = placingZone.cellsL1 [xx].libPart.form.eu_wid;
						if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
							placingZone.cellsL1 [xx].libPart.form.eu_stan_onoff = true;
						else
							placingZone.cellsL1 [xx].libPart.form.eu_stan_onoff = false;

						// ���� 2
						if (DGGetItemValLong (dialogID, CHECKBOX_LEFT_ADDITIONAL_FORM) == TRUE) {
							xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
							yLen = (placingZone.coreDepth/2 + placingZone.venThick);
							lineLen = sqrt (xLen*xLen + yLen*yLen);
							rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
							rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

							placingZone.cellsL2 [xx].objType = EUROFORM;
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
							placingZone.cellsL2 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_3);
							placingZone.cellsL2 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_3);
							placingZone.cellsL2 [xx].libPart.form.eu_hei = 1.200;
							placingZone.cellsL2 [xx].libPart.form.eu_hei2 = 1.200;
							placingZone.cellsL2 [xx].libPart.form.u_ins_wall = true;
							formWidth = placingZone.cellsL2 [xx].libPart.form.eu_wid;
							if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
								placingZone.cellsL2 [xx].libPart.form.eu_stan_onoff = true;
							else
								placingZone.cellsL2 [xx].libPart.form.eu_stan_onoff = false;
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
						placingZone.cellsR1 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_2);
						placingZone.cellsR1 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_2);
						placingZone.cellsR1 [xx].libPart.form.eu_hei = 1.200;
						placingZone.cellsR1 [xx].libPart.form.eu_hei2 = 1.200;
						placingZone.cellsR1 [xx].libPart.form.u_ins_wall = true;
						formWidth = placingZone.cellsR1 [xx].libPart.form.eu_wid;
						if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
							placingZone.cellsR1 [xx].libPart.form.eu_stan_onoff = true;
						else
							placingZone.cellsR1 [xx].libPart.form.eu_stan_onoff = false;

						// ���� 2
						if (DGGetItemValLong (dialogID, CHECKBOX_RIGHT_ADDITIONAL_FORM) == TRUE) { 
							xLen = (placingZone.coreWidth/2 + placingZone.venThick);
							yLen = (placingZone.coreDepth/2 + placingZone.venThick);
							lineLen = sqrt (xLen*xLen + yLen*yLen);
							rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
							rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

							placingZone.cellsR2 [xx].objType = EUROFORM;
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
							placingZone.cellsR2 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_3);
							placingZone.cellsR2 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_3);
							placingZone.cellsR2 [xx].libPart.form.eu_hei = 1.200;
							placingZone.cellsR2 [xx].libPart.form.eu_hei2 = 1.200;
							placingZone.cellsR2 [xx].libPart.form.u_ins_wall = true;
							formWidth = placingZone.cellsR2 [xx].libPart.form.eu_wid;
							if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
								placingZone.cellsR2 [xx].libPart.form.eu_stan_onoff = true;
							else
								placingZone.cellsR2 [xx].libPart.form.eu_stan_onoff = false;
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
						placingZone.cellsB1 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_2);
						placingZone.cellsB1 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_2);
						placingZone.cellsB1 [xx].libPart.form.eu_hei = 1.200;
						placingZone.cellsB1 [xx].libPart.form.eu_hei2 = 1.200;
						placingZone.cellsB1 [xx].libPart.form.u_ins_wall = true;
						formWidth = placingZone.cellsB1 [xx].libPart.form.eu_wid;
						if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
							placingZone.cellsB1 [xx].libPart.form.eu_stan_onoff = true;
						else
							placingZone.cellsB1 [xx].libPart.form.eu_stan_onoff = false;

						// �Ʒ��� 2
						if (DGGetItemValLong (dialogID, CHECKBOX_BOTTOM_ADDITIONAL_FORM) == TRUE) {
							xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
							yLen = -(placingZone.coreDepth/2 + placingZone.venThick);
							lineLen = sqrt (xLen*xLen + yLen*yLen);
							rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
							rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

							placingZone.cellsB2 [xx].objType = EUROFORM;
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
							placingZone.cellsB2 [xx].libPart.form.eu_wid = DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_3);
							placingZone.cellsB2 [xx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_3);
							placingZone.cellsB2 [xx].libPart.form.eu_hei = 1.200;
							placingZone.cellsB2 [xx].libPart.form.eu_hei2 = 1.200;
							placingZone.cellsB2 [xx].libPart.form.u_ins_wall = true;
							formWidth = placingZone.cellsB2 [xx].libPart.form.eu_wid;
							if ( (abs (formWidth - 0.600) < EPS) || (abs (formWidth - 0.500) < EPS) || (abs (formWidth - 0.450) < EPS) || (abs (formWidth - 0.400) < EPS) || (abs (formWidth - 0.300) < EPS) || (abs (formWidth - 0.200) < EPS) )
								placingZone.cellsB2 [xx].libPart.form.eu_stan_onoff = true;
							else
								placingZone.cellsB2 [xx].libPart.form.eu_stan_onoff = false;
						}
					}

					// ��չ�� Ÿ��
					if (DGGetItemValLong (dialogID, RADIO_COLUMN_BAND_1) == TRUE) {
						placingZone.typeOfColumnBand = 1;	// ��չ��
					} else {
						placingZone.typeOfColumnBand = 2;	// ����
					}

					// ���̾� ��ȣ ����
					layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM);
					layerInd_OutPanel		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL);
					layerInd_OutAngle		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE);
					layerInd_SqrPipe		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_SQUARE_PIPE);
					layerInd_Pinbolt		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT);
					layerInd_Hanger			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_HANGER);
					layerInd_Head			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE);
					layerInd_ColumnBand		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_COLUMN_BAND);
					layerInd_Plywood		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD);

					break;

				case BUTTON_AUTOSET:
					item = 0;

					DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, FALSE);

					layerInd_Euroform	= makeTemporaryLayer (structuralObject_forTableformColumn, "UFOM", NULL);
					layerInd_OutPanel	= makeTemporaryLayer (structuralObject_forTableformColumn, "OUTP", NULL);
					layerInd_OutAngle	= makeTemporaryLayer (structuralObject_forTableformColumn, "OUTA", NULL);
					layerInd_SqrPipe	= makeTemporaryLayer (structuralObject_forTableformColumn, "SPIP", NULL);
					layerInd_Pinbolt	= makeTemporaryLayer (structuralObject_forTableformColumn, "PINB", NULL);
					layerInd_Hanger		= makeTemporaryLayer (structuralObject_forTableformColumn, "JOIB", NULL);
					layerInd_Head		= makeTemporaryLayer (structuralObject_forTableformColumn, "HEAD", NULL);
					layerInd_ColumnBand	= makeTemporaryLayer (structuralObject_forTableformColumn, "BDCM", NULL);
					layerInd_Plywood	= makeTemporaryLayer (structuralObject_forTableformColumn, "PLYW", NULL);

					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, layerInd_Euroform);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, layerInd_OutPanel);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, layerInd_OutAngle);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_SQUARE_PIPE, layerInd_SqrPipe);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, layerInd_Pinbolt);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_HANGER, layerInd_Hanger);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, layerInd_Head);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_COLUMN_BAND, layerInd_ColumnBand);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, layerInd_Plywood);

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
			DGSetDialogTitle (dialogID, "��տ� ��ġ - ��� ����");

			//////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			// Ȯ�� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 30, 100, 100, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "Ȯ��");
			DGShowItem (dialogID, DG_OK);

			// ��� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 30, 140, 100, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "���");
			DGShowItem (dialogID, DG_CANCEL);

			// ������Ʈ ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 30, 60, 100, 25);
			DGSetItemFont (dialogID, DG_UPDATE_BUTTON, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_UPDATE_BUTTON, "������Ʈ");
			DGShowItem (dialogID, DG_UPDATE_BUTTON);
			DGDisableItem (dialogID, DG_UPDATE_BUTTON);

			// ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 30, 180, 100, 25);
			DGSetItemFont (dialogID, DG_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_PREV, "����");
			DGShowItem (dialogID, DG_PREV);

			// ��: ��� ����
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 10, 10, 100, 23);
			DGSetItemFont (dialogID, LABEL_COLUMN_SIDE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_COLUMN_SIDE, "��� ����");
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
				DGSetItemText (dialogID, itmIdx, "��\n����");
			else
				DGSetItemText (dialogID, itmIdx, "��\n����");

			// ���� ��ġ
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 235, 110, 40, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "����");
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
			DGSetItemText (dialogID, itmIdx, "�߰�");
			DGShowItem (dialogID, itmIdx);
			ADD_CELLS = itmIdx;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 310, 142, 50, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "����");
			DGShowItem (dialogID, itmIdx);
			DEL_CELLS = itmIdx;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 290, 134, 20, 20);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "��");
			DGShowItem (dialogID, itmIdx);

			// �� �ܸ��� �ǹ��ϴ� ���簢��
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 500, 100, 80, 80);
			DGShowItem (dialogID, itmIdx);

			// ��: ��������
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 530, 105, 25, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "��");
			DGShowItem (dialogID, itmIdx);
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 530, 155, 25, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "��");
			DGShowItem (dialogID, itmIdx);
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 505, 130, 25, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "��");
			DGShowItem (dialogID, itmIdx);
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 550, 130, 25, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "��");
			DGShowItem (dialogID, itmIdx);

			// üũ�ڽ�: �������� ���� ä�� ����
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 505, 50, 80, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "���� ä��");
			DGSetItemValLong (dialogID, itmIdx, TRUE);
			DGShowItem (dialogID, itmIdx);	// ��
			CHECKBOX_NORTH_MARGIN = itmIdx;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 505, 210, 80, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "���� ä��");
			DGSetItemValLong (dialogID, itmIdx, TRUE);
			DGShowItem (dialogID, itmIdx);	// ��
			CHECKBOX_SOUTH_MARGIN = itmIdx;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 420, 115, 80, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "���� ä��");
			DGSetItemValLong (dialogID, itmIdx, TRUE);
			DGShowItem (dialogID, itmIdx);	// ��
			CHECKBOX_WEST_MARGIN = itmIdx;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 590, 115, 80, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "���� ä��");
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
					DGSetItemText (dialogID, itmIdx, "��\n����");
				else
					DGSetItemText (dialogID, itmIdx, "��\n����");

				// ���� ��ġ
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 235, 110, 40, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
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
				DGSetItemText (dialogID, itmIdx, "�߰�");
				DGShowItem (dialogID, itmIdx);
				ADD_CELLS = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 310, 142, 50, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);
				DEL_CELLS = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 290, 134, 20, 20);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��");
				DGShowItem (dialogID, itmIdx);

				// �� �ܸ��� �ǹ��ϴ� ���簢��
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 500, 100, 80, 80);
				DGShowItem (dialogID, itmIdx);

				// ��: ��������
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 530, 105, 25, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��");
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 530, 155, 25, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��");
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 505, 130, 25, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��");
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 550, 130, 25, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��");
				DGShowItem (dialogID, itmIdx);

				// üũ�ڽ�: �������� ���� ä�� ����
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 505, 50, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "���� ä��");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtNorth == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// ��
				CHECKBOX_NORTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 505, 210, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "���� ä��");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtSouth == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// ��
				CHECKBOX_SOUTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 420, 115, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "���� ä��");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtWest == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// ��
				CHECKBOX_WEST_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 590, 115, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "���� ä��");
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
					DGSetItemText (dialogID, itmIdx, "��\n����");
				else
					DGSetItemText (dialogID, itmIdx, "��\n����");

				// ���� ��ġ
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 235, 110, 40, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
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
				DGSetItemText (dialogID, itmIdx, "�߰�");
				DGShowItem (dialogID, itmIdx);
				ADD_CELLS = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 310, 142, 50, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);
				DEL_CELLS = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 290, 134, 20, 20);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��");
				DGShowItem (dialogID, itmIdx);

				// �� �ܸ��� �ǹ��ϴ� ���簢��
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 500, 100, 80, 80);
				DGShowItem (dialogID, itmIdx);

				// ��: ��������
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 530, 105, 25, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��");
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 530, 155, 25, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��");
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 505, 130, 25, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��");
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 550, 130, 25, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��");
				DGShowItem (dialogID, itmIdx);

				// üũ�ڽ�: �������� ���� ä�� ����
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 505, 50, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "���� ä��");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtNorth == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// ��
				CHECKBOX_NORTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 505, 210, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "���� ä��");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtSouth == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// ��
				CHECKBOX_SOUTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 420, 115, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "���� ä��");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtWest == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// ��
				CHECKBOX_WEST_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 590, 115, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "���� ä��");
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
					DGSetItemText (dialogID, itmIdx, "��\n����");
				else
					DGSetItemText (dialogID, itmIdx, "��\n����");

				// ���� ��ġ
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 235, 110, 40, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
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
				DGSetItemText (dialogID, itmIdx, "�߰�");
				DGShowItem (dialogID, itmIdx);
				ADD_CELLS = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 310, 142, 50, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);
				DEL_CELLS = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 290, 134, 20, 20);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��");
				DGShowItem (dialogID, itmIdx);

				// �� �ܸ��� �ǹ��ϴ� ���簢��
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 500, 100, 80, 80);
				DGShowItem (dialogID, itmIdx);

				// ��: ��������
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 530, 105, 25, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��");
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 530, 155, 25, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��");
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 505, 130, 25, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��");
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 550, 130, 25, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��");
				DGShowItem (dialogID, itmIdx);

				// üũ�ڽ�: �������� ���� ä�� ����
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 505, 50, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "���� ä��");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtNorth == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// ��
				CHECKBOX_NORTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 505, 210, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "���� ä��");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtSouth == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// ��
				CHECKBOX_SOUTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 420, 115, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "���� ä��");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtWest == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// ��
				CHECKBOX_WEST_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 590, 115, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "���� ä��");
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
					DGSetItemText (dialogID, itmIdx, "��\n����");
				else
					DGSetItemText (dialogID, itmIdx, "��\n����");

				// ���� ��ġ
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 235, 110, 40, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
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
				DGSetItemText (dialogID, itmIdx, "�߰�");
				DGShowItem (dialogID, itmIdx);
				ADD_CELLS = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 310, 142, 50, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);
				DEL_CELLS = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 290, 134, 20, 20);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��");
				DGShowItem (dialogID, itmIdx);

				// �� �ܸ��� �ǹ��ϴ� ���簢��
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 500, 100, 80, 80);
				DGShowItem (dialogID, itmIdx);

				// ��: ��������
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 530, 105, 25, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��");
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 530, 155, 25, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��");
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 505, 130, 25, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��");
				DGShowItem (dialogID, itmIdx);
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 550, 130, 25, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��");
				DGShowItem (dialogID, itmIdx);

				// üũ�ڽ�: �������� ���� ä�� ����
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 505, 50, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "���� ä��");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtNorth == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// ��
				CHECKBOX_NORTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 505, 210, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "���� ä��");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtSouth == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// ��
				CHECKBOX_SOUTH_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 420, 115, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "���� ä��");
				DGSetItemValLong (dialogID, itmIdx, (placingZone.bFillMarginTopAtWest == true) ? TRUE : FALSE);
				DGShowItem (dialogID, itmIdx);	// ��
				CHECKBOX_WEST_MARGIN = itmIdx;
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 590, 115, 80, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "���� ä��");
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
			DGSetDialogTitle (dialogID, "�� ����");

			//////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			// ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 30, 160, 60, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "����");
			DGShowItem (dialogID, DG_OK);

			// ��� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 110, 160, 60, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "���");
			DGShowItem (dialogID, DG_CANCEL);

			//////////////////////////////////////////////////////////// �ʵ� ���� (Ŭ���� ��)
			// ��: ��ü Ÿ��
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 10, 20, 70, 23);
			DGSetItemFont (dialogID, LABEL_OBJ_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_OBJ_TYPE, "��ü Ÿ��");
			DGShowItem (dialogID, LABEL_OBJ_TYPE);

			// �˾���Ʈ��: ��ü Ÿ���� �ٲ� �� �ִ� �޺��ڽ��� �� ���� ����
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 90, 20-7, 100, 25);
			DGSetItemFont (dialogID, POPUP_OBJ_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, "����");
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, "������");
			DGShowItem (dialogID, POPUP_OBJ_TYPE);

			// üũ�ڽ�: �԰���
			DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 20, 60, 70, 25-5);
			DGSetItemFont (dialogID, CHECKBOX_SET_STANDARD, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, CHECKBOX_SET_STANDARD, "�԰���");

			// ��: ����
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 20, 90, 50, 23);
			DGSetItemFont (dialogID, LABEL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_LENGTH, "����");

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
				if (placingZone.cellsB1 [idx].libPart.form.eu_stan_onoff == true) {
					DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD, TRUE);
					DGShowItem (dialogID, POPUP_LENGTH);
					if (abs (placingZone.cellsB1 [idx].libPart.form.eu_hei - 1.200) < EPS)		popupSelectedIdx = 1;
					if (abs (placingZone.cellsB1 [idx].libPart.form.eu_hei - 0.900) < EPS)		popupSelectedIdx = 2;
					if (abs (placingZone.cellsB1 [idx].libPart.form.eu_hei - 0.600) < EPS)		popupSelectedIdx = 3;
					DGPopUpSelectItem (dialogID, POPUP_LENGTH, popupSelectedIdx);
				} else {
					DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD, FALSE);
					DGShowItem (dialogID, EDITCONTROL_LENGTH);
					DGSetItemValDouble (dialogID, EDITCONTROL_LENGTH, placingZone.cellsB1 [idx].libPart.form.eu_hei2);
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
						placingZone.cellsL1 [idx].objType = NONE;
						placingZone.cellsL2 [idx].objType = NONE;
						placingZone.cellsR1 [idx].objType = NONE;
						placingZone.cellsR2 [idx].objType = NONE;
						placingZone.cellsB1 [idx].objType = NONE;
						placingZone.cellsB2 [idx].objType = NONE;

					} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == EUROFORM + 1) {

						// �԰������� ������ ���
						if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == TRUE)
							length = atof (DGPopUpGetItemText (dialogID, POPUP_LENGTH, DGPopUpGetSelected (dialogID, POPUP_LENGTH)).ToCStr ()) / 1000.0;
						// ��԰������� ������ ���
						else
							length = DGGetItemValDouble (dialogID, EDITCONTROL_LENGTH);

						if (placingZone.bUseOutcornerPanel == true) {
							placingZone.cellsLT [idx].objType = OUTPANEL;
							placingZone.cellsLT [idx].height = length;
							placingZone.cellsLT [idx].libPart.outpanel.hei_s = length;

							placingZone.cellsRT [idx].objType = OUTPANEL;
							placingZone.cellsRT [idx].height = length;
							placingZone.cellsRT [idx].libPart.outpanel.hei_s = length;

							placingZone.cellsLB [idx].objType = OUTPANEL;
							placingZone.cellsLB [idx].height = length;
							placingZone.cellsLB [idx].libPart.outpanel.hei_s = length;

							placingZone.cellsRB [idx].objType = OUTPANEL;
							placingZone.cellsRB [idx].height = length;
							placingZone.cellsRB [idx].libPart.outpanel.hei_s = length;
						} else {
							placingZone.cellsLT [idx].objType = OUTANGLE;
							placingZone.cellsLT [idx].height = length;
							placingZone.cellsLT [idx].libPart.outangle.a_leng = length;

							placingZone.cellsRT [idx].objType = OUTANGLE;
							placingZone.cellsRT [idx].height = length;
							placingZone.cellsRT [idx].libPart.outangle.a_leng = length;

							placingZone.cellsLB [idx].objType = OUTANGLE;
							placingZone.cellsLB [idx].height = length;
							placingZone.cellsLB [idx].libPart.outangle.a_leng = length;

							placingZone.cellsRB [idx].objType = OUTANGLE;
							placingZone.cellsRB [idx].height = length;
							placingZone.cellsRB [idx].libPart.outangle.a_leng = length;
						}
							
						placingZone.cellsT1 [idx].objType = EUROFORM;
						placingZone.cellsT1 [idx].height = length;
						placingZone.cellsT1 [idx].libPart.form.eu_hei = length;
						placingZone.cellsT1 [idx].libPart.form.eu_hei2 = length;

						placingZone.cellsT2 [idx].objType = EUROFORM;
						placingZone.cellsT2 [idx].height = length;
						placingZone.cellsT2 [idx].libPart.form.eu_hei = length;
						placingZone.cellsT2 [idx].libPart.form.eu_hei2 = length;

						placingZone.cellsL1 [idx].objType = EUROFORM;
						placingZone.cellsL1 [idx].height = length;
						placingZone.cellsL1 [idx].libPart.form.eu_hei = length;
						placingZone.cellsL1 [idx].libPart.form.eu_hei2 = length;

						placingZone.cellsL2 [idx].objType = EUROFORM;
						placingZone.cellsL2 [idx].height = length;
						placingZone.cellsL2 [idx].libPart.form.eu_hei = length;
						placingZone.cellsL2 [idx].libPart.form.eu_hei2 = length;

						placingZone.cellsR1 [idx].objType = EUROFORM;
						placingZone.cellsR1 [idx].height = length;
						placingZone.cellsR1 [idx].libPart.form.eu_hei = length;
						placingZone.cellsR1 [idx].libPart.form.eu_hei2 = length;

						placingZone.cellsR2 [idx].objType = EUROFORM;
						placingZone.cellsR2 [idx].height = length;
						placingZone.cellsR2 [idx].libPart.form.eu_hei = length;
						placingZone.cellsR2 [idx].libPart.form.eu_hei2 = length;

						placingZone.cellsB1 [idx].objType = EUROFORM;
						placingZone.cellsB1 [idx].height = length;
						placingZone.cellsB1 [idx].libPart.form.eu_hei = length;
						placingZone.cellsB1 [idx].libPart.form.eu_hei2 = length;

						placingZone.cellsB2 [idx].objType = EUROFORM;
						placingZone.cellsB2 [idx].height = length;
						placingZone.cellsB2 [idx].libPart.form.eu_hei = length;
						placingZone.cellsB2 [idx].libPart.form.eu_hei2 = length;
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
