#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "LowSideTableformPlacer.hpp"

using namespace lowSideTableformPlacerDG;

static LowSideTableformPlacingZone	placingZone;		// ���� ������ ���� ���� ����

static InfoWall						infoWall;			// �� ��ü ����
static InfoBeam						infoBeam;			// �� ��ü ����
static InfoSlab						infoSlab;			// ������ ��ü ����
static InfoMesh						infoMesh;			// �޽� ��ü ����
short floorInd;

API_Guid		structuralObject_forTableformLowSide;	// ���� ��ü�� GUID

static short	layerInd_Euroform;			// ���̾� ��ȣ: ������ (����)
static short	layerInd_RectPipe;			// ���̾� ��ȣ: ��� ������ (����)
static short	layerInd_PinBolt;			// ���̾� ��ȣ: �ɺ�Ʈ ��Ʈ
static short	layerInd_WallTie;			// ���̾� ��ȣ: ��ü Ÿ�� (�� �̻� ������� ����)
static short	layerInd_Clamp;				// ���̾� ��ȣ: ���� Ŭ���� (�� �̻� ������� ����)
static short	layerInd_HeadPiece;			// ���̾� ��ȣ: ����ǽ�
static short	layerInd_Join;				// ���̾� ��ȣ: ����ö��
static short	layerInd_Plywood;			// ���̾� ��ȣ: ���� (����)
static short	layerInd_Timber;			// ���̾� ��ȣ: ���� (����)
static short	layerInd_EuroformHook;		// ���̾� ��ȣ: ������ ��ũ
static short	layerInd_CrossJointBar;		// ���̾� ��ȣ: ���� ����Ʈ ��
static short	layerInd_BlueClamp;			// ���̾� ��ȣ: ��� Ŭ����
static short	layerInd_BlueTimberRail;	// ���̾� ��ȣ: ��� ���
static short	layerInd_Hidden;			// ���̾� ��ȣ: ���� (�� �̻� ������� ����)

static short	layerInd_SlabTableform;		// ���̾� ��ȣ: ������ ���̺���
static short	layerInd_Profile;			// ���̾� ��ȣ: KS��������
static short	layerInd_Steelform;			// ���̾� ��ȣ: ��ƿ��
static short	layerInd_Fillersp;			// ���̾� ��ȣ: �ٷ������̼�
static short	layerInd_OutcornerAngle;	// ���̾� ��ȣ: �ƿ��ڳʾޱ�
static short	layerInd_OutcornerPanel;	// ���̾� ��ȣ: �ƿ��ڳ��ǳ�
static short	layerInd_IncornerPanel;		// ���̾� ��ȣ: ���ڳ��ǳ�
static short	layerInd_RectpipeHanger;	// ���̾� ��ȣ: �������� ���

static bool		bLayerInd_Euroform;			// ���̾� ��ȣ: ������
static bool		bLayerInd_RectPipe;			// ���̾� ��ȣ: ��� ������
static bool		bLayerInd_PinBolt;			// ���̾� ��ȣ: �ɺ�Ʈ ��Ʈ
static bool		bLayerInd_WallTie;			// ���̾� ��ȣ: ��ü Ÿ��
static bool		bLayerInd_HeadPiece;		// ���̾� ��ȣ: ����ǽ�
static bool		bLayerInd_Join;				// ���̾� ��ȣ: ����ö��
static bool		bLayerInd_Plywood;			// ���̾� ��ȣ: ����
static bool		bLayerInd_Timber;			// ���̾� ��ȣ: ����
static bool		bLayerInd_EuroformHook;		// ���̾� ��ȣ: ������ ��ũ
static bool		bLayerInd_CrossJointBar;	// ���̾� ��ȣ: ���� ����Ʈ ��
static bool		bLayerInd_BlueClamp;		// ���̾� ��ȣ: ��� Ŭ����
static bool		bLayerInd_BlueTimberRail;	// ���̾� ��ȣ: ��� ���
static bool		bLayerInd_Hidden;			// ���̾� ��ȣ: ����

static bool		bLayerInd_SlabTableform;	// ���̾� ��ȣ: ������ ���̺���
static bool		bLayerInd_Profile;			// ���̾� ��ȣ: KS��������
static bool		bLayerInd_Steelform;		// ���̾� ��ȣ: ��ƿ��
static bool		bLayerInd_Fillersp;			// ���̾� ��ȣ: �ٷ������̼�
static bool		bLayerInd_OutcornerAngle;	// ���̾� ��ȣ: �ƿ��ڳʾޱ�
static bool		bLayerInd_OutcornerPanel;	// ���̾� ��ȣ: �ƿ��ڳ��ǳ�
static bool		bLayerInd_IncornerPanel;	// ���̾� ��ȣ: ���ڳ��ǳ�
static bool		bLayerInd_RectpipeHanger;	// ���̾� ��ȣ: �������� ���

static GS::Array<API_Guid>	elemList;	// �׷�ȭ�� ���� ������ ��������� GUID�� ���� ������

static int	clickedIndex;	// Ŭ���� ��ư�� �ε���


// ���� ������ ���鿡 ���̺����� ��ġ�ϴ� ���� ��ƾ
GSErrCode	placeTableformOnLowSide (void)
{
	GSErrCode	err = NoError;
	short		result;
	long		nSel;
	short		xx;
	double		dx, dy;

	// Selection Manager ���� ����
	API_SelectionInfo		selectionInfo;
	API_Element				tElem;
	API_Neig				**selNeigs;
	
	GS::Array<API_Guid>		wall;
	GS::Array<API_Guid>		beam;
	GS::Array<API_Guid>		slab;
	GS::Array<API_Guid>		mesh;
	GS::Array<API_Guid>		morph;

	long	nWall;
	long	nBeam;
	long	nSlab;
	long	nMesh;
	long	nMorph;

	// ��ü ���� ��������
	API_Element				elem;
	API_ElemInfo3D			info3D;

	// ���� ��ü ����
	InfoMorphForLowSideTableform	infoMorph;

	// �۾� �� ����
	API_StoryInfo	storyInfo;
	double			workLevel_structural;	// ���� ����� �۾� �� ����

	// ������ ��� ��������
	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err == APIERR_NOPLAN) {
		WriteReport_Alert ("���� ������Ʈ â�� �����ϴ�.");
	}
	if (err == APIERR_NOSEL) {
		WriteReport_Alert ("�ƹ� �͵� �������� �ʾҽ��ϴ�.\n�ʼ� ����: ���� ��� (��, ��, ������, �޽� �� 1����), ���� ��� ������ ���� ���� (1��)");
	}
	if (err != NoError) {
		BMKillHandle ((GSHandle *) &selNeigs);
		return err;
	}

	// ���� ��� 1��, ���� ���� 1�� �����ؾ� ��
	if (selectionInfo.typeID != API_SelEmpty) {
		nSel = BMGetHandleSize ((GSHandle) selNeigs) / sizeof (API_Neig);
		for (xx = 0 ; xx < nSel && err == NoError ; ++xx) {
			tElem.header.typeID = Neig_To_ElemID ((*selNeigs)[xx].neigID);

			tElem.header.guid = (*selNeigs)[xx].guid;
			if (ACAPI_Element_Get (&tElem) != NoError)	// ������ �� �ִ� ����ΰ�?
				continue;

			if (tElem.header.typeID == API_WallID)		wall.Push (tElem.header.guid);		// ���ΰ�?
			if (tElem.header.typeID == API_BeamID)		beam.Push (tElem.header.guid);		// ���ΰ�?
			if (tElem.header.typeID == API_SlabID)		slab.Push (tElem.header.guid);		// �������ΰ�?
			if (tElem.header.typeID == API_MeshID)		mesh.Push (tElem.header.guid);		// �޽��ΰ�?
			
			if (tElem.header.typeID == API_MorphID)		morph.Push (tElem.header.guid);		// �����ΰ�?
		}
	}
	BMKillHandle ((GSHandle *) &selNeigs);

	nWall = wall.GetSize ();
	nBeam = beam.GetSize ();
	nSlab = slab.GetSize ();
	nMesh = mesh.GetSize ();

	nMorph = morph.GetSize ();

	// ���� ��Ұ� 1���ΰ�?
	if (nWall + nBeam + nSlab + nMesh != 1) {
		WriteReport_Alert ("���� ���(��, ��, ������, �޽�)�� 1�� �����ؾ� �մϴ�.");
		err = APIERR_GENERAL;
		return err;
	}

	// ������ 1���ΰ�?
	if (nMorph != 1) {
		WriteReport_Alert ("���� ����� ������ ���� ������ 1�� �����ؾ� �մϴ�.");
		err = APIERR_GENERAL;
		return err;
	}

	// �� ������ ������
	if (nWall == 1) {
		BNZeroMemory (&elem, sizeof (API_Element));
		elem.header.guid = wall.Pop ();
		structuralObject_forTableformLowSide = elem.header.guid;
		err = ACAPI_Element_Get (&elem);

		infoWall.guid			= elem.header.guid;
		infoWall.floorInd		= elem.header.floorInd;
		infoWall.wallThk		= elem.wall.thickness;
		infoWall.bottomOffset	= elem.wall.bottomOffset;
		infoWall.begX			= elem.wall.begC.x;
		infoWall.begY			= elem.wall.begC.y;
		infoWall.endX			= elem.wall.endC.x;
		infoWall.endY			= elem.wall.endC.y;
	}

	// �� ������ ������
	else if (nBeam == 1) {
		BNZeroMemory (&elem, sizeof (API_Element));
		elem.header.guid = beam.Pop ();
		structuralObject_forTableformLowSide = elem.header.guid;
		err = ACAPI_Element_Get (&elem);

		infoBeam.guid			= elem.header.guid;
		infoBeam.floorInd		= elem.header.floorInd;
		infoBeam.begC			= elem.beam.begC;
		infoBeam.endC			= elem.beam.endC;
		infoBeam.height			= elem.beam.height;
		infoBeam.level			= elem.beam.level;
		infoBeam.offset			= elem.beam.offset;
		infoBeam.width			= elem.beam.width;
	}

	// ������ ������ ������
	else if (nSlab == 1) {
		BNZeroMemory (&elem, sizeof (API_Element));
		elem.header.guid = slab.Pop ();
		structuralObject_forTableformLowSide = elem.header.guid;
		err = ACAPI_Element_Get (&elem);

		infoSlab.guid			= elem.header.guid;
		infoSlab.floorInd		= elem.header.floorInd;
		infoSlab.level			= elem.slab.level;
		infoSlab.offsetFromTop	= elem.slab.offsetFromTop;
		infoSlab.thickness		= elem.slab.thickness;
	}

	// �޽� ������ ������
	else if (nMesh == 1) {
		BNZeroMemory (&elem, sizeof (API_Element));
		elem.header.guid = mesh.Pop ();
		structuralObject_forTableformLowSide = elem.header.guid;
		err = ACAPI_Element_Get (&elem);

		infoMesh.guid			= elem.header.guid;
		infoMesh.floorInd		= elem.header.floorInd;
		infoMesh.level			= elem.mesh.level;
		infoMesh.skirtLevel		= elem.mesh.skirtLevel;
	}

	// ���� ������ ������
	BNZeroMemory (&elem, sizeof (API_Element));
	elem.header.guid = morph.Pop ();
	err = ACAPI_Element_Get (&elem);
	err = ACAPI_Element_Get3DInfo (elem.header, &info3D);

	if (abs (info3D.bounds.zMax - info3D.bounds.zMin) < EPS) {
		WriteReport_Alert ("������ ������ ���� �ʽ��ϴ�.");
		err = APIERR_GENERAL;
		return err;
	}

	infoMorph.guid = elem.header.guid;

	// ������ ���ϴ�, ���� �� ����
	if (abs (elem.morph.tranmat.tmx [11] - info3D.bounds.zMin) < EPS) {
		// ���ϴ� ��ǥ ����
		infoMorph.leftBottomX = elem.morph.tranmat.tmx [3];
		infoMorph.leftBottomY = elem.morph.tranmat.tmx [7];
		infoMorph.leftBottomZ = elem.morph.tranmat.tmx [11];

		// ���� ��ǥ��?
		if (abs (infoMorph.leftBottomX - info3D.bounds.xMin) < EPS)
			infoMorph.rightTopX = info3D.bounds.xMax;
		else
			infoMorph.rightTopX = info3D.bounds.xMin;
		if (abs (infoMorph.leftBottomY - info3D.bounds.yMin) < EPS)
			infoMorph.rightTopY = info3D.bounds.yMax;
		else
			infoMorph.rightTopY = info3D.bounds.yMin;
		if (abs (infoMorph.leftBottomZ - info3D.bounds.zMin) < EPS)
			infoMorph.rightTopZ = info3D.bounds.zMax;
		else
			infoMorph.rightTopZ = info3D.bounds.zMin;
	} else {
		// ���� ��ǥ ����
		infoMorph.rightTopX = elem.morph.tranmat.tmx [3];
		infoMorph.rightTopY = elem.morph.tranmat.tmx [7];
		infoMorph.rightTopZ = elem.morph.tranmat.tmx [11];

		// ���ϴ� ��ǥ��?
		if (abs (infoMorph.rightTopX - info3D.bounds.xMin) < EPS)
			infoMorph.leftBottomX = info3D.bounds.xMax;
		else
			infoMorph.leftBottomX = info3D.bounds.xMin;
		if (abs (infoMorph.rightTopY - info3D.bounds.yMin) < EPS)
			infoMorph.leftBottomY = info3D.bounds.yMax;
		else
			infoMorph.leftBottomY = info3D.bounds.yMin;
		if (abs (infoMorph.rightTopZ - info3D.bounds.zMin) < EPS)
			infoMorph.leftBottomZ = info3D.bounds.zMax;
		else
			infoMorph.leftBottomZ = info3D.bounds.zMin;
	}

	// ������ Z�� ȸ�� ���� (���� ��ġ ����)
	dx = infoMorph.rightTopX - infoMorph.leftBottomX;
	dy = infoMorph.rightTopY - infoMorph.leftBottomY;
	infoMorph.ang = RadToDegree (atan2 (dy, dx));

	// ������ ���� ����
	infoMorph.horLen = GetDistance (info3D.bounds.xMin, info3D.bounds.yMin, info3D.bounds.xMax, info3D.bounds.yMax);
	
	// ������ ���� ����
	infoMorph.verLen = abs (info3D.bounds.zMax - info3D.bounds.zMin);

	// ���� ���� ����
	API_Elem_Head* headList = new API_Elem_Head [1];
	headList [0] = elem.header;
	err = ACAPI_Element_Delete (&headList, 1);
	delete headList;

	// �۾� �� ���� �ݿ� -- ����
	BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	workLevel_structural = 0.0;
	ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
	for (xx = 0 ; xx <= (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
		if (nWall == 1)			floorInd = infoWall.floorInd;
		else if (nBeam == 1)	floorInd = infoBeam.floorInd;
		else if (nSlab == 1)	floorInd = infoSlab.floorInd;
		else if (nMesh == 1)	floorInd = infoMesh.floorInd;

		if (storyInfo.data [0][xx].index == floorInd) {
			workLevel_structural = storyInfo.data [0][xx].level;
			break;
		}
	}
	BMKillHandle ((GSHandle *) &storyInfo.data);

	// ���� ������ ������Ʈ
	placingZone.leftBottomX = infoMorph.leftBottomX;
	placingZone.leftBottomY = infoMorph.leftBottomY;
	placingZone.leftBottomZ = infoMorph.leftBottomZ;
	placingZone.horLen = infoMorph.horLen;
	placingZone.verLen = infoMorph.verLen;
	placingZone.ang = infoMorph.ang;

	if (nWall == 1)			placingZone.leftBottomZ = infoWall.bottomOffset;
	else if (nBeam == 1)	placingZone.leftBottomZ = infoBeam.level - infoBeam.height;
	else if (nSlab == 1)	placingZone.leftBottomZ = infoSlab.level + infoSlab.offsetFromTop - infoSlab.thickness;
	else if (nMesh == 1)	placingZone.leftBottomZ = infoMesh.level - infoMesh.skirtLevel;

	// ���̺��� ���� ���� (600mm �̸��̸� ����, �̻��̸� ����)
	if (placingZone.verLen < 0.600 - EPS) {
		placingZone.bVertical = false;
	} else {
		placingZone.bVertical = true;
	}

	// ���� ���� �� ����
	placingZone.nCellsInHor = (short)(floor (placingZone.horLen / 3.600));

FIRST:

	// [DIALOG] 1��° ���̾�α׿��� ���ڳ��ǳ�/�ƿ��ڳ��ǳ�/�ƿ��ڳʾޱ� ���� �� ����, ���̺����� ����� ����/���� ���� �������� ������ ������ ���̸� ������
	result = DGBlankModalDialog (600, 350, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, lowSideTableformPlacerHandler1, 0);

	if (result != DG_OK)
		return err;

	// [DIALOG] 2��° ���̾�α׿��� ���纰 ���̾ ������
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32503, ACAPI_GetOwnResModule (), lowSideTableformPlacerHandler2, 0);

	if (result != DG_OK)
		goto FIRST;

	// ��ü ��ġ�ϱ�
	placingZone.placeObjects (&placingZone);

	// ȭ�� ���ΰ�ħ
	ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	bool	regenerate = true;
	ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

	return	err;
}

