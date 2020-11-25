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


// 5�� �޴�: ��տ� �������� ��ġ�ϴ� ���� ��ƾ
GSErrCode	placeEuroformOnColumn (void)
{
	GSErrCode		err = NoError;
	long			nSel;
	short			xx, yy;
	double			dx, dy, ang;
	short			result;

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
	API_Elem_Head* headList = new API_Elem_Head [1];
	headList [0] = elem.header;
	err = ACAPI_Element_Delete (&headList, 1);
	delete headList;

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

		API_Coord	rotatedPoint;
		double		lineLen;

		lineLen = sqrt (pow (infoColumn.coreWidth/2, 2) + pow (infoColumn.coreDepth/2, 2));
		rotatedPoint.x = infoColumn.origoPos.x + lineLen * cos(atan2 (infoColumn.coreDepth/2, infoColumn.coreWidth/2) + infoColumn.angle);
		rotatedPoint.y = infoColumn.origoPos.y + lineLen * sin(atan2 (infoColumn.coreDepth/2, infoColumn.coreWidth/2) + infoColumn.angle);
		placeCoordinateLabel (rotatedPoint.x, rotatedPoint.y, infoColumn.bottomOffset, true, "RT", 1, infoColumn.floorInd);

		lineLen = sqrt (pow (infoColumn.coreWidth/2, 2) + pow (infoColumn.coreDepth/2, 2));
		rotatedPoint.x = infoColumn.origoPos.x + lineLen * cos(atan2 (infoColumn.coreDepth/2, -infoColumn.coreWidth/2) + infoColumn.angle);
		rotatedPoint.y = infoColumn.origoPos.y + lineLen * sin(atan2 (infoColumn.coreDepth/2, -infoColumn.coreWidth/2) + infoColumn.angle);
		placeCoordinateLabel (rotatedPoint.x, rotatedPoint.y, infoColumn.bottomOffset, true, "LT", 1, infoColumn.floorInd);

		lineLen = sqrt (pow (infoColumn.coreWidth/2, 2) + pow (infoColumn.coreDepth/2, 2));
		rotatedPoint.x = infoColumn.origoPos.x + lineLen * cos(atan2 (-infoColumn.coreDepth/2, -infoColumn.coreWidth/2) + infoColumn.angle);
		rotatedPoint.y = infoColumn.origoPos.y + lineLen * sin(atan2 (-infoColumn.coreDepth/2, -infoColumn.coreWidth/2) + infoColumn.angle);
		placeCoordinateLabel (rotatedPoint.x, rotatedPoint.y, infoColumn.bottomOffset, true, "LB", 1, infoColumn.floorInd);

		lineLen = sqrt (pow (infoColumn.coreWidth/2, 2) + pow (infoColumn.coreDepth/2, 2));
		rotatedPoint.x = infoColumn.origoPos.x + lineLen * cos(atan2 (-infoColumn.coreDepth/2, infoColumn.coreWidth/2) + infoColumn.angle);
		rotatedPoint.y = infoColumn.origoPos.y + lineLen * sin(atan2 (-infoColumn.coreDepth/2, infoColumn.coreWidth/2) + infoColumn.angle);
		placeCoordinateLabel (rotatedPoint.x, rotatedPoint.y, infoColumn.bottomOffset, true, "RB", 1, infoColumn.floorInd);

FIRST_SOLE_COLUMN:

		// ���� ���� �� ���� �� ���� ���� ������Ʈ
		// !!!
		//- ���� ��հ� ������� ���踦 �ľ�
		//	- ��� ���� ���� ã�´�.
		//	- �� �� ���� ���� ���� ���� ã�ƾ� ��

		// [DIALOG] 1��° ���̾�α׿��� ������ ���� �Է� ����
		result = DGModalDialog (ACAPI_GetOwnResModule (), 32531, ACAPI_GetOwnResModule (), columnPlacerHandler1, 0);

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
		placingZone->cellsT1 [xx].anchor = LEFT_TOP;

		placingZone->cellsT2 [xx].objType = NONE;
		placingZone->cellsT2 [xx].leftBottomX = 0.0;
		placingZone->cellsT2 [xx].leftBottomY = 0.0;
		placingZone->cellsT2 [xx].leftBottomZ = 0.0;
		placingZone->cellsT2 [xx].ang = 0.0;
		placingZone->cellsT2 [xx].horLen = 0.0;
		placingZone->cellsT2 [xx].verLen = 0.0;
		placingZone->cellsT2 [xx].height = 0.0;
		placingZone->cellsT2 [xx].anchor = RIGHT_TOP;

		placingZone->cellsL1 [xx].objType = NONE;
		placingZone->cellsL1 [xx].leftBottomX = 0.0;
		placingZone->cellsL1 [xx].leftBottomY = 0.0;
		placingZone->cellsL1 [xx].leftBottomZ = 0.0;
		placingZone->cellsL1 [xx].ang = 0.0;
		placingZone->cellsL1 [xx].horLen = 0.0;
		placingZone->cellsL1 [xx].verLen = 0.0;
		placingZone->cellsL1 [xx].height = 0.0;
		placingZone->cellsL1 [xx].anchor = LEFT_TOP;

		placingZone->cellsL2 [xx].objType = NONE;
		placingZone->cellsL2 [xx].leftBottomX = 0.0;
		placingZone->cellsL2 [xx].leftBottomY = 0.0;
		placingZone->cellsL2 [xx].leftBottomZ = 0.0;
		placingZone->cellsL2 [xx].ang = 0.0;
		placingZone->cellsL2 [xx].horLen = 0.0;
		placingZone->cellsL2 [xx].verLen = 0.0;
		placingZone->cellsL2 [xx].height = 0.0;
		placingZone->cellsL2 [xx].anchor = LEFT_BOTTOM;

		placingZone->cellsR1 [xx].objType = NONE;
		placingZone->cellsR1 [xx].leftBottomX = 0.0;
		placingZone->cellsR1 [xx].leftBottomY = 0.0;
		placingZone->cellsR1 [xx].leftBottomZ = 0.0;
		placingZone->cellsR1 [xx].ang = 0.0;
		placingZone->cellsR1 [xx].horLen = 0.0;
		placingZone->cellsR1 [xx].verLen = 0.0;
		placingZone->cellsR1 [xx].height = 0.0;
		placingZone->cellsR1 [xx].anchor = RIGHT_TOP;

		placingZone->cellsR2 [xx].objType = NONE;
		placingZone->cellsR2 [xx].leftBottomX = 0.0;
		placingZone->cellsR2 [xx].leftBottomY = 0.0;
		placingZone->cellsR2 [xx].leftBottomZ = 0.0;
		placingZone->cellsR2 [xx].ang = 0.0;
		placingZone->cellsR2 [xx].horLen = 0.0;
		placingZone->cellsR2 [xx].verLen = 0.0;
		placingZone->cellsR2 [xx].height = 0.0;
		placingZone->cellsR2 [xx].anchor = RIGHT_BOTTOM;

		placingZone->cellsB1 [xx].objType = NONE;
		placingZone->cellsB1 [xx].leftBottomX = 0.0;
		placingZone->cellsB1 [xx].leftBottomY = 0.0;
		placingZone->cellsB1 [xx].leftBottomZ = 0.0;
		placingZone->cellsB1 [xx].ang = 0.0;
		placingZone->cellsB1 [xx].horLen = 0.0;
		placingZone->cellsB1 [xx].verLen = 0.0;
		placingZone->cellsB1 [xx].height = 0.0;
		placingZone->cellsB1 [xx].anchor = LEFT_BOTTOM;

		placingZone->cellsB2 [xx].objType = NONE;
		placingZone->cellsB2 [xx].leftBottomX = 0.0;
		placingZone->cellsB2 [xx].leftBottomY = 0.0;
		placingZone->cellsB2 [xx].leftBottomZ = 0.0;
		placingZone->cellsB2 [xx].ang = 0.0;
		placingZone->cellsB2 [xx].horLen = 0.0;
		placingZone->cellsB2 [xx].verLen = 0.0;
		placingZone->cellsB2 [xx].height = 0.0;
		placingZone->cellsB2 [xx].anchor = RIGHT_BOTTOM;
	}

	// �� ���� �ʱ�ȭ
	placingZone->nCells = 0;
}

