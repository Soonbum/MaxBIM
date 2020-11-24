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
	infoColumn.angle		= elem.column.angle;			// ��� ���� �߽����� �� ȸ�� ���� (����: Radian)
	infoColumn.origoPos		= elem.column.origoPos;			// ��� �߽� ��ġ

	ACAPI_DisposeElemMemoHdls (&memo);

	// �� ���� ����
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

	// !!!
	// �ܵ� ����� ��� -> �̰��� ���� ������ ��

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

	// ���� ������ �� (�� 1�������� �ν��ϸ� �ǳ�?)
	// ���� ���� �־�� ��
	// ���� -> zMax - zMin : ���� ���̿� �ش���
	// ����� ������ 4���� ã��, �� 4���� ã�´�.



	/*
	- ���� ��հ� ������� ���踦 �ľ�
		- ��� ���� ���� ã�´�.
		- �� �� ���� ���� ���� ���� ã�ƾ� ��
	- 1�� DG (�� �ܸ�)
		- ��� �ڳ�: �ƿ��ڳ��ǳ� (��� �ڳʰ� �� �ӿ� ���� ����)
		- ���� �´�� �κ�: ���ڳ��ǳ� (���� ����� ��쿡��)
		- ������: ��� ���, �׸��� ���� (��� ������ ���ӿ� �Ÿ��� ���)
	- 2�� DG (�� ����)
		- ���� ���� �� �ϴ� ���ΰ� ������ �ֻ�� ���� ���� ������ ǥ���ؾ� �� (80mm �̻��̾�� ��)
		- UI���� ��ȿ���� ���� ������ ������ �ؽ�Ʈ�� ���!
		- ��� ������ ������ ������ �������� �����
	*/

	return	err;
}