// �⺻ ������
LowSideTableformPlacingZone::LowSideTableformPlacingZone ()
{
	this->presetWidthVertical_tableform [0]		= 3600;
	this->presetWidthVertical_tableform [1]		= 3500;
	this->presetWidthVertical_tableform [2]		= 3450;
	this->presetWidthVertical_tableform [3]		= 3400;
	this->presetWidthVertical_tableform [4]		= 3350;
	this->presetWidthVertical_tableform [5]		= 3300;
	this->presetWidthVertical_tableform [6]		= 3250;
	this->presetWidthVertical_tableform [7]		= 3200;
	this->presetWidthVertical_tableform [8]		= 3150;
	this->presetWidthVertical_tableform [9]		= 3100;
	this->presetWidthVertical_tableform [10]	= 3050;
	this->presetWidthVertical_tableform [11]	= 3000;
	this->presetWidthVertical_tableform [12]	= 2950;
	this->presetWidthVertical_tableform [13]	= 2900;
	this->presetWidthVertical_tableform [14]	= 2850;
	this->presetWidthVertical_tableform [15]	= 2800;
	this->presetWidthVertical_tableform [16]	= 2750;
	this->presetWidthVertical_tableform [17]	= 2700;
	this->presetWidthVertical_tableform [18]	= 2650;
	this->presetWidthVertical_tableform [19]	= 2600;
	this->presetWidthVertical_tableform [20]	= 2550;
	this->presetWidthVertical_tableform [21]	= 2500;
	this->presetWidthVertical_tableform [22]	= 2450;
	this->presetWidthVertical_tableform [23]	= 2400;
	this->presetWidthVertical_tableform [24]	= 2350;
	this->presetWidthVertical_tableform [25]	= 2300;
	this->presetWidthVertical_tableform [26]	= 2250;
	this->presetWidthVertical_tableform [27]	= 2200;
	this->presetWidthVertical_tableform [28]	= 2150;
	this->presetWidthVertical_tableform [29]	= 2100;
	this->presetWidthVertical_tableform [30]	= 2050;
	this->presetWidthVertical_tableform [31]	= 2000;
	this->presetWidthVertical_tableform [32]	= 1950;
	this->presetWidthVertical_tableform [33]	= 1900;
	this->presetWidthVertical_tableform [34]	= 1850;
	this->presetWidthVertical_tableform [35]	= 1800;
	this->presetWidthVertical_tableform [36]	= 1750;
	this->presetWidthVertical_tableform [37]	= 1700;
	this->presetWidthVertical_tableform [38]	= 1650;
	this->presetWidthVertical_tableform [39]	= 1600;
	this->presetWidthVertical_tableform [40]	= 1550;
	this->presetWidthVertical_tableform [41]	= 1500;
	this->presetWidthVertical_tableform [42]	= 1450;
	this->presetWidthVertical_tableform [43]	= 1400;
	this->presetWidthVertical_tableform [44]	= 1350;
	this->presetWidthVertical_tableform [45]	= 1300;
	this->presetWidthVertical_tableform [46]	= 1250;
	this->presetWidthVertical_tableform [47]	= 1200;
	this->presetWidthVertical_tableform [48]	= 1150;
	this->presetWidthVertical_tableform [49]	= 1100;
	this->presetWidthVertical_tableform [50]	= 1050;
	this->presetWidthVertical_tableform [51]	= 1000;
	this->presetWidthVertical_tableform [52]	= 950;
	this->presetWidthVertical_tableform [53]	= 900;
	this->presetWidthVertical_tableform [54]	= 850;
	this->presetWidthVertical_tableform [55]	= 800;
	this->presetWidthVertical_tableform [56]	= 750;
	this->presetWidthVertical_tableform [57]	= 700;
	this->presetWidthVertical_tableform [58]	= 650;
	this->presetWidthVertical_tableform [59]	= 600;
	this->presetWidthVertical_tableform [60]	= 500;
	this->presetWidthVertical_tableform [61]	= 450;
	this->presetWidthVertical_tableform [62]	= 400;
	this->presetWidthVertical_tableform [63]	= 300;
	this->presetWidthVertical_tableform [64]	= 200;

	this->presetWidthHorizontal_tableform [0]	= 3600;
	this->presetWidthHorizontal_tableform [1]	= 3300;
	this->presetWidthHorizontal_tableform [2]	= 3000;
	this->presetWidthHorizontal_tableform [3]	= 2700;
	this->presetWidthHorizontal_tableform [4]	= 2400;
	this->presetWidthHorizontal_tableform [5]	= 2100;
	this->presetWidthHorizontal_tableform [6]	= 1800;
	this->presetWidthHorizontal_tableform [7]	= 1500;
	this->presetWidthHorizontal_tableform [8]	= 1200;
	this->presetWidthHorizontal_tableform [9]	= 900;
	this->presetWidthHorizontal_tableform [10]	= 600;
	
	this->presetWidthVertical_euroform [0]		= 600;
	this->presetWidthVertical_euroform [1]		= 500;
	this->presetWidthVertical_euroform [2]		= 450;
	this->presetWidthVertical_euroform [3]		= 400;
	this->presetWidthVertical_euroform [4]		= 300;
	this->presetWidthVertical_euroform [5]		= 200;
	this->presetWidthVertical_euroform [6]		= 0;

	this->presetHeightHorizontal_euroform [0]	= 1200;
	this->presetHeightHorizontal_euroform [1]	= 900;
	this->presetHeightHorizontal_euroform [2]	= 600;
	this->presetHeightHorizontal_euroform [3]	= 0;

	this->presetWidth_config_vertical [0][0] = 6;	this->presetWidth_config_vertical [0][1] = 600;		this->presetWidth_config_vertical [0][2] = 600;		this->presetWidth_config_vertical [0][3] = 600;
													this->presetWidth_config_vertical [0][4] = 600;		this->presetWidth_config_vertical [0][5] = 600;		this->presetWidth_config_vertical [0][6] = 600;		// 3600
	this->presetWidth_config_vertical [1][0] = 6;	this->presetWidth_config_vertical [1][1] = 600;		this->presetWidth_config_vertical [1][2] = 600;		this->presetWidth_config_vertical [1][3] = 600;
													this->presetWidth_config_vertical [1][4] = 600;		this->presetWidth_config_vertical [1][5] = 500;		this->presetWidth_config_vertical [1][6] = 600;		// 3500
	this->presetWidth_config_vertical [2][0] = 6;	this->presetWidth_config_vertical [2][1] = 600;		this->presetWidth_config_vertical [2][2] = 600;		this->presetWidth_config_vertical [2][3] = 600;
													this->presetWidth_config_vertical [2][4] = 600;		this->presetWidth_config_vertical [2][5] = 450;		this->presetWidth_config_vertical [2][6] = 600;		// 3450
	this->presetWidth_config_vertical [3][0] = 6;	this->presetWidth_config_vertical [3][1] = 600;		this->presetWidth_config_vertical [3][2] = 600;		this->presetWidth_config_vertical [3][3] = 600;
													this->presetWidth_config_vertical [3][4] = 600;		this->presetWidth_config_vertical [3][5] = 400;		this->presetWidth_config_vertical [3][6] = 600;		// 3400
	this->presetWidth_config_vertical [4][0] = 6;	this->presetWidth_config_vertical [4][1] = 600;		this->presetWidth_config_vertical [4][2] = 600;		this->presetWidth_config_vertical [4][3] = 600;
													this->presetWidth_config_vertical [4][4] = 500;		this->presetWidth_config_vertical [4][5] = 450;		this->presetWidth_config_vertical [4][6] = 600;		// 3350
	this->presetWidth_config_vertical [5][0] = 6;	this->presetWidth_config_vertical [5][1] = 600;		this->presetWidth_config_vertical [5][2] = 600;		this->presetWidth_config_vertical [5][3] = 600;
													this->presetWidth_config_vertical [5][4] = 600;		this->presetWidth_config_vertical [5][5] = 300;		this->presetWidth_config_vertical [5][6] = 600;		// 3300
	this->presetWidth_config_vertical [6][0] = 6;	this->presetWidth_config_vertical [6][1] = 600;		this->presetWidth_config_vertical [6][2] = 600;		this->presetWidth_config_vertical [6][3] = 600;
													this->presetWidth_config_vertical [6][4] = 450;		this->presetWidth_config_vertical [6][5] = 400;		this->presetWidth_config_vertical [6][6] = 600;		// 3250
	this->presetWidth_config_vertical [7][0] = 6;	this->presetWidth_config_vertical [7][1] = 600;		this->presetWidth_config_vertical [7][2] = 600;		this->presetWidth_config_vertical [7][3] = 600;
													this->presetWidth_config_vertical [7][4] = 600;		this->presetWidth_config_vertical [7][5] = 200;		this->presetWidth_config_vertical [7][6] = 600;		// 3200
	this->presetWidth_config_vertical [8][0] = 6;	this->presetWidth_config_vertical [8][1] = 600;		this->presetWidth_config_vertical [8][2] = 600;		this->presetWidth_config_vertical [8][3] = 600;
													this->presetWidth_config_vertical [8][4] = 450;		this->presetWidth_config_vertical [8][5] = 300;		this->presetWidth_config_vertical [8][6] = 600;		// 3150
	this->presetWidth_config_vertical [9][0] = 6;	this->presetWidth_config_vertical [9][1] = 600;		this->presetWidth_config_vertical [9][2] = 600;		this->presetWidth_config_vertical [9][3] = 600;
													this->presetWidth_config_vertical [9][4] = 400;		this->presetWidth_config_vertical [9][5] = 300;		this->presetWidth_config_vertical [9][6] = 600;		// 3100
	this->presetWidth_config_vertical [10][0] = 6;	this->presetWidth_config_vertical [10][1] = 600;	this->presetWidth_config_vertical [10][2] = 600;	this->presetWidth_config_vertical [10][3] = 600;
													this->presetWidth_config_vertical [10][4] = 450;	this->presetWidth_config_vertical [10][5] = 200;	this->presetWidth_config_vertical [10][6] = 600;	// 3050
	this->presetWidth_config_vertical [11][0] = 5;	this->presetWidth_config_vertical [11][1] = 600;	this->presetWidth_config_vertical [11][2] = 600;	this->presetWidth_config_vertical [11][3] = 600;
													this->presetWidth_config_vertical [11][4] = 600;	this->presetWidth_config_vertical [11][5] = 600;	this->presetWidth_config_vertical [11][6] = 0;		// 3000
	this->presetWidth_config_vertical [12][0] = 6;	this->presetWidth_config_vertical [12][1] = 600;	this->presetWidth_config_vertical [12][2] = 600;	this->presetWidth_config_vertical [12][3] = 600;
													this->presetWidth_config_vertical [12][4] = 400;	this->presetWidth_config_vertical [12][5] = 300;	this->presetWidth_config_vertical [12][6] = 450;	// 2950
	this->presetWidth_config_vertical [13][0] = 6;	this->presetWidth_config_vertical [13][1] = 600;	this->presetWidth_config_vertical [13][2] = 600;	this->presetWidth_config_vertical [13][3] = 600;
													this->presetWidth_config_vertical [13][4] = 450;	this->presetWidth_config_vertical [13][5] = 200;	this->presetWidth_config_vertical [13][6] = 450;	// 2900
	this->presetWidth_config_vertical [14][0] = 5;	this->presetWidth_config_vertical [14][1] = 600;	this->presetWidth_config_vertical [14][2] = 600;	this->presetWidth_config_vertical [14][3] = 600;
													this->presetWidth_config_vertical [14][4] = 600;	this->presetWidth_config_vertical [14][5] = 450;	this->presetWidth_config_vertical [14][6] = 0;		// 2850
	this->presetWidth_config_vertical [15][0] = 5;	this->presetWidth_config_vertical [15][1] = 600;	this->presetWidth_config_vertical [15][2] = 600;	this->presetWidth_config_vertical [15][3] = 600;
													this->presetWidth_config_vertical [15][4] = 600;	this->presetWidth_config_vertical [15][5] = 400;	this->presetWidth_config_vertical [15][6] = 0;		// 2800
	this->presetWidth_config_vertical [16][0] = 6;	this->presetWidth_config_vertical [16][1] = 600;	this->presetWidth_config_vertical [16][2] = 600;	this->presetWidth_config_vertical [16][3] = 600;
													this->presetWidth_config_vertical [16][4] = 450;	this->presetWidth_config_vertical [16][5] = 200;	this->presetWidth_config_vertical [16][6] = 300;	// 2750
	this->presetWidth_config_vertical [17][0] = 5;	this->presetWidth_config_vertical [17][1] = 600;	this->presetWidth_config_vertical [17][2] = 600;	this->presetWidth_config_vertical [17][3] = 600;
													this->presetWidth_config_vertical [17][4] = 600;	this->presetWidth_config_vertical [17][5] = 300;	this->presetWidth_config_vertical [17][6] = 0;		// 2700
	this->presetWidth_config_vertical [18][0] = 5;	this->presetWidth_config_vertical [18][1] = 600;	this->presetWidth_config_vertical [18][2] = 600;	this->presetWidth_config_vertical [18][3] = 600;
													this->presetWidth_config_vertical [18][4] = 400;	this->presetWidth_config_vertical [18][5] = 450;	this->presetWidth_config_vertical [18][6] = 0;		// 2650
	this->presetWidth_config_vertical [19][0] = 5;	this->presetWidth_config_vertical [19][1] = 600;	this->presetWidth_config_vertical [19][2] = 600;	this->presetWidth_config_vertical [19][3] = 600;
													this->presetWidth_config_vertical [19][4] = 500;	this->presetWidth_config_vertical [19][5] = 300;	this->presetWidth_config_vertical [19][6] = 0;		// 2600
	this->presetWidth_config_vertical [20][0] = 5;	this->presetWidth_config_vertical [20][1] = 600;	this->presetWidth_config_vertical [20][2] = 600;	this->presetWidth_config_vertical [20][3] = 600;
													this->presetWidth_config_vertical [20][4] = 450;	this->presetWidth_config_vertical [20][5] = 300;	this->presetWidth_config_vertical [20][6] = 0;		// 2550
	this->presetWidth_config_vertical [21][0] = 5;	this->presetWidth_config_vertical [21][1] = 600;	this->presetWidth_config_vertical [21][2] = 600;	this->presetWidth_config_vertical [21][3] = 600;
													this->presetWidth_config_vertical [21][4] = 400;	this->presetWidth_config_vertical [21][5] = 300;	this->presetWidth_config_vertical [21][6] = 0;		// 2500
	this->presetWidth_config_vertical [22][0] = 5;	this->presetWidth_config_vertical [22][1] = 600;	this->presetWidth_config_vertical [22][2] = 600;	this->presetWidth_config_vertical [22][3] = 600;
													this->presetWidth_config_vertical [22][4] = 200;	this->presetWidth_config_vertical [22][5] = 450;	this->presetWidth_config_vertical [22][6] = 0;		// 2450
	this->presetWidth_config_vertical [23][0] = 4;	this->presetWidth_config_vertical [23][1] = 600;	this->presetWidth_config_vertical [23][2] = 600;	this->presetWidth_config_vertical [23][3] = 600;
													this->presetWidth_config_vertical [23][4] = 600;	this->presetWidth_config_vertical [23][5] = 0;		this->presetWidth_config_vertical [23][6] = 0;		// 2400
	this->presetWidth_config_vertical [24][0] = 5;	this->presetWidth_config_vertical [24][1] = 600;	this->presetWidth_config_vertical [24][2] = 600;	this->presetWidth_config_vertical [24][3] = 450;
													this->presetWidth_config_vertical [24][4] = 400;	this->presetWidth_config_vertical [24][5] = 300;	this->presetWidth_config_vertical [24][6] = 0;		// 2350
	this->presetWidth_config_vertical [25][0] = 4;	this->presetWidth_config_vertical [25][1] = 600;	this->presetWidth_config_vertical [25][2] = 600;	this->presetWidth_config_vertical [25][3] = 500;
													this->presetWidth_config_vertical [25][4] = 600;	this->presetWidth_config_vertical [25][5] = 0;		this->presetWidth_config_vertical [25][6] = 0;		// 2300
	this->presetWidth_config_vertical [26][0] = 4;	this->presetWidth_config_vertical [26][1] = 600;	this->presetWidth_config_vertical [26][2] = 600;	this->presetWidth_config_vertical [26][3] = 450;
													this->presetWidth_config_vertical [26][4] = 600;	this->presetWidth_config_vertical [26][5] = 0;		this->presetWidth_config_vertical [26][6] = 0;		// 2250
	this->presetWidth_config_vertical [27][0] = 4;	this->presetWidth_config_vertical [27][1] = 600;	this->presetWidth_config_vertical [27][2] = 600;	this->presetWidth_config_vertical [27][3] = 400;
													this->presetWidth_config_vertical [27][4] = 600;	this->presetWidth_config_vertical [27][5] = 0;		this->presetWidth_config_vertical [27][6] = 0;		// 2200
	this->presetWidth_config_vertical [28][0] = 4;	this->presetWidth_config_vertical [28][1] = 600;	this->presetWidth_config_vertical [28][2] = 500;	this->presetWidth_config_vertical [28][3] = 450;
													this->presetWidth_config_vertical [28][4] = 600;	this->presetWidth_config_vertical [28][5] = 0;		this->presetWidth_config_vertical [28][6] = 0;		// 2150
	this->presetWidth_config_vertical [29][0] = 4;	this->presetWidth_config_vertical [29][1] = 600;	this->presetWidth_config_vertical [29][2] = 600;	this->presetWidth_config_vertical [29][3] = 300;
													this->presetWidth_config_vertical [29][4] = 600;	this->presetWidth_config_vertical [29][5] = 0;		this->presetWidth_config_vertical [29][6] = 0;		// 2100
	this->presetWidth_config_vertical [30][0] = 4;	this->presetWidth_config_vertical [30][1] = 600;	this->presetWidth_config_vertical [30][2] = 450;	this->presetWidth_config_vertical [30][3] = 400;
													this->presetWidth_config_vertical [30][4] = 600;	this->presetWidth_config_vertical [30][5] = 0;		this->presetWidth_config_vertical [30][6] = 0;		// 2050
	this->presetWidth_config_vertical [31][0] = 4;	this->presetWidth_config_vertical [31][1] = 600;	this->presetWidth_config_vertical [31][2] = 600;	this->presetWidth_config_vertical [31][3] = 200;
													this->presetWidth_config_vertical [31][4] = 600;	this->presetWidth_config_vertical [31][5] = 0;		this->presetWidth_config_vertical [31][6] = 0;		// 2000
	this->presetWidth_config_vertical [32][0] = 4;	this->presetWidth_config_vertical [32][1] = 600;	this->presetWidth_config_vertical [32][2] = 450;	this->presetWidth_config_vertical [32][3] = 300;
													this->presetWidth_config_vertical [32][4] = 600;	this->presetWidth_config_vertical [32][5] = 0;		this->presetWidth_config_vertical [32][6] = 0;		// 1950
	this->presetWidth_config_vertical [33][0] = 4;	this->presetWidth_config_vertical [33][1] = 600;	this->presetWidth_config_vertical [33][2] = 400;	this->presetWidth_config_vertical [33][3] = 300;
													this->presetWidth_config_vertical [33][4] = 600;	this->presetWidth_config_vertical [33][5] = 0;		this->presetWidth_config_vertical [33][6] = 0;		// 1900
	this->presetWidth_config_vertical [34][0] = 4;	this->presetWidth_config_vertical [34][1] = 600;	this->presetWidth_config_vertical [34][2] = 450;	this->presetWidth_config_vertical [34][3] = 200;
													this->presetWidth_config_vertical [34][4] = 600;	this->presetWidth_config_vertical [34][5] = 0;		this->presetWidth_config_vertical [34][6] = 0;		// 1850
	this->presetWidth_config_vertical [35][0] = 3;	this->presetWidth_config_vertical [35][1] = 600;	this->presetWidth_config_vertical [35][2] = 600;	this->presetWidth_config_vertical [35][3] = 600;
													this->presetWidth_config_vertical [35][4] = 0;		this->presetWidth_config_vertical [35][5] = 0;		this->presetWidth_config_vertical [35][6] = 0;		// 1800
	this->presetWidth_config_vertical [36][0] = 4;	this->presetWidth_config_vertical [36][1] = 600;	this->presetWidth_config_vertical [36][2] = 400;	this->presetWidth_config_vertical [36][3] = 450;
													this->presetWidth_config_vertical [36][4] = 300;	this->presetWidth_config_vertical [36][5] = 0;		this->presetWidth_config_vertical [36][6] = 0;		// 1750
	this->presetWidth_config_vertical [37][0] = 3;	this->presetWidth_config_vertical [37][1] = 600;	this->presetWidth_config_vertical [37][2] = 500;	this->presetWidth_config_vertical [37][3] = 600;
													this->presetWidth_config_vertical [37][4] = 0;		this->presetWidth_config_vertical [37][5] = 0;		this->presetWidth_config_vertical [37][6] = 0;		// 1700
	this->presetWidth_config_vertical [38][0] = 3;	this->presetWidth_config_vertical [38][1] = 600;	this->presetWidth_config_vertical [38][2] = 450;	this->presetWidth_config_vertical [38][3] = 600;
													this->presetWidth_config_vertical [38][4] = 0;		this->presetWidth_config_vertical [38][5] = 0;		this->presetWidth_config_vertical [38][6] = 0;		// 1650
	this->presetWidth_config_vertical [39][0] = 3;	this->presetWidth_config_vertical [39][1] = 600;	this->presetWidth_config_vertical [39][2] = 400;	this->presetWidth_config_vertical [39][3] = 600;
													this->presetWidth_config_vertical [39][4] = 0;		this->presetWidth_config_vertical [39][5] = 0;		this->presetWidth_config_vertical [39][6] = 0;		// 1600
	this->presetWidth_config_vertical [40][0] = 4;	this->presetWidth_config_vertical [40][1] = 600;	this->presetWidth_config_vertical [40][2] = 450;	this->presetWidth_config_vertical [40][3] = 200;
													this->presetWidth_config_vertical [40][4] = 300;	this->presetWidth_config_vertical [40][5] = 0;		this->presetWidth_config_vertical [40][6] = 0;		// 1550
	this->presetWidth_config_vertical [41][0] = 3;	this->presetWidth_config_vertical [41][1] = 600;	this->presetWidth_config_vertical [41][2] = 300;	this->presetWidth_config_vertical [41][3] = 600;
													this->presetWidth_config_vertical [41][4] = 0;		this->presetWidth_config_vertical [41][5] = 0;		this->presetWidth_config_vertical [41][6] = 0;		// 1500
	this->presetWidth_config_vertical [42][0] = 3;	this->presetWidth_config_vertical [42][1] = 600;	this->presetWidth_config_vertical [42][2] = 400;	this->presetWidth_config_vertical [42][3] = 450;
													this->presetWidth_config_vertical [42][4] = 0;		this->presetWidth_config_vertical [42][5] = 0;		this->presetWidth_config_vertical [42][6] = 0;		// 1450
	this->presetWidth_config_vertical [43][0] = 3;	this->presetWidth_config_vertical [43][1] = 600;	this->presetWidth_config_vertical [43][2] = 200;	this->presetWidth_config_vertical [43][3] = 600;
													this->presetWidth_config_vertical [43][4] = 0;		this->presetWidth_config_vertical [43][5] = 0;		this->presetWidth_config_vertical [43][6] = 0;		// 1400
	this->presetWidth_config_vertical [44][0] = 3;	this->presetWidth_config_vertical [44][1] = 600;	this->presetWidth_config_vertical [44][2] = 300;	this->presetWidth_config_vertical [44][3] = 450;
													this->presetWidth_config_vertical [44][4] = 0;		this->presetWidth_config_vertical [44][5] = 0;		this->presetWidth_config_vertical [44][6] = 0;		// 1350
	this->presetWidth_config_vertical [45][0] = 3;	this->presetWidth_config_vertical [45][1] = 600;	this->presetWidth_config_vertical [45][2] = 400;	this->presetWidth_config_vertical [45][3] = 300;
													this->presetWidth_config_vertical [45][4] = 0;		this->presetWidth_config_vertical [45][5] = 0;		this->presetWidth_config_vertical [45][6] = 0;		// 1300
	this->presetWidth_config_vertical [46][0] = 3;	this->presetWidth_config_vertical [46][1] = 600;	this->presetWidth_config_vertical [46][2] = 200;	this->presetWidth_config_vertical [46][3] = 450;
													this->presetWidth_config_vertical [46][4] = 0;		this->presetWidth_config_vertical [46][5] = 0;		this->presetWidth_config_vertical [46][6] = 0;		// 1250
	this->presetWidth_config_vertical [47][0] = 2;	this->presetWidth_config_vertical [47][1] = 600;	this->presetWidth_config_vertical [47][2] = 600;	this->presetWidth_config_vertical [47][3] = 0;
													this->presetWidth_config_vertical [47][4] = 0;		this->presetWidth_config_vertical [47][5] = 0;		this->presetWidth_config_vertical [47][6] = 0;		// 1200
	this->presetWidth_config_vertical [48][0] = 3;	this->presetWidth_config_vertical [48][1] = 450;	this->presetWidth_config_vertical [48][2] = 400;	this->presetWidth_config_vertical [48][3] = 300;
													this->presetWidth_config_vertical [48][4] = 0;		this->presetWidth_config_vertical [48][5] = 0;		this->presetWidth_config_vertical [48][6] = 0;		// 1150
	this->presetWidth_config_vertical [49][0] = 3;	this->presetWidth_config_vertical [49][1] = 400;	this->presetWidth_config_vertical [49][2] = 400;	this->presetWidth_config_vertical [49][3] = 300;
													this->presetWidth_config_vertical [49][4] = 0;		this->presetWidth_config_vertical [49][5] = 0;		this->presetWidth_config_vertical [49][6] = 0;		// 1100
	this->presetWidth_config_vertical [50][0] = 3;	this->presetWidth_config_vertical [50][1] = 450;	this->presetWidth_config_vertical [50][2] = 300;	this->presetWidth_config_vertical [50][3] = 300;
													this->presetWidth_config_vertical [50][4] = 0;		this->presetWidth_config_vertical [50][5] = 0;		this->presetWidth_config_vertical [50][6] = 0;		// 1050
	this->presetWidth_config_vertical [51][0] = 2;	this->presetWidth_config_vertical [51][1] = 600;	this->presetWidth_config_vertical [51][2] = 400;	this->presetWidth_config_vertical [51][3] = 0;
													this->presetWidth_config_vertical [51][4] = 0;		this->presetWidth_config_vertical [51][5] = 0;		this->presetWidth_config_vertical [51][6] = 0;		// 1000
	this->presetWidth_config_vertical [52][0] = 2;	this->presetWidth_config_vertical [52][1] = 500;	this->presetWidth_config_vertical [52][2] = 450;	this->presetWidth_config_vertical [52][3] = 0;
													this->presetWidth_config_vertical [52][4] = 0;		this->presetWidth_config_vertical [52][5] = 0;		this->presetWidth_config_vertical [52][6] = 0;		// 950
	this->presetWidth_config_vertical [53][0] = 2;	this->presetWidth_config_vertical [53][1] = 600;	this->presetWidth_config_vertical [53][2] = 300;	this->presetWidth_config_vertical [53][3] = 0;
													this->presetWidth_config_vertical [53][4] = 0;		this->presetWidth_config_vertical [53][5] = 0;		this->presetWidth_config_vertical [53][6] = 0;		// 900
	this->presetWidth_config_vertical [54][0] = 2;	this->presetWidth_config_vertical [54][1] = 400;	this->presetWidth_config_vertical [54][2] = 450;	this->presetWidth_config_vertical [54][3] = 0;
													this->presetWidth_config_vertical [54][4] = 0;		this->presetWidth_config_vertical [54][5] = 0;		this->presetWidth_config_vertical [54][6] = 0;		// 850
	this->presetWidth_config_vertical [55][0] = 2;	this->presetWidth_config_vertical [55][1] = 400;	this->presetWidth_config_vertical [55][2] = 400;	this->presetWidth_config_vertical [55][3] = 0;
													this->presetWidth_config_vertical [55][4] = 0;		this->presetWidth_config_vertical [55][5] = 0;		this->presetWidth_config_vertical [55][6] = 0;		// 800
	this->presetWidth_config_vertical [56][0] = 2;	this->presetWidth_config_vertical [56][1] = 450;	this->presetWidth_config_vertical [56][2] = 300;	this->presetWidth_config_vertical [56][3] = 0;
													this->presetWidth_config_vertical [56][4] = 0;		this->presetWidth_config_vertical [56][5] = 0;		this->presetWidth_config_vertical [56][6] = 0;		// 750
	this->presetWidth_config_vertical [57][0] = 2;	this->presetWidth_config_vertical [57][1] = 400;	this->presetWidth_config_vertical [57][2] = 300;	this->presetWidth_config_vertical [57][3] = 0;
													this->presetWidth_config_vertical [57][4] = 0;		this->presetWidth_config_vertical [57][5] = 0;		this->presetWidth_config_vertical [57][6] = 0;		// 700
	this->presetWidth_config_vertical [58][0] = 2;	this->presetWidth_config_vertical [58][1] = 450;	this->presetWidth_config_vertical [58][2] = 200;	this->presetWidth_config_vertical [58][3] = 0;
													this->presetWidth_config_vertical [58][4] = 0;		this->presetWidth_config_vertical [58][5] = 0;		this->presetWidth_config_vertical [58][6] = 0;		// 650
	this->presetWidth_config_vertical [59][0] = 1;	this->presetWidth_config_vertical [59][1] = 600;	this->presetWidth_config_vertical [59][2] = 0;		this->presetWidth_config_vertical [59][3] = 0;
													this->presetWidth_config_vertical [59][4] = 0;		this->presetWidth_config_vertical [59][5] = 0;		this->presetWidth_config_vertical [59][6] = 0;		// 600
	this->presetWidth_config_vertical [60][0] = 1;	this->presetWidth_config_vertical [60][1] = 500;	this->presetWidth_config_vertical [60][2] = 0;		this->presetWidth_config_vertical [60][3] = 0;
													this->presetWidth_config_vertical [60][4] = 0;		this->presetWidth_config_vertical [60][5] = 0;		this->presetWidth_config_vertical [60][6] = 0;		// 500
	this->presetWidth_config_vertical [61][0] = 1;	this->presetWidth_config_vertical [61][1] = 450;	this->presetWidth_config_vertical [61][2] = 0;		this->presetWidth_config_vertical [61][3] = 0;
													this->presetWidth_config_vertical [61][4] = 0;		this->presetWidth_config_vertical [61][5] = 0;		this->presetWidth_config_vertical [61][6] = 0;		// 450
	this->presetWidth_config_vertical [62][0] = 1;	this->presetWidth_config_vertical [62][1] = 400;	this->presetWidth_config_vertical [62][2] = 0;		this->presetWidth_config_vertical [62][3] = 0;
													this->presetWidth_config_vertical [62][4] = 0;		this->presetWidth_config_vertical [62][5] = 0;		this->presetWidth_config_vertical [62][6] = 0;		// 400
	this->presetWidth_config_vertical [63][0] = 1;	this->presetWidth_config_vertical [63][1] = 300;	this->presetWidth_config_vertical [63][2] = 0;		this->presetWidth_config_vertical [63][3] = 0;
													this->presetWidth_config_vertical [63][4] = 0;		this->presetWidth_config_vertical [63][5] = 0;		this->presetWidth_config_vertical [63][6] = 0;		// 300
	this->presetWidth_config_vertical [64][0] = 1;	this->presetWidth_config_vertical [64][1] = 200;	this->presetWidth_config_vertical [64][2] = 0;		this->presetWidth_config_vertical [64][3] = 0;
													this->presetWidth_config_vertical [64][4] = 0;		this->presetWidth_config_vertical [64][5] = 0;		this->presetWidth_config_vertical [64][6] = 0;		// 200
	
	this->presetWidth_config_horizontal [0][0] = 3;		this->presetWidth_config_horizontal [0][1] = 1200;	this->presetWidth_config_horizontal [0][2] = 1200;	this->presetWidth_config_horizontal [0][3] = 1200;	// 3600
	this->presetWidth_config_horizontal [1][0] = 3;		this->presetWidth_config_horizontal [1][1] = 1200;	this->presetWidth_config_horizontal [1][2] = 1200;	this->presetWidth_config_horizontal [1][3] = 900;	// 3300
	this->presetWidth_config_horizontal [2][0] = 3;		this->presetWidth_config_horizontal [2][1] = 1200;	this->presetWidth_config_horizontal [2][2] = 1200;	this->presetWidth_config_horizontal [2][3] = 600;	// 3000
	this->presetWidth_config_horizontal [3][0] = 3;		this->presetWidth_config_horizontal [3][1] = 1200;	this->presetWidth_config_horizontal [3][2] = 900;	this->presetWidth_config_horizontal [3][3] = 600;	// 2700
	this->presetWidth_config_horizontal [4][0] = 2;		this->presetWidth_config_horizontal [4][1] = 1200;	this->presetWidth_config_horizontal [4][2] = 1200;	this->presetWidth_config_horizontal [4][3] = 0;		// 2400
	this->presetWidth_config_horizontal [5][0] = 2;		this->presetWidth_config_horizontal [5][1] = 1200;	this->presetWidth_config_horizontal [5][2] = 900;	this->presetWidth_config_horizontal [5][3] = 0;		// 2100
	this->presetWidth_config_horizontal [6][0] = 2;		this->presetWidth_config_horizontal [6][1] = 900;	this->presetWidth_config_horizontal [6][2] = 900;	this->presetWidth_config_horizontal [6][3] = 0;		// 1800
	this->presetWidth_config_horizontal [7][0] = 2;		this->presetWidth_config_horizontal [7][1] = 900;	this->presetWidth_config_horizontal [7][2] = 600;	this->presetWidth_config_horizontal [7][3] = 0;		// 1500
	this->presetWidth_config_horizontal [8][0] = 1;		this->presetWidth_config_horizontal [8][1] = 1200;	this->presetWidth_config_horizontal [8][2] = 0;		this->presetWidth_config_horizontal [8][3] = 0;		// 1200
	this->presetWidth_config_horizontal [9][0] = 1;		this->presetWidth_config_horizontal [9][1] = 900;	this->presetWidth_config_horizontal [9][2] = 0;		this->presetWidth_config_horizontal [9][3] = 0;		// 900
	this->presetWidth_config_horizontal [10][0] = 1;	this->presetWidth_config_horizontal [10][1] = 600;	this->presetWidth_config_horizontal [10][2] = 0;	this->presetWidth_config_horizontal [10][3] = 0;	// 600
}