// 1�� ��ġ�� ���� ���Ǹ� ��û�ϴ� 1�� ���̾�α�
short DGCALLBACK columnPlacerHandler1 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short		result;
	short		xx;
	//double		h1, h2, h3, h4, hRest;	// ������ ���� ����� ���� ����
	API_UCCallbackType	ucb;

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
			//DGSetItemText (dialogID, LABEL_BEAM_SECTION, "�� �ܸ�");
			//DGSetItemText (dialogID, LABEL_BEAM_HEIGHT, "�� ����");
			//DGSetItemText (dialogID, LABEL_BEAM_WIDTH, "�� �ʺ�");
			//DGSetItemText (dialogID, LABEL_TOTAL_HEIGHT, "�� ����");
			//DGSetItemText (dialogID, LABEL_TOTAL_WIDTH, "�� �ʺ�");

			//DGSetItemText (dialogID, LABEL_REST_SIDE, "������");
			//DGSetItemText (dialogID, CHECKBOX_WOOD_SIDE, "����");
			//DGSetItemText (dialogID, CHECKBOX_T_FORM_SIDE, "������");
			//DGSetItemText (dialogID, CHECKBOX_FILLER_SIDE, "�ٷ�");
			//DGSetItemText (dialogID, CHECKBOX_B_FORM_SIDE, "������");

			//DGSetItemText (dialogID, CHECKBOX_L_FORM_BOTTOM, "������");
			//DGSetItemText (dialogID, CHECKBOX_FILLER_BOTTOM, "�ٷ�");
			//DGSetItemText (dialogID, CHECKBOX_R_FORM_BOTTOM, "������");

			// ��: ���̾� ����
			//DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, "���纰 ���̾� ����");
			//DGSetItemText (dialogID, LABEL_LAYER_EUROFORM, "������");
			//DGSetItemText (dialogID, LABEL_LAYER_FILLERSPACER, "�ٷ������̼�");
			//DGSetItemText (dialogID, LABEL_LAYER_PLYWOOD, "����");
			//DGSetItemText (dialogID, LABEL_LAYER_WOOD, "����");
			//DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER_ANGLE, "�ƿ��ڳʾޱ�");

			// ���� ��Ʈ�� �ʱ�ȭ
			//BNZeroMemory (&ucb, sizeof (ucb));
			//ucb.dialogID = dialogID;
			//ucb.type	 = APIUserControlType_Layer;
			//ucb.itemID	 = USERCONTROL_LAYER_EUROFORM;
			//ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			//DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, 1);

			//ucb.itemID	 = USERCONTROL_LAYER_FILLERSPACER;
			//ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			//DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSPACER, 1);

			//ucb.itemID	 = USERCONTROL_LAYER_PLYWOOD;
			//ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			//DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, 1);

			//ucb.itemID	 = USERCONTROL_LAYER_WOOD;
			//ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			//DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, 1);

			//ucb.itemID	 = USERCONTROL_LAYER_OUTCORNER_ANGLE;
			//ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			//DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, 1);

			// �� ����/�ʺ� ���
			//DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_HEIGHT, placingZone.areaHeight);
			//DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_WIDTH, infoBeam.width);

			// �� ����/�ʺ� ���
			//DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_HEIGHT, placingZone.areaHeight + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM));
			//DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_WIDTH, infoBeam.width + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_SIDE1)*2);

			// ���纰 üũ�ڽ�-�԰� ����
			//(DGGetItemValLong (dialogID, CHECKBOX_WOOD_SIDE) == TRUE) ?		DGEnableItem (dialogID, EDITCONTROL_WOOD_SIDE)		:	DGDisableItem (dialogID, EDITCONTROL_WOOD_SIDE);
			//(DGGetItemValLong (dialogID, CHECKBOX_T_FORM_SIDE) == TRUE) ?	DGEnableItem (dialogID, POPUP_T_FORM_SIDE)			:	DGDisableItem (dialogID, POPUP_T_FORM_SIDE);
			//(DGGetItemValLong (dialogID, CHECKBOX_FILLER_SIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_FILLER_SIDE)	:	DGDisableItem (dialogID, EDITCONTROL_FILLER_SIDE);
			//(DGGetItemValLong (dialogID, CHECKBOX_FILLER_BOTTOM) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_FILLER_BOTTOM)	:	DGDisableItem (dialogID, EDITCONTROL_FILLER_BOTTOM);
			//(DGGetItemValLong (dialogID, CHECKBOX_R_FORM_BOTTOM) == TRUE) ?	DGEnableItem (dialogID, POPUP_R_FORM_BOTTOM)		:	DGDisableItem (dialogID, POPUP_R_FORM_BOTTOM);

			// ���� 0��, �Ϻ� 0�� ���� ������ ����ؾ� ��
			//DGSetItemValLong (dialogID, CHECKBOX_B_FORM_SIDE, TRUE);
			//DGSetItemValLong (dialogID, CHECKBOX_L_FORM_BOTTOM, TRUE);
			//DGDisableItem (dialogID, CHECKBOX_B_FORM_SIDE);
			//DGDisableItem (dialogID, CHECKBOX_L_FORM_BOTTOM);

			// ������ �� ���
			// ...

			// ���� �����ؼ��� �� �Ǵ� �׸� ��ױ�
			//DGDisableItem (dialogID, EDITCONTROL_GAP_SIDE2);
			//DGDisableItem (dialogID, EDITCONTROL_BEAM_HEIGHT);
			//DGDisableItem (dialogID, EDITCONTROL_BEAM_WIDTH);
			//DGDisableItem (dialogID, EDITCONTROL_TOTAL_HEIGHT);
			//DGDisableItem (dialogID, EDITCONTROL_TOTAL_WIDTH);
			//DGDisableItem (dialogID, EDITCONTROL_REST_SIDE);

			break;
		
		case DG_MSG_CHANGE:
			// ������ �� ���
			// ...

			switch (item) {
				// �� ����/�ʺ� ���
				// ...

				// ���纰 üũ�ڽ�-�԰� ����
				//case CHECKBOX_WOOD_SIDE:
				//	(DGGetItemValLong (dialogID, CHECKBOX_WOOD_SIDE) == TRUE) ?		DGEnableItem (dialogID, EDITCONTROL_WOOD_SIDE)		:	DGDisableItem (dialogID, EDITCONTROL_WOOD_SIDE);
				//	break;
				//case CHECKBOX_T_FORM_SIDE:
				//	(DGGetItemValLong (dialogID, CHECKBOX_T_FORM_SIDE) == TRUE) ?	DGEnableItem (dialogID, POPUP_T_FORM_SIDE)			:	DGDisableItem (dialogID, POPUP_T_FORM_SIDE);
				//	break;
				//case CHECKBOX_FILLER_SIDE:
				//	(DGGetItemValLong (dialogID, CHECKBOX_FILLER_SIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_FILLER_SIDE)	:	DGDisableItem (dialogID, EDITCONTROL_FILLER_SIDE);
				//	break;
				//case CHECKBOX_B_FORM_SIDE:
				//	(DGGetItemValLong (dialogID, CHECKBOX_B_FORM_SIDE) == TRUE) ?	DGEnableItem (dialogID, POPUP_B_FORM_SIDE)			:	DGDisableItem (dialogID, POPUP_B_FORM_SIDE);
				//	break;
			}

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					//////////////////////////////////////// ���̾�α� â ������ �Է� ����
					// �� ���� ����
					// ...

					// ���̾� ��ȣ ����
					//layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM);
					//layerInd_Fillerspacer	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSPACER);
					//layerInd_Plywood		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD);
					//layerInd_Wood			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD);
					//layerInd_OutcornerAngle	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE);

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