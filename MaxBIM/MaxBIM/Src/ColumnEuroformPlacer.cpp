#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "ColumnEuroformPlacer.hpp"

using namespace columnPlacerDG;

static ColumnPlacingZone	placingZone;			// �⺻ ��� ���� ����
static InfoColumn			infoColumn;				// ��� ��ü ����
static InfoWallForColumn	infoWall;				// ���� �� ����
static short				nInterfereBeams;		// ���� �� ����
static InfoBeamForColumn	infoOtherBeams [10];	// ���� �� ����
static short			layerInd_Euroform;			// ���̾� ��ȣ: ������
static short			layerInd_Incorner;			// ���̾� ��ȣ: ���ڳ��ǳ�
static short			layerInd_Outcorner;			// ���̾� ��ȣ: �ƿ��ڳ��ǳ�
static short			layerInd_Plywood;			// ���̾� ��ȣ: ����
static short			layerInd_MagicBar;			// ���̾� ��ȣ: ������
static short			layerInd_MagicIncorner;		// ���̾� ��ȣ: �������ڳ�
static short			clickedBtnItemIdx;			// �׸��� ��ư���� Ŭ���� ��ư�� �ε��� ��ȣ�� ����
static bool				clickedOKButton;			// OK ��ư�� �������ϱ�?
static bool				clickedPrevButton;			// ���� ��ư�� �������ϱ�?
static GS::Array<API_Guid>	elemList;				// �׷�ȭ�� ���� ������ ��������� GUID�� ���� ������

// �߰�/���� ��ư �ε��� ����
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