// �� ���� �ʱ�ȭ
void	LowSideTableformPlacingZone::initCells (LowSideTableformPlacingZone* placingZone, bool bVertical)
{
	short	xx;

	// �� ���� ä��� (�� �ʺ� ������ �̸� ä��)
	// ���ι����̸�
	if (bVertical == true) {
		for (xx = 0 ; xx < sizeof (placingZone->cells) / sizeof (CellForLowSideTableform) ; ++xx) {
			placingZone->cells [xx].objType = TABLEFORM;
			placingZone->cells [xx].horLen = 3600;
			placingZone->cells [xx].tableInHor [0] = 600;
			placingZone->cells [xx].tableInHor [1] = 600;
			placingZone->cells [xx].tableInHor [2] = 600;
			placingZone->cells [xx].tableInHor [3] = 600;
			placingZone->cells [xx].tableInHor [4] = 600;
			placingZone->cells [xx].tableInHor [5] = 600;
			placingZone->cells [xx].tableInHor [6] = 0;
			placingZone->cells [xx].tableInHor [7] = 0;
			placingZone->cells [xx].tableInHor [8] = 0;
			placingZone->cells [xx].tableInHor [9] = 0;
		}

	// ���ι����̸�
	} else {
		for (xx = 0 ; xx < sizeof (placingZone->cells) / sizeof (CellForLowSideTableform) ; ++xx) {
			placingZone->cells [xx].objType = TABLEFORM;
			placingZone->cells [xx].horLen = 3600;
			placingZone->cells [xx].tableInHor [0] = 1200;
			placingZone->cells [xx].tableInHor [1] = 1200;
			placingZone->cells [xx].tableInHor [2] = 1200;
			placingZone->cells [xx].tableInHor [3] = 0;
			placingZone->cells [xx].tableInHor [4] = 0;
			placingZone->cells [xx].tableInHor [5] = 0;
			placingZone->cells [xx].tableInHor [6] = 0;
			placingZone->cells [xx].tableInHor [7] = 0;
			placingZone->cells [xx].tableInHor [8] = 0;
			placingZone->cells [xx].tableInHor [9] = 0;
		}
	}
}

// ��(0-��� �ε��� ��ȣ)�� ���ϴ� �� ��ġ X ��ǥ�� ����
double	LowSideTableformPlacingZone::getCellPositionLeftBottomX (LowSideTableformPlacingZone* placingZone, short idx)
{
	double	distance = 0.0;

	if ((placingZone->typeLcorner == INCORNER_PANEL) || (placingZone->typeLcorner == OUTCORNER_PANEL))
		distance = placingZone->lenLcorner;

	for (short xx = 0 ; xx < idx ; ++xx)
		distance += (double)placingZone->cells [xx].horLen / 1000.0;

	return distance;
}

// �� ��ġ�� �ٸ��� ������
void	LowSideTableformPlacingZone::adjustCellsPosition (LowSideTableformPlacingZone* placingZone)
{
	for (short xx = 0 ; xx < placingZone->nCellsInHor ; ++xx) {
		placingZone->cells [xx].ang = placingZone->ang;
		placingZone->cells [xx].leftBottomX = placingZone->leftBottomX + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * cos(placingZone->ang));
		placingZone->cells [xx].leftBottomY = placingZone->leftBottomY + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * sin(placingZone->ang));
		placingZone->cells [xx].leftBottomZ = placingZone->leftBottomZ;
	}
}

// �� ������ ������� ��ü���� ��ġ��
GSErrCode	LowSideTableformPlacingZone::placeObjects (LowSideTableformPlacingZone* placingZone)
{
	GSErrCode	err = NoError;
	short	xx;

	// ================================================== ���ڳ� ��ġ
	// ���� ���ڳ� ��ġ
	if (placingZone->typeLcorner == INCORNER_PANEL) {
		EasyObjectPlacement incornerPanel;
		incornerPanel.init (L("���ڳ��ǳ�v1.0.gsm"), layerInd_IncornerPanel, floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang - DegreeToRad (90.0));
		incornerPanel.placeObject (5,
			"in_comp", APIParT_CString, "���ڳ��ǳ�",
			"wid_s", APIParT_Length, format_string ("%f", 0.100),
			"leng_s", APIParT_Length, format_string ("%f", placingZone->lenLcorner),
			"hei_s", APIParT_Length, format_string ("%f", placingZone->verLen),
			"dir_s", APIParT_CString, "�����");
	} else if (placingZone->typeLcorner == OUTCORNER_PANEL) {
		EasyObjectPlacement outcornerPanel;
		outcornerPanel.init (L("�ƿ��ڳ��ǳ�v1.0.gsm"), layerInd_OutcornerPanel, floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang);
		outcornerPanel.placeObject (4,
			"wid_s", APIParT_Length, format_string ("%f", placingZone->lenLcorner),
			"leng_s", APIParT_Length, format_string ("%f", 0.100),
			"hei_s", APIParT_Length, format_string ("%f", placingZone->verLen),
			"dir_s", APIParT_CString, "�����");
	} else if (placingZone->typeLcorner == OUTCORNER_ANGLE) {
		EasyObjectPlacement outcornerAngle;
		outcornerAngle.init (L("�ƿ��ڳʾޱ�v1.0.gsm"), layerInd_OutcornerAngle, floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang + DegreeToRad (180.0));
		outcornerAngle.placeObject (2,
			"a_leng", APIParT_Length, format_string ("%f", placingZone->verLen),
			"a_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)));
	}

	// ���� ���ڳ� ��ġ
	if (placingZone->typeRcorner == INCORNER_PANEL) {
		EasyObjectPlacement incornerPanel;
		incornerPanel.init (L("���ڳ��ǳ�v1.0.gsm"), layerInd_IncornerPanel, floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang - DegreeToRad (180.0));
		moveIn3D ('x', incornerPanel.radAng + DegreeToRad (180.0), placingZone->horLen, &incornerPanel.posX, &incornerPanel.posY, &incornerPanel.posZ);
		incornerPanel.placeObject (5,
			"in_comp", APIParT_CString, "���ڳ��ǳ�",
			"wid_s", APIParT_Length, format_string ("%f", placingZone->lenRcorner),
			"leng_s", APIParT_Length, format_string ("%f", 0.100),
			"hei_s", APIParT_Length, format_string ("%f", placingZone->verLen),
			"dir_s", APIParT_CString, "�����");
	} else if (placingZone->typeRcorner == OUTCORNER_PANEL) {
		EasyObjectPlacement outcornerPanel;
		outcornerPanel.init (L("�ƿ��ڳ��ǳ�v1.0.gsm"), layerInd_OutcornerPanel, floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang + DegreeToRad (90.0));
		moveIn3D ('x', outcornerPanel.radAng - DegreeToRad (90.0), placingZone->horLen, &outcornerPanel.posX, &outcornerPanel.posY, &outcornerPanel.posZ);
		outcornerPanel.placeObject (4,
			"wid_s", APIParT_Length, format_string ("%f", 0.100),
			"leng_s", APIParT_Length, format_string ("%f", placingZone->lenRcorner),
			"hei_s", APIParT_Length, format_string ("%f", placingZone->verLen),
			"dir_s", APIParT_CString, "�����");
	} else if (placingZone->typeRcorner == OUTCORNER_ANGLE) {
		EasyObjectPlacement outcornerAngle;
		outcornerAngle.init (L("�ƿ��ڳʾޱ�v1.0.gsm"), layerInd_OutcornerAngle, floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang + DegreeToRad (270.0));
		moveIn3D ('x', outcornerAngle.radAng - DegreeToRad (270.0), placingZone->horLen, &outcornerAngle.posX, &outcornerAngle.posY, &outcornerAngle.posZ);
		outcornerAngle.placeObject (2,
			"a_leng", APIParT_Length, format_string ("%f", placingZone->verLen),
			"a_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)));
	}

	// ================================================== ������ ��ġ
	EasyObjectPlacement euroform;
	euroform.init (L("������v2.0.gsm"), layerInd_Euroform, floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang);

	moveIn3D ('x', euroform.radAng, placingZone->lenLcorner, &euroform.posX, &euroform.posY, &euroform.posZ);		// ���� �ڳ� ���̸�ŭ x �̵�

	for (xx = 0 ; xx < placingZone->nCellsInHor ; ++xx) {
		if (placingZone->cells [xx].objType == EUROFORM) {
			if (placingZone->bVertical == true) {
				// ���ι���
				elemList.Push (euroform.placeObject (5,
					"eu_stan_onoff", APIParT_Boolean, "1.0",
					"eu_wid", APIParT_CString, format_string ("%d", placingZone->cells [xx].horLen),
					"eu_hei", APIParT_CString, format_string ("%d", (int)(placingZone->verLen * 1000)),
					"u_ins", APIParT_CString, "�������",
					"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
			} else {
				// ���ι���
				moveIn3D ('x', euroform.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
				elemList.Push (euroform.placeObject (5,
					"eu_stan_onoff", APIParT_Boolean, "1.0",
					"eu_wid", APIParT_CString, format_string ("%d", (int)(placingZone->verLen * 1000)),
					"eu_hei", APIParT_CString, format_string ("%d", placingZone->cells [xx].horLen),
					"u_ins", APIParT_CString, "��������",
					"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
				moveIn3D ('x', euroform.radAng, -(double)placingZone->cells [xx].horLen / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
			}

			// ����� ��ü �׷�ȭ
			if (!elemList.IsEmpty ()) {
				GSSize nElems = elemList.GetSize ();
				API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
				if (elemHead != NULL) {
					for (GSIndex i = 0; i < nElems; i++)
						(*elemHead)[i].guid = elemList [i];

					ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

					BMKillHandle ((GSHandle *) &elemHead);
				}
				elemList.Clear ();
			}
		}

		// ������ ���� �������� �̵�
		moveIn3D ('x', euroform.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
	}

	// ================================================== �ٷ������̼� ��ġ (�׻� ���ι���)
	EasyObjectPlacement fillersp;
	fillersp.init (L("�ٷ������̼�v1.0.gsm"), layerInd_Fillersp, floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang);

	moveIn3D ('x', fillersp.radAng, placingZone->lenLcorner, &fillersp.posX, &fillersp.posY, &fillersp.posZ);		// ���� �ڳ� ���̸�ŭ x �̵�

	for (xx = 0 ; xx < placingZone->nCellsInHor ; ++xx) {
		if (placingZone->cells [xx].objType == FILLERSP) {
			moveIn3D ('x', fillersp.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &fillersp.posX, &fillersp.posY, &fillersp.posZ);
			elemList.Push (fillersp.placeObject (4,
				"f_thk", APIParT_Length, format_string ("%f", (double)placingZone->cells [xx].horLen / 1000.0),
				"f_leng", APIParT_Length, format_string ("%f", placingZone->verLen),
				"f_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
				"f_rota", APIParT_Angle, format_string ("%f", 0.0)));

			moveIn3D ('x', fillersp.radAng, -(double)placingZone->cells [xx].horLen / 1000.0, &fillersp.posX, &fillersp.posY, &fillersp.posZ);

			// ����� ��ü �׷�ȭ
			if (!elemList.IsEmpty ()) {
				GSSize nElems = elemList.GetSize ();
				API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
				if (elemHead != NULL) {
					for (GSIndex i = 0; i < nElems; i++)
						(*elemHead)[i].guid = elemList [i];

					ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

					BMKillHandle ((GSHandle *) &elemHead);
				}
				elemList.Clear ();
			}
		}

		// ������ ���� �������� �̵�
		moveIn3D ('x', fillersp.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &fillersp.posX, &fillersp.posY, &fillersp.posZ);
	}

	// ================================================== ���� ��ġ (�׻� ���ι���)
	EasyObjectPlacement plywood;
	plywood.init (L("����v1.0.gsm"), layerInd_Plywood, floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang);

	moveIn3D ('x', plywood.radAng, placingZone->lenLcorner, &plywood.posX, &plywood.posY, &plywood.posZ);		// ���� �ڳ� ���̸�ŭ x �̵�

	for (xx = 0 ; xx < placingZone->nCellsInHor ; ++xx) {
		if (placingZone->cells [xx].objType == PLYWOOD) {
			elemList.Push (plywood.placeObject (13,
				"p_stan", APIParT_CString, "��԰�",
				"w_dir", APIParT_CString, "�������",
				"p_thk", APIParT_CString, "11.5T",
				"p_wid", APIParT_Length, format_string ("%f", (double)placingZone->cells [xx].horLen / 1000.0),
				"p_leng", APIParT_Length, format_string ("%f", placingZone->verLen),
				"p_ang", APIParT_Angle, format_string ("%f", 0.0),
				"sogak", APIParT_Boolean, "1.0",
				"bInverseSogak", APIParT_Boolean, "1.0",
				"prof", APIParT_CString, "�Ұ�",
				"gap_a", APIParT_Length, format_string ("%f", 0.0),
				"gap_b", APIParT_Length, format_string ("%f", 0.0),
				"gap_c", APIParT_Length, format_string ("%f", 0.0),
				"gap_d", APIParT_Length, format_string ("%f", 0.0)));

			// ����� ��ü �׷�ȭ
			if (!elemList.IsEmpty ()) {
				GSSize nElems = elemList.GetSize ();
				API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
				if (elemHead != NULL) {
					for (GSIndex i = 0; i < nElems; i++)
						(*elemHead)[i].guid = elemList [i];

					ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

					BMKillHandle ((GSHandle *) &elemHead);
				}
				elemList.Clear ();
			}
		}

		// ������ ���� �������� �̵�
		moveIn3D ('x', plywood.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &plywood.posX, &plywood.posY, &plywood.posZ);
	}

	// ================================================== ���� ��ġ (�׻� ���ι���)
	EasyObjectPlacement timber;
	timber.init (L("����v1.0.gsm"), layerInd_Timber, floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang);

	moveIn3D ('x', timber.radAng, placingZone->lenLcorner, &timber.posX, &timber.posY, &timber.posZ);		// ���� �ڳ� ���̸�ŭ x �̵�

	for (xx = 0 ; xx < placingZone->nCellsInHor ; ++xx) {
		if (placingZone->cells [xx].objType == TIMBER) {
			moveIn3D ('x', timber.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &timber.posX, &timber.posY, &timber.posZ);
			elemList.Push (timber.placeObject (6,
				"w_ins", APIParT_CString, "�������",
				"w_w", APIParT_Length, format_string ("%f", 0.050),
				"w_h", APIParT_Length, format_string ("%f", (double)placingZone->cells [xx].horLen / 1000.0),
				"w_leng", APIParT_Length, format_string ("%f", placingZone->verLen),
				"w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
				"torsion_ang", APIParT_Angle, format_string ("%f", 0.0)));
			moveIn3D ('x', timber.radAng, -(double)placingZone->cells [xx].horLen / 1000.0, &timber.posX, &timber.posY, &timber.posZ);
			
			// ����� ��ü �׷�ȭ
			if (!elemList.IsEmpty ()) {
				GSSize nElems = elemList.GetSize ();
				API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
				if (elemHead != NULL) {
					for (GSIndex i = 0; i < nElems; i++)
						(*elemHead)[i].guid = elemList [i];

					ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

					BMKillHandle ((GSHandle *) &elemHead);
				}
				elemList.Clear ();
			}
		}

		// ������ ���� �������� �̵�
		moveIn3D ('x', timber.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &timber.posX, &timber.posY, &timber.posZ);
	}

	// ================================================== ���̺��� ��ġ
	for (xx = 0 ; xx < placingZone->nCellsInHor ; ++xx) {
		placingZone->placeEuroformsOfTableform (this, xx);		// ���̺����� ������ ��ġ (Ÿ�� �ҹ� ����)
		
		if (placingZone->tableformType == 1)		placingZone->placeTableformA (this, xx);

		// ����� ��ü �׷�ȭ
		if (!elemList.IsEmpty ()) {
			GSSize nElems = elemList.GetSize ();
			API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
			if (elemHead != NULL) {
				for (GSIndex i = 0; i < nElems; i++)
					(*elemHead)[i].guid = elemList [i];

				ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

				BMKillHandle ((GSHandle *) &elemHead);
			}
			elemList.Clear ();
		}
	}

	return err;
}

// ���̺��� �� ������ ��ġ (����)
void	LowSideTableformPlacingZone::placeEuroformsOfTableform (LowSideTableformPlacingZone* placingZone, short idxCell)
{
	short	xx;

	EasyObjectPlacement euroform;

	if (placingZone->cells [idxCell].objType == TABLEFORM) {
		if (placingZone->bVertical == true) {
			// ���ι���
			euroform.init (L("������v2.0.gsm"), layerInd_Euroform, floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

			for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++xx) {
				if ((placingZone->cells [idxCell].tableInHor [xx] > 0) && (placingZone->verLen > EPS)) {
					elemList.Push (euroform.placeObject (5,
						"eu_stan_onoff", APIParT_Boolean, "1.0",
						"eu_wid", APIParT_CString, format_string ("%d", placingZone->cells [idxCell].tableInHor [xx]),
						"eu_hei", APIParT_CString, format_string ("%d", (int)(placingZone->verLen * 1000)),
						"u_ins", APIParT_CString, "�������",
						"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
				}
				moveIn3D ('x', euroform.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
			}
		} else {
			// ���ι���
			euroform.init (L("������v2.0.gsm"), layerInd_Euroform, floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

			for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++xx) {
				if ((placingZone->cells [idxCell].tableInHor [xx] > 0) && (placingZone->verLen > EPS)) {
					moveIn3D ('x', euroform.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
					elemList.Push (euroform.placeObject (5,
						"eu_stan_onoff", APIParT_Boolean, "1.0",
						"eu_wid", APIParT_CString, format_string ("%d", (int)(placingZone->verLen * 1000)),
						"eu_hei", APIParT_CString, format_string ("%d", placingZone->cells [idxCell].tableInHor [xx]),
						"u_ins", APIParT_CString, "��������",
						"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
					moveIn3D ('x', euroform.radAng, -(double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
				}
				moveIn3D ('x', euroform.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
			}
		}
	}
}

// ���̺��� Ÿ��A ��ġ (������ ����) - �������� 2��
void	LowSideTableformPlacingZone::placeTableformA (LowSideTableformPlacingZone* placingZone, short idxCell)
{
	short	xx;
	int		pipeLength;
	double	sideMargin;
	
	int		firstWidth, lastWidth;
	bool	bFoundFirstWidth;
	short	realWidthCount, count;

	// ���̺��� �� 1��°�� ������ �������� ������ �ʺ� ������ (���̰� 0�� �������� ���) - ���ι���
	realWidthCount = 0;
	bFoundFirstWidth = false;
	for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++xx) {
		if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
			if (bFoundFirstWidth == false) {
				firstWidth = placingZone->cells [idxCell].tableInHor [xx];
				bFoundFirstWidth = true;
			}
			lastWidth = placingZone->cells [idxCell].tableInHor [xx];
			++realWidthCount;
		}
	}

	if (placingZone->cells [idxCell].objType != TABLEFORM)
		return;

	if (placingZone->cells [idxCell].horLen == 0)
		return;

	if (placingZone->bVertical == true) {
		// ================================================== ���ι���
		// ���� ������ ��ġ
	//	if (placingZone->cells [idxCell].horLen % 100 == 0) {
			pipeLength = placingZone->cells [idxCell].horLen - 100;
			sideMargin = 0.050;
	//	} else {
	//		pipeLength = placingZone->cells [idxCell].horLen - 50;
	//		sideMargin = 0.025;
	//	}

		EasyObjectPlacement rectPipe;

		// �Ϻ�
		rectPipe.init (L("���������v1.0.gsm"), layerInd_RectPipe, floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('x', rectPipe.radAng, sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('y', rectPipe.radAng, -(0.0635 + 0.025), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('z', rectPipe.radAng, 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		moveIn3D ('z', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		elemList.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('z', rectPipe.radAng, 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		elemList.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('z', rectPipe.radAng, -0.031 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		// ���
		if (placingZone->bVertical == true) {
			rectPipe.init (L("���������v1.0.gsm"), layerInd_RectPipe, floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

			moveIn3D ('x', rectPipe.radAng, sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			moveIn3D ('y', rectPipe.radAng, -(0.0635 + 0.025), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			moveIn3D ('z', rectPipe.radAng, placingZone->verLen - 0.300, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

			moveIn3D ('z', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			elemList.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
			moveIn3D ('z', rectPipe.radAng, 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			elemList.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
			moveIn3D ('z', rectPipe.radAng, -0.031 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		}

		// ���� ������ ��ġ
	//	if (placingZone->cells [idxCell].verLenBasic % 100 == 0) {
			pipeLength = (int)(placingZone->verLen * 1000) - 100;
			sideMargin = 0.050;
	//	} else {
	//		pipeLength = placingZone->verLen - 50;
	//		sideMargin = 0.025;
	//	}

		rectPipe.init (L("���������v1.0.gsm"), layerInd_RectPipe, floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('x', rectPipe.radAng, (double)firstWidth / 1000.0 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('y', rectPipe.radAng, -(0.0635 + 0.075), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('z', rectPipe.radAng, sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		moveIn3D ('x', rectPipe.radAng, -0.035, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// ����
		elemList.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('x', rectPipe.radAng, 0.070, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		elemList.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('x', rectPipe.radAng, -0.035 + 0.150 - (double)firstWidth / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		moveIn3D ('x', rectPipe.radAng, (double)(placingZone->cells [idxCell].horLen - lastWidth) / 1000.0 + 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('x', rectPipe.radAng, -0.035, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// ������
		elemList.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('x', rectPipe.radAng, 0.070, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		elemList.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));

		// �ɺ�Ʈ ��Ʈ
		EasyObjectPlacement pinbolt;
		pinbolt.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_PinBolt, floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('y', pinbolt.radAng, -0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

		count = realWidthCount - 1;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) - 1 ; ++xx) {	// �Ϻ�
			if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
				moveIn3D ('x', pinbolt.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
				if (count > 0) {
					pinbolt.radAng += DegreeToRad (90.0);
					elemList.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
					pinbolt.radAng -= DegreeToRad (90.0);

					--count;
				}
			}
		}

		pinbolt.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_PinBolt, floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('y', pinbolt.radAng, -0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		moveIn3D ('z', pinbolt.radAng, placingZone->verLen - 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

		count = realWidthCount - 1;
		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) - 1 ; ++xx) {	// ���
			if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
				moveIn3D ('x', pinbolt.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
				if (count > 0) {
					pinbolt.radAng += DegreeToRad (90.0);
					elemList.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
					pinbolt.radAng -= DegreeToRad (90.0);

					--count;
				}
			}
		}

		// ����ö��
		EasyObjectPlacement join;
		join.init (L("����ö�� (�簢�ͼ�Ȱ��) v1.0.gsm"), layerInd_Join, floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('x', join.radAng, (double)firstWidth / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);
		moveIn3D ('y', join.radAng, -0.1815, &join.posX, &join.posY, &join.posZ);
		moveIn3D ('z', join.radAng, 0.150, &join.posX, &join.posY, &join.posZ);

		elemList.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "������Ʈ", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('x', join.radAng, (double)(150 - firstWidth + placingZone->cells [idxCell].horLen - lastWidth + 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
		elemList.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "������Ʈ", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

		join.init (L("����ö�� (�簢�ͼ�Ȱ��) v1.0.gsm"), layerInd_Join, floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('x', join.radAng, (double)firstWidth / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);
		moveIn3D ('y', join.radAng, -0.1815, &join.posX, &join.posY, &join.posZ);
		moveIn3D ('z', join.radAng, placingZone->verLen - 0.300, &join.posX, &join.posY, &join.posZ);

		elemList.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "������Ʈ", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('x', join.radAng, (double)(150 - firstWidth + placingZone->cells [idxCell].horLen - lastWidth + 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
		elemList.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "������Ʈ", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		
		// ����ǽ�
		EasyObjectPlacement headpiece;
		headpiece.init (L("RS Push-Pull Props ����ǽ� v2.0 (�ξ�� ����).gsm"), layerInd_HeadPiece, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('x', headpiece.radAng, (double)(firstWidth - 150 - 100) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, -0.1725, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('z', headpiece.radAng, 0.300, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

		elemList.Push (headpiece.placeObject (4, "type", APIParT_CString, "Ÿ�� A", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('x', headpiece.radAng, (double)(-(firstWidth - 150 - 100) + placingZone->cells [idxCell].horLen + (-lastWidth + 150 - 100)) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		elemList.Push (headpiece.placeObject (4, "type", APIParT_CString, "Ÿ�� A", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

		headpiece.init (L("RS Push-Pull Props ����ǽ� v2.0 (�ξ�� ����).gsm"), layerInd_HeadPiece, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);
	} else {
	//	// ================================================== ���ι���
	//	// ���� ������ ��ġ
	//	if (placingZone->cells [idxCell].verLenBasic % 100 == 0) {
			pipeLength = (int)(placingZone->verLen * 1000) - 100;
			sideMargin = 0.050;
	//	} else {
	//		pipeLength = placingZone->verLen - 50;
	//		sideMargin = 0.025;
	//	}

		EasyObjectPlacement rectPipe;
		rectPipe.init (L("���������v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('z', rectPipe.radAng, sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('y', rectPipe.radAng, -(0.0635 + 0.025 + 0.050), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('x', rectPipe.radAng, 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		moveIn3D ('x', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// �Ϻ�
		elemList.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('x', rectPipe.radAng, 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		elemList.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('x', rectPipe.radAng, -0.031 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		for (xx = 0 ; xx < realWidthCount - 1 ; ++xx) {												// �߰�
			if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
				moveIn3D ('x', rectPipe.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

				moveIn3D ('x', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
				elemList.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
				moveIn3D ('x', rectPipe.radAng, 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
				elemList.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
				moveIn3D ('x', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
			}
		}
		moveIn3D ('x', rectPipe.radAng, (double)placingZone->cells [idxCell].tableInHor [realWidthCount - 1] / 1000.0 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		moveIn3D ('x', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// ���
		elemList.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('x', rectPipe.radAng, 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		elemList.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('x', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		// ���� ������ ��ġ
	//	if (placingZone->cells [idxCell].horLen % 100 == 0) {
			pipeLength = placingZone->cells [idxCell].horLen - 100;
			sideMargin = 0.050;
	//	} else {
	//		pipeLength = placingZone->cells [idxCell].horLen - 50;
	//		sideMargin = 0.025;
	//	}

		rectPipe.init (L("���������v1.0.gsm"), layerInd_RectPipe, floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('z', rectPipe.radAng, 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('y', rectPipe.radAng, -(0.0635 + 0.025), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		moveIn3D ('x', rectPipe.radAng, sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		moveIn3D ('z', rectPipe.radAng, 0.035, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);		// ����
		elemList.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('z', rectPipe.radAng, -0.070, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
		elemList.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
		moveIn3D ('z', rectPipe.radAng, 0.035, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

		// �ɺ�Ʈ ��Ʈ
		EasyObjectPlacement pinbolt;
		pinbolt.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_PinBolt, floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
		moveIn3D ('y', pinbolt.radAng, -0.1635 - 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

		for (xx = 0 ; xx < realWidthCount - 1 ; ++xx) {
			if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
				moveIn3D ('x', pinbolt.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

				pinbolt.radAng += DegreeToRad (90.0);
				elemList.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
				pinbolt.radAng -= DegreeToRad (90.0);
			}
		}

		// ����ö��
		EasyObjectPlacement join;
		join.init (L("����ö�� (�簢�ͼ�Ȱ��) v1.0.gsm"), layerInd_Join, floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('z', join.radAng, 0.150, &join.posX, &join.posY, &join.posZ);
		moveIn3D ('y', join.radAng, -0.1815, &join.posX, &join.posY, &join.posZ);
		moveIn3D ('x', join.radAng, 0.150, &join.posX, &join.posY, &join.posZ);

		elemList.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "������Ʈ", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
		moveIn3D ('x', join.radAng, (double)(-150 + placingZone->cells [idxCell].horLen - 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
		elemList.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "������Ʈ", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

		// ����ǽ�
		EasyObjectPlacement headpiece;
		headpiece.init (L("RS Push-Pull Props ����ǽ� v2.0 (�ξ�� ����).gsm"), layerInd_HeadPiece, floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

		moveIn3D ('z', headpiece.radAng, 0.250, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('y', headpiece.radAng, -0.1725 + 0.050, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
		moveIn3D ('x', headpiece.radAng, 0.450, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

		elemList.Push (headpiece.placeObject (4, "type", APIParT_CString, "Ÿ�� B", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));

		moveIn3D ('x', headpiece.radAng, (double)(-450 + placingZone->cells [idxCell].horLen - 650) / 1000, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

		elemList.Push (headpiece.placeObject (4, "type", APIParT_CString, "Ÿ�� B", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));

		// ����� ��ü �׷�ȭ
		if (!elemList.IsEmpty ()) {
			GSSize nElems = elemList.GetSize ();
			API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
			if (elemHead != NULL) {
				for (GSIndex i = 0; i < nElems; i++)
					(*elemHead)[i].guid = elemList [i];

				ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

				BMKillHandle ((GSHandle *) &elemHead);
			}
			elemList.Clear ();
		}
	}
}

// ���̺���/������/�ٷ������̼�/����/���� ��ġ�� ���� ���̾�α� (���̺��� ����, ��� ����, ���� �� ����)
short DGCALLBACK lowSideTableformPlacerHandler1 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;
	short	itmPosX, itmPosY;
	short	xx, yy, zz;
	char	buffer [256];
	char	numbuf [32];
	int		cellWidthValue, accumLength;
	const short		maxCol = 50;		// �� �ִ� ����
	double			totalWidth;
	static short	dialogSizeX = 600;			// ���� ���̾�α� ũ�� X
	static short	dialogSizeY = 350;			// ���� ���̾�α� ũ�� Y

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, "���̺���/���ڳ�/������/�ٷ������̼�/����/���� ����");

			//////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			// ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 225, 300, 70, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "Ȯ ��");
			DGShowItem (dialogID, DG_OK);

			// ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 325, 300, 70, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "�� ��");
			DGShowItem (dialogID, DG_CANCEL);

			// ��: ���̺��� ����
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 20, 20, 80, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "���̺��� ����");
			DGShowItem (dialogID, itmIdx);

			// �˾���Ʈ��: ���̺��� ����
			placingZone.POPUP_DIRECTION = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, 105, 15, 70, 23);
			DGSetItemFont (dialogID, placingZone.POPUP_DIRECTION, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, placingZone.POPUP_DIRECTION, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_DIRECTION, DG_POPUP_BOTTOM, "����");
			DGPopUpInsertItem (dialogID, placingZone.POPUP_DIRECTION, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_DIRECTION, DG_POPUP_BOTTOM, "����");
			if (placingZone.bVertical == true)
				DGPopUpSelectItem (dialogID, placingZone.POPUP_DIRECTION, 1);
			else
				DGPopUpSelectItem (dialogID, placingZone.POPUP_DIRECTION, 2);
			DGShowItem (dialogID, placingZone.POPUP_DIRECTION);

			// ��: ���̺��� Ÿ��
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 220, 20, 80, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "���̺��� Ÿ��");
			DGShowItem (dialogID, itmIdx);

			// �˾���Ʈ��: ���̺��� Ÿ��
			placingZone.POPUP_TABLEFORM_TYPE = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, 305, 15, 70, 23);
			DGSetItemFont (dialogID, placingZone.POPUP_TABLEFORM_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, placingZone.POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_TABLEFORM_TYPE, DG_POPUP_BOTTOM, "Ÿ��A");
			DGPopUpSelectItem (dialogID, placingZone.POPUP_TABLEFORM_TYPE, DG_POPUP_TOP);
			DGShowItem (dialogID, placingZone.POPUP_TABLEFORM_TYPE);

			// ��ư: �߰�
			placingZone.BUTTON_ADD_HOR = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 55, 70, 25);
			DGSetItemFont (dialogID, placingZone.BUTTON_ADD_HOR, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, placingZone.BUTTON_ADD_HOR, "�߰�");
			DGShowItem (dialogID, placingZone.BUTTON_ADD_HOR);

			// ��ư: ����
			placingZone.BUTTON_DEL_HOR = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 100, 55, 70, 25);
			DGSetItemFont (dialogID, placingZone.BUTTON_DEL_HOR, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, placingZone.BUTTON_DEL_HOR, "����");
			DGShowItem (dialogID, placingZone.BUTTON_DEL_HOR);

			// ��: ���� �ʺ�
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 220, 60, 70, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "���� �ʺ�");
			DGShowItem (dialogID, itmIdx);

			// Edit��Ʈ��: ���� �ʺ�
			placingZone.EDITCONTROL_REMAIN_WIDTH = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 300, 55, 80, 25);
			DGDisableItem (dialogID, placingZone.EDITCONTROL_REMAIN_WIDTH);
			DGShowItem (dialogID, placingZone.EDITCONTROL_REMAIN_WIDTH);

			// ��: ���� ����
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 400, 60, 70, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "���� ����");
			DGShowItem (dialogID, itmIdx);

			// Edit��Ʈ��: ���� ����
			placingZone.EDITCONTROL_CURRENT_HEIGHT = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 480, 55, 80, 25);
			DGDisableItem (dialogID, placingZone.EDITCONTROL_CURRENT_HEIGHT);
			DGShowItem (dialogID, placingZone.EDITCONTROL_CURRENT_HEIGHT);

			//////////////////////////////////////////////////////////// �� ���� �ʱ�ȭ
			placingZone.initCells (&placingZone, placingZone.bVertical);

			//////////////////////////////////////////////////////////// ������ ��ġ (���� ���� ��ư)

			// ���� ���ڳ��ǳ�/�ƿ��ڳ��ǳ�/�ƿ��ڳʾޱ�
			// ��ư
			placingZone.BUTTON_LCORNER = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 137, 71, 66);
			DGSetItemFont (dialogID, placingZone.BUTTON_LCORNER, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, placingZone.BUTTON_LCORNER, "����");
			DGShowItem (dialogID, placingZone.BUTTON_LCORNER);
			DGDisableItem (dialogID, placingZone.BUTTON_LCORNER);
			// ��ü Ÿ�� (�˾���Ʈ��)
			placingZone.POPUP_OBJ_TYPE_LCORNER = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, 20, 137 - 25, 70, 23);
			DGSetItemFont (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER, DG_IS_EXTRASMALL | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER, DG_POPUP_BOTTOM, "����");
			DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER, DG_POPUP_BOTTOM, "���ڳ��ǳ�");
			DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER, DG_POPUP_BOTTOM, "�ƿ��ڳ��ǳ�");
			DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER, DG_POPUP_BOTTOM, "�ƿ��ڳʾޱ�");
			DGPopUpSelectItem (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER, DG_POPUP_TOP);
			DGShowItem (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER);
			// �ʺ� (Edit��Ʈ��)
			placingZone.EDITCONTROL_WIDTH_LCORNER = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 20, 137 + 68, 70, 23);
			DGShowItem (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER);
			DGDisableItem (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER);

			// �Ϲ� ��: �⺻���� ���̺���
			itmPosX = 90;
			itmPosY = 137;
			for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
				// ��ư
				placingZone.BUTTON_OBJ [xx] = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 71, 66);
				DGSetItemFont (dialogID, placingZone.BUTTON_OBJ [xx], DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, placingZone.BUTTON_OBJ [xx], "���̺���");
				DGShowItem (dialogID, placingZone.BUTTON_OBJ [xx]);

				// ��ü Ÿ�� (�˾���Ʈ��)
				placingZone.POPUP_OBJ_TYPE [xx] = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY - 25, 70, 23);
				DGSetItemFont (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_IS_EXTRASMALL | DG_IS_PLAIN);
				DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "����");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "���̺���");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "������");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "�ٷ������̼�");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "����");
				DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "����");
				DGPopUpSelectItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_TOP+1);
				DGShowItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx]);

				// �ʺ� (�˾���Ʈ��)
				placingZone.POPUP_WIDTH [xx] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY + 68, 70, 23);
				DGSetItemFont (dialogID, placingZone.POPUP_WIDTH [xx], DG_IS_LARGE | DG_IS_PLAIN);
				if (placingZone.bVertical == true) {
					for (yy = 0 ; yy < sizeof (placingZone.presetWidthVertical_tableform) / sizeof (int) ; ++yy) {
						DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
						_itoa (placingZone.presetWidthVertical_tableform [yy], numbuf, 10);
						DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
					}
				} else {
					for (yy = 0 ; yy < sizeof (placingZone.presetWidthHorizontal_tableform) / sizeof (int) ; ++yy) {
						DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
						_itoa (placingZone.presetWidthHorizontal_tableform [yy], numbuf, 10);
						DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
					}
				}
				DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
				DGShowItem (dialogID, placingZone.POPUP_WIDTH [xx]);

				// �ʺ� (�˾���Ʈ��) - ó������ ����
				placingZone.EDITCONTROL_WIDTH [xx] = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, itmPosY + 68, 70, 23);
				DGHideItem (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);

				itmPosX += 70;
			}

			// ������ ���ڳ��ǳ�/�ƿ��ڳ��ǳ�/�ƿ��ڳʾޱ�
			// ��ư
			placingZone.BUTTON_RCORNER = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, 137, 71, 66);
			DGSetItemFont (dialogID, placingZone.BUTTON_RCORNER, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, placingZone.BUTTON_RCORNER, "����");
			DGShowItem (dialogID, placingZone.BUTTON_RCORNER);
			DGDisableItem (dialogID, placingZone.BUTTON_RCORNER);
			// ��ü Ÿ�� (�˾���Ʈ��)
			placingZone.POPUP_OBJ_TYPE_RCORNER = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, 137 - 25, 70, 23);
			DGSetItemFont (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_IS_EXTRASMALL | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM, "����");
			DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM, "���ڳ��ǳ�");
			DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM, "�ƿ��ڳ��ǳ�");
			DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM, "�ƿ��ڳʾޱ�");
			DGPopUpSelectItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_TOP);
			DGShowItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER);
			// �ʺ� (Edit��Ʈ��)
			placingZone.EDITCONTROL_WIDTH_RCORNER = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, 137 + 68, 70, 23);
			DGShowItem (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);
			DGDisableItem (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);

			//////////////////////////////////////////////////////////// ���̾�α� ũ�� ����, ���� �ʺ� �� ���� ���
			// ���̾�α� ũ�� ����
			dialogSizeX = 600;
			dialogSizeY = 350;
			if (placingZone.nCellsInHor >= 5) {
				DGSetDialogSize (dialogID, DG_CLIENT, dialogSizeX + 70 * (placingZone.nCellsInHor - 5), dialogSizeY, DG_TOPLEFT, true);
			}

			// ���� �ʺ� ���
			totalWidth = 0.0;
			if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER) != NOCORNER)	totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER);
			if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER) != NOCORNER)	totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);
			for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
				if ((DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) || (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM))
					totalWidth += atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH [xx])).ToCStr ().Get ()) / 1000;
				else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == NONE)
					totalWidth += 0.0;
				else
					totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
			}
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_WIDTH, placingZone.horLen - totalWidth);

			// ���� ���� ���
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_CURRENT_HEIGHT, placingZone.verLen);

			break;

		case DG_MSG_CHANGE:
			// ����/���� ������ ��
			if (item == placingZone.POPUP_DIRECTION) {
				strcpy (buffer, DGPopUpGetItemText (dialogID, placingZone.POPUP_DIRECTION, DGPopUpGetSelected (dialogID, placingZone.POPUP_DIRECTION)).ToCStr ().Get ());

				// ������ ���
				if (my_strcmp (buffer, "����") == 0) {
					// �� ���� �ʱ�ȭ
					placingZone.initCells (&placingZone, false);
					placingZone.bVertical = false;

					for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
						if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) {
							// �˾� ���� ����
							DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// �˾� ���� �ٽ� ä���
							for (yy = 0 ; yy < sizeof (placingZone.presetWidthHorizontal_tableform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetWidthHorizontal_tableform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
						} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM) {
							// �˾� ���� ����
							DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// �˾� ���� �ٽ� ä���
							for (yy = 0 ; yy < sizeof (placingZone.presetHeightHorizontal_euroform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetHeightHorizontal_euroform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
						}
					}

				// ������ ���
				} else {
					// �� ���� �ʱ�ȭ
					placingZone.initCells (&placingZone, true);
					placingZone.bVertical = true;

					for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
						if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) {
							// �˾� ���� ����
							DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// �˾� ���� �ٽ� ä���
							for (yy = 0 ; yy < sizeof (placingZone.presetWidthVertical_tableform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetWidthVertical_tableform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
						} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM) {
							// �˾� ���� ����
							DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// �˾� ���� �ٽ� ä���
							for (yy = 0 ; yy < sizeof (placingZone.presetWidthVertical_euroform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetWidthVertical_euroform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
						}
					}
				}
			}

			// ���ڳ��ǳ�/�ƿ��ڳ��ǳ�/�ƿ��ڳʾޱ� �����
			if (item == placingZone.POPUP_OBJ_TYPE_LCORNER) {
				if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER) == NOCORNER) {
					DGSetItemText (dialogID, placingZone.BUTTON_LCORNER, "����");
					DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER, 0.0);
					DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER, 0.0);
					DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER, 0.0);
					DGDisableItem (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER);
				} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER) == INCORNER_PANEL) {
					DGSetItemText (dialogID, placingZone.BUTTON_LCORNER, "���ڳ��ǳ�");
					DGEnableItem (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER);
					DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER, 0.080);
					DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER, 0.500);
				} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER) == OUTCORNER_PANEL) {
					DGSetItemText (dialogID, placingZone.BUTTON_LCORNER, "�ƿ��ڳ��ǳ�");
					DGEnableItem (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER);
					DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER, 0.080);
					DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER, 0.500);
				} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER) == OUTCORNER_ANGLE) {
					DGSetItemText (dialogID, placingZone.BUTTON_LCORNER, "�ƿ��ڳʾޱ�");
					DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER, 0.0);
					DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER, 0.0);
					DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER, 0.0);
					DGDisableItem (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER);
				}
			}

			if (item == placingZone.POPUP_OBJ_TYPE_RCORNER) {
				if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER) == NOCORNER) {
					DGSetItemText (dialogID, placingZone.BUTTON_RCORNER, "����");
					DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER, 0.0);
					DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER, 0.0);
					DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER, 0.0);
					DGDisableItem (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);
				} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER) == INCORNER_PANEL) {
					DGSetItemText (dialogID, placingZone.BUTTON_RCORNER, "���ڳ��ǳ�");
					DGEnableItem (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);
					DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER, 0.080);
					DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER, 0.500);
				} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER) == OUTCORNER_PANEL) {
					DGSetItemText (dialogID, placingZone.BUTTON_RCORNER, "�ƿ��ڳ��ǳ�");
					DGEnableItem (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);
					DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER, 0.080);
					DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER, 0.500);
				} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER) == OUTCORNER_ANGLE) {
					DGSetItemText (dialogID, placingZone.BUTTON_RCORNER, "�ƿ��ڳʾޱ�");
					DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER, 0.0);
					DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER, 0.0);
					DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER, 0.0);
					DGDisableItem (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);
				}
			}

			// ��ü Ÿ�� ������ ��
			for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
				if (item == placingZone.POPUP_OBJ_TYPE [xx]) {
					// �ش� ��ư�� �̸� ����
					DGSetItemText (dialogID, placingZone.BUTTON_OBJ [xx], DGPopUpGetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx])));
					
					// ����/���� ���� ���ο� ���� �˾���Ʈ���� ���빰�� �ٲ�
					if (placingZone.bVertical == false) {
						if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) {
							// �˾� ���� ����
							DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// �˾� ���� �ٽ� ä���
							for (yy = 0 ; yy < sizeof (placingZone.presetWidthHorizontal_tableform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetWidthHorizontal_tableform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
						} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM) {
							// �˾� ���� ����
							DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// �˾� ���� �ٽ� ä���
							for (yy = 0 ; yy < sizeof (placingZone.presetHeightHorizontal_euroform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetHeightHorizontal_euroform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
						}
					} else {
						if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) {
							// �˾� ���� ����
							DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// �˾� ���� �ٽ� ä���
							for (yy = 0 ; yy < sizeof (placingZone.presetWidthVertical_tableform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetWidthVertical_tableform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
						} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM) {
							// �˾� ���� ����
							DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

							// �˾� ���� �ٽ� ä���
							for (yy = 0 ; yy < sizeof (placingZone.presetWidthVertical_euroform) / sizeof (int) ; ++yy) {
								DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
								_itoa (placingZone.presetWidthVertical_euroform [yy], numbuf, 10);
								DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
						}
					}

					// ���̺��� Ÿ���� �ƴϸ� ��ư�� ���
					if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) != TABLEFORM)
						DGDisableItem (dialogID, placingZone.BUTTON_OBJ [xx]);
					else
						DGEnableItem (dialogID, placingZone.BUTTON_OBJ [xx]);

					// ���̺���/�������̸� �ʺ� �˾���Ʈ��, �� �ܿ��� Edit��Ʈ��, ������ ��� ����
					if ((DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) || (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM)) {
						if (!DGIsItemVisible (dialogID, placingZone.POPUP_WIDTH [xx]))			DGShowItem (dialogID, placingZone.POPUP_WIDTH [xx]);
						if (DGIsItemVisible (dialogID, placingZone.EDITCONTROL_WIDTH [xx]))		DGHideItem (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
					} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == NONE) {
						if (DGIsItemVisible (dialogID, placingZone.EDITCONTROL_WIDTH [xx]))		DGHideItem (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
						if (DGIsItemVisible (dialogID, placingZone.POPUP_WIDTH [xx]))			DGHideItem (dialogID, placingZone.POPUP_WIDTH [xx]);
					} else {
						if (!DGIsItemVisible (dialogID, placingZone.EDITCONTROL_WIDTH [xx]))	DGShowItem (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
						if (DGIsItemVisible (dialogID, placingZone.POPUP_WIDTH [xx]))			DGHideItem (dialogID, placingZone.POPUP_WIDTH [xx]);

						// �ٷ������̼�, ����, ������ ���� ���� ������ �ּ�, �ִ밪
						if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == FILLERSP) {
							DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx], 0.010);
							DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx], 0.050);
						} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == PLYWOOD) {
							DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx], 0.090);
							DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx], 1.220);
						} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TIMBER) {
							DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx], 0.005);
							DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx], 1.000);
						}
					}
				}
			}

			// ���̺��� �ʺ� ������ ��
			for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
				if (item == placingZone.POPUP_WIDTH [xx]) {
					if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) {
						cellWidthValue = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], (DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH [xx]))).ToCStr ().Get ());
						if (placingZone.bVertical == true) {
							for (yy = 0 ; yy < sizeof (placingZone.presetWidthVertical_tableform) / sizeof (int) ; ++yy) {
								if (cellWidthValue == placingZone.presetWidthVertical_tableform [yy]) {
									for (zz = 0 ; zz < sizeof (placingZone.cells [xx].tableInHor) / sizeof (int) ; ++zz) {
										if ((zz >= 0) && (zz < placingZone.presetWidth_config_vertical [yy][0]))
											placingZone.cells [xx].tableInHor [zz] = placingZone.presetWidth_config_vertical [yy][zz+1];
										else
											placingZone.cells [xx].tableInHor [zz] = 0;
									}
								}
							}
						} else {
							for (yy = 0 ; yy < sizeof (placingZone.presetWidthHorizontal_tableform) / sizeof (int) ; ++yy) {
								if (cellWidthValue == placingZone.presetWidthHorizontal_tableform [yy]) {
									for (zz = 0 ; zz < sizeof (placingZone.cells [xx].tableInHor) / sizeof (int) ; ++zz) {
										if ((zz >= 0) && (zz < placingZone.presetWidth_config_horizontal [yy][0]))
											placingZone.cells [xx].tableInHor [zz] = placingZone.presetWidth_config_horizontal [yy][zz+1];
										else
											placingZone.cells [xx].tableInHor [zz] = 0;
									}
								}
							}
						}
						placingZone.cells [xx].horLen = cellWidthValue;
					}
				}
			}

			// ���� �ʺ� ���
			totalWidth = 0.0;
			if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER) != NOCORNER)	totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER);
			if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER) != NOCORNER)	totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);
			for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
				if ((DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) || (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM))
					totalWidth += atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH [xx])).ToCStr ().Get ()) / 1000;
				else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == NONE)
					totalWidth += 0.0;
				else
					totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
			}
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_WIDTH, placingZone.horLen - totalWidth);

			break;

		case DG_MSG_CLICK:
			// Ȯ�� ��ư
			if (item == DG_OK) {
				// ���̺��� ����
				strcpy (buffer, DGPopUpGetItemText (dialogID, placingZone.POPUP_DIRECTION, DGPopUpGetSelected (dialogID, placingZone.POPUP_DIRECTION)).ToCStr ().Get ());
				if (my_strcmp (buffer, "����") == 0)
					placingZone.bVertical = true;
				else
					placingZone.bVertical = false;

				// ���̺��� Ÿ��
				strcpy (buffer, DGPopUpGetItemText (dialogID, placingZone.POPUP_TABLEFORM_TYPE, DGPopUpGetSelected (dialogID, placingZone.POPUP_TABLEFORM_TYPE)).ToCStr ().Get ());
				if (my_strcmp (buffer, "Ÿ��A") == 0)
					placingZone.tableformType = 1;

				// ���ڳ��ǳ�/�ƿ��ڳ��ǳ�/�ƿ��ڳʾޱ� ���� �� ����
				placingZone.typeLcorner = DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER);
				placingZone.lenLcorner = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER);
				placingZone.typeRcorner = DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER);
				placingZone.lenRcorner = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);

				// �� ���� ������Ʈ
				for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
					// ��ü Ÿ��
					placingZone.cells [xx].objType = DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]);

					// ���� ���� ���� ����
					if (placingZone.cells [xx].objType == NONE) {
						// ���� ���� 0
						placingZone.cells [xx].horLen = 0;

						// ���̺� �� ���� ���� ��� 0
						for (yy = 0 ; yy < sizeof (placingZone.cells [xx].tableInHor) / sizeof (int) ; ++yy)
							placingZone.cells [xx].tableInHor [yy] = 0;

					} else if (placingZone.cells [xx].objType == EUROFORM) {
						// ������ �ʺ�
						placingZone.cells [xx].horLen = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH [xx])).ToCStr ().Get ());

						// ���̺� �� ���� ���� ��� 0
						for (yy = 0 ; yy < sizeof (placingZone.cells [xx].tableInHor) / sizeof (int) ; ++yy)
							placingZone.cells [xx].tableInHor [yy] = 0;

					} else if ((placingZone.cells [xx].objType == FILLERSP) || (placingZone.cells [xx].objType == PLYWOOD) || (placingZone.cells [xx].objType == TIMBER)) {
						// �ٷ������̼�, ����, ������ �ʺ�
						placingZone.cells [xx].horLen = (int)(DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx]) * 1000);

						// ���̺� �� ���� ���� ��� 0
						for (yy = 0 ; yy < sizeof (placingZone.cells [xx].tableInHor) / sizeof (int) ; ++yy)
							placingZone.cells [xx].tableInHor [yy] = 0;
					}
				}

				// ������ ���� ����
				if (placingZone.verLen < 0.200 + EPS)		placingZone.verLen = 0.200;
				else if (placingZone.verLen < 0.300 + EPS)	placingZone.verLen = 0.300;
				else if (placingZone.verLen < 0.400 + EPS)	placingZone.verLen = 0.400;
				else if (placingZone.verLen < 0.450 + EPS)	placingZone.verLen = 0.450;
				else if (placingZone.verLen < 0.500 + EPS)	placingZone.verLen = 0.500;
				else if (placingZone.verLen < 0.600 + EPS)	placingZone.verLen = 0.600;
				else if (placingZone.verLen < 0.900 + EPS)	placingZone.verLen = 0.900;
				else										placingZone.verLen = 1.200;

				// ���̾� ����
				bLayerInd_Euroform = true;			// ������ �׻� On
				bLayerInd_RectPipe = false;
				bLayerInd_PinBolt = false;
				bLayerInd_WallTie = false;
				bLayerInd_HeadPiece = false;
				bLayerInd_Join = false;

				bLayerInd_SlabTableform = false;
				bLayerInd_Profile = false;

				bLayerInd_Steelform = false;
				bLayerInd_Plywood = false;
				bLayerInd_Timber = false;
				bLayerInd_IncornerPanel = false;
				bLayerInd_OutcornerAngle = false;
				bLayerInd_OutcornerPanel = false;
				bLayerInd_RectpipeHanger = false;
				bLayerInd_EuroformHook = false;
				bLayerInd_CrossJointBar = false;
				bLayerInd_Fillersp = false;
				bLayerInd_BlueClamp = false;
				bLayerInd_BlueTimberRail = false;
				bLayerInd_Hidden = false;

				// �ٷ������̼�, ����, ����
				for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
					if (placingZone.cells [xx].objType == FILLERSP)	bLayerInd_Fillersp = true;
					if (placingZone.cells [xx].objType == PLYWOOD)	bLayerInd_Plywood = true;
					if (placingZone.cells [xx].objType == TIMBER)	bLayerInd_Timber = true;
				}

				// ���ڳ��ǳ�
				if ((placingZone.typeLcorner == INCORNER_PANEL) || (placingZone.typeRcorner == INCORNER_PANEL))
					bLayerInd_IncornerPanel = true;

				// �ƿ��ڳ��ǳ�
				if ((placingZone.typeLcorner == OUTCORNER_PANEL) || (placingZone.typeRcorner == OUTCORNER_PANEL))
					bLayerInd_OutcornerPanel = true;

				// �ƿ��ڳʾޱ�
				if ((placingZone.typeLcorner == OUTCORNER_ANGLE) || (placingZone.typeRcorner == OUTCORNER_ANGLE))
					bLayerInd_OutcornerAngle = true;

				if (placingZone.tableformType == 1) {
					bLayerInd_Euroform = true;
					bLayerInd_RectPipe = true;
					bLayerInd_PinBolt = true;
					bLayerInd_HeadPiece = true;
					bLayerInd_Join = true;
				}

				placingZone.adjustCellsPosition (&placingZone);		// ���� ��ġ ����

			} else if (item == DG_CANCEL) {
				// �ƹ� �۾��� ���� ����
			} else {
				if ((item == placingZone.BUTTON_ADD_HOR) || (item == placingZone.BUTTON_DEL_HOR)) {
					// �߰� ��ư Ŭ��
					if (item == placingZone.BUTTON_ADD_HOR) {
						if (placingZone.nCellsInHor < maxCol) {
							// ���� ���ڳ� ��ư�� �����
							DGRemoveDialogItem (dialogID, placingZone.BUTTON_RCORNER);
							DGRemoveDialogItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER);
							DGRemoveDialogItem (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);

							// ������ �� ��ư �����ʿ� ���ο� �� ��ư�� �߰��ϰ�
							itmPosX = 90 + (70 * placingZone.nCellsInHor);
							itmPosY = 137;
							// ��ư
							placingZone.BUTTON_OBJ [placingZone.nCellsInHor] = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 71, 66);
							DGSetItemFont (dialogID, placingZone.BUTTON_OBJ [placingZone.nCellsInHor], DG_IS_LARGE | DG_IS_PLAIN);
							DGSetItemText (dialogID, placingZone.BUTTON_OBJ [placingZone.nCellsInHor], "���̺���");
							DGShowItem (dialogID, placingZone.BUTTON_OBJ [placingZone.nCellsInHor]);

							// ��ü Ÿ�� (�˾���Ʈ��)
							placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor] = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY - 25, 70, 23);
							DGSetItemFont (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_IS_EXTRASMALL | DG_IS_PLAIN);
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM, "����");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM, "���̺���");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM, "������");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM, "�ٷ������̼�");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM, "����");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM, "����");
							DGPopUpSelectItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_TOP+1);
							DGShowItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor]);

							// �ʺ� (�˾���Ʈ��)
							placingZone.POPUP_WIDTH [placingZone.nCellsInHor] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY + 68, 70, 23);
							DGSetItemFont (dialogID, placingZone.POPUP_WIDTH [placingZone.nCellsInHor], DG_IS_LARGE | DG_IS_PLAIN);
							if (placingZone.bVertical == true) {
								for (yy = 0 ; yy < sizeof (placingZone.presetWidthVertical_tableform) / sizeof (int) ; ++yy) {
									DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
									_itoa (placingZone.presetWidthVertical_tableform [yy], numbuf, 10);
									DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [placingZone.nCellsInHor], DG_POPUP_BOTTOM, numbuf);
								}
							} else {
								for (yy = 0 ; yy < sizeof (placingZone.presetWidthHorizontal_tableform) / sizeof (int) ; ++yy) {
									DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
									_itoa (placingZone.presetWidthHorizontal_tableform [yy], numbuf, 10);
									DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [placingZone.nCellsInHor], DG_POPUP_BOTTOM, numbuf);
								}
							}
							DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCellsInHor], DG_POPUP_TOP);
							DGShowItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCellsInHor]);

							// �ʺ� (�˾���Ʈ��) - ó������ ����
							placingZone.EDITCONTROL_WIDTH [placingZone.nCellsInHor] = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, itmPosY + 68, 70, 23);

							itmPosX += 70;

							// ���� ���ڳ� ��ư�� ������ ���� ����
							// ��ư
							placingZone.BUTTON_RCORNER = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, 137, 71, 66);
							DGSetItemFont (dialogID, placingZone.BUTTON_RCORNER, DG_IS_LARGE | DG_IS_PLAIN);
							DGSetItemText (dialogID, placingZone.BUTTON_RCORNER, "����");
							DGShowItem (dialogID, placingZone.BUTTON_RCORNER);
							DGDisableItem (dialogID, placingZone.BUTTON_RCORNER);
							// ��ü Ÿ�� (�˾���Ʈ��)
							placingZone.POPUP_OBJ_TYPE_RCORNER = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, 137 - 25, 70, 23);
							DGSetItemFont (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_IS_EXTRASMALL | DG_IS_PLAIN);
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM, "����");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM, "���ڳ��ǳ�");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM, "�ƿ��ڳ��ǳ�");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM, "�ƿ��ڳʾޱ�");
							DGPopUpSelectItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_TOP);
							DGShowItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER);
							// �ʺ� (Edit��Ʈ��)
							placingZone.EDITCONTROL_WIDTH_RCORNER = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, 137 + 68, 70, 23);
							DGShowItem (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);
							DGDisableItem (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);

							++placingZone.nCellsInHor;
						}
					}

					// ���� ��ư Ŭ��
					else if (item == placingZone.BUTTON_DEL_HOR) {
						if (placingZone.nCellsInHor > 1) {
							// ���� ���ڳ� ��ư�� �����
							DGRemoveDialogItem (dialogID, placingZone.BUTTON_RCORNER);
							DGRemoveDialogItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER);
							DGRemoveDialogItem (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);

							// ������ �� ��ư�� �����
							DGRemoveDialogItem (dialogID, placingZone.BUTTON_OBJ [placingZone.nCellsInHor - 1]);
							DGRemoveDialogItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor - 1]);
							DGRemoveDialogItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCellsInHor - 1]);
							DGRemoveDialogItem (dialogID, placingZone.EDITCONTROL_WIDTH [placingZone.nCellsInHor - 1]);

							// ���� ���ڳ� ��ư�� ������ ���� ����
							itmPosX = 90 + (70 * (placingZone.nCellsInHor - 1));
							itmPosY = 137;
							// ��ư
							placingZone.BUTTON_RCORNER = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, 137, 71, 66);
							DGSetItemFont (dialogID, placingZone.BUTTON_RCORNER, DG_IS_LARGE | DG_IS_PLAIN);
							DGSetItemText (dialogID, placingZone.BUTTON_RCORNER, "����");
							DGShowItem (dialogID, placingZone.BUTTON_RCORNER);
							DGDisableItem (dialogID, placingZone.BUTTON_RCORNER);
							// ��ü Ÿ�� (�˾���Ʈ��)
							placingZone.POPUP_OBJ_TYPE_RCORNER = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, 137 - 25, 70, 23);
							DGSetItemFont (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_IS_EXTRASMALL | DG_IS_PLAIN);
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM, "����");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM, "���ڳ��ǳ�");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM, "�ƿ��ڳ��ǳ�");
							DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_BOTTOM, "�ƿ��ڳʾޱ�");
							DGPopUpSelectItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER, DG_POPUP_TOP);
							DGShowItem (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER);
							// �ʺ� (Edit��Ʈ��)
							placingZone.EDITCONTROL_WIDTH_RCORNER = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, 137 + 68, 70, 23);
							DGShowItem (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);
							DGDisableItem (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);

							--placingZone.nCellsInHor;
						}
					}

				} else {
					// ��ü ��ư Ŭ�� (���̺����� ��쿡�� ��ȿ��)
					for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
						if ((item == placingZone.BUTTON_OBJ [xx]) && (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM)) {
							clickedIndex = xx;
							if (placingZone.bVertical == true) {
								// ���̺��� Ÿ�� (���� ����)�� ���, 3��° ���̾�α�(���ι���) ����
								result = DGBlankModalDialog (770, 180, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, lowSideTableformPlacerHandler3_Vertical, (short) 0);
							} else {
								// ���̺��� Ÿ�� (���� ����)�� ���, 3��° ���̾�α�(���ι���) ����
								result = DGBlankModalDialog (770, 180, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, lowSideTableformPlacerHandler3_Horizontal, (short) 0);
							}

							// �޺��ڽ��� ��ü �ʺ� �� ����
							accumLength = 0;

							for (yy = 0 ; yy < sizeof (placingZone.cells [xx].tableInHor) / sizeof (int) ; ++yy)
								accumLength += placingZone.cells [xx].tableInHor [yy];

							for (yy = 1 ; yy <= DGPopUpGetItemCount (dialogID, placingZone.POPUP_WIDTH [xx]) ; ++yy) {
								if (accumLength == atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], yy).ToCStr ().Get ())) {
									DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], yy);
									break;
								}
							}
						}
					}
				}

				// ���̾�α� ũ�� ����
				dialogSizeX = 600;
				dialogSizeY = 350;
				if (placingZone.nCellsInHor >= 5) {
					DGSetDialogSize (dialogID, DG_CLIENT, dialogSizeX + 70 * (placingZone.nCellsInHor - 5), dialogSizeY, DG_TOPLEFT, true);
				}

				// ���� �ʺ� ���
				totalWidth = 0.0;
				if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_LCORNER) != NOCORNER)	totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_LCORNER);
				if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE_RCORNER) != NOCORNER)	totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH_RCORNER);
				for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
					if ((DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) || (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM))
						totalWidth += atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH [xx])).ToCStr ().Get ()) / 1000;
					else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == NONE)
						totalWidth += 0.0;
					else
						totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
				}
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_WIDTH, placingZone.horLen - totalWidth);

				item = 0;
			}
			break;

		case DG_MSG_CLOSE:
			switch (item) {
				case DG_CLOSEBOX:
					break;
			}
	}

	result = item;

	return	result;
}