// 5�� �޴�: ��տ� �������� ��ġ�ϴ� ���� ��ƾ
GSErrCode	placeEuroformOnColumn (void)
{
	GSErrCode		err = NoError;
	long			nSel;
	short			xx, yy;
	double			dx, dy, ang;
	short			result;
	double			lowestBeamBottomLevel;

	// Selection Manager ���� ����
	API_SelectionInfo		selectionInfo;
	API_Element				tElem;
	API_Neig				**selNeigs;
	GS::Array<API_Guid>&	morphs = GS::Array<API_Guid> ();
	GS::Array<API_Guid>&	columns = GS::Array<API_Guid> ();
	GS::Array<API_Guid>&	walls = GS::Array<API_Guid> ();
	GS::Array<API_Guid>&	beams = GS::Array<API_Guid> ();
	long					nMorphs = 0;
	long					nColumns = 0;
	long					nWalls = 0;
	long					nBeams = 0;

	// ��ü ���� ��������
	API_Element				elem;
	API_ElementMemo			memo;
	API_ElemInfo3D			info3D;

	// ���� ��ü ����
	InfoMorphForColumn		infoMorph;

	// �۾� �� ����
	API_StoryInfo			storyInfo;
	double					workLevel_column;


	// ������ ��� ��������
	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err == APIERR_NOPLAN) {
		ACAPI_WriteReport ("���� ������Ʈ â�� �����ϴ�.", true);
		return err;
	}
	if (err == APIERR_NOSEL) {
		ACAPI_WriteReport ("�ƹ� �͵� �������� �ʾҽ��ϴ�.\n�ʼ� ����: ��� (1��), ��� ������ ���� ���� (1��)\n�ɼ� ����: ��հ� �´�ų� �����ϴ� ��(1��), ��հ� �´�� �� (�ټ�)", true);
		return err;
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

			if (tElem.header.typeID == API_WallID)		// ���ΰ�?
				walls.Push (tElem.header.guid);

			if (tElem.header.typeID == API_BeamID)		// ���ΰ�?
				beams.Push (tElem.header.guid);
		}
	}
	BMKillHandle ((GSHandle *) &selNeigs);
	nColumns = columns.GetSize ();
	nMorphs = morphs.GetSize ();
	nWalls = walls.GetSize ();
	nBeams = beams.GetSize ();

	// ����� 1���ΰ�?
	if (nColumns != 1) {
		ACAPI_WriteReport ("����� 1�� �����ؾ� �մϴ�.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// ���� 1�� �����ΰ�?
	if (nWalls > 1) {
		ACAPI_WriteReport ("��տ� �����ϴ� ���� 1���� �ν� �����մϴ�.", true);
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

	// �� ���� ����
	if (nWalls != 0) {
		infoWall.guid = walls.Pop ();

		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		elem.header.guid = infoWall.guid;
		err = ACAPI_Element_Get (&elem);
		err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

		infoWall.wallThk		= elem.wall.thickness;		// �� �β�
		infoWall.floorInd		= elem.header.floorInd;		// �� �ε���
		infoWall.bottomOffset	= elem.wall.bottomOffset;	// �� �ϴ� ������
		infoWall.offset			= elem.wall.offset;			// ���������� ���۷��� �������κ��� ���� ���� ������ ������
		infoWall.angle			= elem.wall.angle;			// ȸ�� ���� (����: Radian)

		infoWall.offsetFromOutside		= elem.wall.offsetFromOutside;		// ���� ���۷��� ���ΰ� ���� �ٱ��� �� ���� �Ÿ�
		infoWall.referenceLineLocation	= elem.wall.referenceLineLocation;	// ���۷��� ������ ��ġ

		infoWall.begX			= elem.wall.begC.x;			// ������ X
		infoWall.begY			= elem.wall.begC.y;			// ������ Y
		infoWall.endX			= elem.wall.endC.x;			// ���� X
		infoWall.endY			= elem.wall.endC.y;			// ���� Y

		ACAPI_DisposeElemMemoHdls (&memo);
	}

	// �� ���� ����
	nInterfereBeams = nBeams;

	for (xx = 0 ; xx < nInterfereBeams ; ++xx) {
		infoOtherBeams [xx].guid = beams.Pop ();

		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		elem.header.guid = infoOtherBeams [xx].guid;
		err = ACAPI_Element_Get (&elem);
		err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

		infoOtherBeams [xx].floorInd	= elem.header.floorInd;		// �� �ε���
		infoOtherBeams [xx].height		= elem.beam.height;			// �� ����
		infoOtherBeams [xx].width		= elem.beam.width;			// �� �ʺ�
		infoOtherBeams [xx].offset		= elem.beam.offset;			// �� �߽����κ��� ���� ���۷��� ������ �������Դϴ�.
		infoOtherBeams [xx].level		= elem.beam.level;			// �ٴ� ������ ���� ���� ���ʸ� �����Դϴ�.
		infoOtherBeams [xx].begC		= elem.beam.begC;			// �� ���� ��ǥ
		infoOtherBeams [xx].endC		= elem.beam.endC;			// �� �� ��ǥ

		ACAPI_DisposeElemMemoHdls (&memo);
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
	//API_Elem_Head* headList = new API_Elem_Head [1];
	//headList [0] = elem.header;
	//err = ACAPI_Element_Delete (&headList, 1);
	//delete headList;

	// �۾� �� ���� �ݿ�
	BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	workLevel_column = 0.0;
	ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
	for (xx = 0 ; xx < (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
		if (storyInfo.data [0][xx].index == infoColumn.floorInd) {
			workLevel_column = storyInfo.data [0][xx].level;
			break;
		}
	}
	BMKillHandle ((GSHandle *) &storyInfo.data);

	// ���� ������ �� ���� ���� - ���ʿ�

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
	initCellsForColumn (&placingZone);

	// �ܵ� ����� ���
	if (nWalls == 0) {

FIRST_SOLE_COLUMN:
	
		// ���� ���� �� ���� �� ���� ���� ������Ʈ
		if (nInterfereBeams > 0) {
			placingZone.bInterfereBeam = true;
			placingZone.nInterfereBeams = nInterfereBeams;
			
			for (xx = 0 ; xx < 4 ; ++xx) {
				placingZone.bottomLevelOfBeams [xx] = 0.0;
				placingZone.bExistBeams [xx] = false;
			}

			for (xx = 0 ; xx < nInterfereBeams ; ++xx) {

				// !!!
				//char msg [200];
				//sprintf (msg, "�� ���� [%d]: %.4f", xx+1, infoOtherBeams [xx].level - infoOtherBeams [xx].height);
				//ACAPI_WriteReport (msg, true);

				// ����� ��/��/��/�� ���⿡ �ִ� ���� �ϴ� ������ ������
				if ( (infoOtherBeams [xx].begC.x <= (placingZone.origoPos.x + placingZone.coreWidth/2)) && (infoOtherBeams [xx].begC.x >= (placingZone.origoPos.x - placingZone.coreWidth/2)) && (infoOtherBeams [xx].begC.y > (placingZone.origoPos.y + placingZone.coreDepth/2)) ) {
					placingZone.bottomLevelOfBeams [NORTH] = infoOtherBeams [xx].level - infoOtherBeams [xx].height;
					placingZone.bExistBeams [NORTH] = true;
				}
				if ( (infoOtherBeams [xx].begC.x <= (placingZone.origoPos.x + placingZone.coreWidth/2)) && (infoOtherBeams [xx].begC.x >= (placingZone.origoPos.x - placingZone.coreWidth/2)) && (infoOtherBeams [xx].begC.y < (placingZone.origoPos.y - placingZone.coreDepth/2)) ) {
					placingZone.bottomLevelOfBeams [SOUTH] = infoOtherBeams [xx].level - infoOtherBeams [xx].height;
					placingZone.bExistBeams [SOUTH] = true;
				}
				if ( (infoOtherBeams [xx].begC.y <= (placingZone.origoPos.y + placingZone.coreDepth/2)) && (infoOtherBeams [xx].begC.y >= (placingZone.origoPos.y - placingZone.coreDepth/2)) && (infoOtherBeams [xx].begC.x > (placingZone.origoPos.x + placingZone.coreWidth/2)) ) {
					placingZone.bottomLevelOfBeams [EAST] = infoOtherBeams [xx].level - infoOtherBeams [xx].height;
					placingZone.bExistBeams [EAST] = true;
				}
				if ( (infoOtherBeams [xx].begC.y <= (placingZone.origoPos.y + placingZone.coreDepth/2)) && (infoOtherBeams [xx].begC.y >= (placingZone.origoPos.y - placingZone.coreDepth/2)) && (infoOtherBeams [xx].begC.x < (placingZone.origoPos.x - placingZone.coreWidth/2)) ) {
					placingZone.bottomLevelOfBeams [WEST] = infoOtherBeams [xx].level - infoOtherBeams [xx].height;
					placingZone.bExistBeams [WEST] = true;
				}
			}
		} else {
			placingZone.bInterfereBeam = false;
			placingZone.nInterfereBeams = 0;

			for (xx = 0 ; xx < 4 ; ++xx) {
				placingZone.bottomLevelOfBeams [xx] = 0.0;
				placingZone.bExistBeams [xx] = false;
			}
		}

		// [DIALOG] 1��° ���̾�α׿��� ������ ���� �Է� ����
		result = DGModalDialog (ACAPI_GetOwnResModule (), 32531, ACAPI_GetOwnResModule (), columnPlacerHandler1, 0);

		if (result == DG_CANCEL)
			return err;

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

		// [DIALOG] 2��° ���̾�α׿��� ������ ��ġ�� �����մϴ�.
		clickedOKButton = false;
		clickedPrevButton = false;
		result = DGBlankModalDialog (700, 300, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, columnPlacerHandler2, 0);
	
		// ���� ��ư�� ������ 1��° ���̾�α� �ٽ� ����
		if (clickedPrevButton == true)
			goto FIRST_SOLE_COLUMN;

		// 2��° ���̾�α׿��� OK ��ư�� �����߸� ���� �ܰ�� �Ѿ
		if (clickedOKButton != true)
			return err;

		// 1, 2��° ���̾�α׸� ���� �Էµ� �����͸� ������� ��ü�� ��ġ
		// ...

		// ������ ���� ä��� - ����, ����
		//err = fillRestAreasForColumn (&placingZone);

		// ����� ��ü �׷�ȭ
		//if (!elemList.IsEmpty ()) {
		//	GSSize nElems = elemList.GetSize ();
		//	API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
		//	if (elemHead != NULL) {
		//		for (GSIndex i = 0; i < nElems; i++)
		//			(*elemHead)[i].guid = elemList[i];

		//		ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

		//		BMKillHandle ((GSHandle *) &elemHead);
		//	}
		//}

	// ��ü�� �´�ų� ��ü �� ����� ���
	} else {

		// ...
		// ��ü �� ����� ���
		// 1. ���� ����� �߽��� ã��, ȸ������ �ʾ��� ���� ����� �� �������� ã�´�. (ȸ�������� ���� �������� ��� �ִ��� Ȯ���� �� ��)
		// 2. ���� ����/�ٱ��� ���� 2���� ������ ����/������ ã��.. ����� �߽ɰ� ���� ���� ���� �Ÿ��� �����Ѵ�. (�Ÿ��� ����� �ʺ�/2 �Ǵ� ����/2 �����̸� �ٰų� ����)
		// 3. ���� ��ġ�� ���ϴ� ��
		//		- ��� �߽��� �������� ���� �� ���� ��� Y���� ũ��, X���� �ϳ��� �۰� �ٸ� �ϳ��� ŭ -> ���� ���ʿ� ����
		//		- ��� �߽��� �������� ���� �� ���� ��� Y���� �۰�, X���� �ϳ��� �۰� �ٸ� �ϳ��� ŭ -> ���� ���ʿ� ����
		//		- ��� �߽��� �������� ���� �� ���� ��� X���� ũ��, Y���� �ϳ��� �۰� �ٸ� �ϳ��� ŭ -> ���� ���ʿ� ����
		//		- ��� �߽��� �������� ���� �� ���� ��� X���� �۰�, Y���� �ϳ��� �۰� �ٸ� �ϳ��� ŭ -> ���� ���ʿ� ����
		// 4. ���� ħ�� �Ÿ�
		//		- ???
	}

	return	err;
}

// Cell �迭�� �ʱ�ȭ��
void	initCellsForColumn (ColumnPlacingZone* placingZone)
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
		placingZone->cellsLT [xx].anchor = LEFT_TOP;

		placingZone->cellsRT [xx].objType = NONE;
		placingZone->cellsRT [xx].leftBottomX = 0.0;
		placingZone->cellsRT [xx].leftBottomY = 0.0;
		placingZone->cellsRT [xx].leftBottomZ = 0.0;
		placingZone->cellsRT [xx].ang = 0.0;
		placingZone->cellsRT [xx].horLen = 0.0;
		placingZone->cellsRT [xx].verLen = 0.0;
		placingZone->cellsRT [xx].height = 0.0;
		placingZone->cellsRT [xx].anchor = RIGHT_TOP;

		placingZone->cellsLB [xx].objType = NONE;
		placingZone->cellsLB [xx].leftBottomX = 0.0;
		placingZone->cellsLB [xx].leftBottomY = 0.0;
		placingZone->cellsLB [xx].leftBottomZ = 0.0;
		placingZone->cellsLB [xx].ang = 0.0;
		placingZone->cellsLB [xx].horLen = 0.0;
		placingZone->cellsLB [xx].verLen = 0.0;
		placingZone->cellsLB [xx].height = 0.0;
		placingZone->cellsLB [xx].anchor = LEFT_BOTTOM;

		placingZone->cellsRB [xx].objType = NONE;
		placingZone->cellsRB [xx].leftBottomX = 0.0;
		placingZone->cellsRB [xx].leftBottomY = 0.0;
		placingZone->cellsRB [xx].leftBottomZ = 0.0;
		placingZone->cellsRB [xx].ang = 0.0;
		placingZone->cellsRB [xx].horLen = 0.0;
		placingZone->cellsRB [xx].verLen = 0.0;
		placingZone->cellsRB [xx].height = 0.0;
		placingZone->cellsRB [xx].anchor = RIGHT_BOTTOM;

		placingZone->cellsT1 [xx].objType = NONE;
		placingZone->cellsT1 [xx].leftBottomX = 0.0;
		placingZone->cellsT1 [xx].leftBottomY = 0.0;
		placingZone->cellsT1 [xx].leftBottomZ = 0.0;
		placingZone->cellsT1 [xx].ang = 0.0;
		placingZone->cellsT1 [xx].horLen = 0.0;
		placingZone->cellsT1 [xx].verLen = 0.0;
		placingZone->cellsT1 [xx].height = 0.0;
		placingZone->cellsT1 [xx].anchor = TOP;

		placingZone->cellsT2 [xx].objType = NONE;
		placingZone->cellsT2 [xx].leftBottomX = 0.0;
		placingZone->cellsT2 [xx].leftBottomY = 0.0;
		placingZone->cellsT2 [xx].leftBottomZ = 0.0;
		placingZone->cellsT2 [xx].ang = 0.0;
		placingZone->cellsT2 [xx].horLen = 0.0;
		placingZone->cellsT2 [xx].verLen = 0.0;
		placingZone->cellsT2 [xx].height = 0.0;
		placingZone->cellsT2 [xx].anchor = TOP;

		placingZone->cellsL1 [xx].objType = NONE;
		placingZone->cellsL1 [xx].leftBottomX = 0.0;
		placingZone->cellsL1 [xx].leftBottomY = 0.0;
		placingZone->cellsL1 [xx].leftBottomZ = 0.0;
		placingZone->cellsL1 [xx].ang = 0.0;
		placingZone->cellsL1 [xx].horLen = 0.0;
		placingZone->cellsL1 [xx].verLen = 0.0;
		placingZone->cellsL1 [xx].height = 0.0;
		placingZone->cellsL1 [xx].anchor = LEFT;

		placingZone->cellsL2 [xx].objType = NONE;
		placingZone->cellsL2 [xx].leftBottomX = 0.0;
		placingZone->cellsL2 [xx].leftBottomY = 0.0;
		placingZone->cellsL2 [xx].leftBottomZ = 0.0;
		placingZone->cellsL2 [xx].ang = 0.0;
		placingZone->cellsL2 [xx].horLen = 0.0;
		placingZone->cellsL2 [xx].verLen = 0.0;
		placingZone->cellsL2 [xx].height = 0.0;
		placingZone->cellsL2 [xx].anchor = LEFT;

		placingZone->cellsR1 [xx].objType = NONE;
		placingZone->cellsR1 [xx].leftBottomX = 0.0;
		placingZone->cellsR1 [xx].leftBottomY = 0.0;
		placingZone->cellsR1 [xx].leftBottomZ = 0.0;
		placingZone->cellsR1 [xx].ang = 0.0;
		placingZone->cellsR1 [xx].horLen = 0.0;
		placingZone->cellsR1 [xx].verLen = 0.0;
		placingZone->cellsR1 [xx].height = 0.0;
		placingZone->cellsR1 [xx].anchor = RIGHT;

		placingZone->cellsR2 [xx].objType = NONE;
		placingZone->cellsR2 [xx].leftBottomX = 0.0;
		placingZone->cellsR2 [xx].leftBottomY = 0.0;
		placingZone->cellsR2 [xx].leftBottomZ = 0.0;
		placingZone->cellsR2 [xx].ang = 0.0;
		placingZone->cellsR2 [xx].horLen = 0.0;
		placingZone->cellsR2 [xx].verLen = 0.0;
		placingZone->cellsR2 [xx].height = 0.0;
		placingZone->cellsR2 [xx].anchor = RIGHT;

		placingZone->cellsB1 [xx].objType = NONE;
		placingZone->cellsB1 [xx].leftBottomX = 0.0;
		placingZone->cellsB1 [xx].leftBottomY = 0.0;
		placingZone->cellsB1 [xx].leftBottomZ = 0.0;
		placingZone->cellsB1 [xx].ang = 0.0;
		placingZone->cellsB1 [xx].horLen = 0.0;
		placingZone->cellsB1 [xx].verLen = 0.0;
		placingZone->cellsB1 [xx].height = 0.0;
		placingZone->cellsB1 [xx].anchor = BOTTOM;

		placingZone->cellsB2 [xx].objType = NONE;
		placingZone->cellsB2 [xx].leftBottomX = 0.0;
		placingZone->cellsB2 [xx].leftBottomY = 0.0;
		placingZone->cellsB2 [xx].leftBottomZ = 0.0;
		placingZone->cellsB2 [xx].ang = 0.0;
		placingZone->cellsB2 [xx].horLen = 0.0;
		placingZone->cellsB2 [xx].verLen = 0.0;
		placingZone->cellsB2 [xx].height = 0.0;
		placingZone->cellsB2 [xx].anchor = BOTTOM;
	}

	// �� ���� �ʱ�ȭ
	placingZone->nCells = 0;
}

// ����⿡ �� �߰�
void	addTopCell (ColumnPlacingZone* target_zone)
{
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
void	delTopCell (ColumnPlacingZone* target_zone)
{
	target_zone->nCells --;
}

// 1�� ��ġ�� ���� ���Ǹ� ��û�ϴ� 1�� ���̾�α�
short DGCALLBACK columnPlacerHandler1 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
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
			// �� �� üũ�ڽ�
			DGSetItemText (dialogID, LABEL_COLUMN_SECTION, "��� �ܸ�");
			DGSetItemText (dialogID, LABEL_COLUMN_WIDTH, "����");
			DGSetItemText (dialogID, LABEL_COLUMN_DEPTH, "����");

			// ��: ���̾� ����
			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, "���纰 ���̾� ����");
			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM, "������");
			DGSetItemText (dialogID, LABEL_LAYER_INCORNER, "���ڳ�");
			DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER, "�ƿ��ڳ�");
			DGSetItemText (dialogID, LABEL_LAYER_PLYWOOD, "����");
			DGSetItemText (dialogID, LABEL_LAYER_MAGIC_BAR, "���� ��");
			DGSetItemText (dialogID, LABEL_LAYER_MAGIC_INCORNER, "���� ���ڳ�");

			// ���� ��Ʈ�� �ʱ�ȭ
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_LAYER_EUROFORM;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, 1);

			ucb.itemID	 = USERCONTROL_LAYER_INCORNER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER, 1);

			ucb.itemID	 = USERCONTROL_LAYER_OUTCORNER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER, 1);

			ucb.itemID	 = USERCONTROL_LAYER_PLYWOOD;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, 1);

			ucb.itemID	 = USERCONTROL_LAYER_MAGIC_BAR;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_MAGIC_BAR, 1);

			ucb.itemID	 = USERCONTROL_LAYER_MAGIC_INCORNER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_MAGIC_INCORNER, 1);

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

			// ���纰 üũ�ڽ�-�԰� ����
			if (DGGetItemValLong (dialogID, CHECKBOX_LEFT_ADDITIONAL_FORM) == TRUE) {
				DGEnableItem (dialogID, EDITCONTROL_LEFT_3);
				DGEnableItem (dialogID, EDITCONTROL_RIGHT_3);
				DGSetItemValLong (dialogID, CHECKBOX_RIGHT_ADDITIONAL_FORM, TRUE);
			} else {
				DGDisableItem (dialogID, EDITCONTROL_LEFT_3);
				DGDisableItem (dialogID, EDITCONTROL_RIGHT_3);
				DGSetItemValDouble (dialogID, EDITCONTROL_LEFT_3, 0.0);
				DGSetItemValDouble (dialogID, EDITCONTROL_RIGHT_3, 0.0);
				DGSetItemValLong (dialogID, CHECKBOX_RIGHT_ADDITIONAL_FORM, FALSE);
			}
			if (DGGetItemValLong (dialogID, CHECKBOX_BOTTOM_ADDITIONAL_FORM) == TRUE) {
				DGEnableItem (dialogID, EDITCONTROL_BOTTOM_3);
				DGEnableItem (dialogID, EDITCONTROL_TOP_3);
				DGSetItemValLong (dialogID, CHECKBOX_TOP_ADDITIONAL_FORM, TRUE);
			} else {
				DGDisableItem (dialogID, EDITCONTROL_BOTTOM_3);
				DGDisableItem (dialogID, EDITCONTROL_TOP_3);
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

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					//////////////////////////////////////// ���̾�α� â ������ �Է� ����
					// �� ���� ����
					for (xx = 0 ; xx < 20 ; ++xx) {
						// �»��
						xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
						yLen = (placingZone.coreDepth/2 + placingZone.venThick);
						lineLen = sqrt (xLen*xLen + yLen*yLen);
						rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
						rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

						placingZone.cellsLT [xx].objType = OUTCORNER;
						placingZone.cellsLT [xx].leftBottomX = rotatedPoint.x;
						placingZone.cellsLT [xx].leftBottomY = rotatedPoint.y;
						placingZone.cellsLT [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
						placingZone.cellsLT [xx].ang = placingZone.angle - DegreeToRad (90);
						placingZone.cellsLT [xx].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_TOP_1);
						placingZone.cellsLT [xx].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_1);
						placingZone.cellsLT [xx].height = 1.200;
						placingZone.cellsLT [xx].libPart.outcorner.leng_s = DGGetItemValDouble (dialogID, EDITCONTROL_TOP_1);
						placingZone.cellsLT [xx].libPart.outcorner.wid_s = DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_1);
						placingZone.cellsLT [xx].libPart.outcorner.hei_s = 1.200;

						// ����
						xLen = (placingZone.coreWidth/2 + placingZone.venThick);
						yLen = (placingZone.coreDepth/2 + placingZone.venThick);
						lineLen = sqrt (xLen*xLen + yLen*yLen);
						rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
						rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

						placingZone.cellsRT [xx].objType = OUTCORNER;
						placingZone.cellsRT [xx].leftBottomX = rotatedPoint.x;
						placingZone.cellsRT [xx].leftBottomY = rotatedPoint.y;
						placingZone.cellsRT [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
						placingZone.cellsRT [xx].ang = placingZone.angle + DegreeToRad (180);
						placingZone.cellsRT [xx].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_TOP_4);
						placingZone.cellsRT [xx].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_1);
						placingZone.cellsRT [xx].height = 1.200;
						placingZone.cellsRT [xx].libPart.outcorner.leng_s = DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_1);
						placingZone.cellsRT [xx].libPart.outcorner.wid_s = DGGetItemValDouble (dialogID, EDITCONTROL_TOP_4);
						placingZone.cellsRT [xx].libPart.outcorner.hei_s = 1.200;

						// ���ϴ�
						xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
						yLen = -(placingZone.coreDepth/2 + placingZone.venThick);
						lineLen = sqrt (xLen*xLen + yLen*yLen);
						rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
						rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

						placingZone.cellsLB [xx].objType = OUTCORNER;
						placingZone.cellsLB [xx].leftBottomX = rotatedPoint.x;
						placingZone.cellsLB [xx].leftBottomY = rotatedPoint.y;
						placingZone.cellsLB [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
						placingZone.cellsLB [xx].ang = placingZone.angle;
						placingZone.cellsLB [xx].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_1);
						placingZone.cellsLB [xx].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_4);
						placingZone.cellsLB [xx].height = 1.200;
						placingZone.cellsLB [xx].libPart.outcorner.leng_s = DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_4);
						placingZone.cellsLB [xx].libPart.outcorner.wid_s = DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_1);
						placingZone.cellsLB [xx].libPart.outcorner.hei_s = 1.200;

						// ���ϴ�
						xLen = (placingZone.coreWidth/2 + placingZone.venThick);
						yLen = -(placingZone.coreDepth/2 + placingZone.venThick);
						lineLen = sqrt (xLen*xLen + yLen*yLen);
						rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
						rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

						placingZone.cellsRB [xx].objType = OUTCORNER;
						placingZone.cellsRB [xx].leftBottomX = rotatedPoint.x;
						placingZone.cellsRB [xx].leftBottomY = rotatedPoint.y;
						placingZone.cellsRB [xx].leftBottomZ = placingZone.bottomOffset + (1.200 * xx);
						placingZone.cellsRB [xx].ang = placingZone.angle + DegreeToRad (90);
						placingZone.cellsRB [xx].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_4);
						placingZone.cellsRB [xx].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_4);
						placingZone.cellsRB [xx].height = 1.200;
						placingZone.cellsRB [xx].libPart.outcorner.leng_s = DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_4);
						placingZone.cellsRB [xx].libPart.outcorner.wid_s = DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_4);
						placingZone.cellsRB [xx].libPart.outcorner.hei_s = 1.200;

						// ���� 1
						xLen = -(placingZone.coreWidth/2 + placingZone.venThick);
						yLen = (placingZone.coreDepth/2 + placingZone.venThick);
						lineLen = sqrt (xLen*xLen + yLen*yLen);
						rotatedPoint.x = placingZone.origoPos.x + lineLen*cos(atan2 (yLen, xLen) + placingZone.angle);
						rotatedPoint.y = placingZone.origoPos.y + lineLen*sin(atan2 (yLen, xLen) + placingZone.angle);

						placingZone.cellsT1 [xx].objType = EUROFORM;
						placingZone.cellsT1 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, EDITCONTROL_TOP_1) + DGGetItemValDouble (dialogID, EDITCONTROL_TOP_2)) * cos(placingZone.angle);
						placingZone.cellsT1 [xx].leftBottomY = rotatedPoint.y + (DGGetItemValDouble (dialogID, EDITCONTROL_TOP_1) + DGGetItemValDouble (dialogID, EDITCONTROL_TOP_2)) * sin(placingZone.angle);
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
							placingZone.cellsT2 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, EDITCONTROL_TOP_1) + DGGetItemValDouble (dialogID, EDITCONTROL_TOP_2) + DGGetItemValDouble (dialogID, EDITCONTROL_TOP_3)) * cos(placingZone.angle);
							placingZone.cellsT2 [xx].leftBottomY = rotatedPoint.y + (DGGetItemValDouble (dialogID, EDITCONTROL_TOP_1) + DGGetItemValDouble (dialogID, EDITCONTROL_TOP_2) + DGGetItemValDouble (dialogID, EDITCONTROL_TOP_3)) * sin(placingZone.angle);
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
						placingZone.cellsL1 [xx].leftBottomX = rotatedPoint.x + DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_1) * cos(placingZone.angle);
						placingZone.cellsL1 [xx].leftBottomY = rotatedPoint.y - DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_1) * sin(placingZone.angle);
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
							placingZone.cellsL2 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_1) + DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_2)) * cos(placingZone.angle);
							placingZone.cellsL2 [xx].leftBottomY = rotatedPoint.y - (DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_1) + DGGetItemValDouble (dialogID, EDITCONTROL_LEFT_2)) * sin(placingZone.angle);
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
						placingZone.cellsR1 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_1) + DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_2)) * cos(placingZone.angle);
						placingZone.cellsR1 [xx].leftBottomY = rotatedPoint.y - (DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_1) + DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_2)) * sin(placingZone.angle);
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
							placingZone.cellsR2 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_1) + DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_2) + DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_3)) * cos(placingZone.angle);
							placingZone.cellsR2 [xx].leftBottomY = rotatedPoint.y - (DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_1) + DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_2) + DGGetItemValDouble (dialogID, EDITCONTROL_RIGHT_3)) * sin(placingZone.angle);
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
						placingZone.cellsB1 [xx].leftBottomX = rotatedPoint.x + DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_1) * cos(placingZone.angle);
						placingZone.cellsB1 [xx].leftBottomY = rotatedPoint.y + DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_1) * sin(placingZone.angle);
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
							placingZone.cellsB2 [xx].leftBottomX = rotatedPoint.x + (DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_1) + DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_2)) * cos(placingZone.angle);
							placingZone.cellsB2 [xx].leftBottomY = rotatedPoint.y + (DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_1) + DGGetItemValDouble (dialogID, EDITCONTROL_BOTTOM_2)) * sin(placingZone.angle);
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

					// ���̾� ��ȣ ����
					layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM);
					layerInd_Incorner		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER);
					layerInd_Outcorner		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER);
					layerInd_Plywood		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD);
					layerInd_MagicBar		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_MAGIC_BAR);
					layerInd_MagicIncorner	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_MAGIC_INCORNER);

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
short DGCALLBACK columnPlacerHandler2 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
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

			// �� �ܸ�
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 230, 40, btnSizeX, btnSizeY);
			DGSetItemFont (dialogID, BUTTON_BEAM_SECTION, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, BUTTON_BEAM_SECTION);
			if (placingZone.bInterfereBeam == true) {
				DGEnableItem (dialogID, BUTTON_BEAM_SECTION);
				DGSetItemText (dialogID, BUTTON_BEAM_SECTION, "��\n����");
			} else {
				DGDisableItem (dialogID, BUTTON_BEAM_SECTION);
				DGSetItemText (dialogID, BUTTON_BEAM_SECTION, "��\n����");
			}

			// ���� ��ġ
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 235, 110, 40, 23);
			DGSetItemFont (dialogID, LABEL_MARGIN_PLACE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_MARGIN_PLACE, "����");
			DGShowItem (dialogID, LABEL_MARGIN_PLACE);

			// ���� �ǹ��ϴ� ���簢��
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 220, 30, 70, 175 + (btnSizeY * (placingZone.nCells-1)));
			DGShowItem (dialogID, itmIdx);

			// ��ư ����
			btnPosX = 230;
			btnPosY = 140 + (btnSizeY * (placingZone.nCells-1));
			for (xx = placingZone.nCells-1 ; xx >= 0 ; --xx) {

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
			//dialogSizeX = max<short>(500, 150 + (btnSizeX * (placingZone.nCellsFromBeginAtBottom + placingZone.nCellsFromEndAtBottom + 1)) + 150);
			//dialogSizeY = 490;
			//DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);

			break;

		case DG_MSG_CHANGE:

			break;

		case DG_MSG_CLICK:

			// ������Ʈ ��ư
			if (item == DG_UPDATE_BUTTON) {
				item = 0;

				//// ����� ���� ���� ���� ���� ����
				//if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_BEGIN_AT_SIDE) == TRUE)
				//	placingZone.bFillMarginBeginAtSide = true;
				//else
				//	placingZone.bFillMarginBeginAtSide = false;

				//// ����� ���� �� ���� ���� ����
				//if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_END_AT_SIDE) == TRUE)
				//	placingZone.bFillMarginEndAtSide = true;
				//else
				//	placingZone.bFillMarginEndAtSide = false;

				//// ����� �Ϻ� ���� ���� ���� ����
				//if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_BEGIN_AT_BOTTOM) == TRUE)
				//	placingZone.bFillMarginBeginAtBottom = true;
				//else
				//	placingZone.bFillMarginBeginAtBottom = false;

				//// ����� �Ϻ� �� ���� ���� ����
				//if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_END_AT_BOTTOM) == TRUE)
				//	placingZone.bFillMarginEndAtBottom = true;
				//else
				//	placingZone.bFillMarginEndAtBottom = false;

				//// ���� ���� �ٴ� �� ���� ���� ����
				//placingZone.centerLengthAtSide = DGGetItemValDouble (dialogID, EDITCONTROL_CENTER_LENGTH_SIDE);

				//// �� ���� ���� �߻�, ��� ���� ��ġ ���� ������Ʈ
				//alignPlacingZoneForBeam (&placingZone);

				//// ���� ���ɼ��� �ִ� DG �׸� ��� ����
				//DGRemoveDialogItems (dialogID, AFTER_ALL);

				//// ���� ���� �κ� ���� ä�� ���� - bFillMarginBeginAtSide
				//// ���� ��ư: ���� (ä��)
				//itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 111, 90, 110, 70, 25);
				//DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				//DGSetItemText (dialogID, itmIdx, "���� ä��");
				//DGShowItem (dialogID, itmIdx);
				//MARGIN_FILL_FROM_BEGIN_AT_SIDE = itmIdx;
				// ...

				//// ���� â ũ�⸦ ����
				//dialogSizeX = max<short>(500, 150 + (btnSizeX * (placingZone.nCellsFromBeginAtBottom + placingZone.nCellsFromEndAtBottom + 1)) + 150);
				//dialogSizeY = 490;
				//DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);
			}

			// ���� ��ư
			if (item == DG_PREV) {
				clickedPrevButton = true;
			}

			// Ȯ�� ��ư
			if (item == DG_OK) {
				clickedOKButton = true;

				//// ���� ä��/��� ���� ����
				//if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_BEGIN_AT_SIDE) == TRUE)
				//	placingZone.bFillMarginBeginAtSide = true;
				//if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_END_AT_SIDE) == TRUE)
				//	placingZone.bFillMarginEndAtSide = true;
				//if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_BEGIN_AT_BOTTOM) == TRUE)
				//	placingZone.bFillMarginBeginAtBottom = true;
				//if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_END_AT_BOTTOM) == TRUE)
				//	placingZone.bFillMarginEndAtBottom = true;

				//placingZone.centerLengthAtSide = DGGetItemValDouble (dialogID, EDITCONTROL_CENTER_LENGTH_SIDE);

				//// �� ���� ���� �߻�, ��� ���� ��ġ ���� ������Ʈ
				//alignPlacingZoneForBeam (&placingZone);
			}

			// ��� ��ư
			if (item == DG_CANCEL) {
			}

			//// �� �߰�/���� ��ư 8��
			//if (item == ADD_CELLS_FROM_BEGIN_AT_SIDE) {
			//	addNewColFromBeginAtSide (&placingZone);
			//}
			//if (item == DEL_CELLS_FROM_BEGIN_AT_SIDE) {
			//	delLastColFromBeginAtSide (&placingZone);
			//}
			//if (item == ADD_CELLS_FROM_END_AT_SIDE) {
			//	addNewColFromEndAtSide (&placingZone);
			//}
			//if (item == DEL_CELLS_FROM_END_AT_SIDE) {
			//	delLastColFromEndAtSide (&placingZone);
			//}
			//if (item == ADD_CELLS_FROM_BEGIN_AT_BOTTOM) {
			//	addNewColFromBeginAtBottom (&placingZone);
			//}
			//if (item == DEL_CELLS_FROM_BEGIN_AT_BOTTOM) {
			//	delLastColFromBeginAtBottom (&placingZone);
			//}
			//if (item == ADD_CELLS_FROM_END_AT_BOTTOM) {
			//	addNewColFromEndAtBottom (&placingZone);
			//}
			//if (item == DEL_CELLS_FROM_END_AT_BOTTOM) {
			//	delLastColFromEndAtBottom (&placingZone);
			//}

			//if ( (item == ADD_CELLS_FROM_BEGIN_AT_SIDE) || (item == DEL_CELLS_FROM_BEGIN_AT_SIDE) || (item == ADD_CELLS_FROM_END_AT_SIDE) || (item == DEL_CELLS_FROM_END_AT_SIDE) ||
			//	 (item == ADD_CELLS_FROM_BEGIN_AT_BOTTOM) || (item == DEL_CELLS_FROM_BEGIN_AT_BOTTOM) || (item == ADD_CELLS_FROM_END_AT_BOTTOM) || (item == DEL_CELLS_FROM_END_AT_BOTTOM)) {

			//	item = 0;

			//	// �� ���� ���� �߻�, ��� ���� ��ġ ���� ������Ʈ
			//	alignPlacingZoneForBeam (&placingZone);

			//	// ���� ���ɼ��� �ִ� DG �׸� ��� ����
			//	DGRemoveDialogItems (dialogID, AFTER_ALL);

			//	// ���� ���� �κ� ���� ä�� ���� - bFillMarginBeginAtSide
			//	// ���� ��ư: ���� (ä��)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 111, 90, 110, 70, 25);
			// ...

			//	// ���� â ũ�⸦ ����
			//	dialogSizeX = max<short>(500, 150 + (btnSizeX * (placingZone.nCellsFromBeginAtBottom + placingZone.nCellsFromEndAtBottom + 1)) + 150);
			//	dialogSizeY = 490;
			//	DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);
			//}

			//// ��ġ ��ư (���� ���� �κ�)
			//if ((item >= START_INDEX_FROM_BEGIN_AT_SIDE) && (item < START_INDEX_FROM_BEGIN_AT_SIDE + placingZone.nCellsFromBeginAtSide)) {
			//	// [DIALOG] �׸��� ��ư�� ������ Cell�� �����ϱ� ���� ���� â(3��° ���̾�α�)�� ����
			//	clickedBtnItemIdx = item;
			//	result = DGBlankModalDialog (200, 200, DG_DLG_NOGROW, 0, DG_DLG_NORMALFRAME, beamPlacerHandler3, 0);
			//	item = 0;
			//}
			//// ��ġ ��ư (���� �߾� �κ�)
			//if (item == START_INDEX_CENTER_AT_SIDE) {
			//	// [DIALOG] �׸��� ��ư�� ������ Cell�� �����ϱ� ���� ���� â(3��° ���̾�α�)�� ����
			//	clickedBtnItemIdx = item;
			//	result = DGBlankModalDialog (200, 200, DG_DLG_NOGROW, 0, DG_DLG_NORMALFRAME, beamPlacerHandler3, 0);
			//	item = 0;
			//}
			//// ��ġ ��ư (���� �� �κ�)
			//if ((item >= END_INDEX_FROM_END_AT_SIDE) && (item < END_INDEX_FROM_END_AT_SIDE + placingZone.nCellsFromEndAtSide)) {
			//	// [DIALOG] �׸��� ��ư�� ������ Cell�� �����ϱ� ���� ���� â(3��° ���̾�α�)�� ����
			//	clickedBtnItemIdx = item;
			//	result = DGBlankModalDialog (200, 200, DG_DLG_NOGROW, 0, DG_DLG_NORMALFRAME, beamPlacerHandler3, 0);
			//	item = 0;
			//}

			//// ��ġ ��ư (�Ϻ� ���� �κ�)
			//if ((item >= START_INDEX_FROM_BEGIN_AT_BOTTOM) && (item < START_INDEX_FROM_BEGIN_AT_BOTTOM + placingZone.nCellsFromBeginAtBottom)) {
			//	// [DIALOG] �׸��� ��ư�� ������ Cell�� �����ϱ� ���� ���� â(3��° ���̾�α�)�� ����
			//	clickedBtnItemIdx = item;
			//	result = DGBlankModalDialog (200, 200, DG_DLG_NOGROW, 0, DG_DLG_NORMALFRAME, beamPlacerHandler3, 0);
			//	item = 0;
			//}
			//// ��ġ ��ư (�Ϻ� �߾� �κ�)
			//if (item == START_INDEX_CENTER_AT_BOTTOM) {
			//	// [DIALOG] �׸��� ��ư�� ������ Cell�� �����ϱ� ���� ���� â(3��° ���̾�α�)�� ����
			//	clickedBtnItemIdx = item;
			//	result = DGBlankModalDialog (200, 200, DG_DLG_NOGROW, 0, DG_DLG_NORMALFRAME, beamPlacerHandler3, 0);
			//	item = 0;
			//}
			//// ��ġ ��ư (�Ϻ� �� �κ�)
			//if ((item >= END_INDEX_FROM_END_AT_BOTTOM) && (item < END_INDEX_FROM_END_AT_BOTTOM + placingZone.nCellsFromEndAtBottom)) {
			//	// [DIALOG] �׸��� ��ư�� ������ Cell�� �����ϱ� ���� ���� â(3��° ���̾�α�)�� ����
			//	clickedBtnItemIdx = item;
			//	result = DGBlankModalDialog (200, 200, DG_DLG_NOGROW, 0, DG_DLG_NORMALFRAME, beamPlacerHandler3, 0);
			//	item = 0;
			//}
		case DG_MSG_CLOSE:
			switch (item) {
				case DG_CLOSEBOX:
					break;
			}
	}

	result = item;

	return	result;
}