// ��ü�� ���̾ �����ϱ� ���� ���̾�α�
short DGCALLBACK lowSideTableformPlacerHandler2 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	xx;
	short	result;
	API_UCCallbackType	ucb;

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, "������ ���̾� �����ϱ�");

			//////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			// ���� ��ư
			DGSetItemText (dialogID, DG_OK, "Ȯ ��");

			// ���� ��ư
			DGSetItemText (dialogID, DG_CANCEL, "�� ��");

			// üũ�ڽ�: ���̾� ����
			DGSetItemText (dialogID, CHECKBOX_LAYER_COUPLING, "���̾� ����");
			DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, TRUE);

			// ���̾� ���� ��
			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, "���纰 ���̾� ����");
			DGSetItemText (dialogID, LABEL_LAYER_SLABTABLEFORM, "������ ���̺���");
			DGSetItemText (dialogID, LABEL_LAYER_PROFILE, "C����");
			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM, "������");
			DGSetItemText (dialogID, LABEL_LAYER_RECTPIPE, "��� ������");
			DGSetItemText (dialogID, LABEL_LAYER_PINBOLT, "�ɺ�Ʈ ��Ʈ");
			DGSetItemText (dialogID, LABEL_LAYER_WALLTIE, "��ü Ÿ��");
			DGSetItemText (dialogID, LABEL_LAYER_JOIN, "����ö��");
			DGSetItemText (dialogID, LABEL_LAYER_HEADPIECE, "����ǽ�");
			DGSetItemText (dialogID, LABEL_LAYER_STEELFORM, "��ƿ��");
			DGSetItemText (dialogID, LABEL_LAYER_PLYWOOD, "����");
			DGSetItemText (dialogID, LABEL_LAYER_TIMBER, "����");
			DGSetItemText (dialogID, LABEL_LAYER_FILLERSP, "�ٷ������̼�");
			DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER_ANGLE, "�ƿ��ڳʾޱ�");
			DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER_PANEL, "�ƿ��ڳ��ǳ�");
			DGSetItemText (dialogID, LABEL_LAYER_INCORNER_PANEL, "���ڳ��ǳ�");
			DGSetItemText (dialogID, LABEL_LAYER_RECTPIPE_HANGER, "�����������");
			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM_HOOK, "������ ��ũ");
			DGSetItemText (dialogID, LABEL_LAYER_CROSS_JOINT_BAR, "��������Ʈ��");
			DGSetItemText (dialogID, LABEL_LAYER_BLUE_CLAMP, "���Ŭ����");
			DGSetItemText (dialogID, LABEL_LAYER_BLUE_TIMBER_RAIL, "�����");
			DGSetItemText (dialogID, LABEL_LAYER_HIDDEN, "����");

			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 120, 700, 160, 25);
			DGSetItemFont (dialogID, BUTTON_AUTOSET, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_AUTOSET, "���̾� �ڵ� ����");
			DGShowItem (dialogID, BUTTON_AUTOSET);

			// ���� ��Ʈ�� �ʱ�ȭ
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_LAYER_SLABTABLEFORM;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, 1);
			if (bLayerInd_SlabTableform == true) {
				DGEnableItem (dialogID, LABEL_LAYER_SLABTABLEFORM);
				DGEnableItem (dialogID, USERCONTROL_LAYER_SLABTABLEFORM);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_SLABTABLEFORM);
				DGDisableItem (dialogID, USERCONTROL_LAYER_SLABTABLEFORM);
			}

			ucb.itemID	 = USERCONTROL_LAYER_PROFILE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, 1);
			if (bLayerInd_Profile == true) {
				DGEnableItem (dialogID, LABEL_LAYER_PROFILE);
				DGEnableItem (dialogID, USERCONTROL_LAYER_PROFILE);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_PROFILE);
				DGDisableItem (dialogID, USERCONTROL_LAYER_PROFILE);
			}

			ucb.itemID	 = USERCONTROL_LAYER_EUROFORM;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, 1);
			if (bLayerInd_Euroform == true) {
				DGEnableItem (dialogID, LABEL_LAYER_EUROFORM);
				DGEnableItem (dialogID, USERCONTROL_LAYER_EUROFORM);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_EUROFORM);
				DGDisableItem (dialogID, USERCONTROL_LAYER_EUROFORM);
			}

			ucb.itemID	 = USERCONTROL_LAYER_RECTPIPE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, 1);
			if (bLayerInd_RectPipe == true) {
				DGEnableItem (dialogID, LABEL_LAYER_RECTPIPE);
				DGEnableItem (dialogID, USERCONTROL_LAYER_RECTPIPE);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_RECTPIPE);
				DGDisableItem (dialogID, USERCONTROL_LAYER_RECTPIPE);
			}

			ucb.itemID	 = USERCONTROL_LAYER_PINBOLT;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, 1);
			if (bLayerInd_PinBolt == true) {
				DGEnableItem (dialogID, LABEL_LAYER_PINBOLT);
				DGEnableItem (dialogID, USERCONTROL_LAYER_PINBOLT);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_PINBOLT);
				DGDisableItem (dialogID, USERCONTROL_LAYER_PINBOLT);
			}

			ucb.itemID	 = USERCONTROL_LAYER_WALLTIE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, 1);
			if (bLayerInd_WallTie == true) {
				DGEnableItem (dialogID, LABEL_LAYER_WALLTIE);
				DGEnableItem (dialogID, USERCONTROL_LAYER_WALLTIE);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_WALLTIE);
				DGDisableItem (dialogID, USERCONTROL_LAYER_WALLTIE);
			}

			ucb.itemID	 = USERCONTROL_LAYER_JOIN;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, 1);
			if (bLayerInd_Join == true) {
				DGEnableItem (dialogID, LABEL_LAYER_JOIN);
				DGEnableItem (dialogID, USERCONTROL_LAYER_JOIN);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_JOIN);
				DGDisableItem (dialogID, USERCONTROL_LAYER_JOIN);
			}

			ucb.itemID	 = USERCONTROL_LAYER_HEADPIECE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, 1);
			if (bLayerInd_HeadPiece == true) {
				DGEnableItem (dialogID, LABEL_LAYER_HEADPIECE);
				DGEnableItem (dialogID, USERCONTROL_LAYER_HEADPIECE);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_HEADPIECE);
				DGDisableItem (dialogID, USERCONTROL_LAYER_HEADPIECE);
			}

			ucb.itemID	 = USERCONTROL_LAYER_STEELFORM;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, 1);
			if (bLayerInd_Steelform == true) {
				DGEnableItem (dialogID, LABEL_LAYER_STEELFORM);
				DGEnableItem (dialogID, USERCONTROL_LAYER_STEELFORM);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_STEELFORM);
				DGDisableItem (dialogID, USERCONTROL_LAYER_STEELFORM);
			}

			ucb.itemID	 = USERCONTROL_LAYER_PLYWOOD;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, 1);
			if (bLayerInd_Plywood == true) {
				DGEnableItem (dialogID, LABEL_LAYER_PLYWOOD);
				DGEnableItem (dialogID, USERCONTROL_LAYER_PLYWOOD);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_PLYWOOD);
				DGDisableItem (dialogID, USERCONTROL_LAYER_PLYWOOD);
			}

			ucb.itemID	 = USERCONTROL_LAYER_TIMBER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, 1);
			if (bLayerInd_Plywood == true) {
				DGEnableItem (dialogID, LABEL_LAYER_TIMBER);
				DGEnableItem (dialogID, USERCONTROL_LAYER_TIMBER);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_TIMBER);
				DGDisableItem (dialogID, USERCONTROL_LAYER_TIMBER);
			}

			ucb.itemID	 = USERCONTROL_LAYER_FILLERSP;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, 1);
			if (bLayerInd_Fillersp == true) {
				DGEnableItem (dialogID, LABEL_LAYER_FILLERSP);
				DGEnableItem (dialogID, USERCONTROL_LAYER_FILLERSP);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_FILLERSP);
				DGDisableItem (dialogID, USERCONTROL_LAYER_FILLERSP);
			}

			ucb.itemID	 = USERCONTROL_LAYER_OUTCORNER_ANGLE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, 1);
			if (bLayerInd_OutcornerAngle == true) {
				DGEnableItem (dialogID, LABEL_LAYER_OUTCORNER_ANGLE);
				DGEnableItem (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_OUTCORNER_ANGLE);
				DGDisableItem (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE);
			}

			ucb.itemID	 = USERCONTROL_LAYER_OUTCORNER_PANEL;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, 1);
			if (bLayerInd_OutcornerPanel == true) {
				DGEnableItem (dialogID, LABEL_LAYER_OUTCORNER_PANEL);
				DGEnableItem (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_OUTCORNER_PANEL);
				DGDisableItem (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL);
			}

			ucb.itemID	 = USERCONTROL_LAYER_INCORNER_PANEL;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, 1);
			if (bLayerInd_IncornerPanel == true) {
				DGEnableItem (dialogID, LABEL_LAYER_INCORNER_PANEL);
				DGEnableItem (dialogID, USERCONTROL_LAYER_INCORNER_PANEL);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_INCORNER_PANEL);
				DGDisableItem (dialogID, USERCONTROL_LAYER_INCORNER_PANEL);
			}

			ucb.itemID	 = USERCONTROL_LAYER_RECTPIPE_HANGER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, 1);
			if (bLayerInd_RectpipeHanger == true) {
				DGEnableItem (dialogID, LABEL_LAYER_RECTPIPE_HANGER);
				DGEnableItem (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_RECTPIPE_HANGER);
				DGDisableItem (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER);
			}

			ucb.itemID	 = USERCONTROL_LAYER_EUROFORM_HOOK;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, 1);
			if (bLayerInd_EuroformHook == true) {
				DGEnableItem (dialogID, LABEL_LAYER_EUROFORM_HOOK);
				DGEnableItem (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_EUROFORM_HOOK);
				DGDisableItem (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK);
			}

			ucb.itemID	 = USERCONTROL_LAYER_CROSS_JOINT_BAR;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_CROSS_JOINT_BAR, 1);
			if (bLayerInd_CrossJointBar == true) {
				DGEnableItem (dialogID, LABEL_LAYER_CROSS_JOINT_BAR);
				DGEnableItem (dialogID, USERCONTROL_LAYER_CROSS_JOINT_BAR);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_CROSS_JOINT_BAR);
				DGDisableItem (dialogID, USERCONTROL_LAYER_CROSS_JOINT_BAR);
			}

			ucb.itemID	 = USERCONTROL_LAYER_BLUE_CLAMP;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_CLAMP, 1);
			if (bLayerInd_BlueClamp == true) {
				DGEnableItem (dialogID, LABEL_LAYER_BLUE_CLAMP);
				DGEnableItem (dialogID, USERCONTROL_LAYER_BLUE_CLAMP);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_BLUE_CLAMP);
				DGDisableItem (dialogID, USERCONTROL_LAYER_BLUE_CLAMP);
			}

			ucb.itemID	 = USERCONTROL_LAYER_BLUE_TIMBER_RAIL;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL, 1);
			if (bLayerInd_BlueTimberRail == true) {
				DGEnableItem (dialogID, LABEL_LAYER_BLUE_TIMBER_RAIL);
				DGEnableItem (dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_BLUE_TIMBER_RAIL);
				DGDisableItem (dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL);
			}

			ucb.itemID	 = USERCONTROL_LAYER_HIDDEN;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_HIDDEN, 1);
			if (bLayerInd_Hidden == true) {
				DGEnableItem (dialogID, LABEL_LAYER_HIDDEN);
				DGEnableItem (dialogID, USERCONTROL_LAYER_HIDDEN);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_HIDDEN);
				DGDisableItem (dialogID, USERCONTROL_LAYER_HIDDEN);
			}
			break;

		case DG_MSG_CHANGE:
			// ���̾� ���� �ٲ�
			if (DGGetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING) == 1) {
				long selectedLayer;

				selectedLayer = DGGetItemValLong (dialogID, item);

				for (xx = USERCONTROL_LAYER_SLABTABLEFORM ; xx <= USERCONTROL_LAYER_BLUE_TIMBER_RAIL ; ++xx)
					DGSetItemValLong (dialogID, xx, selectedLayer);
			}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// ���̾� ��ȣ ����
					if (bLayerInd_SlabTableform == true)	layerInd_SlabTableform	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM);
					if (bLayerInd_Profile == true)			layerInd_Profile		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE);
					if (bLayerInd_Euroform == true)			layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM);
					if (bLayerInd_RectPipe == true)			layerInd_RectPipe		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE);
					if (bLayerInd_PinBolt == true)			layerInd_PinBolt		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT);
					if (bLayerInd_WallTie == true)			layerInd_WallTie		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE);
					if (bLayerInd_Join == true)				layerInd_Join			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN);
					if (bLayerInd_HeadPiece == true)		layerInd_HeadPiece		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE);
					if (bLayerInd_Steelform == true)		layerInd_Steelform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM);
					if (bLayerInd_Plywood == true)			layerInd_Plywood		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD);
					if (bLayerInd_Timber == true)			layerInd_Timber			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER);
					if (bLayerInd_Fillersp == true)			layerInd_Fillersp		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP);
					if (bLayerInd_OutcornerAngle == true)	layerInd_OutcornerAngle	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE);
					if (bLayerInd_OutcornerPanel == true)	layerInd_OutcornerPanel	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL);
					if (bLayerInd_IncornerPanel == true)	layerInd_IncornerPanel	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL);
					if (bLayerInd_RectpipeHanger == true)	layerInd_RectpipeHanger	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER);
					if (bLayerInd_EuroformHook == true)		layerInd_EuroformHook	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK);
					if (bLayerInd_CrossJointBar == true)	layerInd_CrossJointBar	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_CROSS_JOINT_BAR);
					if (bLayerInd_BlueClamp == true)		layerInd_BlueClamp		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_CLAMP);
					if (bLayerInd_BlueTimberRail == true)	layerInd_BlueTimberRail	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL);
					if (bLayerInd_Hidden == true)			layerInd_Hidden			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_HIDDEN);

					break;

				case BUTTON_AUTOSET:
					item = 0;

					DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, FALSE);

					if (placingZone.tableformType == 1) {
						layerInd_Euroform		= makeTemporaryLayer (structuralObject_forTableformLowSide, "UFOM", NULL);
						layerInd_RectPipe		= makeTemporaryLayer (structuralObject_forTableformLowSide, "SPIP", NULL);
						layerInd_PinBolt		= makeTemporaryLayer (structuralObject_forTableformLowSide, "PINB", NULL);
						layerInd_Join			= makeTemporaryLayer (structuralObject_forTableformLowSide, "CLAM", NULL);
						layerInd_HeadPiece		= makeTemporaryLayer (structuralObject_forTableformLowSide, "HEAD", NULL);
						layerInd_Plywood		= makeTemporaryLayer (structuralObject_forTableformLowSide, "PLYW", NULL);
						layerInd_Timber			= makeTemporaryLayer (structuralObject_forTableformLowSide, "TIMB", NULL);
						layerInd_BlueClamp		= makeTemporaryLayer (structuralObject_forTableformLowSide, "UFCL", NULL);
						layerInd_BlueTimberRail	= makeTemporaryLayer (structuralObject_forTableformLowSide, "RAIL", NULL);

						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, layerInd_Euroform);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, layerInd_RectPipe);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, layerInd_PinBolt);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, layerInd_Join);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, layerInd_HeadPiece);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, layerInd_Plywood);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, layerInd_Timber);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_CLAMP, layerInd_BlueClamp);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL, layerInd_BlueTimberRail);

					} else if (placingZone.tableformType == 2) {
						layerInd_Euroform		= makeTemporaryLayer (structuralObject_forTableformLowSide, "UFOM", NULL);
						layerInd_RectPipe		= makeTemporaryLayer (structuralObject_forTableformLowSide, "SPIP", NULL);
						layerInd_PinBolt		= makeTemporaryLayer (structuralObject_forTableformLowSide, "PINB", NULL);
						layerInd_Join			= makeTemporaryLayer (structuralObject_forTableformLowSide, "CLAM", NULL);
						layerInd_HeadPiece		= makeTemporaryLayer (structuralObject_forTableformLowSide, "HEAD", NULL);
						layerInd_Plywood		= makeTemporaryLayer (structuralObject_forTableformLowSide, "PLYW", NULL);
						layerInd_Timber			= makeTemporaryLayer (structuralObject_forTableformLowSide, "TIMB", NULL);
						layerInd_BlueClamp		= makeTemporaryLayer (structuralObject_forTableformLowSide, "UFCL", NULL);
						layerInd_BlueTimberRail	= makeTemporaryLayer (structuralObject_forTableformLowSide, "RAIL", NULL);

						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, layerInd_Euroform);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, layerInd_RectPipe);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, layerInd_PinBolt);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, layerInd_Join);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, layerInd_HeadPiece);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, layerInd_Plywood);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, layerInd_Timber);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_CLAMP, layerInd_BlueClamp);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL, layerInd_BlueTimberRail);

					} else if (placingZone.tableformType == 3) {
						layerInd_Euroform		= makeTemporaryLayer (structuralObject_forTableformLowSide, "UFOM", NULL);
						layerInd_RectPipe		= makeTemporaryLayer (structuralObject_forTableformLowSide, "SPIP", NULL);
						layerInd_PinBolt		= makeTemporaryLayer (structuralObject_forTableformLowSide, "PINB", NULL);
						layerInd_Join			= makeTemporaryLayer (structuralObject_forTableformLowSide, "CLAM", NULL);
						layerInd_HeadPiece		= makeTemporaryLayer (structuralObject_forTableformLowSide, "HEAD", NULL);
						layerInd_CrossJointBar	= makeTemporaryLayer (structuralObject_forTableformLowSide, "CROS", NULL);
						layerInd_Plywood		= makeTemporaryLayer (structuralObject_forTableformLowSide, "PLYW", NULL);
						layerInd_Timber			= makeTemporaryLayer (structuralObject_forTableformLowSide, "TIMB", NULL);
						layerInd_BlueClamp		= makeTemporaryLayer (structuralObject_forTableformLowSide, "UFCL", NULL);
						layerInd_BlueTimberRail	= makeTemporaryLayer (structuralObject_forTableformLowSide, "RAIL", NULL);

						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, layerInd_Euroform);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, layerInd_RectPipe);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, layerInd_PinBolt);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, layerInd_Join);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, layerInd_HeadPiece);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_CROSS_JOINT_BAR, layerInd_CrossJointBar);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, layerInd_Plywood);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, layerInd_Timber);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_CLAMP, layerInd_BlueClamp);
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL, layerInd_BlueTimberRail);
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

// ���̺��� ���ι��⿡ ���Ͽ� �������� ���� �迭�� �����ϱ� ���� ���̾�α�
short DGCALLBACK lowSideTableformPlacerHandler3_Vertical (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	// clickedIndex: ���� ���̾�α׿��� ���� ��ư�� 0-��� �ε��� ��ȣ (BUTTON_OBJ [xx])

	short	result;
	short	itmIdx;
	short	itmPosX, itmPosY;
	short	xx, yy;
	int		accumLength;
	char	buffer [256];
	char	numbuf [32];

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, "���̺��� (���ι���) �迭 ����");

			//////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			// ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 310, 140, 70, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "Ȯ ��");
			DGShowItem (dialogID, DG_OK);

			// ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 390, 140, 70, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "�� ��");
			DGShowItem (dialogID, DG_CANCEL);

			// ���� �ʺ� (��)
			accumLength = 0;
			for (xx = 0 ; xx < sizeof (placingZone.cells [clickedIndex].tableInHor) / sizeof (int) ; ++xx)
				accumLength += placingZone.cells [clickedIndex].tableInHor [xx];
			sprintf (buffer, "���� �ʺ�: %d", accumLength);
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 270, 20, 100, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, buffer);
			DGShowItem (dialogID, itmIdx);

			// ����� �ʺ� (��)
			sprintf (buffer, "����� �ʺ�: %d", 0);
			placingZone.LABEL_TOTAL_WIDTH = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 400, 20, 100, 23);
			DGSetItemFont (dialogID, placingZone.LABEL_TOTAL_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, placingZone.LABEL_TOTAL_WIDTH, buffer);
			DGShowItem (dialogID, placingZone.LABEL_TOTAL_WIDTH);

			itmPosX = 35;
			itmPosY = 55;

			for (xx = 0 ; xx < 10 ; ++xx) {
				// ������
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 70, 70);
				DGShowItem (dialogID, itmIdx);

				// �ؽ�Ʈ(������)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX + 10, itmPosY + 10, 50, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "������");
				DGShowItem (dialogID, itmIdx);

				// �޺��ڽ�
				placingZone.POPUP_WIDTH_IN_TABLE [xx] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX + 5, itmPosY + 40, 60, 25);
				DGSetItemFont (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DG_IS_LARGE | DG_IS_PLAIN);
				for (yy = 0 ; yy < sizeof (placingZone.presetWidthVertical_euroform) / sizeof (int) ; ++yy) {
					DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DG_POPUP_BOTTOM);
					_itoa (placingZone.presetWidthVertical_euroform [yy], numbuf, 10);
					DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DG_POPUP_BOTTOM, numbuf);
				}
				for (yy = 1 ; yy <= DGPopUpGetItemCount (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx]) ; ++yy) {
					if (placingZone.cells [clickedIndex].tableInHor [xx] == atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], yy).ToCStr ().Get ())) {
						DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], yy);
						break;
					}
				}
				DGShowItem (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx]);

				itmPosX += 70;
			}

			// ����� �ʺ� (��) ������Ʈ
			accumLength = 0;
			for (xx = 0 ; xx < 10 ; ++xx) {
				accumLength += atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
			}
			sprintf (buffer, "����� �ʺ�: %d", accumLength);
			DGSetItemText (dialogID, placingZone.LABEL_TOTAL_WIDTH, buffer);

			break;

		case DG_MSG_CHANGE:

			// ����� �ʺ� (��) ������Ʈ
			accumLength = 0;
			for (xx = 0 ; xx < 10 ; ++xx) {
				accumLength += atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
			}
			sprintf (buffer, "����� �ʺ�: %d", accumLength);
			DGSetItemText (dialogID, placingZone.LABEL_TOTAL_WIDTH, buffer);

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// ������ �޺��ڽ����� ���� ������� ����ü ���� ������
					for (xx = 0 ; xx < 10 ; ++xx) {
						placingZone.cells [clickedIndex].tableInHor [xx] = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
					}

					accumLength = 0;
					for (xx = 0 ; xx < 10 ; ++xx) {
						accumLength += atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
					}
					placingZone.cells [clickedIndex].horLen = accumLength;
					break;

				case DG_CANCEL:
					break;
			}

			break;

		case DG_MSG_CLOSE:
			switch (item) {
				case DG_CLOSEBOX:
					break;
			}
	}

	result = item;

	return	result;
}

// ���̺��� ���ι��⿡ ���Ͽ� �������� ���� �迭�� �����ϱ� ���� ���̾�α�
short DGCALLBACK lowSideTableformPlacerHandler3_Horizontal (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	// clickedIndex: ���� ���̾�α׿��� ���� ��ư�� 0-��� �ε��� ��ȣ (BUTTON_OBJ [xx])

	short	result;
	short	itmIdx;
	short	itmPosX, itmPosY;
	short	xx, yy;
	int		accumLength;
	char	buffer [256];
	char	numbuf [32];

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, "���̺��� (���ι���) �迭 ����");

			//////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			// ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 310, 140, 70, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "Ȯ ��");
			DGShowItem (dialogID, DG_OK);

			// ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 390, 140, 70, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "�� ��");
			DGShowItem (dialogID, DG_CANCEL);

			// ���� �ʺ� (��)
			accumLength = 0;
			for (xx = 0 ; xx < sizeof (placingZone.cells [clickedIndex].tableInHor) / sizeof (int) ; ++xx)
				accumLength += placingZone.cells [clickedIndex].tableInHor [xx];
			sprintf (buffer, "���� �ʺ�: %d", accumLength);
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 270, 20, 100, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, buffer);
			DGShowItem (dialogID, itmIdx);
			
			// ����� �ʺ� (��)
			sprintf (buffer, "����� �ʺ�: %d", 0);
			placingZone.LABEL_TOTAL_WIDTH = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 400, 20, 100, 23);
			DGSetItemFont (dialogID, placingZone.LABEL_TOTAL_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, placingZone.LABEL_TOTAL_WIDTH, buffer);
			DGShowItem (dialogID, placingZone.LABEL_TOTAL_WIDTH);

			itmPosX = 35;
			itmPosY = 55;

			for (xx = 0 ; xx < 10 ; ++xx) {
				// ������
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 70, 70);
				DGShowItem (dialogID, itmIdx);

				// �ؽ�Ʈ(������)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX + 10, itmPosY + 10, 50, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "������");
				DGShowItem (dialogID, itmIdx);

				// �޺��ڽ�
				placingZone.POPUP_WIDTH_IN_TABLE [xx] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX + 5, itmPosY + 40, 60, 25);
				DGSetItemFont (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DG_IS_LARGE | DG_IS_PLAIN);
				for (yy = 0 ; yy < sizeof (placingZone.presetHeightHorizontal_euroform) / sizeof (int) ; ++yy) {
					DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DG_POPUP_BOTTOM);
					_itoa (placingZone.presetHeightHorizontal_euroform [yy], numbuf, 10);
					DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DG_POPUP_BOTTOM, numbuf);
				}
				for (yy = 1 ; yy <= DGPopUpGetItemCount (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx]) ; ++yy) {
					if (placingZone.cells [clickedIndex].tableInHor [xx] == atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], yy).ToCStr ().Get ())) {
						DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], yy);
						break;
					}
				}
				DGShowItem (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx]);

				itmPosX += 70;
			}

			// ����� �ʺ� (��) ������Ʈ
			accumLength = 0;
			for (xx = 0 ; xx < 10 ; ++xx) {
				accumLength += atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
			}
			sprintf (buffer, "����� �ʺ�: %d", accumLength);
			DGSetItemText (dialogID, placingZone.LABEL_TOTAL_WIDTH, buffer);

			break;

		case DG_MSG_CHANGE:

			// ����� �ʺ� (��) ������Ʈ
			accumLength = 0;
			for (xx = 0 ; xx < 10 ; ++xx) {
				accumLength += atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
			}
			sprintf (buffer, "����� �ʺ�: %d", accumLength);
			DGSetItemText (dialogID, placingZone.LABEL_TOTAL_WIDTH, buffer);

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// ������ �޺��ڽ����� ���� ������� ����ü ���� ������
					for (xx = 0 ; xx < 10 ; ++xx) {
						placingZone.cells [clickedIndex].tableInHor [xx] = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
					}

					accumLength = 0;
					for (xx = 0 ; xx < 10 ; ++xx) {
						accumLength += atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
					}
					placingZone.cells [clickedIndex].horLen = accumLength;
					break;

				case DG_CANCEL:
					break;
			}

			break;

		case DG_MSG_CLOSE:
			switch (item) {
				case DG_CLOSEBOX:
					break;
			}
	}

	result = item;

	return	result;
}
