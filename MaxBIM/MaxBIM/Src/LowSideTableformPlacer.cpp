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

API_Guid		structuralObject_forTableformLowSide;	// ���� ��ü�� GUID

//static short	layerInd_Euroform;			// ���̾� ��ȣ: ������ (����)
//static short	layerInd_RectPipe;			// ���̾� ��ȣ: ��� ������ (����)
//static short	layerInd_PinBolt;			// ���̾� ��ȣ: �ɺ�Ʈ ��Ʈ
//static short	layerInd_WallTie;			// ���̾� ��ȣ: ��ü Ÿ�� (�� �̻� ������� ����)
//static short	layerInd_Clamp;				// ���̾� ��ȣ: ���� Ŭ���� (�� �̻� ������� ����)
//static short	layerInd_HeadPiece;			// ���̾� ��ȣ: ����ǽ�
//static short	layerInd_Join;				// ���̾� ��ȣ: ����ö��
//static short	layerInd_Plywood;			// ���̾� ��ȣ: ���� (����)
//static short	layerInd_Timber;			// ���̾� ��ȣ: ���� (����)
//static short	layerInd_EuroformHook;		// ���̾� ��ȣ: ������ ��ũ
//static short	layerInd_CrossJointBar;		// ���̾� ��ȣ: ���� ����Ʈ ��
//static short	layerInd_BlueClamp;			// ���̾� ��ȣ: ��� Ŭ����
//static short	layerInd_BlueTimberRail;	// ���̾� ��ȣ: ��� ���
//static short	layerInd_Hidden;			// ���̾� ��ȣ: ���� (�� �̻� ������� ����)
//
//static short	layerInd_SlabTableform;		// ���̾� ��ȣ: ������ ���̺���
//static short	layerInd_Profile;			// ���̾� ��ȣ: KS��������
//static short	layerInd_Steelform;			// ���̾� ��ȣ: ��ƿ��
//static short	layerInd_Fillersp;			// ���̾� ��ȣ: �ٷ������̼�
//static short	layerInd_OutcornerAngle;	// ���̾� ��ȣ: �ƿ��ڳʾޱ�
//static short	layerInd_OutcornerPanel;	// ���̾� ��ȣ: �ƿ��ڳ��ǳ�
//static short	layerInd_IncornerPanel;		// ���̾� ��ȣ: ���ڳ��ǳ�
//static short	layerInd_RectpipeHanger;	// ���̾� ��ȣ: �������� ���
//
//static bool		bLayerInd_Euroform;			// ���̾� ��ȣ: ������
//static bool		bLayerInd_RectPipe;			// ���̾� ��ȣ: ��� ������
//static bool		bLayerInd_PinBolt;			// ���̾� ��ȣ: �ɺ�Ʈ ��Ʈ
//static bool		bLayerInd_WallTie;			// ���̾� ��ȣ: ��ü Ÿ��
//static bool		bLayerInd_HeadPiece;		// ���̾� ��ȣ: ����ǽ�
//static bool		bLayerInd_Join;				// ���̾� ��ȣ: ����ö��
//static bool		bLayerInd_Plywood;			// ���̾� ��ȣ: ����
//static bool		bLayerInd_Timber;			// ���̾� ��ȣ: ����
//static bool		bLayerInd_EuroformHook;		// ���̾� ��ȣ: ������ ��ũ
//static bool		bLayerInd_CrossJointBar;	// ���̾� ��ȣ: ���� ����Ʈ ��
//static bool		bLayerInd_BlueClamp;		// ���̾� ��ȣ: ��� Ŭ����
//static bool		bLayerInd_BlueTimberRail;	// ���̾� ��ȣ: ��� ���
//static bool		bLayerInd_Hidden;			// ���̾� ��ȣ: ����
//
//static bool		bLayerInd_SlabTableform;	// ���̾� ��ȣ: ������ ���̺���
//static bool		bLayerInd_Profile;			// ���̾� ��ȣ: KS��������
//static bool		bLayerInd_Steelform;		// ���̾� ��ȣ: ��ƿ��
//static bool		bLayerInd_Fillersp;			// ���̾� ��ȣ: �ٷ������̼�
//static bool		bLayerInd_OutcornerAngle;	// ���̾� ��ȣ: �ƿ��ڳʾޱ�
//static bool		bLayerInd_OutcornerPanel;	// ���̾� ��ȣ: �ƿ��ڳ��ǳ�
//static bool		bLayerInd_IncornerPanel;	// ���̾� ��ȣ: ���ڳ��ǳ�
//static bool		bLayerInd_RectpipeHanger;	// ���̾� ��ȣ: �������� ���
//
//static GS::Array<API_Guid>	elemList_Front;	// �׷�ȭ�� ���� ������ ��������� GUID�� ���� ������ (�ո�)
//static GS::Array<API_Guid>	elemList_Back;	// �׷�ȭ�� ���� ������ ��������� GUID�� ���� ������ (�޸�)
//
//static int	clickedIndex;	// Ŭ���� ��ư�� �ε���
//static int	iMarginSide;	// �� ����� ä����� �ϴµ� ���� ����Ǵ� ���� ��� �� ���ΰ�? (1-������, 2-������)


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
		short floorInd;

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
	else if (nMesh == 1)	placingZone.leftBottomZ = infoMesh.level;

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
	result = DGBlankModalDialog (600, 500, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, lowSideTableformPlacerHandler1, 0);

	if (result != DG_OK)
		return err;

	// [DIALOG] 2��° ���̾�α׿��� ���纰 ���̾ ������
	//result = DGModalDialog (ACAPI_GetOwnResModule (), 32503, ACAPI_GetOwnResModule (), lowSideTableformPlacerHandler2, 0);

	if (result != DG_OK)
		goto FIRST;

	// ��ü ��ġ�ϱ�
	//placingZone.placeObjects (&placingZone);

	// ȭ�� ���ΰ�ħ
	ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	bool	regenerate = true;
	ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

	// 1�� UI
		// ���̺��� ����: ����, ����
		// ���̺��� Ÿ��: Ÿ��A
		// �߰� / ���� ��ư
		// ���� ����, ���� �ʺ�
		// ��: ���ڳ�, �ƿ��ڳʾޱ�, �ƿ��ڳ��ǳ�, ���̺���(������ ����: ���� �����̸� 3��, ���� �����̸� 6�� - �ִ� �ʺ� 3600), ������, �ٷ������̼�, ����, ����
	// 2�� UI : ���̾� �����ϱ�
		// ������, ��� ������, �ɺ�Ʈ ��Ʈ, ����ö��, ����ǽ�, ����, ����, �ٷ������̼�, �ƿ��ڳʾޱ�, �ƿ��ڳ��ǳ�, ���ڳ�

	// ... �� ũ�� ���� ����
		// �ʺ� ���� ����
			// ���� �����̸� 1200���� ä��
			// ���� �����̸� 600���� ä��
		// ���� ����
			// �����갡 1200 �����̸� 1200
			// �����갡 900 �����̸� 900
			// �����갡 600 �����̸� 600
			// �����갡 500 �����̸� 500
			// �����갡 450 �����̸� 450
			// �����갡 400 �����̸� 400
			// �����갡 300 �����̸� 300
			// �����갡 200 �����̸� 200

	// ... �������� ����
		// �Ϻ� ����� ����: 150
		// ��� ����� ���� (������ ��쿡��): ������ 300
		// ������ ��ġ/����: �� ���� 50�� ������

	return	err;
}

// �⺻ ������ ...
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
													this->presetWidth_config_vertical [9][4] = 500;		this->presetWidth_config_vertical [9][5] = 200;		this->presetWidth_config_vertical [9][6] = 600;		// 3100
	this->presetWidth_config_vertical [10][0] = 6;	this->presetWidth_config_vertical [10][1] = 600;	this->presetWidth_config_vertical [10][2] = 600;	this->presetWidth_config_vertical [10][3] = 600;
													this->presetWidth_config_vertical [10][4] = 450;	this->presetWidth_config_vertical [10][5] = 200;	this->presetWidth_config_vertical [10][6] = 600;	// 3050
	this->presetWidth_config_vertical [11][0] = 5;	this->presetWidth_config_vertical [11][1] = 600;	this->presetWidth_config_vertical [11][2] = 600;	this->presetWidth_config_vertical [11][3] = 600;
													this->presetWidth_config_vertical [11][4] = 600;	this->presetWidth_config_vertical [11][5] = 600;	this->presetWidth_config_vertical [11][6] = 0;		// 3000
	this->presetWidth_config_vertical [12][0] = 6;	this->presetWidth_config_vertical [12][1] = 600;	this->presetWidth_config_vertical [12][2] = 600;	this->presetWidth_config_vertical [12][3] = 600;
													this->presetWidth_config_vertical [12][4] = 500;	this->presetWidth_config_vertical [12][5] = 200;	this->presetWidth_config_vertical [12][6] = 450;	// 2950
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
	this->presetWidth_config_vertical [33][0] = 4;	this->presetWidth_config_vertical [33][1] = 600;	this->presetWidth_config_vertical [33][2] = 500;	this->presetWidth_config_vertical [33][3] = 200;
													this->presetWidth_config_vertical [33][4] = 600;	this->presetWidth_config_vertical [33][5] = 0;		this->presetWidth_config_vertical [33][6] = 0;		// 1900
	this->presetWidth_config_vertical [34][0] = 4;	this->presetWidth_config_vertical [34][1] = 600;	this->presetWidth_config_vertical [34][2] = 450;	this->presetWidth_config_vertical [34][3] = 200;
													this->presetWidth_config_vertical [34][4] = 600;	this->presetWidth_config_vertical [34][5] = 0;		this->presetWidth_config_vertical [34][6] = 0;		// 1850
	// !!!
	//this->presetWidth_config_vertical [10][0] = 3;	this->presetWidth_config_vertical [10][1] = 600;	this->presetWidth_config_vertical [10][2] = 600;	this->presetWidth_config_vertical [10][3] = 600;	this->presetWidth_config_vertical [10][4] = 0;		// 1800
	//this->presetWidth_config_vertical [11][0] = 4;	this->presetWidth_config_vertical [11][1] = 600;	this->presetWidth_config_vertical [11][2] = 200;	this->presetWidth_config_vertical [11][3] = 450;	this->presetWidth_config_vertical [11][4] = 500;	// 1750
	//this->presetWidth_config_vertical [12][0] = 3;	this->presetWidth_config_vertical [12][1] = 600;	this->presetWidth_config_vertical [12][2] = 500;	this->presetWidth_config_vertical [12][3] = 600;	this->presetWidth_config_vertical [12][4] = 0;		// 1700
	//this->presetWidth_config_vertical [13][0] = 3;	this->presetWidth_config_vertical [13][1] = 600;	this->presetWidth_config_vertical [13][2] = 450;	this->presetWidth_config_vertical [13][3] = 600;	this->presetWidth_config_vertical [13][4] = 0;		// 1650
	//this->presetWidth_config_vertical [14][0] = 3;	this->presetWidth_config_vertical [14][1] = 600;	this->presetWidth_config_vertical [14][2] = 400;	this->presetWidth_config_vertical [14][3] = 600;	this->presetWidth_config_vertical [14][4] = 0;		// 1600
	//this->presetWidth_config_vertical [15][0] = 3;	this->presetWidth_config_vertical [15][1] = 600;	this->presetWidth_config_vertical [15][2] = 450;	this->presetWidth_config_vertical [15][3] = 500;	this->presetWidth_config_vertical [15][4] = 0;		// 1550
	//this->presetWidth_config_vertical [16][0] = 3;	this->presetWidth_config_vertical [16][1] = 600;	this->presetWidth_config_vertical [16][2] = 300;	this->presetWidth_config_vertical [16][3] = 600;	this->presetWidth_config_vertical [16][4] = 0;		// 1500
	//this->presetWidth_config_vertical [17][0] = 3;	this->presetWidth_config_vertical [17][1] = 500;	this->presetWidth_config_vertical [17][2] = 450;	this->presetWidth_config_vertical [17][3] = 500;	this->presetWidth_config_vertical [17][4] = 0;		// 1450
	//this->presetWidth_config_vertical [18][0] = 3;	this->presetWidth_config_vertical [18][1] = 500;	this->presetWidth_config_vertical [18][2] = 400;	this->presetWidth_config_vertical [18][3] = 500;	this->presetWidth_config_vertical [18][4] = 0;		// 1400
	//this->presetWidth_config_vertical [19][0] = 3;	this->presetWidth_config_vertical [19][1] = 600;	this->presetWidth_config_vertical [19][2] = 300;	this->presetWidth_config_vertical [19][3] = 450;	this->presetWidth_config_vertical [19][4] = 0;		// 1350
	//this->presetWidth_config_vertical [20][0] = 3;	this->presetWidth_config_vertical [20][1] = 600;	this->presetWidth_config_vertical [20][2] = 200;	this->presetWidth_config_vertical [20][3] = 500;	this->presetWidth_config_vertical [20][4] = 0;		// 1300
	//this->presetWidth_config_vertical [21][0] = 3;	this->presetWidth_config_vertical [21][1] = 600;	this->presetWidth_config_vertical [21][2] = 200;	this->presetWidth_config_vertical [21][3] = 450;	this->presetWidth_config_vertical [21][4] = 0;		// 1250
	//this->presetWidth_config_vertical [22][0] = 2;	this->presetWidth_config_vertical [22][1] = 600;	this->presetWidth_config_vertical [22][2] = 600;	this->presetWidth_config_vertical [22][3] = 0;		this->presetWidth_config_vertical [22][4] = 0;		// 1200
	//this->presetWidth_config_vertical [23][0] = 3;	this->presetWidth_config_vertical [23][1] = 450;	this->presetWidth_config_vertical [23][2] = 300;	this->presetWidth_config_vertical [23][3] = 400;	this->presetWidth_config_vertical [23][4] = 0;		// 1150
	//this->presetWidth_config_vertical [24][0] = 3;	this->presetWidth_config_vertical [24][1] = 400;	this->presetWidth_config_vertical [24][2] = 300;	this->presetWidth_config_vertical [24][3] = 400;	this->presetWidth_config_vertical [24][4] = 0;		// 1100
	//this->presetWidth_config_vertical [25][0] = 3;	this->presetWidth_config_vertical [25][1] = 450;	this->presetWidth_config_vertical [25][2] = 300;	this->presetWidth_config_vertical [25][3] = 300;	this->presetWidth_config_vertical [25][4] = 0;		// 1050
	//this->presetWidth_config_vertical [26][0] = 2;	this->presetWidth_config_vertical [26][1] = 600;	this->presetWidth_config_vertical [26][2] = 400;	this->presetWidth_config_vertical [26][3] = 0;		this->presetWidth_config_vertical [26][4] = 0;		// 1000
	//this->presetWidth_config_vertical [27][0] = 2;	this->presetWidth_config_vertical [27][1] = 450;	this->presetWidth_config_vertical [27][2] = 500;	this->presetWidth_config_vertical [27][3] = 0;		this->presetWidth_config_vertical [27][4] = 0;		// 950
	//this->presetWidth_config_vertical [28][0] = 2;	this->presetWidth_config_vertical [28][1] = 600;	this->presetWidth_config_vertical [28][2] = 300;	this->presetWidth_config_vertical [28][3] = 0;		this->presetWidth_config_vertical [28][4] = 0;		// 900
	//this->presetWidth_config_vertical [29][0] = 2;	this->presetWidth_config_vertical [29][1] = 400;	this->presetWidth_config_vertical [29][2] = 450;	this->presetWidth_config_vertical [29][3] = 0;		this->presetWidth_config_vertical [29][4] = 0;		// 850
	//this->presetWidth_config_vertical [30][0] = 2;	this->presetWidth_config_vertical [30][1] = 400;	this->presetWidth_config_vertical [30][2] = 400;	this->presetWidth_config_vertical [30][3] = 0;		this->presetWidth_config_vertical [30][4] = 0;		// 800
	//this->presetWidth_config_vertical [31][0] = 2;	this->presetWidth_config_vertical [31][1] = 450;	this->presetWidth_config_vertical [31][2] = 300;	this->presetWidth_config_vertical [31][3] = 0;		this->presetWidth_config_vertical [31][4] = 0;		// 750
	//this->presetWidth_config_vertical [32][0] = 2;	this->presetWidth_config_vertical [32][1] = 400;	this->presetWidth_config_vertical [32][2] = 300;	this->presetWidth_config_vertical [32][3] = 0;		this->presetWidth_config_vertical [32][4] = 0;		// 700
	//this->presetWidth_config_vertical [33][0] = 2;	this->presetWidth_config_vertical [33][1] = 450;	this->presetWidth_config_vertical [33][2] = 200;	this->presetWidth_config_vertical [33][3] = 0;		this->presetWidth_config_vertical [33][4] = 0;		// 650
	//this->presetWidth_config_vertical [34][0] = 1;	this->presetWidth_config_vertical [34][1] = 600;	this->presetWidth_config_vertical [34][2] = 0;		this->presetWidth_config_vertical [34][3] = 0;		this->presetWidth_config_vertical [34][4] = 0;		// 600
	//this->presetWidth_config_vertical [35][0] = 1;	this->presetWidth_config_vertical [35][1] = 500;	this->presetWidth_config_vertical [35][2] = 0;		this->presetWidth_config_vertical [35][3] = 0;		this->presetWidth_config_vertical [35][4] = 0;		// 500
	//this->presetWidth_config_vertical [36][0] = 1;	this->presetWidth_config_vertical [36][1] = 450;	this->presetWidth_config_vertical [36][2] = 0;		this->presetWidth_config_vertical [36][3] = 0;		this->presetWidth_config_vertical [36][4] = 0;		// 450
	//this->presetWidth_config_vertical [37][0] = 1;	this->presetWidth_config_vertical [37][1] = 400;	this->presetWidth_config_vertical [37][2] = 0;		this->presetWidth_config_vertical [37][3] = 0;		this->presetWidth_config_vertical [37][4] = 0;		// 400
	//this->presetWidth_config_vertical [38][0] = 1;	this->presetWidth_config_vertical [38][1] = 300;	this->presetWidth_config_vertical [38][2] = 0;		this->presetWidth_config_vertical [38][3] = 0;		this->presetWidth_config_vertical [38][4] = 0;		// 300
	//this->presetWidth_config_vertical [39][0] = 1;	this->presetWidth_config_vertical [39][1] = 200;	this->presetWidth_config_vertical [39][2] = 0;		this->presetWidth_config_vertical [39][3] = 0;		this->presetWidth_config_vertical [39][4] = 0;		// 200

	//this->presetHeight_config_vertical [0][0] = 5;	this->presetHeight_config_vertical [0][1] = 1200;	this->presetHeight_config_vertical [0][2] = 1200;	this->presetHeight_config_vertical [0][3] = 1200;	this->presetHeight_config_vertical [0][4] = 1200;	this->presetHeight_config_vertical [0][5] = 1200;	// 6000
	//this->presetHeight_config_vertical [1][0] = 5;	this->presetHeight_config_vertical [1][1] = 1200;	this->presetHeight_config_vertical [1][2] = 1200;	this->presetHeight_config_vertical [1][3] = 1200;	this->presetHeight_config_vertical [1][4] = 1200;	this->presetHeight_config_vertical [1][5] = 900;	// 5700
	//this->presetHeight_config_vertical [2][0] = 5;	this->presetHeight_config_vertical [2][1] = 1200;	this->presetHeight_config_vertical [2][2] = 1200;	this->presetHeight_config_vertical [2][3] = 1200;	this->presetHeight_config_vertical [2][4] = 900;	this->presetHeight_config_vertical [2][5] = 900;	// 5400
	//this->presetHeight_config_vertical [3][0] = 5;	this->presetHeight_config_vertical [3][1] = 1200;	this->presetHeight_config_vertical [3][2] = 1200;	this->presetHeight_config_vertical [3][3] = 1200;	this->presetHeight_config_vertical [3][4] = 900;	this->presetHeight_config_vertical [3][5] = 600;	// 5100
	//this->presetHeight_config_vertical [4][0] = 4;	this->presetHeight_config_vertical [4][1] = 1200;	this->presetHeight_config_vertical [4][2] = 1200;	this->presetHeight_config_vertical [4][3] = 1200;	this->presetHeight_config_vertical [4][4] = 1200;	this->presetHeight_config_vertical [4][5] = 0;		// 4800
	//this->presetHeight_config_vertical [5][0] = 4;	this->presetHeight_config_vertical [5][1] = 1200;	this->presetHeight_config_vertical [5][2] = 1200;	this->presetHeight_config_vertical [5][3] = 1200;	this->presetHeight_config_vertical [5][4] = 900;	this->presetHeight_config_vertical [5][5] = 0;		// 4500
	//this->presetHeight_config_vertical [6][0] = 4;	this->presetHeight_config_vertical [6][1] = 1200;	this->presetHeight_config_vertical [6][2] = 1200;	this->presetHeight_config_vertical [6][3] = 900;	this->presetHeight_config_vertical [6][4] = 900;	this->presetHeight_config_vertical [6][5] = 0;		// 4200
	//this->presetHeight_config_vertical [7][0] = 4;	this->presetHeight_config_vertical [7][1] = 1200;	this->presetHeight_config_vertical [7][2] = 1200;	this->presetHeight_config_vertical [7][3] = 900;	this->presetHeight_config_vertical [7][4] = 600;	this->presetHeight_config_vertical [7][5] = 0;		// 3900
	//this->presetHeight_config_vertical [8][0] = 3;	this->presetHeight_config_vertical [8][1] = 1200;	this->presetHeight_config_vertical [8][2] = 1200;	this->presetHeight_config_vertical [8][3] = 1200;	this->presetHeight_config_vertical [8][4] = 0;		this->presetHeight_config_vertical [8][5] = 0;		// 3600
	//this->presetHeight_config_vertical [9][0] = 3;	this->presetHeight_config_vertical [9][1] = 1200;	this->presetHeight_config_vertical [9][2] = 1200;	this->presetHeight_config_vertical [9][3] = 900;	this->presetHeight_config_vertical [9][4] = 0;		this->presetHeight_config_vertical [9][5] = 0;		// 3300
	//this->presetHeight_config_vertical [10][0] = 3;	this->presetHeight_config_vertical [10][1] = 1200;	this->presetHeight_config_vertical [10][2] = 1200;	this->presetHeight_config_vertical [10][3] = 600;	this->presetHeight_config_vertical [10][4] = 0;		this->presetHeight_config_vertical [10][5] = 0;		// 3000
	//this->presetHeight_config_vertical [11][0] = 3;	this->presetHeight_config_vertical [11][1] = 1200;	this->presetHeight_config_vertical [11][2] = 900;	this->presetHeight_config_vertical [11][3] = 600;	this->presetHeight_config_vertical [11][4] = 0;		this->presetHeight_config_vertical [11][5] = 0;		// 2700
	//this->presetHeight_config_vertical [12][0] = 2;	this->presetHeight_config_vertical [12][1] = 1200;	this->presetHeight_config_vertical [12][2] = 1200;	this->presetHeight_config_vertical [12][3] = 0;		this->presetHeight_config_vertical [12][4] = 0;		this->presetHeight_config_vertical [12][5] = 0;		// 2400
	//this->presetHeight_config_vertical [13][0] = 2;	this->presetHeight_config_vertical [13][1] = 1200;	this->presetHeight_config_vertical [13][2] = 900;	this->presetHeight_config_vertical [13][3] = 0;		this->presetHeight_config_vertical [13][4] = 0;		this->presetHeight_config_vertical [13][5] = 0;		// 2100
	//this->presetHeight_config_vertical [14][0] = 2;	this->presetHeight_config_vertical [14][1] = 900;	this->presetHeight_config_vertical [14][2] = 900;	this->presetHeight_config_vertical [14][3] = 0;		this->presetHeight_config_vertical [14][4] = 0;		this->presetHeight_config_vertical [14][5] = 0;		// 1800
	//this->presetHeight_config_vertical [15][0] = 2;	this->presetHeight_config_vertical [15][1] = 900;	this->presetHeight_config_vertical [15][2] = 600;	this->presetHeight_config_vertical [15][3] = 0;		this->presetHeight_config_vertical [15][4] = 0;		this->presetHeight_config_vertical [15][5] = 0;		// 1500

	//this->presetWidth_config_horizontal [0][0] = 5;	this->presetWidth_config_horizontal [0][1] = 1200;	this->presetWidth_config_horizontal [0][2] = 1200;	this->presetWidth_config_horizontal [0][3] = 1200;	this->presetWidth_config_horizontal [0][4] = 1200;	this->presetWidth_config_horizontal [0][5] = 1200;	// 6000
	//this->presetWidth_config_horizontal [1][0] = 5;	this->presetWidth_config_horizontal [1][1] = 1200;	this->presetWidth_config_horizontal [1][2] = 1200;	this->presetWidth_config_horizontal [1][3] = 1200;	this->presetWidth_config_horizontal [1][4] = 1200;	this->presetWidth_config_horizontal [1][5] = 900;	// 5700
	//this->presetWidth_config_horizontal [2][0] = 5;	this->presetWidth_config_horizontal [2][1] = 1200;	this->presetWidth_config_horizontal [2][2] = 1200;	this->presetWidth_config_horizontal [2][3] = 1200;	this->presetWidth_config_horizontal [2][4] = 900;	this->presetWidth_config_horizontal [2][5] = 900;	// 5400
	//this->presetWidth_config_horizontal [3][0] = 5;	this->presetWidth_config_horizontal [3][1] = 1200;	this->presetWidth_config_horizontal [3][2] = 1200;	this->presetWidth_config_horizontal [3][3] = 1200;	this->presetWidth_config_horizontal [3][4] = 900;	this->presetWidth_config_horizontal [3][5] = 600;	// 5100
	//this->presetWidth_config_horizontal [4][0] = 4;	this->presetWidth_config_horizontal [4][1] = 1200;	this->presetWidth_config_horizontal [4][2] = 1200;	this->presetWidth_config_horizontal [4][3] = 1200;	this->presetWidth_config_horizontal [4][4] = 1200;	this->presetWidth_config_horizontal [4][5] = 0;		// 4800
	//this->presetWidth_config_horizontal [5][0] = 4;	this->presetWidth_config_horizontal [5][1] = 1200;	this->presetWidth_config_horizontal [5][2] = 1200;	this->presetWidth_config_horizontal [5][3] = 1200;	this->presetWidth_config_horizontal [5][4] = 900;	this->presetWidth_config_horizontal [5][5] = 0;		// 4500
	//this->presetWidth_config_horizontal [6][0] = 4;	this->presetWidth_config_horizontal [6][1] = 1200;	this->presetWidth_config_horizontal [6][2] = 1200;	this->presetWidth_config_horizontal [6][3] = 900;	this->presetWidth_config_horizontal [6][4] = 900;	this->presetWidth_config_horizontal [6][5] = 0;		// 4200
	//this->presetWidth_config_horizontal [7][0] = 4;	this->presetWidth_config_horizontal [7][1] = 1200;	this->presetWidth_config_horizontal [7][2] = 1200;	this->presetWidth_config_horizontal [7][3] = 900;	this->presetWidth_config_horizontal [7][4] = 600;	this->presetWidth_config_horizontal [7][5] = 0;		// 3900
	//this->presetWidth_config_horizontal [8][0] = 3;	this->presetWidth_config_horizontal [8][1] = 1200;	this->presetWidth_config_horizontal [8][2] = 1200;	this->presetWidth_config_horizontal [8][3] = 1200;	this->presetWidth_config_horizontal [8][4] = 0;		this->presetWidth_config_horizontal [8][5] = 0;		// 3600
	//this->presetWidth_config_horizontal [9][0] = 3;	this->presetWidth_config_horizontal [9][1] = 1200;	this->presetWidth_config_horizontal [9][2] = 1200;	this->presetWidth_config_horizontal [9][3] = 900;	this->presetWidth_config_horizontal [9][4] = 0;		this->presetWidth_config_horizontal [9][5] = 0;		// 3300
	//this->presetWidth_config_horizontal [10][0] = 3;this->presetWidth_config_horizontal [10][1] = 1200;	this->presetWidth_config_horizontal [10][2] = 1200;	this->presetWidth_config_horizontal [10][3] = 600;	this->presetWidth_config_horizontal [10][4] = 0;	this->presetWidth_config_horizontal [10][5] = 0;	// 3000
	//this->presetWidth_config_horizontal [11][0] = 3;this->presetWidth_config_horizontal [11][1] = 1200;	this->presetWidth_config_horizontal [11][2] = 900;	this->presetWidth_config_horizontal [11][3] = 600;	this->presetWidth_config_horizontal [11][4] = 0;	this->presetWidth_config_horizontal [11][5] = 0;	// 2700
	//this->presetWidth_config_horizontal [12][0] = 2;this->presetWidth_config_horizontal [12][1] = 1200;	this->presetWidth_config_horizontal [12][2] = 1200;	this->presetWidth_config_horizontal [12][3] = 0;	this->presetWidth_config_horizontal [12][4] = 0;	this->presetWidth_config_horizontal [12][5] = 0;	// 2400
	//this->presetWidth_config_horizontal [13][0] = 2;this->presetWidth_config_horizontal [13][1] = 1200;	this->presetWidth_config_horizontal [13][2] = 900;	this->presetWidth_config_horizontal [13][3] = 0;	this->presetWidth_config_horizontal [13][4] = 0;	this->presetWidth_config_horizontal [13][5] = 0;	// 2100
	//this->presetWidth_config_horizontal [14][0] = 2;this->presetWidth_config_horizontal [14][1] = 900;	this->presetWidth_config_horizontal [14][2] = 900;	this->presetWidth_config_horizontal [14][3] = 0;	this->presetWidth_config_horizontal [14][4] = 0;	this->presetWidth_config_horizontal [14][5] = 0;	// 1800
	//this->presetWidth_config_horizontal [15][0] = 2;this->presetWidth_config_horizontal [15][1] = 900;	this->presetWidth_config_horizontal [15][2] = 600;	this->presetWidth_config_horizontal [15][3] = 0;	this->presetWidth_config_horizontal [15][4] = 0;	this->presetWidth_config_horizontal [15][5] = 0;	// 1500

	//this->presetHeight_config_horizontal [0][0] = 4;	this->presetHeight_config_horizontal [0][1] = 600;	this->presetHeight_config_horizontal [0][2] = 600;	this->presetHeight_config_horizontal [0][3] = 600;	this->presetHeight_config_horizontal [0][4] = 500;		// 2300
	//this->presetHeight_config_horizontal [1][0] = 4;	this->presetHeight_config_horizontal [1][1] = 600;	this->presetHeight_config_horizontal [1][2] = 600;	this->presetHeight_config_horizontal [1][3] = 450;	this->presetHeight_config_horizontal [1][4] = 600;		// 2250
	//this->presetHeight_config_horizontal [2][0] = 4;	this->presetHeight_config_horizontal [2][1] = 600;	this->presetHeight_config_horizontal [2][2] = 600;	this->presetHeight_config_horizontal [2][3] = 600;	this->presetHeight_config_horizontal [2][4] = 400;		// 2200
	//this->presetHeight_config_horizontal [3][0] = 4;	this->presetHeight_config_horizontal [3][1] = 600;	this->presetHeight_config_horizontal [3][2] = 450;	this->presetHeight_config_horizontal [3][3] = 600;	this->presetHeight_config_horizontal [3][4] = 500;		// 2150
	//this->presetHeight_config_horizontal [4][0] = 4;	this->presetHeight_config_horizontal [4][1] = 600;	this->presetHeight_config_horizontal [4][2] = 300;	this->presetHeight_config_horizontal [4][3] = 600;	this->presetHeight_config_horizontal [4][4] = 600;		// 2100
	//this->presetHeight_config_horizontal [5][0] = 4;	this->presetHeight_config_horizontal [5][1] = 600;	this->presetHeight_config_horizontal [5][2] = 600;	this->presetHeight_config_horizontal [5][3] = 450;	this->presetHeight_config_horizontal [5][4] = 400;		// 2050
	//this->presetHeight_config_horizontal [6][0] = 4;	this->presetHeight_config_horizontal [6][1] = 600;	this->presetHeight_config_horizontal [6][2] = 600;	this->presetHeight_config_horizontal [6][3] = 600;	this->presetHeight_config_horizontal [6][4] = 200;		// 2000
	//this->presetHeight_config_horizontal [7][0] = 4;	this->presetHeight_config_horizontal [7][1] = 600;	this->presetHeight_config_horizontal [7][2] = 300;	this->presetHeight_config_horizontal [7][3] = 450;	this->presetHeight_config_horizontal [7][4] = 600;		// 1950
	//this->presetHeight_config_horizontal [8][0] = 4;	this->presetHeight_config_horizontal [8][1] = 600;	this->presetHeight_config_horizontal [8][2] = 600;	this->presetHeight_config_horizontal [8][3] = 200;	this->presetHeight_config_horizontal [8][4] = 500;		// 1900
	//this->presetHeight_config_horizontal [9][0] = 4;	this->presetHeight_config_horizontal [9][1] = 600;	this->presetHeight_config_horizontal [9][2] = 600;	this->presetHeight_config_horizontal [9][3] = 450;	this->presetHeight_config_horizontal [9][4] = 200;		// 1850
	//this->presetHeight_config_horizontal [10][0] = 3;	this->presetHeight_config_horizontal [10][1] = 600;	this->presetHeight_config_horizontal [10][2] = 600;	this->presetHeight_config_horizontal [10][3] = 600;	this->presetHeight_config_horizontal [10][4] = 0;		// 1800
	//this->presetHeight_config_horizontal [11][0] = 4;	this->presetHeight_config_horizontal [11][1] = 600;	this->presetHeight_config_horizontal [11][2] = 450;	this->presetHeight_config_horizontal [11][3] = 200;	this->presetHeight_config_horizontal [11][4] = 500;		// 1750
	//this->presetHeight_config_horizontal [12][0] = 3;	this->presetHeight_config_horizontal [12][1] = 600;	this->presetHeight_config_horizontal [12][2] = 600;	this->presetHeight_config_horizontal [12][3] = 500;	this->presetHeight_config_horizontal [12][4] = 0;		// 1700
	//this->presetHeight_config_horizontal [13][0] = 3;	this->presetHeight_config_horizontal [13][1] = 600;	this->presetHeight_config_horizontal [13][2] = 450;	this->presetHeight_config_horizontal [13][3] = 600;	this->presetHeight_config_horizontal [13][4] = 0;		// 1650
	//this->presetHeight_config_horizontal [14][0] = 3;	this->presetHeight_config_horizontal [14][1] = 600;	this->presetHeight_config_horizontal [14][2] = 600;	this->presetHeight_config_horizontal [14][3] = 400;	this->presetHeight_config_horizontal [14][4] = 0;		// 1600
	//this->presetHeight_config_horizontal [15][0] = 3;	this->presetHeight_config_horizontal [15][1] = 600;	this->presetHeight_config_horizontal [15][2] = 450;	this->presetHeight_config_horizontal [15][3] = 500;	this->presetHeight_config_horizontal [15][4] = 0;		// 1550
	//this->presetHeight_config_horizontal [16][0] = 3;	this->presetHeight_config_horizontal [16][1] = 600;	this->presetHeight_config_horizontal [16][2] = 300;	this->presetHeight_config_horizontal [16][3] = 600;	this->presetHeight_config_horizontal [16][4] = 0;		// 1500
	//this->presetHeight_config_horizontal [17][0] = 3;	this->presetHeight_config_horizontal [17][1] = 600;	this->presetHeight_config_horizontal [17][2] = 450;	this->presetHeight_config_horizontal [17][3] = 400;	this->presetHeight_config_horizontal [17][4] = 0;		// 1450
	//this->presetHeight_config_horizontal [18][0] = 3;	this->presetHeight_config_horizontal [18][1] = 600;	this->presetHeight_config_horizontal [18][2] = 300;	this->presetHeight_config_horizontal [18][3] = 500;	this->presetHeight_config_horizontal [18][4] = 0;		// 1400
	//this->presetHeight_config_horizontal [19][0] = 3;	this->presetHeight_config_horizontal [19][1] = 450;	this->presetHeight_config_horizontal [19][2] = 300;	this->presetHeight_config_horizontal [19][3] = 600;	this->presetHeight_config_horizontal [19][4] = 0;		// 1350
	//this->presetHeight_config_horizontal [20][0] = 3;	this->presetHeight_config_horizontal [20][1] = 600;	this->presetHeight_config_horizontal [20][2] = 200;	this->presetHeight_config_horizontal [20][3] = 500;	this->presetHeight_config_horizontal [20][4] = 0;		// 1300
	//this->presetHeight_config_horizontal [21][0] = 3;	this->presetHeight_config_horizontal [21][1] = 450;	this->presetHeight_config_horizontal [21][2] = 600;	this->presetHeight_config_horizontal [21][3] = 200;	this->presetHeight_config_horizontal [21][4] = 0;		// 1250
	//this->presetHeight_config_horizontal [22][0] = 2;	this->presetHeight_config_horizontal [22][1] = 600;	this->presetHeight_config_horizontal [22][2] = 600;	this->presetHeight_config_horizontal [22][3] = 0;	this->presetHeight_config_horizontal [22][4] = 0;		// 1200
	//this->presetHeight_config_horizontal [23][0] = 3;	this->presetHeight_config_horizontal [23][1] = 450;	this->presetHeight_config_horizontal [23][2] = 300;	this->presetHeight_config_horizontal [23][3] = 400;	this->presetHeight_config_horizontal [23][4] = 0;		// 1150
	//this->presetHeight_config_horizontal [24][0] = 2;	this->presetHeight_config_horizontal [24][1] = 600;	this->presetHeight_config_horizontal [24][2] = 500;	this->presetHeight_config_horizontal [24][3] = 0;	this->presetHeight_config_horizontal [24][4] = 0;		// 1100
	//this->presetHeight_config_horizontal [25][0] = 3;	this->presetHeight_config_horizontal [25][1] = 300;	this->presetHeight_config_horizontal [25][2] = 300;	this->presetHeight_config_horizontal [25][3] = 450;	this->presetHeight_config_horizontal [25][4] = 0;		// 1050
	//this->presetHeight_config_horizontal [26][0] = 2;	this->presetHeight_config_horizontal [26][1] = 600;	this->presetHeight_config_horizontal [26][2] = 400;	this->presetHeight_config_horizontal [26][3] = 0;	this->presetHeight_config_horizontal [26][4] = 0;		// 1000
	//this->presetHeight_config_horizontal [27][0] = 2;	this->presetHeight_config_horizontal [27][1] = 450;	this->presetHeight_config_horizontal [27][2] = 500;	this->presetHeight_config_horizontal [27][3] = 0;	this->presetHeight_config_horizontal [27][4] = 0;		// 950
	//this->presetHeight_config_horizontal [28][0] = 2;	this->presetHeight_config_horizontal [28][1] = 600;	this->presetHeight_config_horizontal [28][2] = 300;	this->presetHeight_config_horizontal [28][3] = 0;	this->presetHeight_config_horizontal [28][4] = 0;		// 900
	//this->presetHeight_config_horizontal [29][0] = 2;	this->presetHeight_config_horizontal [29][1] = 450;	this->presetHeight_config_horizontal [29][2] = 400;	this->presetHeight_config_horizontal [29][3] = 0;	this->presetHeight_config_horizontal [29][4] = 0;		// 850
	//this->presetHeight_config_horizontal [30][0] = 2;	this->presetHeight_config_horizontal [30][1] = 400;	this->presetHeight_config_horizontal [30][2] = 400;	this->presetHeight_config_horizontal [30][3] = 0;	this->presetHeight_config_horizontal [30][4] = 0;		// 800
	//this->presetHeight_config_horizontal [31][0] = 2;	this->presetHeight_config_horizontal [31][1] = 300;	this->presetHeight_config_horizontal [31][2] = 450;	this->presetHeight_config_horizontal [31][3] = 0;	this->presetHeight_config_horizontal [31][4] = 0;		// 750
	//this->presetHeight_config_horizontal [32][0] = 2;	this->presetHeight_config_horizontal [32][1] = 300;	this->presetHeight_config_horizontal [32][2] = 400;	this->presetHeight_config_horizontal [32][3] = 0;	this->presetHeight_config_horizontal [32][4] = 0;		// 700
	//this->presetHeight_config_horizontal [33][0] = 2;	this->presetHeight_config_horizontal [33][1] = 450;	this->presetHeight_config_horizontal [33][2] = 200;	this->presetHeight_config_horizontal [33][3] = 0;	this->presetHeight_config_horizontal [33][4] = 0;		// 650
	//this->presetHeight_config_horizontal [34][0] = 1;	this->presetHeight_config_horizontal [34][1] = 600;	this->presetHeight_config_horizontal [34][2] = 0;	this->presetHeight_config_horizontal [34][3] = 0;	this->presetHeight_config_horizontal [34][4] = 0;		// 600
	//this->presetHeight_config_horizontal [35][0] = 1;	this->presetHeight_config_horizontal [35][1] = 500;	this->presetHeight_config_horizontal [35][2] = 0;	this->presetHeight_config_horizontal [35][3] = 0;	this->presetHeight_config_horizontal [35][4] = 0;		// 500
	//this->presetHeight_config_horizontal [36][0] = 1;	this->presetHeight_config_horizontal [36][1] = 450;	this->presetHeight_config_horizontal [36][2] = 0;	this->presetHeight_config_horizontal [36][3] = 0;	this->presetHeight_config_horizontal [36][4] = 0;		// 450
	//this->presetHeight_config_horizontal [37][0] = 1;	this->presetHeight_config_horizontal [37][1] = 400;	this->presetHeight_config_horizontal [37][2] = 0;	this->presetHeight_config_horizontal [37][3] = 0;	this->presetHeight_config_horizontal [37][4] = 0;		// 400
	//this->presetHeight_config_horizontal [38][0] = 1;	this->presetHeight_config_horizontal [38][1] = 300;	this->presetHeight_config_horizontal [38][2] = 0;	this->presetHeight_config_horizontal [38][3] = 0;	this->presetHeight_config_horizontal [38][4] = 0;		// 300
	//this->presetHeight_config_horizontal [39][0] = 1;	this->presetHeight_config_horizontal [39][1] = 200;	this->presetHeight_config_horizontal [39][2] = 0;	this->presetHeight_config_horizontal [39][3] = 0;	this->presetHeight_config_horizontal [39][4] = 0;		// 200
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

// ��(0-��� �ε��� ��ȣ)�� ���ϴ� �� ��ġ X ��ǥ�� ���� ...
double	LowSideTableformPlacingZone::getCellPositionLeftBottomX (LowSideTableformPlacingZone* placingZone, short idx)
{
	double	distance = 0.0;
	//double	distance = (placingZone->bLincorner == true) ? placingZone->lenLincorner : 0;

	//for (short xx = 0 ; xx < idx ; ++xx)
	//	distance += (double)placingZone->cells [xx].horLen / 1000.0;

	return distance;
}

// �� ��ġ�� �ٸ��� ������ ...
void	LowSideTableformPlacingZone::adjustCellsPosition (LowSideTableformPlacingZone* placingZone)
{
	//for (short xx = 0 ; xx < placingZone->nCellsInHor ; ++xx) {
	//	placingZone->cells [xx].ang = placingZone->ang;
	//	placingZone->cells [xx].leftBottomX = placingZone->leftBottomX + (placingZone->gap * sin(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * cos(placingZone->ang));
	//	placingZone->cells [xx].leftBottomY = placingZone->leftBottomY - (placingZone->gap * cos(placingZone->ang)) + (placingZone->getCellPositionLeftBottomX (placingZone, xx) * sin(placingZone->ang));
	//	placingZone->cells [xx].leftBottomZ = placingZone->leftBottomZ;
	//}
}

// �� ������ ������� ��ü���� ��ġ�� ...
GSErrCode	LowSideTableformPlacingZone::placeObjects (LowSideTableformPlacingZone* placingZone)
{
	GSErrCode	err = NoError;
	//short	xx, yy, varEnd;
	//double	accumDist;
	//double	lengthDouble;
	//int		lengthInt;
	//int		remainLengthInt;

	//// ================================================== ���ڳ� ��ġ
	//// ���� ���ڳ� ��ġ
	//if (placingZone->bLincorner == true) {
	//	// �ո�
	//	EasyObjectPlacement incorner;
	//	incorner.init (L("���ڳ��ǳ�v1.0.gsm"), layerInd_IncornerPanel, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang - DegreeToRad (90.0));

	//	moveIn3D ('y', incorner.radAng + DegreeToRad (90.0), -placingZone->gap, &incorner.posX, &incorner.posY, &incorner.posZ);	// ������ ���ݸ�ŭ �̵�

	//	for (xx = 0 ; xx < placingZone->nCellsInVerBasic ; ++xx) {
	//		elemList_Front.Push (incorner.placeObject (5,
	//			"in_comp", APIParT_CString, "���ڳ��ǳ�",
	//			"wid_s", APIParT_Length, format_string ("%f", 0.100),
	//			"leng_s", APIParT_Length, format_string ("%f", placingZone->lenLincorner),
	//			"hei_s", APIParT_Length, format_string ("%f", (double)placingZone->cells [0].tableInVerBasic [xx] / 1000.0),
	//			"dir_s", APIParT_CString, "�����"));

	//		moveIn3D ('z', incorner.radAng + DegreeToRad (90.0), (double)placingZone->cells [0].tableInVerBasic [xx] / 1000.0, &incorner.posX, &incorner.posY, &incorner.posZ);
	//	}

	//	// �޸�
	//	if (placingZone->bSingleSide == false) {
	//		incorner.init (L("���ڳ��ǳ�v1.0.gsm"), layerInd_IncornerPanel, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang);

	//		moveIn3D ('y', incorner.radAng, infoWall.wallThk + placingZone->gap, &incorner.posX, &incorner.posY, &incorner.posZ);		// ������ ���ݸ�ŭ �̵�

	//		varEnd = (placingZone->bExtra == true) ? placingZone->nCellsInVerExtra : placingZone->nCellsInVerBasic;

	//		for (xx = 0 ; xx < varEnd ; ++xx) {
	//			if (placingZone->bExtra == true)
	//				lengthDouble = (double)placingZone->cells [0].tableInVerExtra [xx] / 1000.0;
	//			else
	//				lengthDouble = (double)placingZone->cells [0].tableInVerBasic [xx] / 1000.0;

	//			elemList_Back.Push (incorner.placeObject (5,
	//				"in_comp", APIParT_CString, "���ڳ��ǳ�",
	//				"wid_s", APIParT_Length, format_string ("%f", placingZone->lenLincorner),
	//				"leng_s", APIParT_Length, format_string ("%f", 0.100),
	//				"hei_s", APIParT_Length, format_string ("%f", lengthDouble),
	//				"dir_s", APIParT_CString, "�����"));

	//			moveIn3D ('z', incorner.radAng, lengthDouble, &incorner.posX, &incorner.posY, &incorner.posZ);
	//		}
	//	}
	//}
	//
	//// ����� ��ü �׷�ȭ (�ո�)
	//if (!elemList_Front.IsEmpty ()) {
	//	GSSize nElems = elemList_Front.GetSize ();
	//	API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
	//	if (elemHead != NULL) {
	//		for (GSIndex i = 0; i < nElems; i++)
	//			(*elemHead)[i].guid = elemList_Front [i];

	//		ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

	//		BMKillHandle ((GSHandle *) &elemHead);
	//	}
	//	elemList_Front.Clear ();
	//}

	//// ����� ��ü �׷�ȭ (�޸�)
	//if (!elemList_Back.IsEmpty ()) {
	//	GSSize nElems = elemList_Back.GetSize ();
	//	API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
	//	if (elemHead != NULL) {
	//		for (GSIndex i = 0; i < nElems; i++)
	//			(*elemHead)[i].guid = elemList_Back [i];

	//		ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

	//		BMKillHandle ((GSHandle *) &elemHead);
	//	}
	//	elemList_Back.Clear ();
	//}

	//// ���� ���ڳ� ��ġ
	//if (placingZone->bRincorner == true) {
	//	// �ո�
	//	EasyObjectPlacement incorner;
	//	incorner.init (L("���ڳ��ǳ�v1.0.gsm"), layerInd_IncornerPanel, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang - DegreeToRad (180.0));

	//	moveIn3D ('y', incorner.radAng + DegreeToRad (180.0), -placingZone->gap, &incorner.posX, &incorner.posY, &incorner.posZ);		// ������ ���ݸ�ŭ �̵�
	//	moveIn3D ('x', incorner.radAng + DegreeToRad (180.0), placingZone->horLen, &incorner.posX, &incorner.posY, &incorner.posZ);		// ���� �������� �̵�

	//	for (xx = 0 ; xx < placingZone->nCellsInVerBasic ; ++xx) {
	//		elemList_Front.Push (incorner.placeObject (5,
	//			"in_comp", APIParT_CString, "���ڳ��ǳ�",
	//			"wid_s", APIParT_Length, format_string ("%f", placingZone->lenRincorner),
	//			"leng_s", APIParT_Length, format_string ("%f", 0.100),
	//			"hei_s", APIParT_Length, format_string ("%f", (double)placingZone->cells [0].tableInVerBasic [xx] / 1000.0),
	//			"dir_s", APIParT_CString, "�����"));

	//		moveIn3D ('z', incorner.radAng + DegreeToRad (180.0), (double)placingZone->cells [0].tableInVerBasic [xx] / 1000.0, &incorner.posX, &incorner.posY, &incorner.posZ);
	//	}

	//	// �޸�
	//	if (placingZone->bSingleSide == false) {
	//		incorner.init (L("���ڳ��ǳ�v1.0.gsm"), layerInd_IncornerPanel, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang + DegreeToRad (90.0));

	//		moveIn3D ('y', incorner.radAng - DegreeToRad (90.0), infoWall.wallThk + placingZone->gap, &incorner.posX, &incorner.posY, &incorner.posZ);	// ������ ���ݸ�ŭ �̵�
	//		moveIn3D ('x', incorner.radAng - DegreeToRad (90.0), placingZone->horLen, &incorner.posX, &incorner.posY, &incorner.posZ);					// ���� �������� �̵�

	//		varEnd = (placingZone->bExtra == true) ? placingZone->nCellsInVerExtra : placingZone->nCellsInVerBasic;

	//		for (xx = 0 ; xx < varEnd ; ++xx) {
	//			if (placingZone->bExtra == true)
	//				lengthDouble = (double)placingZone->cells [0].tableInVerExtra [xx] / 1000.0;
	//			else
	//				lengthDouble = (double)placingZone->cells [0].tableInVerBasic [xx] / 1000.0;

	//			elemList_Back.Push (incorner.placeObject (5,
	//				"in_comp", APIParT_CString, "���ڳ��ǳ�",
	//				"wid_s", APIParT_Length, format_string ("%f", 0.100),
	//				"leng_s", APIParT_Length, format_string ("%f", placingZone->lenRincorner),
	//				"hei_s", APIParT_Length, format_string ("%f", lengthDouble),
	//				"dir_s", APIParT_CString, "�����"));

	//			moveIn3D ('z', incorner.radAng, lengthDouble, &incorner.posX, &incorner.posY, &incorner.posZ);
	//		}
	//	}
	//}

	//// ����� ��ü �׷�ȭ (�ո�)
	//if (!elemList_Front.IsEmpty ()) {
	//	GSSize nElems = elemList_Front.GetSize ();
	//	API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
	//	if (elemHead != NULL) {
	//		for (GSIndex i = 0; i < nElems; i++)
	//			(*elemHead)[i].guid = elemList_Front [i];

	//		ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

	//		BMKillHandle ((GSHandle *) &elemHead);
	//	}
	//	elemList_Front.Clear ();
	//}

	//// ����� ��ü �׷�ȭ (�޸�)
	//if (!elemList_Back.IsEmpty ()) {
	//	GSSize nElems = elemList_Back.GetSize ();
	//	API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
	//	if (elemHead != NULL) {
	//		for (GSIndex i = 0; i < nElems; i++)
	//			(*elemHead)[i].guid = elemList_Back [i];

	//		ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

	//		BMKillHandle ((GSHandle *) &elemHead);
	//	}
	//	elemList_Back.Clear ();
	//}

	//// ================================================== ������ ��ġ
	//// �ո� ��ġ
	//EasyObjectPlacement euroform;
	//euroform.init (L("������v2.0.gsm"), layerInd_Euroform, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang);

	//if (placingZone->bLincorner == true)	moveIn3D ('x', euroform.radAng, placingZone->lenLincorner, &euroform.posX, &euroform.posY, &euroform.posZ);		// ���� ���ڳ� ������ x �̵�
	//moveIn3D ('y', euroform.radAng, -placingZone->gap, &euroform.posX, &euroform.posY, &euroform.posZ);														// ������ ���ݸ�ŭ �̵�

	//for (xx = 0 ; xx < placingZone->nCellsInHor ; ++xx) {
	//	if (placingZone->cells [xx].objType == EUROFORM) {
	//		accumDist = 0.0;

	//		for (yy = 0 ; yy < placingZone->nCellsInVerBasic ; ++yy) {
	//			if (placingZone->bVertical == true) {
	//				// ���ι���
	//				elemList_Front.Push (euroform.placeObject (5,
	//					"eu_stan_onoff", APIParT_Boolean, "1.0",
	//					"eu_wid", APIParT_CString, format_string ("%d", placingZone->cells [xx].horLen),
	//					"eu_hei", APIParT_CString, format_string ("%d", placingZone->cells [xx].tableInVerBasic [yy]),
	//					"u_ins", APIParT_CString, "�������",
	//					"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
	//			} else {
	//				// ���ι���
	//				moveIn3D ('x', euroform.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
	//				elemList_Front.Push (euroform.placeObject (5,
	//					"eu_stan_onoff", APIParT_Boolean, "1.0",
	//					"eu_wid", APIParT_CString, format_string ("%d", placingZone->cells [xx].tableInVerBasic [yy]),
	//					"eu_hei", APIParT_CString, format_string ("%d", placingZone->cells [xx].horLen),
	//					"u_ins", APIParT_CString, "��������",
	//					"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
	//				moveIn3D ('x', euroform.radAng, -(double)placingZone->cells [xx].horLen / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
	//			}
	//			accumDist += (double)placingZone->cells [xx].tableInVerBasic [yy] / 1000.0;
	//			moveIn3D ('z', euroform.radAng, (double)placingZone->cells [xx].tableInVerBasic [yy] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
	//		}
	//		moveIn3D ('z', euroform.radAng, -accumDist, &euroform.posX, &euroform.posY, &euroform.posZ);

	//		// ����� ��ü �׷�ȭ (�ո�)
	//		if (!elemList_Front.IsEmpty ()) {
	//			GSSize nElems = elemList_Front.GetSize ();
	//			API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
	//			if (elemHead != NULL) {
	//				for (GSIndex i = 0; i < nElems; i++)
	//					(*elemHead)[i].guid = elemList_Front [i];

	//				ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

	//				BMKillHandle ((GSHandle *) &elemHead);
	//			}
	//			elemList_Front.Clear ();
	//		}
	//	}

	//	// ������ ���� �������� �̵�
	//	moveIn3D ('x', euroform.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
	//}

	//// �޸� ��ġ
	//if (placingZone->bSingleSide == false) {
	//	euroform.init (L("������v2.0.gsm"), layerInd_Euroform, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang + DegreeToRad (180.0));

	//	if (placingZone->bLincorner == true)	moveIn3D ('x', euroform.radAng - DegreeToRad (180.0), placingZone->lenLincorner, &euroform.posX, &euroform.posY, &euroform.posZ);	// ���� ���ڳ� ������ x �̵�
	//	moveIn3D ('y', euroform.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap, &euroform.posX, &euroform.posY, &euroform.posZ);									// ������ ���ݸ�ŭ �̵�

	//	for (xx = 0 ; xx < placingZone->nCellsInHor ; ++xx) {
	//		if (placingZone->cells [xx].objType == EUROFORM) {
	//			accumDist = 0.0;

	//			varEnd = (placingZone->bExtra == true) ? placingZone->nCellsInVerExtra : placingZone->nCellsInVerBasic;

	//			for (yy = 0 ; yy < varEnd ; ++yy) {
	//				if (placingZone->bExtra == true)
	//					lengthInt = placingZone->cells [xx].tableInVerExtra [yy];
	//				else
	//					lengthInt = placingZone->cells [xx].tableInVerBasic [yy];

	//				if (placingZone->bVertical == true) {
	//					// ���ι���
	//					moveIn3D ('x', euroform.radAng, -(double)placingZone->cells [xx].horLen / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
	//					elemList_Back.Push (euroform.placeObject (5,
	//						"eu_stan_onoff", APIParT_Boolean, "1.0",
	//						"eu_wid", APIParT_CString, format_string ("%d", placingZone->cells [xx].horLen),
	//						"eu_hei", APIParT_CString, format_string ("%d", lengthInt),
	//						"u_ins", APIParT_CString, "�������",
	//						"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
	//					moveIn3D ('x', euroform.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
	//				} else {
	//					// ���ι���
	//					elemList_Back.Push (euroform.placeObject (5,
	//						"eu_stan_onoff", APIParT_Boolean, "1.0",
	//						"eu_wid", APIParT_CString, format_string ("%d", lengthInt),
	//						"eu_hei", APIParT_CString, format_string ("%d", placingZone->cells [xx].horLen),
	//						"u_ins", APIParT_CString, "��������",
	//						"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
	//				}
	//				accumDist += (double)lengthInt / 1000.0;
	//				moveIn3D ('z', euroform.radAng, (double)lengthInt / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
	//			}
	//			moveIn3D ('z', euroform.radAng, -accumDist, &euroform.posX, &euroform.posY, &euroform.posZ);

	//			// ����� ��ü �׷�ȭ (�޸�)
	//			if (!elemList_Back.IsEmpty ()) {
	//				GSSize nElems = elemList_Back.GetSize ();
	//				API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
	//				if (elemHead != NULL) {
	//					for (GSIndex i = 0; i < nElems; i++)
	//						(*elemHead)[i].guid = elemList_Back [i];

	//					ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

	//					BMKillHandle ((GSHandle *) &elemHead);
	//				}
	//				elemList_Back.Clear ();
	//			}
	//		}

	//		// ������ ���� �������� �̵�
	//		moveIn3D ('x', euroform.radAng, -(double)placingZone->cells [xx].horLen / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
	//	}
	//}

	//// ================================================== �ٷ������̼� ��ġ (�׻� ���ι���)
	//// �ո� ��ġ
	//EasyObjectPlacement fillersp;
	//fillersp.init (L("�ٷ������̼�v1.0.gsm"), layerInd_Fillersp, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang);

	//if (placingZone->bLincorner == true)	moveIn3D ('x', fillersp.radAng, placingZone->lenLincorner, &fillersp.posX, &fillersp.posY, &fillersp.posZ);		// ���� ���ڳ� ������ x �̵�
	//moveIn3D ('y', fillersp.radAng, -placingZone->gap, &fillersp.posX, &fillersp.posY, &fillersp.posZ);														// ������ ���ݸ�ŭ �̵�

	//for (xx = 0 ; xx < placingZone->nCellsInHor ; ++xx) {
	//	if (placingZone->cells [xx].objType == FILLERSP) {
	//		accumDist = 0.0;
	//		remainLengthInt = 0;
	//		for (yy = 0 ; yy < placingZone->nCellsInVerBasic ; ++yy) {
	//			remainLengthInt += placingZone->cells [xx].tableInVerBasic [yy];
	//			accumDist += (double)placingZone->cells [xx].tableInVerBasic [yy] / 1000.0;
	//		}

	//		moveIn3D ('x', fillersp.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &fillersp.posX, &fillersp.posY, &fillersp.posZ);
	//		while (remainLengthInt > 0) {
	//			if (remainLengthInt >= 2400)
	//				lengthInt = 2400;
	//			else
	//				lengthInt = remainLengthInt;

	//			elemList_Front.Push (fillersp.placeObject (4,
	//				"f_thk", APIParT_Length, format_string ("%f", (double)placingZone->cells [xx].horLen / 1000.0),
	//				"f_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0),
	//				"f_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
	//				"f_rota", APIParT_Angle, format_string ("%f", 0.0)));
	//			moveIn3D ('z', fillersp.radAng, (double)lengthInt / 1000.0, &fillersp.posX, &fillersp.posY, &fillersp.posZ);

	//			remainLengthInt -= 2400;
	//		}
	//		moveIn3D ('x', fillersp.radAng, -(double)placingZone->cells [xx].horLen / 1000.0, &fillersp.posX, &fillersp.posY, &fillersp.posZ);
	//		moveIn3D ('z', fillersp.radAng, -accumDist, &fillersp.posX, &fillersp.posY, &fillersp.posZ);

	//		// ����� ��ü �׷�ȭ (�ո�)
	//		if (!elemList_Front.IsEmpty ()) {
	//			GSSize nElems = elemList_Front.GetSize ();
	//			API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
	//			if (elemHead != NULL) {
	//				for (GSIndex i = 0; i < nElems; i++)
	//					(*elemHead)[i].guid = elemList_Front [i];

	//				ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

	//				BMKillHandle ((GSHandle *) &elemHead);
	//			}
	//			elemList_Front.Clear ();
	//		}
	//	}

	//	// ������ ���� �������� �̵�
	//	moveIn3D ('x', fillersp.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &fillersp.posX, &fillersp.posY, &fillersp.posZ);
	//}

	//// �޸� ��ġ
	//if (placingZone->bSingleSide == false) {
	//	fillersp.init (L("�ٷ������̼�v1.0.gsm"), layerInd_Fillersp, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang + DegreeToRad (180.0));

	//	if (placingZone->bLincorner == true)	moveIn3D ('x', fillersp.radAng - DegreeToRad (180.0), placingZone->lenLincorner, &fillersp.posX, &fillersp.posY, &fillersp.posZ);	// ���� ���ڳ� ������ x �̵�
	//	moveIn3D ('y', fillersp.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap, &fillersp.posX, &fillersp.posY, &fillersp.posZ);									// ������ ���ݸ�ŭ �̵�

	//	for (xx = 0 ; xx < placingZone->nCellsInHor ; ++xx) {
	//		if (placingZone->cells [xx].objType == FILLERSP) {
	//			accumDist = 0.0;
	//			remainLengthInt = 0;

	//			varEnd = (placingZone->bExtra == true) ? placingZone->nCellsInVerExtra : placingZone->nCellsInVerBasic;

	//			for (yy = 0 ; yy < varEnd ; ++yy) {
	//				if (placingZone->bExtra == true)
	//					lengthInt = placingZone->cells [xx].tableInVerExtra [yy];
	//				else
	//					lengthInt = placingZone->cells [xx].tableInVerBasic [yy];

	//				remainLengthInt += lengthInt;
	//				accumDist += (double)lengthInt / 1000.0;
	//			}

	//			while (remainLengthInt > 0) {
	//				if (remainLengthInt >= 2400)
	//					lengthInt = 2400;
	//				else
	//					lengthInt = remainLengthInt;

	//				elemList_Back.Push (fillersp.placeObject (4,
	//					"f_thk", APIParT_Length, format_string ("%f", (double)placingZone->cells [xx].horLen / 1000.0),
	//					"f_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0),
	//					"f_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
	//					"f_rota", APIParT_Angle, format_string ("%f", 0.0)));
	//				moveIn3D ('z', fillersp.radAng, (double)lengthInt / 1000.0, &fillersp.posX, &fillersp.posY, &fillersp.posZ);

	//				remainLengthInt -= 2400;
	//			}
	//			moveIn3D ('z', fillersp.radAng, -accumDist, &fillersp.posX, &fillersp.posY, &fillersp.posZ);

	//			// ����� ��ü �׷�ȭ (�޸�)
	//			if (!elemList_Back.IsEmpty ()) {
	//				GSSize nElems = elemList_Back.GetSize ();
	//				API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
	//				if (elemHead != NULL) {
	//					for (GSIndex i = 0; i < nElems; i++)
	//						(*elemHead)[i].guid = elemList_Back [i];

	//					ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

	//					BMKillHandle ((GSHandle *) &elemHead);
	//				}
	//				elemList_Back.Clear ();
	//			}
	//		}

	//		// ������ ���� �������� �̵�
	//		moveIn3D ('x', fillersp.radAng, -(double)placingZone->cells [xx].horLen / 1000.0, &fillersp.posX, &fillersp.posY, &fillersp.posZ);
	//	}
	//}

	//// ================================================== ���� ��ġ (�׻� ���ι���)
	//// �ո� ��ġ
	//EasyObjectPlacement plywood;
	//plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang);

	//if (placingZone->bLincorner == true)	moveIn3D ('x', plywood.radAng, placingZone->lenLincorner, &plywood.posX, &plywood.posY, &plywood.posZ);		// ���� ���ڳ� ������ x �̵�
	//moveIn3D ('y', plywood.radAng, -placingZone->gap, &plywood.posX, &plywood.posY, &plywood.posZ);														// ������ ���ݸ�ŭ �̵�

	//for (xx = 0 ; xx < placingZone->nCellsInHor ; ++xx) {
	//	if (placingZone->cells [xx].objType == PLYWOOD) {
	//		accumDist = 0.0;
	//		remainLengthInt = 0;
	//		for (yy = 0 ; yy < placingZone->nCellsInVerBasic ; ++yy) {
	//			remainLengthInt += placingZone->cells [xx].tableInVerBasic [yy];
	//			accumDist += (double)placingZone->cells [xx].tableInVerBasic [yy] / 1000.0;
	//		}

	//		while (remainLengthInt > 0) {
	//			if (remainLengthInt >= 2400)
	//				lengthInt = 2400;
	//			else
	//				lengthInt = remainLengthInt;

	//			elemList_Front.Push (plywood.placeObject (13,
	//				"p_stan", APIParT_CString, "��԰�",
	//				"w_dir", APIParT_CString, "�������",
	//				"p_thk", APIParT_CString, "11.5T",
	//				"p_wid", APIParT_Length, format_string ("%f", (double)placingZone->cells [xx].horLen / 1000.0),
	//				"p_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0),
	//				"p_ang", APIParT_Angle, format_string ("%f", 0.0),
	//				"sogak", APIParT_Boolean, "1.0",
	//				"bInverseSogak", APIParT_Boolean, "1.0",
	//				"prof", APIParT_CString, "�Ұ�",
	//				"gap_a", APIParT_Length, format_string ("%f", 0.0),
	//				"gap_b", APIParT_Length, format_string ("%f", 0.0),
	//				"gap_c", APIParT_Length, format_string ("%f", 0.0),
	//				"gap_d", APIParT_Length, format_string ("%f", 0.0)));
	//			moveIn3D ('z', plywood.radAng, (double)lengthInt / 1000.0, &plywood.posX, &plywood.posY, &plywood.posZ);

	//			remainLengthInt -= 2400;
	//		}
	//		moveIn3D ('z', plywood.radAng, -accumDist, &plywood.posX, &plywood.posY, &plywood.posZ);

	//		// ����� ��ü �׷�ȭ (�ո�)
	//		if (!elemList_Front.IsEmpty ()) {
	//			GSSize nElems = elemList_Front.GetSize ();
	//			API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
	//			if (elemHead != NULL) {
	//				for (GSIndex i = 0; i < nElems; i++)
	//					(*elemHead)[i].guid = elemList_Front [i];

	//				ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

	//				BMKillHandle ((GSHandle *) &elemHead);
	//			}
	//			elemList_Front.Clear ();
	//		}
	//	}

	//	// ������ ���� �������� �̵�
	//	moveIn3D ('x', plywood.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &plywood.posX, &plywood.posY, &plywood.posZ);
	//}

	//// �޸� ��ġ
	//if (placingZone->bSingleSide == false) {
	//	plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang + DegreeToRad (180.0));

	//	if (placingZone->bLincorner == true)	moveIn3D ('x', plywood.radAng - DegreeToRad (180.0), placingZone->lenLincorner, &plywood.posX, &plywood.posY, &plywood.posZ);		// ���� ���ڳ� ������ x �̵�
	//	moveIn3D ('y', plywood.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap, &plywood.posX, &plywood.posY, &plywood.posZ);										// ������ ���ݸ�ŭ �̵�

	//	for (xx = 0 ; xx < placingZone->nCellsInHor ; ++xx) {
	//		if (placingZone->cells [xx].objType == PLYWOOD) {
	//			accumDist = 0.0;
	//			remainLengthInt = 0;

	//			varEnd = (placingZone->bExtra == true) ? placingZone->nCellsInVerExtra : placingZone->nCellsInVerBasic;

	//			for (yy = 0 ; yy < varEnd ; ++yy) {
	//				if (placingZone->bExtra == true)
	//					lengthInt = placingZone->cells [xx].tableInVerExtra [yy];
	//				else
	//					lengthInt = placingZone->cells [xx].tableInVerBasic [yy];

	//				remainLengthInt += lengthInt;
	//				accumDist += (double)lengthInt / 1000.0;
	//			}

	//			moveIn3D ('x', plywood.radAng, -(double)placingZone->cells [xx].horLen / 1000.0, &plywood.posX, &plywood.posY, &plywood.posZ);
	//			while (remainLengthInt > 0) {
	//				if (remainLengthInt >= 2400)
	//					lengthInt = 2400;
	//				else
	//					lengthInt = remainLengthInt;

	//				elemList_Back.Push (plywood.placeObject (13,
	//					"p_stan", APIParT_CString, "��԰�",
	//					"w_dir", APIParT_CString, "�������",
	//					"p_thk", APIParT_CString, "11.5T",
	//					"p_wid", APIParT_Length, format_string ("%f", (double)placingZone->cells [xx].horLen / 1000.0),
	//					"p_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0),
	//					"p_ang", APIParT_Angle, format_string ("%f", 0.0),
	//					"sogak", APIParT_Boolean, "1.0",
	//					"bInverseSogak", APIParT_Boolean, "1.0",
	//					"prof", APIParT_CString, "�Ұ�",
	//					"gap_a", APIParT_Length, format_string ("%f", 0.0),
	//					"gap_b", APIParT_Length, format_string ("%f", 0.0),
	//					"gap_c", APIParT_Length, format_string ("%f", 0.0),
	//					"gap_d", APIParT_Length, format_string ("%f", 0.0)));
	//				moveIn3D ('z', plywood.radAng, (double)lengthInt / 1000.0, &plywood.posX, &plywood.posY, &plywood.posZ);

	//				remainLengthInt -= 2400;
	//			}
	//			moveIn3D ('x', plywood.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &plywood.posX, &plywood.posY, &plywood.posZ);
	//			moveIn3D ('z', plywood.radAng, -accumDist, &plywood.posX, &plywood.posY, &plywood.posZ);

	//			// ����� ��ü �׷�ȭ (�޸�)
	//			if (!elemList_Back.IsEmpty ()) {
	//				GSSize nElems = elemList_Back.GetSize ();
	//				API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
	//				if (elemHead != NULL) {
	//					for (GSIndex i = 0; i < nElems; i++)
	//						(*elemHead)[i].guid = elemList_Back [i];

	//					ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

	//					BMKillHandle ((GSHandle *) &elemHead);
	//				}
	//				elemList_Back.Clear ();
	//			}
	//		}

	//		// ������ ���� �������� �̵�
	//		moveIn3D ('x', plywood.radAng, -(double)placingZone->cells [xx].horLen / 1000.0, &plywood.posX, &plywood.posY, &plywood.posZ);
	//	}
	//}

	//// ================================================== ���� ��ġ (�׻� ���ι���)
	//// �ո� ��ġ
	//EasyObjectPlacement timber;
	//timber.init (L("����v1.0.gsm"), layerInd_Timber, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang);

	//if (placingZone->bLincorner == true)	moveIn3D ('x', timber.radAng, placingZone->lenLincorner, &timber.posX, &timber.posY, &timber.posZ);		// ���� ���ڳ� ������ x �̵�
	//moveIn3D ('y', timber.radAng, -placingZone->gap, &timber.posX, &timber.posY, &timber.posZ);														// ������ ���ݸ�ŭ �̵�

	//for (xx = 0 ; xx < placingZone->nCellsInHor ; ++xx) {
	//	if (placingZone->cells [xx].objType == TIMBER) {
	//		accumDist = 0.0;
	//		remainLengthInt = 0;
	//		for (yy = 0 ; yy < placingZone->nCellsInVerBasic ; ++yy) {
	//			remainLengthInt += placingZone->cells [xx].tableInVerBasic [yy];
	//			accumDist += (double)placingZone->cells [xx].tableInVerBasic [yy] / 1000.0;
	//		}

	//		moveIn3D ('x', timber.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &timber.posX, &timber.posY, &timber.posZ);
	//		while (remainLengthInt > 0) {
	//			if (remainLengthInt >= 3600)
	//				lengthInt = 3600;
	//			else
	//				lengthInt = remainLengthInt;

	//			elemList_Front.Push (timber.placeObject (6,
	//				"w_ins", APIParT_CString, "�������",
	//				"w_w", APIParT_Length, format_string ("%f", 0.050),
	//				"w_h", APIParT_Length, format_string ("%f", (double)placingZone->cells [xx].horLen / 1000.0),
	//				"w_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0),
	//				"w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
	//				"torsion_ang", APIParT_Angle, format_string ("%f", 0.0)));
	//			moveIn3D ('z', timber.radAng, (double)lengthInt / 1000.0, &timber.posX, &timber.posY, &timber.posZ);

	//			remainLengthInt -= 3600;
	//		}
	//		moveIn3D ('x', timber.radAng, -(double)placingZone->cells [xx].horLen / 1000.0, &timber.posX, &timber.posY, &timber.posZ);
	//		moveIn3D ('z', timber.radAng, -accumDist, &timber.posX, &timber.posY, &timber.posZ);
	//		
	//		// ����� ��ü �׷�ȭ (�ո�)
	//		if (!elemList_Front.IsEmpty ()) {
	//			GSSize nElems = elemList_Front.GetSize ();
	//			API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
	//			if (elemHead != NULL) {
	//				for (GSIndex i = 0; i < nElems; i++)
	//					(*elemHead)[i].guid = elemList_Front [i];

	//				ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

	//				BMKillHandle ((GSHandle *) &elemHead);
	//			}
	//			elemList_Front.Clear ();
	//		}
	//	}

	//	// ������ ���� �������� �̵�
	//	moveIn3D ('x', timber.radAng, (double)placingZone->cells [xx].horLen / 1000.0, &timber.posX, &timber.posY, &timber.posZ);
	//}

	//// �޸� ��ġ
	//if (placingZone->bSingleSide == false) {
	//	timber.init (L("����v1.0.gsm"), layerInd_Timber, infoWall.floorInd, placingZone->leftBottomX, placingZone->leftBottomY, placingZone->leftBottomZ, placingZone->ang + DegreeToRad (180.0));

	//	if (placingZone->bLincorner == true)	moveIn3D ('x', timber.radAng - DegreeToRad (180.0), placingZone->lenLincorner, &timber.posX, &timber.posY, &timber.posZ);		// ���� ���ڳ� ������ x �̵�
	//	moveIn3D ('y', timber.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap, &timber.posX, &timber.posY, &timber.posZ);										// ������ ���ݸ�ŭ �̵�

	//	for (xx = 0 ; xx < placingZone->nCellsInHor ; ++xx) {
	//		if (placingZone->cells [xx].objType == TIMBER) {
	//			accumDist = 0.0;
	//			remainLengthInt = 0;

	//			varEnd = (placingZone->bExtra == true) ? placingZone->nCellsInVerExtra : placingZone->nCellsInVerBasic;

	//			for (yy = 0 ; yy < varEnd ; ++yy) {
	//				if (placingZone->bExtra == true)
	//					lengthInt = placingZone->cells [xx].tableInVerExtra [yy];
	//				else
	//					lengthInt = placingZone->cells [xx].tableInVerBasic [yy];

	//				remainLengthInt += lengthInt;
	//				accumDist += (double)lengthInt / 1000.0;
	//			}

	//			while (remainLengthInt > 0) {
	//				if (remainLengthInt >= 3600)
	//					lengthInt = 3600;
	//				else
	//					lengthInt = remainLengthInt;

	//				elemList_Back.Push (timber.placeObject (6,
	//					"w_ins", APIParT_CString, "�������",
	//					"w_w", APIParT_Length, format_string ("%f", 0.050),
	//					"w_h", APIParT_Length, format_string ("%f", (double)placingZone->cells [xx].horLen / 1000.0),
	//					"w_leng", APIParT_Length, format_string ("%f", (double)lengthInt / 1000.0),
	//					"w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
	//					"torsion_ang", APIParT_Angle, format_string ("%f", 0.0)));
	//				moveIn3D ('z', timber.radAng, (double)lengthInt / 1000.0, &timber.posX, &timber.posY, &timber.posZ);

	//				remainLengthInt -= 3600;
	//			}
	//			moveIn3D ('z', timber.radAng, -accumDist, &timber.posX, &timber.posY, &timber.posZ);

	//			// ����� ��ü �׷�ȭ (�޸�)
	//			if (!elemList_Back.IsEmpty ()) {
	//				GSSize nElems = elemList_Back.GetSize ();
	//				API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
	//				if (elemHead != NULL) {
	//					for (GSIndex i = 0; i < nElems; i++)
	//						(*elemHead)[i].guid = elemList_Back [i];

	//					ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

	//					BMKillHandle ((GSHandle *) &elemHead);
	//				}
	//				elemList_Back.Clear ();
	//			}
	//		}

	//		// ������ ���� �������� �̵�
	//		moveIn3D ('x', timber.radAng, -(double)placingZone->cells [xx].horLen / 1000.0, &timber.posX, &timber.posY, &timber.posZ);
	//	}
	//}
	//
	//// ================================================== ���̺��� ��ġ
	//for (xx = 0 ; xx < placingZone->nCellsInHor ; ++xx) {
	//	placingZone->placeEuroformsOfTableform (this, xx);		// ���̺����� ������ ��ġ (Ÿ�� �ҹ� ����)
	//	
	//	if (placingZone->tableformType == 1)		placingZone->placeTableformA (this, xx);
	//	else if (placingZone->tableformType == 2)	placingZone->placeTableformB (this, xx);
	//	else if (placingZone->tableformType == 3)	placingZone->placeTableformC (this, xx);

	//	// ����� ��ü �׷�ȭ (�ո�)
	//	if (!elemList_Front.IsEmpty ()) {
	//		GSSize nElems = elemList_Front.GetSize ();
	//		API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
	//		if (elemHead != NULL) {
	//			for (GSIndex i = 0; i < nElems; i++)
	//				(*elemHead)[i].guid = elemList_Front [i];

	//			ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

	//			BMKillHandle ((GSHandle *) &elemHead);
	//		}
	//		elemList_Front.Clear ();
	//	}

	//	// ����� ��ü �׷�ȭ (�޸�)
	//	if (!elemList_Back.IsEmpty ()) {
	//		GSSize nElems = elemList_Back.GetSize ();
	//		API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
	//		if (elemHead != NULL) {
	//			for (GSIndex i = 0; i < nElems; i++)
	//				(*elemHead)[i].guid = elemList_Back [i];

	//			ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

	//			BMKillHandle ((GSHandle *) &elemHead);
	//		}
	//		elemList_Back.Clear ();
	//	}
	//}

	return err;
}

// ���̺��� �� ������ ��ġ (����) ...
void	LowSideTableformPlacingZone::placeEuroformsOfTableform (LowSideTableformPlacingZone* placingZone, short idxCell)
{
	//short	xx, yy, varEnd;
	//double	accumDist;
	//int		lengthInt;

	//EasyObjectPlacement euroform;

	//if (placingZone->cells [idxCell].objType == TABLEFORM) {
	//	if (placingZone->bVertical == true) {
	//		// ���ι���
	//		// �ո�
	//		euroform.init (L("������v2.0.gsm"), layerInd_Euroform, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

	//		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++xx) {
	//			accumDist = 0.0;
	//			for (yy = 0 ; yy < placingZone->nCellsInVerBasic ; ++yy) {
	//				if ((placingZone->cells [idxCell].tableInHor [xx] > 0) && (placingZone->cells [idxCell].tableInVerBasic [yy] > 0)) {
	//					elemList_Front.Push (euroform.placeObject (5,
	//						"eu_stan_onoff", APIParT_Boolean, "1.0",
	//						"eu_wid", APIParT_CString, format_string ("%d", placingZone->cells [idxCell].tableInHor [xx]),
	//						"eu_hei", APIParT_CString, format_string ("%d", placingZone->cells [idxCell].tableInVerBasic [yy]),
	//						"u_ins", APIParT_CString, "�������",
	//						"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
	//					accumDist += (double)placingZone->cells [idxCell].tableInVerBasic [yy] / 1000.0;
	//					moveIn3D ('z', euroform.radAng, (double)placingZone->cells [idxCell].tableInVerBasic [yy] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
	//				}
	//			}
	//			moveIn3D ('z', euroform.radAng, -accumDist, &euroform.posX, &euroform.posY, &euroform.posZ);
	//			moveIn3D ('x', euroform.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
	//		}

	//		// �޸�
	//		if (placingZone->bSingleSide == false) {
	//			euroform.init (L("������v2.0.gsm"), layerInd_Euroform, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));
	//			moveIn3D ('y', euroform.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2, &euroform.posX, &euroform.posY, &euroform.posZ);		// ������ ���ݸ�ŭ �̵�

	//			varEnd = (placingZone->bExtra == true) ? placingZone->nCellsInVerExtra : placingZone->nCellsInVerBasic;

	//			for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++xx) {
	//				accumDist = 0.0;
	//				for (yy = 0 ; yy < varEnd ; ++yy) {
	//					if (placingZone->bExtra == true)
	//						lengthInt = placingZone->cells [idxCell].tableInVerExtra [yy];
	//					else
	//						lengthInt = placingZone->cells [idxCell].tableInVerBasic [yy];

	//					if ((placingZone->cells [idxCell].tableInHor [xx] > 0) && (lengthInt > 0)) {
	//						moveIn3D ('x', euroform.radAng, -(double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
	//						elemList_Back.Push (euroform.placeObject (5,
	//							"eu_stan_onoff", APIParT_Boolean, "1.0",
	//							"eu_wid", APIParT_CString, format_string ("%d", placingZone->cells [idxCell].tableInHor [xx]),
	//							"eu_hei", APIParT_CString, format_string ("%d", lengthInt),
	//							"u_ins", APIParT_CString, "�������",
	//							"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
	//						moveIn3D ('x', euroform.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);

	//						accumDist += (double)lengthInt / 1000.0;
	//						moveIn3D ('z', euroform.radAng, (double)lengthInt / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
	//					}
	//				}
	//				moveIn3D ('z', euroform.radAng, -accumDist, &euroform.posX, &euroform.posY, &euroform.posZ);
	//				moveIn3D ('x', euroform.radAng, -(double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
	//			}
	//		}
	//	} else {
	//		// ���ι���
	//		// �ո�
	//		euroform.init (L("������v2.0.gsm"), layerInd_Euroform, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

	//		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++xx) {
	//			accumDist = 0.0;
	//			for (yy = 0 ; yy < placingZone->nCellsInVerBasic ; ++yy) {
	//				if ((placingZone->cells [idxCell].tableInHor [xx] > 0) && (placingZone->cells [idxCell].tableInVerBasic [yy] > 0)) {
	//					moveIn3D ('x', euroform.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
	//					elemList_Front.Push (euroform.placeObject (5,
	//						"eu_stan_onoff", APIParT_Boolean, "1.0",
	//						"eu_wid", APIParT_CString, format_string ("%d", placingZone->cells [idxCell].tableInVerBasic [yy]),
	//						"eu_hei", APIParT_CString, format_string ("%d", placingZone->cells [idxCell].tableInHor [xx]),
	//						"u_ins", APIParT_CString, "��������",
	//						"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));
	//					moveIn3D ('x', euroform.radAng, -(double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);

	//					accumDist += (double)placingZone->cells [idxCell].tableInVerBasic [yy] / 1000.0;
	//					moveIn3D ('z', euroform.radAng, (double)placingZone->cells [idxCell].tableInVerBasic [yy] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
	//				}
	//			}
	//			moveIn3D ('z', euroform.radAng, -accumDist, &euroform.posX, &euroform.posY, &euroform.posZ);
	//			moveIn3D ('x', euroform.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
	//		}

	//		// �޸�
	//		if (placingZone->bSingleSide == false) {
	//			euroform.init (L("������v2.0.gsm"), layerInd_Euroform, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));
	//			moveIn3D ('y', euroform.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2, &euroform.posX, &euroform.posY, &euroform.posZ);		// ������ ���ݸ�ŭ �̵�

	//			varEnd = (placingZone->bExtra == true) ? placingZone->nCellsInVerExtra : placingZone->nCellsInVerBasic;

	//			for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++xx) {
	//				accumDist = 0.0;
	//				for (yy = 0 ; yy < varEnd ; ++yy) {
	//					if (placingZone->bExtra == true)
	//						lengthInt = placingZone->cells [idxCell].tableInVerExtra [yy];
	//					else
	//						lengthInt = placingZone->cells [idxCell].tableInVerBasic [yy];

	//					if ((placingZone->cells [idxCell].tableInHor [xx] > 0) && (lengthInt > 0)) {
	//						elemList_Back.Push (euroform.placeObject (5,
	//							"eu_stan_onoff", APIParT_Boolean, "1.0",
	//							"eu_wid", APIParT_CString, format_string ("%d", lengthInt),
	//							"eu_hei", APIParT_CString, format_string ("%d", placingZone->cells [idxCell].tableInHor [xx]),
	//							"u_ins", APIParT_CString, "��������",
	//							"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)).c_str ()));

	//						accumDist += (double)lengthInt / 1000.0;
	//						moveIn3D ('z', euroform.radAng, (double)lengthInt / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
	//					}
	//				}
	//				moveIn3D ('z', euroform.radAng, -accumDist, &euroform.posX, &euroform.posY, &euroform.posZ);
	//				moveIn3D ('x', euroform.radAng, -(double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &euroform.posX, &euroform.posY, &euroform.posZ);
	//			}
	//		}
	//	}
	//}
}

// ���̺��� Ÿ��A ��ġ (������ ����) - �������� 2�� ...
void	LowSideTableformPlacingZone::placeTableformA (LowSideTableformPlacingZone* placingZone, short idxCell)
{
	//short	xx, yy, varEnd;
	//int		pipeLength;
	//int		headpieceUpPosZ;
	//double	sideMargin;
	//int*	intPointer;
	//int		backHeight;
	//
	//int		firstWidth, lastWidth;
	//bool	bFoundFirstWidth;
	//short	realWidthCount, count;

	//int		topHeight, bottomHeight;
	//bool	bFoundBottomHeight;
	//short	realHeightCount;

	//if (placingZone->bExtra == true) {
	//	varEnd = placingZone->nCellsInVerExtra;
	//	intPointer = placingZone->cells [idxCell].tableInVerExtra;
	//	backHeight = placingZone->cells [idxCell].verLenExtra;
	//} else {
	//	varEnd = placingZone->nCellsInVerBasic;
	//	intPointer = placingZone->cells [idxCell].tableInVerBasic;
	//	backHeight = placingZone->cells [idxCell].verLenBasic;
	//}

	//// ���̺��� �� 1��°�� ������ �������� ������ �ʺ� ������ (���̰� 0�� �������� ���) - ���ι���
	//realWidthCount = 0;
	//bFoundFirstWidth = false;
	//for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++xx) {
	//	if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
	//		if (bFoundFirstWidth == false) {
	//			firstWidth = placingZone->cells [idxCell].tableInHor [xx];
	//			bFoundFirstWidth = true;
	//		}
	//		lastWidth = placingZone->cells [idxCell].tableInHor [xx];
	//		++realWidthCount;
	//	}
	//}

	//if (placingZone->cells [idxCell].objType != TABLEFORM)
	//	return;

	//if (placingZone->cells [idxCell].horLen == 0)
	//	return;

	//if (placingZone->bVertical == true) {
	//	// ================================================== ���ι���
	//	// ���� ������ ��ġ - �ո�
	//	if (placingZone->cells [idxCell].horLen % 100 == 0) {
	//		pipeLength = placingZone->cells [idxCell].horLen - 100;
	//		sideMargin = 0.050;
	//	} else {
	//		pipeLength = placingZone->cells [idxCell].horLen - 50;
	//		sideMargin = 0.025;
	//	}

	//	EasyObjectPlacement rectPipe;
	//	rectPipe.init (L("���������v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

	//	moveIn3D ('x', rectPipe.radAng, sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//	moveIn3D ('y', rectPipe.radAng, -(0.0635 + 0.025), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//	moveIn3D ('z', rectPipe.radAng, 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

	//	moveIn3D ('z', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// �Ϻ�
	//	elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
	//	moveIn3D ('z', rectPipe.radAng, 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//	elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
	//	moveIn3D ('z', rectPipe.radAng, -0.031 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

	//	for (xx = 0 ; xx < placingZone->nCellsInVerBasic - 1 ; ++xx) {								// �߰�
	//		if (placingZone->cells [idxCell].tableInVerBasic [xx] > 0) {
	//			moveIn3D ('z', rectPipe.radAng, (double)placingZone->cells [idxCell].tableInVerBasic [xx] / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

	//			moveIn3D ('z', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//			elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
	//			moveIn3D ('z', rectPipe.radAng, 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//			elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
	//			moveIn3D ('z', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//		}
	//	}
	//	moveIn3D ('z', rectPipe.radAng, (double)placingZone->cells [idxCell].tableInVerBasic [placingZone->nCellsInVerBasic - 1] / 1000.0 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

	//	moveIn3D ('z', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// ���
	//	elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
	//	moveIn3D ('z', rectPipe.radAng, 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//	elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
	//	moveIn3D ('z', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

	//	// ���� ������ ��ġ - �޸�
	//	if (placingZone->bSingleSide == false) {
	//		rectPipe.init (L("���������v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

	//		moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].horLen / 1000.0 - sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//		moveIn3D ('y', rectPipe.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + (0.0635 + 0.025), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//		moveIn3D ('z', rectPipe.radAng - DegreeToRad (180.0), 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

	//		moveIn3D ('z', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// �Ϻ�
	//		elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
	//		moveIn3D ('z', rectPipe.radAng, 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//		elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
	//		moveIn3D ('z', rectPipe.radAng, -0.031 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

	//		for (xx = 0 ; xx < varEnd - 1 ; ++xx) {														// �߰�
	//			if (intPointer [xx] > 0) {
	//				moveIn3D ('z', rectPipe.radAng, (double)intPointer [xx] / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

	//				moveIn3D ('z', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//				elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
	//				moveIn3D ('z', rectPipe.radAng, 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//				elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
	//				moveIn3D ('z', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//			}
	//		}
	//		moveIn3D ('z', rectPipe.radAng, (double)intPointer [varEnd - 1] / 1000.0 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

	//		moveIn3D ('z', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// ���
	//		elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
	//		moveIn3D ('z', rectPipe.radAng, 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//		elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
	//		moveIn3D ('z', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//	}

	//	// ���� ������ ��ġ - �ո�
	//	if (placingZone->cells [idxCell].verLenBasic % 100 == 0) {
	//		pipeLength = placingZone->cells [idxCell].verLenBasic - 100;
	//		sideMargin = 0.050;
	//	} else {
	//		pipeLength = placingZone->cells [idxCell].verLenBasic - 50;
	//		sideMargin = 0.025;
	//	}

	//	rectPipe.init (L("���������v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

	//	moveIn3D ('x', rectPipe.radAng, (double)firstWidth / 1000.0 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//	moveIn3D ('y', rectPipe.radAng, -(0.0635 + 0.075), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//	moveIn3D ('z', rectPipe.radAng, sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

	//	moveIn3D ('x', rectPipe.radAng, -0.035, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// ����
	//	elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
	//	moveIn3D ('x', rectPipe.radAng, 0.070, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//	elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
	//	moveIn3D ('x', rectPipe.radAng, -0.035 + 0.150 - (double)firstWidth / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

	//	moveIn3D ('x', rectPipe.radAng, (double)(placingZone->cells [idxCell].horLen - lastWidth) / 1000.0 + 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//	moveIn3D ('x', rectPipe.radAng, -0.035, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// ������
	//	elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
	//	moveIn3D ('x', rectPipe.radAng, 0.070, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//	elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));

	//	// ���� ������ ��ġ - �޸�
	//	if (placingZone->bSingleSide == false) {
	//		if (backHeight % 100 == 0) {
	//			pipeLength = backHeight - 100;
	//			sideMargin = 0.050;
	//		} else {
	//			pipeLength = backHeight - 50;
	//			sideMargin = 0.025;
	//		}

	//		rectPipe.init (L("���������v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

	//		moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), (double)(placingZone->cells [idxCell].horLen - lastWidth) / 1000.0 + 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//		moveIn3D ('y', rectPipe.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + (0.0635 + 0.075), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//		moveIn3D ('z', rectPipe.radAng - DegreeToRad (180.0), sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

	//		moveIn3D ('x', rectPipe.radAng, -0.035, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// ����
	//		elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
	//		moveIn3D ('x', rectPipe.radAng, 0.070, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//		elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
	//		moveIn3D ('x', rectPipe.radAng, -0.035 + 0.150 - (double)lastWidth / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

	//		moveIn3D ('x', rectPipe.radAng, (double)(placingZone->cells [idxCell].horLen - firstWidth) / 1000.0 + 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//		moveIn3D ('x', rectPipe.radAng, -0.035, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// ������
	//		elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
	//		moveIn3D ('x', rectPipe.radAng, 0.070, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//		elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
	//	}

	//	// �ɺ�Ʈ ��Ʈ - �ո�
	//	EasyObjectPlacement pinbolt;
	//	pinbolt.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

	//	moveIn3D ('y', pinbolt.radAng, -0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//	moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

	//	count = realWidthCount - 1;
	//	for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) - 1 ; ++xx) {	// �Ϻ�
	//		if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
	//			moveIn3D ('x', pinbolt.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//			if (count > 0) {
	//				pinbolt.radAng += DegreeToRad (90.0);
	//				elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
	//				pinbolt.radAng -= DegreeToRad (90.0);

	//				--count;
	//			}
	//		}
	//	}

	//	pinbolt.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

	//	moveIn3D ('y', pinbolt.radAng, -0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

	//	for (xx = 0 ; xx < placingZone->nCellsInVerBasic - 1 ; ++xx) {										// �߰�
	//		if (placingZone->cells [idxCell].tableInVerBasic [xx] > 0) {
	//			moveIn3D ('z', pinbolt.radAng, (double)placingZone->cells [idxCell].tableInVerBasic [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

	//			count = realWidthCount;
	//			for (yy = 0 ; yy < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++yy) {
	//				if (placingZone->cells [idxCell].tableInHor [yy] > 0) {
	//					if (count > 0) {
	//						if (count == realWidthCount) {
	//							// ����
	//							if (placingZone->cells [idxCell].tableInHor [yy] == 600) {
	//								moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//								moveIn3D ('x', pinbolt.radAng, 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//							} else if (placingZone->cells [idxCell].tableInHor [yy] == 500) {
	//								moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//								moveIn3D ('x', pinbolt.radAng, 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//							} else if (placingZone->cells [idxCell].tableInHor [yy] == 450) {
	//								moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//								moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//							} else if (placingZone->cells [idxCell].tableInHor [yy] == 400) {
	//								moveIn3D ('x', pinbolt.radAng, 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//								moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//							} else if (placingZone->cells [idxCell].tableInHor [yy] == 300) {
	//								moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//							} else if (placingZone->cells [idxCell].tableInHor [yy] == 200) {
	//								moveIn3D ('x', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//							}
	//						} else if (count == 1) {
	//							// ����
	//							if (placingZone->cells [idxCell].tableInHor [yy] == 600) {
	//								moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								moveIn3D ('x', pinbolt.radAng, 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//								moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//							} else if (placingZone->cells [idxCell].tableInHor [yy] == 500) {
	//								moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								moveIn3D ('x', pinbolt.radAng, 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//								moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//							} else if (placingZone->cells [idxCell].tableInHor [yy] == 450) {
	//								moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//								moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//							} else if (placingZone->cells [idxCell].tableInHor [yy] == 400) {
	//								moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//								moveIn3D ('x', pinbolt.radAng, 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//							} else if (placingZone->cells [idxCell].tableInHor [yy] == 300) {
	//								moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//							} else if (placingZone->cells [idxCell].tableInHor [yy] == 200) {
	//								moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								moveIn3D ('x', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//							}
	//						} else {
	//							// ������
	//							if (placingZone->cells [idxCell].tableInHor [yy] == 600) {
	//								moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//								moveIn3D ('x', pinbolt.radAng, 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//								moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//							} else if (placingZone->cells [idxCell].tableInHor [yy] == 500) {
	//								moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//								moveIn3D ('x', pinbolt.radAng, 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//								moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//							} else if (placingZone->cells [idxCell].tableInHor [yy] == 450) {
	//								moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//								moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//								moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//							} else if (placingZone->cells [idxCell].tableInHor [yy] == 400) {
	//								moveIn3D ('x', pinbolt.radAng, 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//								moveIn3D ('x', pinbolt.radAng, 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//								moveIn3D ('x', pinbolt.radAng, 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//							} else if (placingZone->cells [idxCell].tableInHor [yy] == 300) {
	//								moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//								moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//							} else if (placingZone->cells [idxCell].tableInHor [yy] == 200) {
	//								moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//								moveIn3D ('x', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//							}
	//						}

	//						--count;
	//					}
	//				}
	//			}
	//			moveIn3D ('x', pinbolt.radAng, -(double)placingZone->cells [idxCell].horLen / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//		}
	//	}

	//	pinbolt.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

	//	moveIn3D ('y', pinbolt.radAng, -0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//	moveIn3D ('z', pinbolt.radAng, (double)placingZone->cells [idxCell].verLenBasic / 1000.0 - 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

	//	count = realWidthCount - 1;
	//	for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) - 1 ; ++xx) {	// ���
	//		if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
	//			moveIn3D ('x', pinbolt.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//			if (count > 0) {
	//				pinbolt.radAng += DegreeToRad (90.0);
	//				elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
	//				pinbolt.radAng -= DegreeToRad (90.0);

	//				--count;
	//			}
	//		}
	//	}

	//	// �ɺ�Ʈ ��Ʈ - �޸�
	//	if (placingZone->bSingleSide == false) {
	//		pinbolt.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

	//		moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//		moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

	//		count = realWidthCount - 1;
	//		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) - 1 ; ++xx) {	// �Ϻ�
	//			if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
	//				moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//				if (count > 0) {
	//					pinbolt.radAng += DegreeToRad (90.0);
	//					elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
	//					pinbolt.radAng -= DegreeToRad (90.0);

	//					--count;
	//				}
	//			}
	//		}

	//		pinbolt.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

	//		moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

	//		for (xx = 0 ; xx < varEnd - 1 ; ++xx) {																// �߰�
	//			if (intPointer [xx] > 0) {
	//				moveIn3D ('z', pinbolt.radAng, (double)intPointer [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

	//				count = realWidthCount;
	//				for (yy = 0 ; yy < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) ; ++yy) {
	//					if (placingZone->cells [idxCell].tableInHor [yy] > 0) {
	//						if (count > 0) {
	//							if (count == realWidthCount) {
	//								// ����
	//								if (placingZone->cells [idxCell].tableInHor [yy] == 600) {
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								} else if (placingZone->cells [idxCell].tableInHor [yy] == 500) {
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								} else if (placingZone->cells [idxCell].tableInHor [yy] == 450) {
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								} else if (placingZone->cells [idxCell].tableInHor [yy] == 400) {
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								} else if (placingZone->cells [idxCell].tableInHor [yy] == 300) {
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								} else if (placingZone->cells [idxCell].tableInHor [yy] == 200) {
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								}
	//							} else if (count == 1) {
	//								// ����
	//								if (placingZone->cells [idxCell].tableInHor [yy] == 600) {
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								} else if (placingZone->cells [idxCell].tableInHor [yy] == 500) {
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								} else if (placingZone->cells [idxCell].tableInHor [yy] == 450) {
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								} else if (placingZone->cells [idxCell].tableInHor [yy] == 400) {
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								} else if (placingZone->cells [idxCell].tableInHor [yy] == 300) {
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								} else if (placingZone->cells [idxCell].tableInHor [yy] == 200) {
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								}
	//							} else {
	//								// ������
	//								if (placingZone->cells [idxCell].tableInHor [yy] == 600) {
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								} else if (placingZone->cells [idxCell].tableInHor [yy] == 500) {
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								} else if (placingZone->cells [idxCell].tableInHor [yy] == 450) {
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								} else if (placingZone->cells [idxCell].tableInHor [yy] == 400) {
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								} else if (placingZone->cells [idxCell].tableInHor [yy] == 300) {
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								} else if (placingZone->cells [idxCell].tableInHor [yy] == 200) {
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								}
	//							}

	//							--count;
	//						}
	//					}
	//				}
	//				moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), -(double)placingZone->cells [idxCell].horLen / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//			}
	//		}

	//		pinbolt.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

	//		moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//		moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), (double)backHeight / 1000.0 - 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

	//		count = realWidthCount - 1;
	//		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInHor) / sizeof (int) - 1 ; ++xx) {	// ���
	//			if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
	//				moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//				if (count > 0) {
	//					pinbolt.radAng += DegreeToRad (90.0);
	//					elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
	//					pinbolt.radAng -= DegreeToRad (90.0);

	//					--count;
	//				}
	//			}
	//		}
	//	}
	//	
	//	// ����ö�� - �ո�
	//	EasyObjectPlacement join;
	//	join.init (L("����ö�� (�簢�ͼ�Ȱ��) v1.0.gsm"), layerInd_Join, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

	//	moveIn3D ('x', join.radAng, (double)firstWidth / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);
	//	moveIn3D ('y', join.radAng, -0.1815, &join.posX, &join.posY, &join.posZ);
	//	moveIn3D ('z', join.radAng, 0.150, &join.posX, &join.posY, &join.posZ);

	//	elemList_Front.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "������Ʈ", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//	moveIn3D ('x', join.radAng, (double)(150 - firstWidth + placingZone->cells [idxCell].horLen - lastWidth + 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
	//	elemList_Front.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "������Ʈ", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

	//	join.init (L("����ö�� (�簢�ͼ�Ȱ��) v1.0.gsm"), layerInd_Join, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

	//	moveIn3D ('x', join.radAng, (double)firstWidth / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);
	//	moveIn3D ('y', join.radAng, -0.1815, &join.posX, &join.posY, &join.posZ);
	//	moveIn3D ('z', join.radAng, (double)placingZone->cells [idxCell].verLenBasic / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);

	//	elemList_Front.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "������Ʈ", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//	moveIn3D ('x', join.radAng, (double)(150 - firstWidth + placingZone->cells [idxCell].horLen - lastWidth + 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
	//	elemList_Front.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "������Ʈ", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//	
	//	// ����ö�� - �޸�
	//	if (placingZone->bSingleSide == false) {
	//		join.init (L("����ö�� (�簢�ͼ�Ȱ��) v1.0.gsm"), layerInd_Join, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

	//		moveIn3D ('x', join.radAng - DegreeToRad (180.0), (double)firstWidth / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);
	//		moveIn3D ('y', join.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1815, &join.posX, &join.posY, &join.posZ);
	//		moveIn3D ('z', join.radAng - DegreeToRad (180.0), 0.150, &join.posX, &join.posY, &join.posZ);

	//		elemList_Back.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "������Ʈ", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//		moveIn3D ('x', join.radAng, -(double)(150 - firstWidth + placingZone->cells [idxCell].horLen - lastWidth + 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
	//		elemList_Back.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "������Ʈ", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

	//		join.init (L("����ö�� (�簢�ͼ�Ȱ��) v1.0.gsm"), layerInd_Join, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

	//		moveIn3D ('x', join.radAng - DegreeToRad (180.0), (double)firstWidth / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);
	//		moveIn3D ('y', join.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1815, &join.posX, &join.posY, &join.posZ);
	//		moveIn3D ('z', join.radAng - DegreeToRad (180.0), (double)backHeight / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);

	//		elemList_Back.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "������Ʈ", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//		moveIn3D ('x', join.radAng, -(double)(150 - firstWidth + placingZone->cells [idxCell].horLen - lastWidth + 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
	//		elemList_Back.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "������Ʈ", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//	}

	//	// ����ǽ� - �ո�
	//	EasyObjectPlacement headpiece;
	//	headpiece.init (L("RS Push-Pull Props ����ǽ� v2.0 (�ξ�� ����).gsm"), layerInd_HeadPiece, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

	//	moveIn3D ('x', headpiece.radAng, (double)(firstWidth - 150 - 100) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
	//	moveIn3D ('y', headpiece.radAng, -0.1725, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
	//	moveIn3D ('z', headpiece.radAng, 0.300, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

	//	elemList_Front.Push (headpiece.placeObject (4, "type", APIParT_CString, "Ÿ�� A", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//	moveIn3D ('x', headpiece.radAng, (double)(-(firstWidth - 150 - 100) + placingZone->cells [idxCell].horLen + (-lastWidth + 150 - 100)) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
	//	elemList_Front.Push (headpiece.placeObject (4, "type", APIParT_CString, "Ÿ�� A", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

	//	headpiece.init (L("RS Push-Pull Props ����ǽ� v2.0 (�ξ�� ����).gsm"), layerInd_HeadPiece, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

	//	if (placingZone->cells [idxCell].verLenBasic >= 5300) {
	//		headpieceUpPosZ = 4200;
	//	} else if (placingZone->cells [idxCell].verLenBasic >= 4600) {
	//		headpieceUpPosZ = 3800;
	//	} else if (placingZone->cells [idxCell].verLenBasic >= 3500) {
	//		headpieceUpPosZ = 2800;
	//	} else if (placingZone->cells [idxCell].verLenBasic >= 3000) {
	//		headpieceUpPosZ = 2200;
	//	} else if (placingZone->cells [idxCell].verLenBasic >= 2500) {
	//		headpieceUpPosZ = 1900;
	//	} else if (placingZone->cells [idxCell].verLenBasic >= 2000) {
	//		headpieceUpPosZ = 1500;
	//	} else if (placingZone->cells [idxCell].verLenBasic >= 1500) {
	//		headpieceUpPosZ = 1100;
	//	} else if (placingZone->cells [idxCell].verLenBasic >= 1000) {
	//		headpieceUpPosZ = 800;
	//	} else {
	//		headpieceUpPosZ = 150;
	//	}

	//	moveIn3D ('x', headpiece.radAng, (double)(firstWidth - 150 - 100) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
	//	moveIn3D ('y', headpiece.radAng, -0.1725, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
	//	moveIn3D ('z', headpiece.radAng, (double)headpieceUpPosZ / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

	//	elemList_Front.Push (headpiece.placeObject (4, "type", APIParT_CString, "Ÿ�� A", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//	moveIn3D ('x', headpiece.radAng, (double)(-(firstWidth - 150 - 100) + placingZone->cells [idxCell].horLen + (-lastWidth + 150 - 100)) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
	//	elemList_Front.Push (headpiece.placeObject (4, "type", APIParT_CString, "Ÿ�� A", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

	//	// ����ǽ� - �޸�
	//	if (placingZone->bSingleSide == false) {
	//		headpiece.init (L("RS Push-Pull Props ����ǽ� v2.0 (�ξ�� ����).gsm"), layerInd_HeadPiece, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

	//		moveIn3D ('x', headpiece.radAng - DegreeToRad (180.0), (double)(firstWidth - 150 + 100) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
	//		moveIn3D ('y', headpiece.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1725, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
	//		moveIn3D ('z', headpiece.radAng - DegreeToRad (180.0), 0.300, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

	//		elemList_Back.Push (headpiece.placeObject (4, "type", APIParT_CString, "Ÿ�� A", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//		moveIn3D ('x', headpiece.radAng - DegreeToRad (180.0), (double)(-(firstWidth - 150 + 100) + placingZone->cells [idxCell].horLen + (-lastWidth + 150 + 100)) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
	//		elemList_Back.Push (headpiece.placeObject (4, "type", APIParT_CString, "Ÿ�� A", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

	//		headpiece.init (L("RS Push-Pull Props ����ǽ� v2.0 (�ξ�� ����).gsm"), layerInd_HeadPiece, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

	//		if (backHeight >= 5300) {
	//			headpieceUpPosZ = 4200;
	//		} else if (backHeight >= 4600) {
	//			headpieceUpPosZ = 3800;
	//		} else if (backHeight >= 3500) {
	//			headpieceUpPosZ = 2800;
	//		} else if (backHeight >= 3000) {
	//			headpieceUpPosZ = 2200;
	//		} else if (backHeight >= 2500) {
	//			headpieceUpPosZ = 1900;
	//		} else if (backHeight >= 2000) {
	//			headpieceUpPosZ = 1500;
	//		} else if (backHeight >= 1500) {
	//			headpieceUpPosZ = 1100;
	//		} else if (backHeight >= 1000) {
	//			headpieceUpPosZ = 800;
	//		} else {
	//			headpieceUpPosZ = 150;
	//		}

	//		moveIn3D ('x', headpiece.radAng - DegreeToRad (180.0), (double)(firstWidth - 150 + 100) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
	//		moveIn3D ('y', headpiece.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1725, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
	//		moveIn3D ('z', headpiece.radAng - DegreeToRad (180.0), (double)headpieceUpPosZ / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

	//		elemList_Back.Push (headpiece.placeObject (4, "type", APIParT_CString, "Ÿ�� A", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//		moveIn3D ('x', headpiece.radAng - DegreeToRad (180.0), (double)(-(firstWidth - 150 + 100) + placingZone->cells [idxCell].horLen + (-lastWidth + 150 + 100)) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
	//		elemList_Back.Push (headpiece.placeObject (4, "type", APIParT_CString, "Ÿ�� A", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//	}
	//} else {
	//	// ================================================== ���ι��� (���ι����� ���������� 90�� �����ٰ� �����ϸ� ��)
	//	// ���� ������ ��ġ - �ո�
	//	if (placingZone->cells [idxCell].verLenBasic % 100 == 0) {
	//		pipeLength = placingZone->cells [idxCell].verLenBasic - 100;
	//		sideMargin = 0.050;
	//	} else {
	//		pipeLength = placingZone->cells [idxCell].verLenBasic - 50;
	//		sideMargin = 0.025;
	//	}

	//	EasyObjectPlacement rectPipe;
	//	rectPipe.init (L("���������v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

	//	moveIn3D ('z', rectPipe.radAng, sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//	moveIn3D ('y', rectPipe.radAng, -(0.0635 + 0.025), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//	moveIn3D ('x', rectPipe.radAng, 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

	//	moveIn3D ('x', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// �Ϻ�
	//	elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
	//	moveIn3D ('x', rectPipe.radAng, 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//	elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
	//	moveIn3D ('x', rectPipe.radAng, -0.031 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

	//	for (xx = 0 ; xx < realWidthCount - 1 ; ++xx) {												// �߰�
	//		if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
	//			moveIn3D ('x', rectPipe.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

	//			moveIn3D ('x', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//			elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
	//			moveIn3D ('x', rectPipe.radAng, 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//			elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
	//			moveIn3D ('x', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//		}
	//	}
	//	moveIn3D ('x', rectPipe.radAng, (double)placingZone->cells [idxCell].tableInHor [realWidthCount - 1] / 1000.0 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

	//	moveIn3D ('x', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// ���
	//	elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
	//	moveIn3D ('x', rectPipe.radAng, 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//	elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
	//	moveIn3D ('x', rectPipe.radAng, -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

	//	// ���� ������ ��ġ - �޸�
	//	if (placingZone->bSingleSide == false) {
	//		if (backHeight % 100 == 0) {
	//			pipeLength = backHeight - 100;
	//			sideMargin = 0.050;
	//		} else {
	//			pipeLength = backHeight - 50;
	//			sideMargin = 0.025;
	//		}

	//		rectPipe.init (L("���������v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

	//		moveIn3D ('z', rectPipe.radAng - DegreeToRad (180.0), sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//		moveIn3D ('y', rectPipe.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + (0.0635 + 0.025), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//		moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

	//		moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// �Ϻ�
	//		elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
	//		moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//		elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
	//		moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), -0.031 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

	//		for (xx = 0 ; xx < realWidthCount - 1 ; ++xx) {												// �߰�
	//			if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
	//				moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

	//				moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//				elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
	//				moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//				elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
	//				moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//			}
	//		}
	//		moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].tableInHor [realWidthCount - 1] / 1000.0 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

	//		moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// ���
	//		elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
	//		moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), 0.062, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//		elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "bPunching", APIParT_Boolean, "0.0"));
	//		moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), -0.031, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//	}

	//	// ���� ������ ��ġ - �ո�
	//	if (placingZone->cells [idxCell].horLen % 100 == 0) {
	//		pipeLength = placingZone->cells [idxCell].horLen - 100;
	//		sideMargin = 0.050;
	//	} else {
	//		pipeLength = placingZone->cells [idxCell].horLen - 50;
	//		sideMargin = 0.025;
	//	}

	//	// ���̺��� �� �ٴڰ� ����� �������� ������ ���̸� ������ (���̰� 0�� �������� ���) - ���ι��� (�ո�)
	//	realHeightCount = 0;
	//	bFoundBottomHeight = false;
	//	for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) ; ++xx) {
	//		if (placingZone->cells [idxCell].tableInVerBasic [xx] > 0) {
	//			if (bFoundBottomHeight == false) {
	//				bottomHeight = placingZone->cells [idxCell].tableInVerBasic [xx];
	//				bFoundBottomHeight = true;
	//			}
	//			topHeight = placingZone->cells [idxCell].tableInVerBasic [xx];
	//			++realHeightCount;
	//		}
	//	}

	//	rectPipe.init (L("���������v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

	//	moveIn3D ('z', rectPipe.radAng, (double)(placingZone->cells [idxCell].verLenBasic - topHeight) / 1000.0 + 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//	moveIn3D ('y', rectPipe.radAng, -(0.0635 + 0.075), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//	moveIn3D ('x', rectPipe.radAng, sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

	//	moveIn3D ('z', rectPipe.radAng, 0.035, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);		// ����
	//	elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
	//	moveIn3D ('z', rectPipe.radAng, -0.070, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//	elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
	//	moveIn3D ('z', rectPipe.radAng, 0.035 - 0.150 + (double)topHeight / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

	//	moveIn3D ('z', rectPipe.radAng, (double)(-placingZone->cells [idxCell].verLenBasic + bottomHeight) / 1000.0 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//	moveIn3D ('z', rectPipe.radAng, 0.035, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);		// ������
	//	elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
	//	moveIn3D ('z', rectPipe.radAng, -0.070, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//	elemList_Front.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));

	//	// ���̺��� �� �ٴڰ� ����� �������� ������ ���̸� ������ (���̰� 0�� �������� ���) - ���ι��� (�޸�)
	//	realHeightCount = 0;
	//	bFoundBottomHeight = false;
	//	for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerExtra) / sizeof (int) ; ++xx) {
	//		if (placingZone->cells [idxCell].tableInVerExtra [xx] > 0) {
	//			if (bFoundBottomHeight == false) {
	//				bottomHeight = placingZone->cells [idxCell].tableInVerExtra [xx];
	//				bFoundBottomHeight = true;
	//			}
	//			topHeight = placingZone->cells [idxCell].tableInVerExtra [xx];
	//			++realHeightCount;
	//		}
	//	}

	//	// ���� ������ ��ġ - �޸�
	//	if (placingZone->bSingleSide == false) {
	//		if (placingZone->cells [idxCell].horLen % 100 == 0) {
	//			pipeLength = placingZone->cells [idxCell].horLen - 100;
	//			sideMargin = 0.050;
	//		} else {
	//			pipeLength = placingZone->cells [idxCell].horLen - 50;
	//			sideMargin = 0.025;
	//		}

	//		rectPipe.init (L("���������v1.0.gsm"), layerInd_RectPipe, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

	//		moveIn3D ('z', rectPipe.radAng - DegreeToRad (180.0), (double)bottomHeight / 1000.0 - 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//		moveIn3D ('y', rectPipe.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + (0.0635 + 0.075), &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//		moveIn3D ('x', rectPipe.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].horLen / 1000.0 - sideMargin, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

	//		moveIn3D ('z', rectPipe.radAng, -0.035, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// ����
	//		elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
	//		moveIn3D ('z', rectPipe.radAng, 0.070, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//		elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
	//		moveIn3D ('z', rectPipe.radAng, -0.035 + 0.150 - (double)bottomHeight / 1000.0, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);

	//		moveIn3D ('z', rectPipe.radAng, (double)(backHeight - topHeight) / 1000.0 + 0.150, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//		moveIn3D ('z', rectPipe.radAng, -0.035, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);	// ������
	//		elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));
	//		moveIn3D ('z', rectPipe.radAng, 0.070, &rectPipe.posX, &rectPipe.posY, &rectPipe.posZ);
	//		elemList_Back.Push (rectPipe.placeObject (4, "p_comp", APIParT_CString, "�簢������", "p_leng", APIParT_Length, format_string ("%f", (double)pipeLength / 1000.0), "p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "bPunching", APIParT_Boolean, "0.0"));

	//		// ���̺��� �� �ٴڰ� ����� �������� ������ ���̸� ������ (���̰� 0�� �������� ���) - ���ι��� (�ո�)
	//		realHeightCount = 0;
	//		bFoundBottomHeight = false;
	//		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) ; ++xx) {
	//			if (placingZone->cells [idxCell].tableInVerBasic [xx] > 0) {
	//				if (bFoundBottomHeight == false) {
	//					bottomHeight = placingZone->cells [idxCell].tableInVerBasic [xx];
	//					bFoundBottomHeight = true;
	//				}
	//				topHeight = placingZone->cells [idxCell].tableInVerBasic [xx];
	//				++realHeightCount;
	//			}
	//		}
	//	}

	//	// �ɺ�Ʈ ��Ʈ - �ո�
	//	EasyObjectPlacement pinbolt;
	//	pinbolt.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

	//	moveIn3D ('y', pinbolt.radAng, -0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//	moveIn3D ('x', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

	//	count = realHeightCount - 1;
	//	for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) - 1 ; ++xx) {	// �Ϻ�
	//		if (placingZone->cells [idxCell].tableInVerBasic [xx] > 0) {
	//			moveIn3D ('z', pinbolt.radAng, (double)placingZone->cells [idxCell].tableInVerBasic [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//			if (count > 0) {
	//				pinbolt.radAng += DegreeToRad (90.0);
	//				elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
	//				pinbolt.radAng -= DegreeToRad (90.0);

	//				--count;
	//			}
	//		}
	//	}

	//	pinbolt.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

	//	moveIn3D ('y', pinbolt.radAng, -0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

	//	for (xx = 0 ; xx < realWidthCount - 1 ; ++xx) {															// �߰�
	//		if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
	//			moveIn3D ('x', pinbolt.radAng, (double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

	//			count = realHeightCount;
	//			for (yy = 0 ; yy < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) ; ++yy) {
	//				if (placingZone->cells [idxCell].tableInVerBasic [yy] > 0) {
	//					if (count > 0) {
	//						if (count == realHeightCount) {
	//							// ����
	//							if (placingZone->cells [idxCell].tableInVerBasic [yy] == 600) {
	//								moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//								moveIn3D ('z', pinbolt.radAng, 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//							} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 500) {
	//								moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//								moveIn3D ('z', pinbolt.radAng, 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//							} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 450) {
	//								moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//								moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//							} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 400) {
	//								moveIn3D ('z', pinbolt.radAng, 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//								moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//							} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 300) {
	//								moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//							} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 200) {
	//								moveIn3D ('z', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//							}
	//						} else if (count == 1) {
	//							// ����
	//							if (placingZone->cells [idxCell].tableInVerBasic [yy] == 600) {
	//								moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								moveIn3D ('z', pinbolt.radAng, 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//								moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//							} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 500) {
	//								moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								moveIn3D ('z', pinbolt.radAng, 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//								moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//							} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 450) {
	//								moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//								moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//							} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 400) {
	//								moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//								moveIn3D ('z', pinbolt.radAng, 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//							} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 300) {
	//								moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//							} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 200) {
	//								moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('y', pinbolt.radAng, -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('y', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								moveIn3D ('z', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//							}
	//						} else {
	//							// ������
	//							if (placingZone->cells [idxCell].tableInVerBasic [yy] == 600) {
	//								moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//								moveIn3D ('z', pinbolt.radAng, 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//								moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//							} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 500) {
	//								moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//								moveIn3D ('z', pinbolt.radAng, 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//								moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//							} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 450) {
	//								moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//								moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//								moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//							} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 400) {
	//								moveIn3D ('z', pinbolt.radAng, 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//								moveIn3D ('z', pinbolt.radAng, 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//								moveIn3D ('z', pinbolt.radAng, 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//							} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 300) {
	//								moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//								moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//							} else if (placingZone->cells [idxCell].tableInVerBasic [yy] == 200) {
	//								moveIn3D ('z', pinbolt.radAng, 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//								moveIn3D ('z', pinbolt.radAng, 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//							}
	//						}

	//						--count;
	//					}
	//				}
	//			}
	//			moveIn3D ('z', pinbolt.radAng, -(double)placingZone->cells [idxCell].verLenBasic / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//		}
	//	}

	//	pinbolt.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

	//	moveIn3D ('y', pinbolt.radAng, -0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//	moveIn3D ('x', pinbolt.radAng, (double)placingZone->cells [idxCell].horLen / 1000.0 - 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

	//	count = realHeightCount - 1;
	//	for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) - 1 ; ++xx) {	// ���
	//		if (placingZone->cells [idxCell].tableInVerBasic [xx] > 0) {
	//			moveIn3D ('z', pinbolt.radAng, (double)placingZone->cells [idxCell].tableInVerBasic [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//			if (count > 0) {
	//				pinbolt.radAng += DegreeToRad (90.0);
	//				elemList_Front.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
	//				pinbolt.radAng -= DegreeToRad (90.0);

	//				--count;
	//			}
	//		}
	//	}

	//	// ���̺��� �� �ٴڰ� ����� �������� ������ ���̸� ������ (���̰� 0�� �������� ���) - ���ι��� (�޸�)
	//	realHeightCount = 0;
	//	bFoundBottomHeight = false;
	//	for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerExtra) / sizeof (int) ; ++xx) {
	//		if (placingZone->cells [idxCell].tableInVerExtra [xx] > 0) {
	//			if (bFoundBottomHeight == false) {
	//				bottomHeight = placingZone->cells [idxCell].tableInVerExtra [xx];
	//				bFoundBottomHeight = true;
	//			}
	//			topHeight = placingZone->cells [idxCell].tableInVerExtra [xx];
	//			++realHeightCount;
	//		}
	//	}

	//	// �ɺ�Ʈ ��Ʈ - �޸�
	//	if (placingZone->bSingleSide == false) {
	//		pinbolt.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

	//		moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//		moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

	//		count = realHeightCount - 1;
	//		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) - 1 ; ++xx) {	// �Ϻ�
	//			if (intPointer [xx] > 0) {
	//				moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), (double)intPointer [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//				if (count > 0) {
	//					pinbolt.radAng += DegreeToRad (90.0);
	//					elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
	//					pinbolt.radAng -= DegreeToRad (90.0);

	//					--count;
	//				}
	//			}
	//		}

	//		pinbolt.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

	//		moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

	//		for (xx = 0 ; xx < realWidthCount - 1 ; ++xx) {															// �߰�
	//			if (placingZone->cells [idxCell].tableInHor [xx] > 0) {
	//				moveIn3D ('x', pinbolt.radAng, -(double)placingZone->cells [idxCell].tableInHor [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

	//				count = realHeightCount;
	//				for (yy = 0 ; yy < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) ; ++yy) {
	//					if (intPointer [yy] > 0) {
	//						if (count > 0) {
	//							if (count == realHeightCount) {
	//								// ����
	//								if (intPointer [yy] == 600) {
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								} else if (intPointer [yy] == 500) {
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								} else if (intPointer [yy] == 450) {
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								} else if (intPointer [yy] == 400) {
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								} else if (intPointer [yy] == 300) {
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								} else if (intPointer [yy] == 200) {
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								}
	//							} else if (count == 1) {
	//								// ����
	//								if (intPointer [yy] == 600) {
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								} else if (intPointer [yy] == 500) {
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								} else if (intPointer [yy] == 450) {
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								} else if (intPointer [yy] == 400) {
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								} else if (intPointer [yy] == 300) {
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								} else if (intPointer [yy] == 200) {
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//										moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), -0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								}
	//							} else {
	//								// ������
	//								if (intPointer [yy] == 600) {
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.300, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								} else if (intPointer [yy] == 500) {
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								} else if (intPointer [yy] == 450) {
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								} else if (intPointer [yy] == 400) {
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.200, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.100, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								} else if (intPointer [yy] == 300) {
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								} else if (intPointer [yy] == 200) {
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//										elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//									moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), 0.050, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//								}
	//							}

	//							--count;
	//						}
	//					}
	//				}
	//				moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), -(double)backHeight / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//			}
	//		}

	//		pinbolt.init (L("�ɺ�Ʈ��Ʈv1.0.gsm"), layerInd_PinBolt, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

	//		moveIn3D ('y', pinbolt.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1635, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//		moveIn3D ('x', pinbolt.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].horLen / 1000.0 - 0.150, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);

	//		count = realHeightCount - 1;
	//		for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) - 1 ; ++xx) {	// ���
	//			if (intPointer [xx] > 0) {
	//				moveIn3D ('z', pinbolt.radAng - DegreeToRad (180.0), (double)intPointer [xx] / 1000.0, &pinbolt.posX, &pinbolt.posY, &pinbolt.posZ);
	//				if (count > 0) {
	//					pinbolt.radAng += DegreeToRad (90.0);
	//					elemList_Back.Push (pinbolt.placeObject (7, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.100), "bolt_dia", APIParT_Length, format_string ("%f", 0.010), "washer_pos", APIParT_Length, format_string ("%f", 0.050), "washer_size", APIParT_Length, format_string ("%f", 0.100), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
	//					pinbolt.radAng -= DegreeToRad (90.0);

	//					--count;
	//				}
	//			}
	//		}
	//	}
	//	
	//	// ���̺��� �� �ٴڰ� ����� �������� ������ ���̸� ������ (���̰� 0�� �������� ���) - ���ι��� (�ո�)
	//	realHeightCount = 0;
	//	bFoundBottomHeight = false;
	//	for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) ; ++xx) {
	//		if (placingZone->cells [idxCell].tableInVerBasic [xx] > 0) {
	//			if (bFoundBottomHeight == false) {
	//				bottomHeight = placingZone->cells [idxCell].tableInVerBasic [xx];
	//				bFoundBottomHeight = true;
	//			}
	//			topHeight = placingZone->cells [idxCell].tableInVerBasic [xx];
	//			++realHeightCount;
	//		}
	//	}

	//	// ����ö�� - �ո�
	//	EasyObjectPlacement join;
	//	join.init (L("����ö�� (�簢�ͼ�Ȱ��) v1.0.gsm"), layerInd_Join, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

	//	moveIn3D ('z', join.radAng, (double)bottomHeight / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);
	//	moveIn3D ('y', join.radAng, -0.1815, &join.posX, &join.posY, &join.posZ);
	//	moveIn3D ('x', join.radAng, 0.150, &join.posX, &join.posY, &join.posZ);

	//	elemList_Front.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "������Ʈ", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//	moveIn3D ('z', join.radAng, (double)(150 - bottomHeight + placingZone->cells [idxCell].verLenBasic - topHeight + 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
	//	elemList_Front.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "������Ʈ", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

	//	join.init (L("����ö�� (�簢�ͼ�Ȱ��) v1.0.gsm"), layerInd_Join, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

	//	moveIn3D ('z', join.radAng, (double)bottomHeight / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);
	//	moveIn3D ('y', join.radAng, -0.1815, &join.posX, &join.posY, &join.posZ);
	//	moveIn3D ('x', join.radAng, (double)placingZone->cells [idxCell].horLen / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);

	//	elemList_Front.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "������Ʈ", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//	moveIn3D ('z', join.radAng, (double)(150 - bottomHeight + placingZone->cells [idxCell].verLenBasic - topHeight + 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
	//	elemList_Front.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "������Ʈ", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//	
	//	// ���̺��� �� �ٴڰ� ����� �������� ������ ���̸� ������ (���̰� 0�� �������� ���) - ���ι��� (�޸�)
	//	realHeightCount = 0;
	//	bFoundBottomHeight = false;
	//	for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerExtra) / sizeof (int) ; ++xx) {
	//		if (placingZone->cells [idxCell].tableInVerExtra [xx] > 0) {
	//			if (bFoundBottomHeight == false) {
	//				bottomHeight = placingZone->cells [idxCell].tableInVerExtra [xx];
	//				bFoundBottomHeight = true;
	//			}
	//			topHeight = placingZone->cells [idxCell].tableInVerExtra [xx];
	//			++realHeightCount;
	//		}
	//	}

	//	// ����ö�� - �޸�
	//	if (placingZone->bSingleSide == false) {
	//		join.init (L("����ö�� (�簢�ͼ�Ȱ��) v1.0.gsm"), layerInd_Join, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

	//		moveIn3D ('z', join.radAng - DegreeToRad (180.0), (double)bottomHeight / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);
	//		moveIn3D ('y', join.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1815, &join.posX, &join.posY, &join.posZ);
	//		moveIn3D ('x', join.radAng - DegreeToRad (180.0), 0.150, &join.posX, &join.posY, &join.posZ);

	//		elemList_Back.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "������Ʈ", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//		moveIn3D ('z', join.radAng, (double)(150 - bottomHeight + backHeight - topHeight + 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
	//		elemList_Back.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "������Ʈ", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

	//		join.init (L("����ö�� (�簢�ͼ�Ȱ��) v1.0.gsm"), layerInd_Join, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

	//		moveIn3D ('z', join.radAng - DegreeToRad (180.0), (double)bottomHeight / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);
	//		moveIn3D ('y', join.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1815, &join.posX, &join.posY, &join.posZ);
	//		moveIn3D ('x', join.radAng - DegreeToRad (180.0), (double)placingZone->cells [idxCell].horLen / 1000.0 - 0.150, &join.posX, &join.posY, &join.posZ);

	//		elemList_Back.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "������Ʈ", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//		moveIn3D ('z', join.radAng, (double)(150 - bottomHeight + backHeight - topHeight + 150) / 1000.0, &join.posX, &join.posY, &join.posZ);
	//		elemList_Back.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "1.0", "bolt_len", APIParT_Length, format_string ("%f", 0.150), "bolt_dia", APIParT_Length, format_string ("%f", 0.012), "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, format_string ("%f", 0.000), "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, format_string ("%f", 0.108), "washer_size", APIParT_Length, format_string ("%f", 0.100), "nutType", APIParT_CString, "������Ʈ", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
	//	}

	//	// ���̺��� �� �ٴڰ� ����� �������� ������ ���̸� ������ (���̰� 0�� �������� ���) - ���ι��� (�ո�)
	//	realHeightCount = 0;
	//	bFoundBottomHeight = false;
	//	for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerBasic) / sizeof (int) ; ++xx) {
	//		if (placingZone->cells [idxCell].tableInVerBasic [xx] > 0) {
	//			if (bFoundBottomHeight == false) {
	//				bottomHeight = placingZone->cells [idxCell].tableInVerBasic [xx];
	//				bFoundBottomHeight = true;
	//			}
	//			topHeight = placingZone->cells [idxCell].tableInVerBasic [xx];
	//			++realHeightCount;
	//		}
	//	}

	//	// ����ǽ� - �ո�
	//	EasyObjectPlacement headpiece;
	//	headpiece.init (L("RS Push-Pull Props ����ǽ� v2.0 (�ξ�� ����).gsm"), layerInd_HeadPiece, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

	//	moveIn3D ('z', headpiece.radAng, (double)(bottomHeight - 150 + 100) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
	//	moveIn3D ('y', headpiece.radAng, -0.1725, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
	//	moveIn3D ('x', headpiece.radAng, 0.300, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

	//	elemList_Front.Push (headpiece.placeObject (4, "type", APIParT_CString, "Ÿ�� B", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
	//	moveIn3D ('z', headpiece.radAng, (double)(-(bottomHeight - 150) + placingZone->cells [idxCell].verLenBasic + (-topHeight + 150)) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
	//	elemList_Front.Push (headpiece.placeObject (4, "type", APIParT_CString, "Ÿ�� B", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));

	//	headpiece.init (L("RS Push-Pull Props ����ǽ� v2.0 (�ξ�� ����).gsm"), layerInd_HeadPiece, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang);

	//	if (placingZone->cells [idxCell].horLen >= 5300) {
	//		headpieceUpPosZ = 4200;
	//	} else if (placingZone->cells [idxCell].horLen >= 4600) {
	//		headpieceUpPosZ = 3800;
	//	} else if (placingZone->cells [idxCell].horLen >= 3500) {
	//		headpieceUpPosZ = 2800;
	//	} else if (placingZone->cells [idxCell].horLen >= 3000) {
	//		headpieceUpPosZ = 2200;
	//	} else if (placingZone->cells [idxCell].horLen >= 2500) {
	//		headpieceUpPosZ = 1900;
	//	} else if (placingZone->cells [idxCell].horLen >= 2000) {
	//		headpieceUpPosZ = 1500;
	//	} else if (placingZone->cells [idxCell].horLen >= 1500) {
	//		headpieceUpPosZ = 1100;
	//	} else if (placingZone->cells [idxCell].horLen >= 1000) {
	//		headpieceUpPosZ = 800;
	//	} else {
	//		headpieceUpPosZ = 150;
	//	}

	//	moveIn3D ('z', headpiece.radAng, (double)(bottomHeight - 150 + 100) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
	//	moveIn3D ('y', headpiece.radAng, -0.1725, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
	//	moveIn3D ('x', headpiece.radAng, (double)(headpieceUpPosZ) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

	//	elemList_Front.Push (headpiece.placeObject (4, "type", APIParT_CString, "Ÿ�� B", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
	//	moveIn3D ('z', headpiece.radAng, (double)(-(bottomHeight - 150) + placingZone->cells [idxCell].verLenBasic + (-topHeight + 150)) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
	//	elemList_Front.Push (headpiece.placeObject (4, "type", APIParT_CString, "Ÿ�� B", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));

	//	// ���̺��� �� �ٴڰ� ����� �������� ������ ���̸� ������ (���̰� 0�� �������� ���) - ���ι��� (�޸�)
	//	realHeightCount = 0;
	//	bFoundBottomHeight = false;
	//	for (xx = 0 ; xx < sizeof (placingZone->cells [idxCell].tableInVerExtra) / sizeof (int) ; ++xx) {
	//		if (placingZone->cells [idxCell].tableInVerExtra [xx] > 0) {
	//			if (bFoundBottomHeight == false) {
	//				bottomHeight = placingZone->cells [idxCell].tableInVerExtra [xx];
	//				bFoundBottomHeight = true;
	//			}
	//			topHeight = placingZone->cells [idxCell].tableInVerExtra [xx];
	//			++realHeightCount;
	//		}
	//	}

	//	// ����ǽ� - �޸�
	//	if (placingZone->bSingleSide == false) {
	//		headpiece.init (L("RS Push-Pull Props ����ǽ� v2.0 (�ξ�� ����).gsm"), layerInd_HeadPiece, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

	//		moveIn3D ('z', headpiece.radAng - DegreeToRad (180.0), (double)(bottomHeight - 150 + 100) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
	//		moveIn3D ('y', headpiece.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1725, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
	//		moveIn3D ('x', headpiece.radAng - DegreeToRad (180.0), 0.300 + 0.200, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

	//		elemList_Back.Push (headpiece.placeObject (4, "type", APIParT_CString, "Ÿ�� B", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
	//		moveIn3D ('z', headpiece.radAng - DegreeToRad (180.0), (double)(-(bottomHeight - 150) + backHeight + (-topHeight + 150)) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
	//		elemList_Back.Push (headpiece.placeObject (4, "type", APIParT_CString, "Ÿ�� B", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));

	//		headpiece.init (L("RS Push-Pull Props ����ǽ� v2.0 (�ξ�� ����).gsm"), layerInd_HeadPiece, infoWall.floorInd, placingZone->cells [idxCell].leftBottomX, placingZone->cells [idxCell].leftBottomY, placingZone->cells [idxCell].leftBottomZ, placingZone->cells [idxCell].ang + DegreeToRad (180.0));

	//		if (placingZone->cells [idxCell].horLen >= 5300) {
	//			headpieceUpPosZ = 4200;
	//		} else if (placingZone->cells [idxCell].horLen >= 4600) {
	//			headpieceUpPosZ = 3800;
	//		} else if (placingZone->cells [idxCell].horLen >= 3500) {
	//			headpieceUpPosZ = 2800;
	//		} else if (placingZone->cells [idxCell].horLen >= 3000) {
	//			headpieceUpPosZ = 2200;
	//		} else if (placingZone->cells [idxCell].horLen >= 2500) {
	//			headpieceUpPosZ = 1900;
	//		} else if (placingZone->cells [idxCell].horLen >= 2000) {
	//			headpieceUpPosZ = 1500;
	//		} else if (placingZone->cells [idxCell].horLen >= 1500) {
	//			headpieceUpPosZ = 1100;
	//		} else if (placingZone->cells [idxCell].horLen >= 1000) {
	//			headpieceUpPosZ = 800;
	//		} else {
	//			headpieceUpPosZ = 150;
	//		}

	//		moveIn3D ('z', headpiece.radAng - DegreeToRad (180.0), (double)(bottomHeight - 150 + 100) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
	//		moveIn3D ('y', headpiece.radAng - DegreeToRad (180.0), infoWall.wallThk + placingZone->gap * 2 + 0.1725, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
	//		moveIn3D ('x', headpiece.radAng - DegreeToRad (180.0), (double)(headpieceUpPosZ + 200) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);

	//		elemList_Back.Push (headpiece.placeObject (4, "type", APIParT_CString, "Ÿ�� B", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
	//		moveIn3D ('z', headpiece.radAng - DegreeToRad (180.0), (double)(-(bottomHeight - 150) + backHeight + (-topHeight + 150)) / 1000.0, &headpiece.posX, &headpiece.posY, &headpiece.posZ);
	//		elemList_Back.Push (headpiece.placeObject (4, "type", APIParT_CString, "Ÿ�� B", "plateThk", APIParT_Length, format_string ("%f", 0.009), "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
	//	}
	//}

	//// ����� ��ü �׷�ȭ (�ո�)
	//if (!elemList_Front.IsEmpty ()) {
	//	GSSize nElems = elemList_Front.GetSize ();
	//	API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
	//	if (elemHead != NULL) {
	//		for (GSIndex i = 0; i < nElems; i++)
	//			(*elemHead)[i].guid = elemList_Front [i];

	//		ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

	//		BMKillHandle ((GSHandle *) &elemHead);
	//	}
	//	elemList_Front.Clear ();
	//}

	//// ����� ��ü �׷�ȭ (�޸�)
	//if (!elemList_Back.IsEmpty ()) {
	//	GSSize nElems = elemList_Back.GetSize ();
	//	API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
	//	if (elemHead != NULL) {
	//		for (GSIndex i = 0; i < nElems; i++)
	//			(*elemHead)[i].guid = elemList_Back [i];

	//		ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

	//		BMKillHandle ((GSHandle *) &elemHead);
	//	}
	//	elemList_Back.Clear ();
	//}
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
	int		presetValue, cellHeightValue, cellWidthValue, accumLength;
	const short		maxCol = 50;		// �� �ִ� ����
	const short		maxRow = 10;		// �� �ִ� ����
	double			totalWidth, totalHeight;
	static short	dialogSizeX = 550;			// ���� ���̾�α� ũ�� X
	static short	dialogSizeY = 950;			// ���� ���̾�α� ũ�� Y

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, "���̺���/���ڳ�/������/�ٷ������̼�/����/���� ����");

			//////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			// ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 225, 450, 70, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "Ȯ ��");
			DGShowItem (dialogID, DG_OK);

			// ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 325, 450, 70, 25);
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
			DGPopUpSelectItem (dialogID, placingZone.POPUP_DIRECTION, DG_POPUP_TOP);
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
			placingZone.initCells (&placingZone, true);

			//////////////////////////////////////////////////////////// ������ ��ġ (���� ���� ��ư)

			// !!! ��: ���ڳ�, �ƿ��ڳʾޱ�, �ƿ��ڳ��ǳ�, ���̺���(������ ����: ���� �����̸� 3��, ���� �����̸� 6�� - �ִ� �ʺ� 3600), ������, �ٷ������̼�, ����, ����
			
			// ���� ���ڳ��ǳ�/�ƿ��ڳ��ǳ�/�ƿ��ڳʾޱ�
			// ��ư
			placingZone.BUTTON_LCORNER = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 137, 71, 66);
			DGSetItemFont (dialogID, placingZone.BUTTON_LCORNER, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, placingZone.BUTTON_LCORNER, "����");
			DGShowItem (dialogID, placingZone.BUTTON_LCORNER);
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
				for (yy = 0 ; yy < sizeof (placingZone.presetWidth_tableform) / sizeof (int) ; ++yy) {
					DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
					_itoa (placingZone.presetWidth_tableform [yy], numbuf, 10);
					DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
				}
				DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP+1);
				DGShowItem (dialogID, placingZone.POPUP_WIDTH [xx]);

				// �ʺ� (�˾���Ʈ��) - ó������ ����
				placingZone.EDITCONTROL_WIDTH [xx] = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, itmPosY + 68, 70, 23);
				DGHideItem (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);

				itmPosX += 70;
			}

			//// ���� ���ڳ� ���� (üũ��ư)
			//placingZone.CHECKBOX_RINCORNER = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, 135, 70, 70);
			//DGSetItemFont (dialogID, placingZone.CHECKBOX_RINCORNER, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, placingZone.CHECKBOX_RINCORNER, "���ڳ�");
			//DGShowItem (dialogID, placingZone.CHECKBOX_RINCORNER);
			//DGSetItemValLong (dialogID, placingZone.CHECKBOX_RINCORNER, TRUE);
			//// ���� ���ڳ� ���� (Edit��Ʈ��)
			//placingZone.EDITCONTROL_RINCORNER = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, 205, 70, 25);
			//DGShowItem (dialogID, placingZone.EDITCONTROL_RINCORNER);
			//DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_RINCORNER, 0.100);

			////////////////////////////////////////////////////////////// ���̾�α� ũ�� ����, ���� �ʺ� �� ���� ���
			//// ���̾�α� ũ�� ����
			//dialogSizeX = 550;
			//dialogSizeY = 950;
			//if (placingZone.nCellsInHor >= 5) {
			//	DGSetDialogSize (dialogID, DG_CLIENT, dialogSizeX + 70 * (placingZone.nCellsInHor - 5), dialogSizeY, DG_TOPLEFT, true);
			//}

			//// ���� �ʺ� ���
			//totalWidth = 0.0;
			//if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_LINCORNER) == TRUE)	totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_LINCORNER);
			//if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_RINCORNER) == TRUE)	totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_RINCORNER);
			//for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
			//	if ((DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) || (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM))
			//		totalWidth += atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH [xx])).ToCStr ().Get ()) / 1000;
			//	else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == NONE)
			//		totalWidth += 0.0;
			//	else
			//		totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
			//}
			//DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_WIDTH, placingZone.horLen - totalWidth);

			//// ���� ���� ��� (������)
			//totalHeight = 0.0;
			//for (xx = 0 ; xx < placingZone.nCellsInVerBasic ; ++xx) {
			//	totalHeight += atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx])).ToCStr ().Get ()) / 1000;
			//}
			//DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_HEIGHT_BASIC, placingZone.verLenBasic - totalHeight);

			//if (placingZone.bExtra == true) {
			//	// ���� ���� ��� (������)
			//	totalHeight = 0.0;
			//	for (xx = 0 ; xx < placingZone.nCellsInVerExtra ; ++xx) {
			//		totalHeight += atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx])).ToCStr ().Get ()) / 1000;
			//	}
			//	DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_HEIGHT_EXTRA, placingZone.verLenExtra - totalHeight);
			//}

			//// �ʱⰪ�� ���ι���
			//placingZone.bVertical = true;

			break;

		case DG_MSG_CHANGE:
		//	// ����/���� ������ ��
		//	if (item == placingZone.POPUP_DIRECTION) {
		//		strcpy (buffer, DGPopUpGetItemText (dialogID, placingZone.POPUP_DIRECTION, DGPopUpGetSelected (dialogID, placingZone.POPUP_DIRECTION)).ToCStr ().Get ());

		//		// ������ ���
		//		if (my_strcmp (buffer, "����") == 0) {
		//			// �� ���� �ʱ�ȭ
		//			placingZone.initCells (&placingZone, false);
		//			placingZone.bVertical = false;

		//			// ����
		//			for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
		//				if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) {
		//					// �˾� ���� ����
		//					DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

		//					// �˾� ���� �ٽ� ä���
		//					for (yy = 0 ; yy < sizeof (placingZone.presetHeight_tableform) / sizeof (int) ; ++yy) {
		//						DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
		//						_itoa (placingZone.presetHeight_tableform [yy], numbuf, 10);
		//						DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
		//					}
		//					DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
		//				} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM) {
		//					// �˾� ���� ����
		//					DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

		//					// �˾� ���� �ٽ� ä���
		//					for (yy = 0 ; yy < sizeof (placingZone.presetHeight_euroform) / sizeof (int) ; ++yy) {
		//						DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
		//						_itoa (placingZone.presetHeight_euroform [yy], numbuf, 10);
		//						DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
		//					}
		//					DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
		//				}
		//			}

		//			// ����
		//			for (xx = 0 ; xx < placingZone.nCellsInVerBasic ; ++xx) {
		//				// �˾� ���� ����
		//				DGPopUpDeleteItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx], DG_ALL_ITEMS);

		//				// �˾� ���� �ٽ� ä���
		//				for (yy = 0 ; yy < sizeof (placingZone.presetWidth_euroform) / sizeof (int) ; ++yy) {
		//					DGPopUpInsertItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx], DG_POPUP_BOTTOM);
		//					_itoa (placingZone.presetWidth_euroform [yy], numbuf, 10);
		//					DGPopUpSetItemText (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx], DG_POPUP_BOTTOM, numbuf);
		//				}
		//				DGPopUpSelectItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx], DG_POPUP_TOP);
		//			}

		//			if (placingZone.bExtra == true) {
		//				for (xx = 0 ; xx < placingZone.nCellsInVerExtra ; ++xx) {
		//					// �˾� ���� ����
		//					DGPopUpDeleteItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx], DG_ALL_ITEMS);

		//					// �˾� ���� �ٽ� ä���
		//					for (yy = 0 ; yy < sizeof (placingZone.presetWidth_euroform) / sizeof (int) ; ++yy) {
		//						DGPopUpInsertItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx], DG_POPUP_BOTTOM);
		//						_itoa (placingZone.presetWidth_euroform [yy], numbuf, 10);
		//						DGPopUpSetItemText (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx], DG_POPUP_BOTTOM, numbuf);
		//					}
		//					DGPopUpSelectItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx], DG_POPUP_TOP);
		//				}
		//			}

		//			// ������
		//			DGPopUpDeleteItem (dialogID, placingZone.POPUP_HEIGHT_PRESET, DG_ALL_ITEMS);

		//			for (yy = 0 ; yy < sizeof (placingZone.presetWidth_tableform) / sizeof (int) ; ++yy) {
		//				DGPopUpInsertItem (dialogID, placingZone.POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM);
		//				_itoa (placingZone.presetWidth_tableform [yy], numbuf, 10);
		//				DGPopUpSetItemText (dialogID, placingZone.POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM, numbuf);
		//			}
		//			DGPopUpInsertItem (dialogID, placingZone.POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM);
		//			DGPopUpSetItemText (dialogID, placingZone.POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM, "Free");
		//			DGPopUpSelectItem (dialogID, placingZone.POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM);

		//		// ������ ���
		//		} else {
		//			// �� ���� �ʱ�ȭ
		//			placingZone.initCells (&placingZone, true);
		//			placingZone.bVertical = true;

		//			// ����
		//			for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
		//				if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) {
		//					// �˾� ���� ����
		//					DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

		//					// �˾� ���� �ٽ� ä���
		//					for (yy = 0 ; yy < sizeof (placingZone.presetWidth_tableform) / sizeof (int) ; ++yy) {
		//						DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
		//						_itoa (placingZone.presetWidth_tableform [yy], numbuf, 10);
		//						DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
		//					}
		//					DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
		//				} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM) {
		//					// �˾� ���� ����
		//					DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

		//					// �˾� ���� �ٽ� ä���
		//					for (yy = 0 ; yy < sizeof (placingZone.presetWidth_euroform) / sizeof (int) ; ++yy) {
		//						DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
		//						_itoa (placingZone.presetWidth_euroform [yy], numbuf, 10);
		//						DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
		//					}
		//					DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
		//				}
		//			}

		//			// ����
		//			for (xx = 0 ; xx < placingZone.nCellsInVerBasic ; ++xx) {
		//				// �˾� ���� ����
		//				DGPopUpDeleteItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx], DG_ALL_ITEMS);

		//				// �˾� ���� �ٽ� ä���
		//				for (yy = 0 ; yy < sizeof (placingZone.presetHeight_euroform) / sizeof (int) ; ++yy) {
		//					DGPopUpInsertItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx], DG_POPUP_BOTTOM);
		//					_itoa (placingZone.presetHeight_euroform [yy], numbuf, 10);
		//					DGPopUpSetItemText (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx], DG_POPUP_BOTTOM, numbuf);
		//				}
		//				DGPopUpSelectItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx], DG_POPUP_TOP);
		//			}

		//			if (placingZone.bExtra == true) {
		//				for (xx = 0 ; xx < placingZone.nCellsInVerExtra ; ++xx) {
		//					// �˾� ���� ����
		//					DGPopUpDeleteItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx], DG_ALL_ITEMS);

		//					// �˾� ���� �ٽ� ä���
		//					for (yy = 0 ; yy < sizeof (placingZone.presetHeight_euroform) / sizeof (int) ; ++yy) {
		//						DGPopUpInsertItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx], DG_POPUP_BOTTOM);
		//						_itoa (placingZone.presetHeight_euroform [yy], numbuf, 10);
		//						DGPopUpSetItemText (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx], DG_POPUP_BOTTOM, numbuf);
		//					}
		//					DGPopUpSelectItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx], DG_POPUP_TOP);
		//				}
		//			}

		//			// ������
		//			DGInvalidateItem (dialogID, placingZone.POPUP_HEIGHT_PRESET);

		//			for (yy = 0 ; yy < sizeof (placingZone.presetHeight_tableform) / sizeof (int) ; ++yy) {
		//				DGPopUpInsertItem (dialogID, placingZone.POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM);
		//				_itoa (placingZone.presetHeight_tableform [yy], numbuf, 10);
		//				DGPopUpSetItemText (dialogID, placingZone.POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM, numbuf);
		//			}
		//			DGPopUpInsertItem (dialogID, placingZone.POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM);
		//			DGPopUpSetItemText (dialogID, placingZone.POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM, "Free");
		//			DGPopUpSelectItem (dialogID, placingZone.POPUP_HEIGHT_PRESET, DG_POPUP_BOTTOM);
		//		}
		//	}

		//	// ��ü Ÿ�� ������ ��
		//	for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
		//		if (item == placingZone.POPUP_OBJ_TYPE [xx]) {
		//			// �ش� ��ư�� �̸� ����
		//			DGSetItemText (dialogID, placingZone.BUTTON_OBJ [xx], DGPopUpGetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx])));
		//			
		//			// ����/���� ���� ���ο� ���� �˾���Ʈ���� ���빰�� �ٲ�
		//			if (placingZone.bVertical == false) {
		//				if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) {
		//					// �˾� ���� ����
		//					DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

		//					// �˾� ���� �ٽ� ä���
		//					for (yy = 0 ; yy < sizeof (placingZone.presetHeight_tableform) / sizeof (int) ; ++yy) {
		//						DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
		//						_itoa (placingZone.presetHeight_tableform [yy], numbuf, 10);
		//						DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
		//					}
		//					DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
		//				} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM) {
		//					// �˾� ���� ����
		//					DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

		//					// �˾� ���� �ٽ� ä���
		//					for (yy = 0 ; yy < sizeof (placingZone.presetHeight_euroform) / sizeof (int) ; ++yy) {
		//						DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
		//						_itoa (placingZone.presetHeight_euroform [yy], numbuf, 10);
		//						DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
		//					}
		//					DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
		//				}
		//			} else {
		//				if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) {
		//					// �˾� ���� ����
		//					DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

		//					// �˾� ���� �ٽ� ä���
		//					for (yy = 0 ; yy < sizeof (placingZone.presetWidth_tableform) / sizeof (int) ; ++yy) {
		//						DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
		//						_itoa (placingZone.presetWidth_tableform [yy], numbuf, 10);
		//						DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
		//					}
		//					DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
		//				} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM) {
		//					// �˾� ���� ����
		//					DGPopUpDeleteItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_ALL_ITEMS);

		//					// �˾� ���� �ٽ� ä���
		//					for (yy = 0 ; yy < sizeof (placingZone.presetWidth_euroform) / sizeof (int) ; ++yy) {
		//						DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
		//						_itoa (placingZone.presetWidth_euroform [yy], numbuf, 10);
		//						DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
		//					}
		//					DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
		//				}
		//			}

		//			// ���̺��� Ÿ���� �ƴϸ� ��ư�� ���
		//			if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) != TABLEFORM)
		//				DGDisableItem (dialogID, placingZone.BUTTON_OBJ [xx]);
		//			else
		//				DGEnableItem (dialogID, placingZone.BUTTON_OBJ [xx]);

		//			// ���̺���/�������̸� �ʺ� �˾���Ʈ��, �� �ܿ��� Edit��Ʈ��, ������ ��� ����
		//			if ((DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) || (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM)) {
		//				if (!DGIsItemVisible (dialogID, placingZone.POPUP_WIDTH [xx]))			DGShowItem (dialogID, placingZone.POPUP_WIDTH [xx]);
		//				if (DGIsItemVisible (dialogID, placingZone.EDITCONTROL_WIDTH [xx]))		DGHideItem (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
		//			} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == NONE) {
		//				if (DGIsItemVisible (dialogID, placingZone.EDITCONTROL_WIDTH [xx]))		DGHideItem (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
		//				if (DGIsItemVisible (dialogID, placingZone.POPUP_WIDTH [xx]))			DGHideItem (dialogID, placingZone.POPUP_WIDTH [xx]);
		//			} else {
		//				if (!DGIsItemVisible (dialogID, placingZone.EDITCONTROL_WIDTH [xx]))	DGShowItem (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
		//				if (DGIsItemVisible (dialogID, placingZone.POPUP_WIDTH [xx]))			DGHideItem (dialogID, placingZone.POPUP_WIDTH [xx]);

		//				// �ٷ������̼�, ����, ������ ���� ���� ������ �ּ�, �ִ밪
		//				if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == FILLERSP) {
		//					DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx], 0.010);
		//					DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx], 0.050);
		//				} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == PLYWOOD) {
		//					DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx], 0.090);
		//					DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx], 1.220);
		//				} else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TIMBER) {
		//					DGSetItemMinDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx], 0.005);
		//					DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx], 1.000);
		//				}
		//			}
		//		}
		//	}

		//	// ������ �����
		//	if (item == placingZone.POPUP_HEIGHT_PRESET) {
		//		// ������ ��
		//		if (placingZone.bVertical == false) {
		//			presetValue = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_PRESET, DGPopUpGetSelected (dialogID, placingZone.POPUP_HEIGHT_PRESET)).ToCStr ().Get ());
		//			for (xx = 0 ; xx < sizeof (placingZone.presetWidth_tableform) / sizeof (int) ; ++xx) {
		//				if (presetValue == placingZone.presetWidth_tableform [xx]) {
		//					for (yy = 0 ; yy < placingZone.nCellsInVerBasic ; ++yy) {
		//						DGSetItemFont (dialogID, placingZone.POPUP_HEIGHT_BASIC [yy], DG_IS_LARGE | DG_IS_PLAIN);
		//						for (zz = 1 ; zz <= DGPopUpGetItemCount (dialogID, placingZone.POPUP_HEIGHT_BASIC [yy]) ; ++zz) {
		//							cellHeightValue = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_BASIC [yy], zz).ToCStr ().Get ());
		//							if (cellHeightValue == placingZone.presetHeight_config_horizontal [xx][yy+1]) {
		//								DGPopUpSelectItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [yy], zz);
		//								DGSetItemFont (dialogID, placingZone.POPUP_HEIGHT_BASIC [yy], DG_IS_LARGE | DG_IS_BOLD);
		//							}
		//						}
		//					}
		//				}
		//			}
		//			if (placingZone.bExtra == true) {
		//				presetValue = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_PRESET, DGPopUpGetSelected (dialogID, placingZone.POPUP_HEIGHT_PRESET)).ToCStr ().Get ());
		//				for (xx = 0 ; xx < sizeof (placingZone.presetWidth_tableform) / sizeof (int) ; ++xx) {
		//					if (presetValue == placingZone.presetWidth_tableform [xx]) {
		//						for (yy = 0 ; yy < placingZone.nCellsInVerExtra ; ++yy) {
		//							DGSetItemFont (dialogID, placingZone.POPUP_HEIGHT_EXTRA [yy], DG_IS_LARGE | DG_IS_PLAIN);
		//							for (zz = 1 ; zz <= DGPopUpGetItemCount (dialogID, placingZone.POPUP_HEIGHT_EXTRA [yy]) ; ++zz) {
		//								cellHeightValue = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_EXTRA [yy], zz).ToCStr ().Get ());
		//								if (cellHeightValue == placingZone.presetHeight_config_horizontal [xx][yy+1]) {
		//									DGPopUpSelectItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [yy], zz);
		//									DGSetItemFont (dialogID, placingZone.POPUP_HEIGHT_EXTRA [yy], DG_IS_LARGE | DG_IS_BOLD);
		//								}
		//							}
		//						}
		//					}
		//				}
		//			}

		//		// ������ ��
		//		} else {
		//			presetValue = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_PRESET, DGPopUpGetSelected (dialogID, placingZone.POPUP_HEIGHT_PRESET)).ToCStr ().Get ());
		//			for (xx = 0 ; xx < sizeof (placingZone.presetHeight_tableform) / sizeof (int) ; ++xx) {
		//				if (presetValue == placingZone.presetHeight_tableform [xx]) {
		//					for (yy = 0 ; yy < placingZone.nCellsInVerBasic ; ++yy) {
		//						DGSetItemFont (dialogID, placingZone.POPUP_HEIGHT_BASIC [yy], DG_IS_LARGE | DG_IS_PLAIN);
		//						for (zz = 1 ; zz <= DGPopUpGetItemCount (dialogID, placingZone.POPUP_HEIGHT_BASIC [yy]) ; ++zz) {
		//							cellHeightValue = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_BASIC [yy], zz).ToCStr ().Get ());
		//							if (cellHeightValue == placingZone.presetHeight_config_vertical [xx][yy+1]) {
		//								DGPopUpSelectItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [yy], zz);
		//								DGSetItemFont (dialogID, placingZone.POPUP_HEIGHT_BASIC [yy], DG_IS_LARGE | DG_IS_BOLD);
		//							}
		//						}
		//					}
		//				}
		//			}
		//			if (placingZone.bExtra == true) {
		//				presetValue = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_PRESET, DGPopUpGetSelected (dialogID, placingZone.POPUP_HEIGHT_PRESET)).ToCStr ().Get ());
		//				for (xx = 0 ; xx < sizeof (placingZone.presetHeight_tableform) / sizeof (int) ; ++xx) {
		//					if (presetValue == placingZone.presetHeight_tableform [xx]) {
		//						for (yy = 0 ; yy < placingZone.nCellsInVerExtra ; ++yy) {
		//							DGSetItemFont (dialogID, placingZone.POPUP_HEIGHT_EXTRA [yy], DG_IS_LARGE | DG_IS_PLAIN);
		//							for (zz = 1 ; zz <= DGPopUpGetItemCount (dialogID, placingZone.POPUP_HEIGHT_EXTRA [yy]) ; ++zz) {
		//								cellHeightValue = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_EXTRA [yy], zz).ToCStr ().Get ());
		//								if (cellHeightValue == placingZone.presetHeight_config_vertical [xx][yy+1]) {
		//									DGPopUpSelectItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [yy], zz);
		//									DGSetItemFont (dialogID, placingZone.POPUP_HEIGHT_EXTRA [yy], DG_IS_LARGE | DG_IS_BOLD);
		//								}
		//							}
		//						}
		//					}
		//				}
		//			}
		//		}
		//	}

		//	// ���̺��� �ʺ� ������ ��
		//	for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
		//		if (item == placingZone.POPUP_WIDTH [xx]) {
		//			if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) {
		//				cellWidthValue = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], (DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH [xx]))).ToCStr ().Get ());
		//				if (placingZone.bVertical == true) {
		//					for (yy = 0 ; yy < sizeof (placingZone.presetWidth_tableform) / sizeof (int) ; ++yy) {
		//						if (cellWidthValue == placingZone.presetWidth_tableform [yy]) {
		//							for (zz = 0 ; zz < sizeof (placingZone.cells [xx].tableInHor) / sizeof (int) ; ++zz) {
		//								if ((zz >= 0) && (zz < placingZone.presetWidth_config_vertical [yy][0]))
		//									placingZone.cells [xx].tableInHor [zz] = placingZone.presetWidth_config_vertical [yy][zz+1];
		//								else
		//									placingZone.cells [xx].tableInHor [zz] = 0;
		//							}
		//						}
		//					}
		//				} else {
		//					for (yy = 0 ; yy < sizeof (placingZone.presetHeight_tableform) / sizeof (int) ; ++yy) {
		//						if (cellWidthValue == placingZone.presetHeight_tableform [yy]) {
		//							for (zz = 0 ; zz < sizeof (placingZone.cells [xx].tableInHor) / sizeof (int) ; ++zz) {
		//								if ((zz >= 0) && (zz < placingZone.presetWidth_config_horizontal [yy][0]))
		//									placingZone.cells [xx].tableInHor [zz] = placingZone.presetWidth_config_horizontal [yy][zz+1];
		//								else
		//									placingZone.cells [xx].tableInHor [zz] = 0;
		//							}
		//						}
		//					}
		//				}
		//				placingZone.cells [xx].horLen = cellWidthValue;
		//			}
		//		}
		//	}

		//	// ���� �ʺ� ���
		//	totalWidth = 0.0;
		//	if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_LINCORNER) == TRUE)	totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_LINCORNER);
		//	if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_RINCORNER) == TRUE)	totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_RINCORNER);
		//	for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
		//		if ((DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) || (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM))
		//			totalWidth += atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH [xx])).ToCStr ().Get ()) / 1000;
		//		else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == NONE)
		//			totalWidth += 0.0;
		//		else
		//			totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
		//	}
		//	DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_WIDTH, placingZone.horLen - totalWidth);

		//	// ���� ���� ��� (������)
		//	totalHeight = 0.0;
		//	for (xx = 0 ; xx < placingZone.nCellsInVerBasic ; ++xx) {
		//		totalHeight += atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx])).ToCStr ().Get ()) / 1000;
		//	}
		//	DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_HEIGHT_BASIC, placingZone.verLenBasic - totalHeight);

		//	if (placingZone.bExtra == true) {
		//		// ���� ���� ��� (������)
		//		totalHeight = 0.0;
		//		for (xx = 0 ; xx < placingZone.nCellsInVerExtra ; ++xx) {
		//			totalHeight += atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx])).ToCStr ().Get ()) / 1000;
		//		}
		//		DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_HEIGHT_EXTRA, placingZone.verLenExtra - totalHeight);
		//	}
			break;

		case DG_MSG_CLICK:
			// Ȯ�� ��ư
			if (item == DG_OK) {

		//		// ������ ����
		//		placingZone.gap = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_GAP);

		//		// �ܸ� ����
		//		placingZone.bSingleSide = (DGGetItemValLong (dialogID, placingZone.CHECKBOX_SINGLESIDE) == TRUE) ? true : false;

		//		// ���̺��� ����
		//		strcpy (buffer, DGPopUpGetItemText (dialogID, placingZone.POPUP_DIRECTION, DGPopUpGetSelected (dialogID, placingZone.POPUP_DIRECTION)).ToCStr ().Get ());
		//		if (my_strcmp (buffer, "����") == 0)
		//			placingZone.bVertical = true;
		//		else
		//			placingZone.bVertical = false;

		//		// ���̺��� Ÿ��
		//		strcpy (buffer, DGPopUpGetItemText (dialogID, placingZone.POPUP_TABLEFORM_TYPE, DGPopUpGetSelected (dialogID, placingZone.POPUP_TABLEFORM_TYPE)).ToCStr ().Get ());
		//		if (my_strcmp (buffer, "Ÿ��A") == 0)
		//			placingZone.tableformType = 1;
		//		else if (my_strcmp (buffer, "Ÿ��B") == 0)
		//			placingZone.tableformType = 2;
		//		else if (my_strcmp (buffer, "Ÿ��C") == 0)
		//			placingZone.tableformType = 3;

		//		// ���ڳ� ���� �� ����
		//		placingZone.bLincorner = (DGGetItemValLong (dialogID, placingZone.CHECKBOX_LINCORNER) == TRUE) ? true : false;
		//		placingZone.bRincorner = (DGGetItemValLong (dialogID, placingZone.CHECKBOX_RINCORNER) == TRUE) ? true : false;
		//		placingZone.lenLincorner = (placingZone.bLincorner == true) ? DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_LINCORNER) : 0.0;
		//		placingZone.lenRincorner = (placingZone.bRincorner == true) ? DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_RINCORNER) : 0.0;

		//		// ���� ����, ���̺� �� ���� ���� ��� 0���� �ʱ�ȭ
		//		for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
		//			placingZone.cells [xx].verLenBasic = 0;
		//			placingZone.cells [xx].verLenExtra = 0;

		//			for (yy = 0 ; yy < sizeof (placingZone.cells [xx].tableInVerBasic) / sizeof (int) ; ++yy)
		//				placingZone.cells [xx].tableInVerBasic [yy] = 0;
		//			for (yy = 0 ; yy < sizeof (placingZone.cells [xx].tableInVerExtra) / sizeof (int) ; ++yy)
		//				placingZone.cells [xx].tableInVerExtra [yy] = 0;
		//		}

		//		// �� ���� ������Ʈ
		//		for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
		//			// ��ü Ÿ��
		//			placingZone.cells [xx].objType = DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]);

		//			// ���� ���� ���� ����
		//			if (placingZone.cells [xx].objType == NONE) {
		//				// ���� ���� 0
		//				placingZone.cells [xx].horLen = 0;

		//				// ���̺� �� ���� ���� ��� 0
		//				for (yy = 0 ; yy < sizeof (placingZone.cells [xx].tableInHor) / sizeof (int) ; ++yy)
		//					placingZone.cells [xx].tableInHor [yy] = 0;

		//			} else if (placingZone.cells [xx].objType == EUROFORM) {
		//				// ������ �ʺ�
		//				placingZone.cells [xx].horLen = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH [xx])).ToCStr ().Get ());

		//				// ���̺� �� ���� ���� ��� 0
		//				for (yy = 0 ; yy < sizeof (placingZone.cells [xx].tableInHor) / sizeof (int) ; ++yy)
		//					placingZone.cells [xx].tableInHor [yy] = 0;

		//			} else if ((placingZone.cells [xx].objType == FILLERSP) || (placingZone.cells [xx].objType == PLYWOOD) || (placingZone.cells [xx].objType == TIMBER)) {
		//				// �ٷ������̼�, ����, ������ �ʺ�
		//				placingZone.cells [xx].horLen = (int)(DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx]) * 1000);

		//				// ���̺� �� ���� ���� ��� 0
		//				for (yy = 0 ; yy < sizeof (placingZone.cells [xx].tableInHor) / sizeof (int) ; ++yy)
		//					placingZone.cells [xx].tableInHor [yy] = 0;
		//			}

		//			// ���� ���� ���� ���� (������)
		//			accumLength = 0;
		//			for (yy = 0 ; yy < placingZone.nCellsInVerBasic ; ++yy) {
		//				// ���̺� �� ���� ���� ����
		//				placingZone.cells [xx].tableInVerBasic [yy] = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_BASIC [yy], DGPopUpGetSelected (dialogID, placingZone.POPUP_HEIGHT_BASIC [yy])).ToCStr ().Get ());
		//				accumLength += placingZone.cells [xx].tableInVerBasic [yy];
		//			}
		//			placingZone.cells [xx].verLenBasic = accumLength;
		//		
		//			// ���� ���� ���� ���� (������)
		//			accumLength = 0;
		//			for (yy = 0 ; yy < placingZone.nCellsInVerExtra ; ++yy) {
		//				// ���̺� �� ���� ���� ����
		//				placingZone.cells [xx].tableInVerExtra [yy] = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_EXTRA [yy], DGPopUpGetSelected (dialogID, placingZone.POPUP_HEIGHT_EXTRA [yy])).ToCStr ().Get ());
		//				accumLength += placingZone.cells [xx].tableInVerExtra [yy];
		//			}
		//			placingZone.cells [xx].verLenExtra = accumLength;
		//		}

		//		// ���̾� ����
		//		bLayerInd_Euroform = true;		// ������ �׻� On
		//		bLayerInd_RectPipe = false;
		//		bLayerInd_PinBolt = false;
		//		bLayerInd_WallTie = false;
		//		bLayerInd_HeadPiece = false;
		//		bLayerInd_Join = false;

		//		bLayerInd_SlabTableform = false;
		//		bLayerInd_Profile = false;

		//		bLayerInd_Steelform = false;
		//		bLayerInd_Plywood = true;		// ���� �׻� On
		//		bLayerInd_Timber = true;		// ���� �׻� On
		//		bLayerInd_OutcornerAngle = false;
		//		bLayerInd_OutcornerPanel = false;
		//		bLayerInd_RectpipeHanger = false;
		//		bLayerInd_EuroformHook = false;
		//		bLayerInd_CrossJointBar = false;
		//		bLayerInd_Hidden = false;

		//		bLayerInd_Fillersp = false;
		//		for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
		//			if (placingZone.cells [xx].objType == FILLERSP) {
		//				bLayerInd_Fillersp = true;
		//				break;
		//			}
		//		}

		//		bLayerInd_IncornerPanel = false;
		//		if ((placingZone.bLincorner == true) || (placingZone.bRincorner == true))
		//			bLayerInd_IncornerPanel = true;

		//		bLayerInd_BlueClamp = true;			// ���Ŭ���� �׻� On
		//		bLayerInd_BlueTimberRail = true;	// ����� �׻� On

		//		if (placingZone.tableformType == 1) {
		//			bLayerInd_Euroform = true;
		//			bLayerInd_RectPipe = true;
		//			bLayerInd_PinBolt = true;
		//			bLayerInd_HeadPiece = true;
		//			bLayerInd_Join = true;

		//		} else if (placingZone.tableformType == 2) {
		//			bLayerInd_Euroform = true;
		//			bLayerInd_RectPipe = true;
		//			bLayerInd_PinBolt = true;
		//			bLayerInd_HeadPiece = true;
		//			bLayerInd_Join = true;
		//		
		//		} else if (placingZone.tableformType == 3) {
		//			bLayerInd_Euroform = true;
		//			bLayerInd_RectPipe = true;
		//			bLayerInd_PinBolt = true;
		//			bLayerInd_HeadPiece = true;
		//			bLayerInd_Join = true;
		//			bLayerInd_CrossJointBar = true;
		//		}

		//		placingZone.marginTopBasic = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_HEIGHT_BASIC);		// ��� ���� (������)
		//		placingZone.marginTopExtra = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_HEIGHT_EXTRA);		// ��� ���� (������)

		//		placingZone.adjustCellsPosition (&placingZone);		// ���� ��ġ ����

			} else if (item == DG_CANCEL) {
				// �ƹ� �۾��� ���� ����
			} else {
		//		if ((item == placingZone.BUTTON_ADD_HOR) || (item == placingZone.BUTTON_DEL_HOR)) {
		//			// ���� - �߰� ��ư Ŭ��
		//			if (item == placingZone.BUTTON_ADD_HOR) {
		//				if (placingZone.nCellsInHor < maxCol) {
		//					// ���� ���ڳ� ��ư�� �����
		//					DGRemoveDialogItem (dialogID, placingZone.CHECKBOX_RINCORNER);
		//					DGRemoveDialogItem (dialogID, placingZone.EDITCONTROL_RINCORNER);

		//					// ������ �� ��ư �����ʿ� ���ο� �� ��ư�� �߰��ϰ�
		//					itmPosX = 90 + (70 * placingZone.nCellsInHor);
		//					itmPosY = 137;
		//					// ��ư
		//					placingZone.BUTTON_OBJ [placingZone.nCellsInHor] = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 71, 66);
		//					DGSetItemFont (dialogID, placingZone.BUTTON_OBJ [placingZone.nCellsInHor], DG_IS_LARGE | DG_IS_PLAIN);
		//					DGSetItemText (dialogID, placingZone.BUTTON_OBJ [placingZone.nCellsInHor], "���̺���");
		//					DGShowItem (dialogID, placingZone.BUTTON_OBJ [placingZone.nCellsInHor]);

		//					// ��ü Ÿ�� (�˾���Ʈ��)
		//					placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor] = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY - 25, 70, 23);
		//					DGSetItemFont (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_IS_EXTRASMALL | DG_IS_PLAIN);
		//					DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
		//					DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM, "����");
		//					DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
		//					DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM, "���̺���");
		//					DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
		//					DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM, "������");
		//					DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
		//					DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM, "�ٷ������̼�");
		//					DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
		//					DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM, "����");
		//					DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
		//					DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_BOTTOM, "����");
		//					DGPopUpSelectItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor], DG_POPUP_TOP+1);
		//					DGShowItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor]);

		//					// �ʺ� (�˾���Ʈ��)
		//					placingZone.POPUP_WIDTH [placingZone.nCellsInHor] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY + 68, 70, 23);
		//					DGSetItemFont (dialogID, placingZone.POPUP_WIDTH [placingZone.nCellsInHor], DG_IS_LARGE | DG_IS_PLAIN);
		//					if (placingZone.bVertical == true) {
		//						for (yy = 0 ; yy < sizeof (placingZone.presetWidth_tableform) / sizeof (int) ; ++yy) {
		//							DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
		//							_itoa (placingZone.presetWidth_tableform [yy], numbuf, 10);
		//							DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [placingZone.nCellsInHor], DG_POPUP_BOTTOM, numbuf);
		//						}
		//					} else {
		//						for (yy = 0 ; yy < sizeof (placingZone.presetHeight_tableform) / sizeof (int) ; ++yy) {
		//							DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCellsInHor], DG_POPUP_BOTTOM);
		//							_itoa (placingZone.presetHeight_tableform [yy], numbuf, 10);
		//							DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [placingZone.nCellsInHor], DG_POPUP_BOTTOM, numbuf);
		//						}
		//					}
		//					DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCellsInHor], DG_POPUP_TOP);
		//					DGShowItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCellsInHor]);

		//					// �ʺ� (�˾���Ʈ��) - ó������ ����
		//					placingZone.EDITCONTROL_WIDTH [placingZone.nCellsInHor] = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, itmPosY + 68, 70, 23);

		//					itmPosX += 70;

		//					// ���� ���ڳ� ��ư�� ������ ���� ����
		//					// ���� ���ڳ� ���� (üũ��ư)
		//					placingZone.CHECKBOX_RINCORNER = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, 135, 70, 70);
		//					DGSetItemFont (dialogID, placingZone.CHECKBOX_RINCORNER, DG_IS_LARGE | DG_IS_PLAIN);
		//					DGSetItemText (dialogID, placingZone.CHECKBOX_RINCORNER, "���ڳ�");
		//					DGShowItem (dialogID, placingZone.CHECKBOX_RINCORNER);
		//					DGSetItemValLong (dialogID, placingZone.CHECKBOX_RINCORNER, TRUE);
		//					// ���� ���ڳ� ���� (Edit��Ʈ��)
		//					placingZone.EDITCONTROL_RINCORNER = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, 205, 70, 25);
		//					DGShowItem (dialogID, placingZone.EDITCONTROL_RINCORNER);
		//					DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_RINCORNER, 0.100);

		//					++placingZone.nCellsInHor;
		//				}
		//			}

		//			// ���� - ���� ��ư Ŭ��
		//			else if (item == placingZone.BUTTON_DEL_HOR) {
		//				if (placingZone.nCellsInHor > 1) {
		//					// ���� ���ڳ� ��ư�� �����
		//					DGRemoveDialogItem (dialogID, placingZone.CHECKBOX_RINCORNER);
		//					DGRemoveDialogItem (dialogID, placingZone.EDITCONTROL_RINCORNER);

		//					// ������ �� ��ư�� �����
		//					DGRemoveDialogItem (dialogID, placingZone.BUTTON_OBJ [placingZone.nCellsInHor - 1]);
		//					DGRemoveDialogItem (dialogID, placingZone.POPUP_OBJ_TYPE [placingZone.nCellsInHor - 1]);
		//					DGRemoveDialogItem (dialogID, placingZone.POPUP_WIDTH [placingZone.nCellsInHor - 1]);
		//					DGRemoveDialogItem (dialogID, placingZone.EDITCONTROL_WIDTH [placingZone.nCellsInHor - 1]);

		//					// 3. ���� ���ڳ� ��ư�� ������ ���� ����
		//					itmPosX = 90 + (70 * (placingZone.nCellsInHor - 1));
		//					itmPosY = 137;
		//					// ���� ���ڳ� ���� (üũ��ư)
		//					placingZone.CHECKBOX_RINCORNER = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, 135, 70, 70);
		//					DGSetItemFont (dialogID, placingZone.CHECKBOX_RINCORNER, DG_IS_LARGE | DG_IS_PLAIN);
		//					DGSetItemText (dialogID, placingZone.CHECKBOX_RINCORNER, "���ڳ�");
		//					DGShowItem (dialogID, placingZone.CHECKBOX_RINCORNER);
		//					DGSetItemValLong (dialogID, placingZone.CHECKBOX_RINCORNER, TRUE);
		//					// ���� ���ڳ� ���� (Edit��Ʈ��)
		//					placingZone.EDITCONTROL_RINCORNER = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, 205, 70, 25);
		//					DGShowItem (dialogID, placingZone.EDITCONTROL_RINCORNER);
		//					DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_RINCORNER, 0.100);

		//					--placingZone.nCellsInHor;
		//				}
		//			}

		//		} else if ((item == placingZone.BUTTON_ADD_VER_BASIC) || (item == placingZone.BUTTON_DEL_VER_BASIC)) {
		//			// ���� - �߰�(L) ��ư Ŭ��
		//			if (item == placingZone.BUTTON_ADD_VER_BASIC) {
		//				if (placingZone.nCellsInVerBasic < maxRow) {
		//					// �� ���� �� �ϳ� �߰�
		//					itmPosX = 105;
		//					itmPosY = 820 - (50 * placingZone.nCellsInVerBasic);

		//					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 70, 50);
		//					DGShowItem (dialogID, itmIdx);

		//					placingZone.POPUP_HEIGHT_BASIC [placingZone.nCellsInVerBasic] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX + 2, itmPosY + 15, 65, 23);
		//					DGSetItemFont (dialogID, placingZone.POPUP_HEIGHT_BASIC [placingZone.nCellsInVerBasic], DG_IS_LARGE | DG_IS_PLAIN);
		//					if (placingZone.bVertical == true) {
		//						for (yy = 0 ; yy < sizeof (placingZone.presetHeight_euroform) / sizeof (int) ; ++yy) {
		//							DGPopUpInsertItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [placingZone.nCellsInVerBasic], DG_POPUP_BOTTOM);
		//							_itoa (placingZone.presetHeight_euroform [yy], numbuf, 10);
		//							DGPopUpSetItemText (dialogID, placingZone.POPUP_HEIGHT_BASIC [placingZone.nCellsInVerBasic], DG_POPUP_BOTTOM, numbuf);
		//						}
		//					} else {
		//						for (yy = 0 ; yy < sizeof (placingZone.presetWidth_euroform) / sizeof (int) ; ++yy) {
		//							DGPopUpInsertItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [placingZone.nCellsInVerBasic], DG_POPUP_BOTTOM);
		//							_itoa (placingZone.presetWidth_euroform [yy], numbuf, 10);
		//							DGPopUpSetItemText (dialogID, placingZone.POPUP_HEIGHT_BASIC [placingZone.nCellsInVerBasic], DG_POPUP_BOTTOM, numbuf);
		//						}
		//					}
		//					DGPopUpSelectItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [placingZone.nCellsInVerBasic], DG_POPUP_TOP);
		//					DGShowItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [placingZone.nCellsInVerBasic]);

		//					++placingZone.nCellsInVerBasic;
		//				}						
		//			}

		//			// ���� - ����(L) ��ư Ŭ��
		//			else if (item == placingZone.BUTTON_DEL_VER_BASIC) {
		//				if (placingZone.nCellsInVerBasic > 1) {
		//					// �� ���� �ִ� �� �ϳ� ����
		//					DGRemoveDialogItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [placingZone.nCellsInVerBasic - 1]);
		//					DGRemoveDialogItem (dialogID, placingZone.POPUP_HEIGHT_BASIC [placingZone.nCellsInVerBasic - 1] - 1);

		//					--placingZone.nCellsInVerBasic;
		//				}
		//			}

		//		} else if ((item == placingZone.BUTTON_ADD_VER_EXTRA) || (item == placingZone.BUTTON_DEL_VER_EXTRA)) {
		//			// ���� - �߰�(H) ��ư Ŭ��
		//			if (item == placingZone.BUTTON_ADD_VER_EXTRA) {
		//				if (placingZone.nCellsInVerExtra < maxRow) {
		//					// �� ���� �� �ϳ� �߰�
		//					itmPosX = 185;
		//					itmPosY = 820 - (50 * placingZone.nCellsInVerExtra);

		//					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 70, 50);
		//					DGShowItem (dialogID, itmIdx);

		//					placingZone.POPUP_HEIGHT_EXTRA [placingZone.nCellsInVerExtra] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX + 2, itmPosY + 15, 65, 23);
		//					DGSetItemFont (dialogID, placingZone.POPUP_HEIGHT_EXTRA [placingZone.nCellsInVerExtra], DG_IS_LARGE | DG_IS_PLAIN);
		//					if (placingZone.bVertical == true) {
		//						for (yy = 0 ; yy < sizeof (placingZone.presetHeight_euroform) / sizeof (int) ; ++yy) {
		//							DGPopUpInsertItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [placingZone.nCellsInVerExtra], DG_POPUP_BOTTOM);
		//							_itoa (placingZone.presetHeight_euroform [yy], numbuf, 10);
		//							DGPopUpSetItemText (dialogID, placingZone.POPUP_HEIGHT_EXTRA [placingZone.nCellsInVerExtra], DG_POPUP_BOTTOM, numbuf);
		//						}
		//					} else {
		//						for (yy = 0 ; yy < sizeof (placingZone.presetWidth_euroform) / sizeof (int) ; ++yy) {
		//							DGPopUpInsertItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [placingZone.nCellsInVerExtra], DG_POPUP_BOTTOM);
		//							_itoa (placingZone.presetWidth_euroform [yy], numbuf, 10);
		//							DGPopUpSetItemText (dialogID, placingZone.POPUP_HEIGHT_EXTRA [placingZone.nCellsInVerExtra], DG_POPUP_BOTTOM, numbuf);
		//						}
		//					}
		//					DGPopUpSelectItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [placingZone.nCellsInVerExtra], DG_POPUP_TOP);
		//					DGShowItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [placingZone.nCellsInVerExtra]);

		//					++placingZone.nCellsInVerExtra;
		//				}
		//			}

		//			// ���� - ����(H) ��ư Ŭ��
		//			else if (item == placingZone.BUTTON_DEL_VER_EXTRA) {
		//				if (placingZone.nCellsInVerExtra > 1) {
		//					// �� ���� �ִ� �� �ϳ� ����
		//					DGRemoveDialogItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [placingZone.nCellsInVerExtra - 1]);
		//					DGRemoveDialogItem (dialogID, placingZone.POPUP_HEIGHT_EXTRA [placingZone.nCellsInVerExtra - 1] - 1);

		//					--placingZone.nCellsInVerExtra;
		//				}
		//			}

		//		} else {
		//			// ��ü ��ư Ŭ�� (���̺����� ��쿡�� ��ȿ��)
		//			for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
		//				if ((item == placingZone.BUTTON_OBJ [xx]) && (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM)) {
		//					clickedIndex = xx;
		//					if (placingZone.bVertical == true) {
		//						// ���̺��� Ÿ�� (���� ����)�� ���, 3��° ���̾�α�(���ι���) ����
		//						result = DGBlankModalDialog (770, 180, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, wallTableformPlacerHandler3_Vertical, (short) 0);
		//					} else {
		//						// ���̺��� Ÿ�� (���� ����)�� ���, 3��° ���̾�α�(���ι���) ����
		//						result = DGBlankModalDialog (770, 180, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, wallTableformPlacerHandler3_Horizontal, (short) 0);
		//					}

		//					// �޺��ڽ��� ��ü �ʺ� �� ����
		//					accumLength = 0;

		//					for (yy = 0 ; yy < sizeof (placingZone.cells [xx].tableInHor) / sizeof (int) ; ++yy)
		//						accumLength += placingZone.cells [xx].tableInHor [yy];

		//					for (yy = 1 ; yy <= DGPopUpGetItemCount (dialogID, placingZone.POPUP_WIDTH [xx]) ; ++yy) {
		//						if (accumLength == atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], yy).ToCStr ().Get ())) {
		//							DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], yy);
		//							break;
		//						}
		//					}
		//				}
		//			}
		//		}

		//		// ���̾�α� ũ�� ����
		//		dialogSizeX = 550;
		//		dialogSizeY = 950;
		//		if (placingZone.nCellsInHor >= 5) {
		//			DGSetDialogSize (dialogID, DG_CLIENT, dialogSizeX + 70 * (placingZone.nCellsInHor - 5), dialogSizeY, DG_TOPLEFT, true);
		//		}

		//		// ���� �ʺ� ���
		//		totalWidth = 0.0;
		//		if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_LINCORNER) == TRUE)	totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_LINCORNER);
		//		if (DGGetItemValLong (dialogID, placingZone.CHECKBOX_RINCORNER) == TRUE)	totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_RINCORNER);
		//		for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
		//			if ((DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == TABLEFORM) || (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == EUROFORM))
		//				totalWidth += atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH [xx])).ToCStr ().Get ()) / 1000;
		//			else if (DGPopUpGetSelected (dialogID, placingZone.POPUP_OBJ_TYPE [xx]) == NONE)
		//				totalWidth += 0.0;
		//			else
		//				totalWidth += DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);
		//		}
		//		DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_WIDTH, placingZone.horLen - totalWidth);

		//		// ���� ���� ��� (������)
		//		totalHeight = 0.0;
		//		for (xx = 0 ; xx < placingZone.nCellsInVerBasic ; ++xx) {
		//			totalHeight += atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_HEIGHT_BASIC [xx])).ToCStr ().Get ()) / 1000;
		//		}
		//		DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_HEIGHT_BASIC, placingZone.verLenBasic - totalHeight);

		//		if (placingZone.bExtra == true) {
		//			// ���� ���� ��� (������)
		//			totalHeight = 0.0;
		//			for (xx = 0 ; xx < placingZone.nCellsInVerExtra ; ++xx) {
		//				totalHeight += atof (DGPopUpGetItemText (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_HEIGHT_EXTRA [xx])).ToCStr ().Get ()) / 1000;
		//			}
		//			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_REMAIN_HEIGHT_EXTRA, placingZone.verLenExtra - totalHeight);
		//		}

		//		item = 0;
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
			//// ���̾�α� Ÿ��Ʋ
			//DGSetDialogTitle (dialogID, "������ ���̾� �����ϱ�");

			////////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			//// ���� ��ư
			//DGSetItemText (dialogID, DG_OK, "Ȯ ��");

			//// ���� ��ư
			//DGSetItemText (dialogID, DG_CANCEL, "�� ��");

			//// üũ�ڽ�: ���̾� ����
			//DGSetItemText (dialogID, CHECKBOX_LAYER_COUPLING, "���̾� ����");
			//DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, TRUE);

			//// ���̾� ���� ��
			//DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, "���纰 ���̾� ����");
			//DGSetItemText (dialogID, LABEL_LAYER_SLABTABLEFORM, "������ ���̺���");
			//DGSetItemText (dialogID, LABEL_LAYER_PROFILE, "C����");
			//DGSetItemText (dialogID, LABEL_LAYER_EUROFORM, "������");
			//DGSetItemText (dialogID, LABEL_LAYER_RECTPIPE, "��� ������");
			//DGSetItemText (dialogID, LABEL_LAYER_PINBOLT, "�ɺ�Ʈ ��Ʈ");
			//DGSetItemText (dialogID, LABEL_LAYER_WALLTIE, "��ü Ÿ��");
			//DGSetItemText (dialogID, LABEL_LAYER_JOIN, "����ö��");
			//DGSetItemText (dialogID, LABEL_LAYER_HEADPIECE, "����ǽ�");
			//DGSetItemText (dialogID, LABEL_LAYER_STEELFORM, "��ƿ��");
			//DGSetItemText (dialogID, LABEL_LAYER_PLYWOOD, "����");
			//DGSetItemText (dialogID, LABEL_LAYER_TIMBER, "����");
			//DGSetItemText (dialogID, LABEL_LAYER_FILLERSP, "�ٷ������̼�");
			//DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER_ANGLE, "�ƿ��ڳʾޱ�");
			//DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER_PANEL, "�ƿ��ڳ��ǳ�");
			//DGSetItemText (dialogID, LABEL_LAYER_INCORNER_PANEL, "���ڳ��ǳ�");
			//DGSetItemText (dialogID, LABEL_LAYER_RECTPIPE_HANGER, "�����������");
			//DGSetItemText (dialogID, LABEL_LAYER_EUROFORM_HOOK, "������ ��ũ");
			//DGSetItemText (dialogID, LABEL_LAYER_CROSS_JOINT_BAR, "��������Ʈ��");
			//DGSetItemText (dialogID, LABEL_LAYER_BLUE_CLAMP, "���Ŭ����");
			//DGSetItemText (dialogID, LABEL_LAYER_BLUE_TIMBER_RAIL, "�����");
			//DGSetItemText (dialogID, LABEL_LAYER_HIDDEN, "����");

			//DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 120, 700, 160, 25);
			//DGSetItemFont (dialogID, BUTTON_AUTOSET, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, BUTTON_AUTOSET, "���̾� �ڵ� ����");
			//DGShowItem (dialogID, BUTTON_AUTOSET);

			//// ���� ��Ʈ�� �ʱ�ȭ
			//BNZeroMemory (&ucb, sizeof (ucb));
			//ucb.dialogID = dialogID;
			//ucb.type	 = APIUserControlType_Layer;
			//ucb.itemID	 = USERCONTROL_LAYER_SLABTABLEFORM;
			//ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			//DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, 1);
			//if (bLayerInd_SlabTableform == true) {
			//	DGEnableItem (dialogID, LABEL_LAYER_SLABTABLEFORM);
			//	DGEnableItem (dialogID, USERCONTROL_LAYER_SLABTABLEFORM);
			//} else {
			//	DGDisableItem (dialogID, LABEL_LAYER_SLABTABLEFORM);
			//	DGDisableItem (dialogID, USERCONTROL_LAYER_SLABTABLEFORM);
			//}

			//ucb.itemID	 = USERCONTROL_LAYER_PROFILE;
			//ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			//DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, 1);
			//if (bLayerInd_Profile == true) {
			//	DGEnableItem (dialogID, LABEL_LAYER_PROFILE);
			//	DGEnableItem (dialogID, USERCONTROL_LAYER_PROFILE);
			//} else {
			//	DGDisableItem (dialogID, LABEL_LAYER_PROFILE);
			//	DGDisableItem (dialogID, USERCONTROL_LAYER_PROFILE);
			//}

			//ucb.itemID	 = USERCONTROL_LAYER_EUROFORM;
			//ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			//DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, 1);
			//if (bLayerInd_Euroform == true) {
			//	DGEnableItem (dialogID, LABEL_LAYER_EUROFORM);
			//	DGEnableItem (dialogID, USERCONTROL_LAYER_EUROFORM);
			//} else {
			//	DGDisableItem (dialogID, LABEL_LAYER_EUROFORM);
			//	DGDisableItem (dialogID, USERCONTROL_LAYER_EUROFORM);
			//}

			//ucb.itemID	 = USERCONTROL_LAYER_RECTPIPE;
			//ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			//DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, 1);
			//if (bLayerInd_RectPipe == true) {
			//	DGEnableItem (dialogID, LABEL_LAYER_RECTPIPE);
			//	DGEnableItem (dialogID, USERCONTROL_LAYER_RECTPIPE);
			//} else {
			//	DGDisableItem (dialogID, LABEL_LAYER_RECTPIPE);
			//	DGDisableItem (dialogID, USERCONTROL_LAYER_RECTPIPE);
			//}

			//ucb.itemID	 = USERCONTROL_LAYER_PINBOLT;
			//ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			//DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, 1);
			//if (bLayerInd_PinBolt == true) {
			//	DGEnableItem (dialogID, LABEL_LAYER_PINBOLT);
			//	DGEnableItem (dialogID, USERCONTROL_LAYER_PINBOLT);
			//} else {
			//	DGDisableItem (dialogID, LABEL_LAYER_PINBOLT);
			//	DGDisableItem (dialogID, USERCONTROL_LAYER_PINBOLT);
			//}

			//ucb.itemID	 = USERCONTROL_LAYER_WALLTIE;
			//ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			//DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, 1);
			//if (bLayerInd_WallTie == true) {
			//	DGEnableItem (dialogID, LABEL_LAYER_WALLTIE);
			//	DGEnableItem (dialogID, USERCONTROL_LAYER_WALLTIE);
			//} else {
			//	DGDisableItem (dialogID, LABEL_LAYER_WALLTIE);
			//	DGDisableItem (dialogID, USERCONTROL_LAYER_WALLTIE);
			//}

			//ucb.itemID	 = USERCONTROL_LAYER_JOIN;
			//ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			//DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, 1);
			//if (bLayerInd_Join == true) {
			//	DGEnableItem (dialogID, LABEL_LAYER_JOIN);
			//	DGEnableItem (dialogID, USERCONTROL_LAYER_JOIN);
			//} else {
			//	DGDisableItem (dialogID, LABEL_LAYER_JOIN);
			//	DGDisableItem (dialogID, USERCONTROL_LAYER_JOIN);
			//}

			//ucb.itemID	 = USERCONTROL_LAYER_HEADPIECE;
			//ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			//DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, 1);
			//if (bLayerInd_HeadPiece == true) {
			//	DGEnableItem (dialogID, LABEL_LAYER_HEADPIECE);
			//	DGEnableItem (dialogID, USERCONTROL_LAYER_HEADPIECE);
			//} else {
			//	DGDisableItem (dialogID, LABEL_LAYER_HEADPIECE);
			//	DGDisableItem (dialogID, USERCONTROL_LAYER_HEADPIECE);
			//}

			//ucb.itemID	 = USERCONTROL_LAYER_STEELFORM;
			//ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			//DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, 1);
			//if (bLayerInd_Steelform == true) {
			//	DGEnableItem (dialogID, LABEL_LAYER_STEELFORM);
			//	DGEnableItem (dialogID, USERCONTROL_LAYER_STEELFORM);
			//} else {
			//	DGDisableItem (dialogID, LABEL_LAYER_STEELFORM);
			//	DGDisableItem (dialogID, USERCONTROL_LAYER_STEELFORM);
			//}

			//ucb.itemID	 = USERCONTROL_LAYER_PLYWOOD;
			//ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			//DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, 1);
			//if (bLayerInd_Plywood == true) {
			//	DGEnableItem (dialogID, LABEL_LAYER_PLYWOOD);
			//	DGEnableItem (dialogID, USERCONTROL_LAYER_PLYWOOD);
			//} else {
			//	DGDisableItem (dialogID, LABEL_LAYER_PLYWOOD);
			//	DGDisableItem (dialogID, USERCONTROL_LAYER_PLYWOOD);
			//}

			//ucb.itemID	 = USERCONTROL_LAYER_TIMBER;
			//ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			//DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, 1);
			//if (bLayerInd_Plywood == true) {
			//	DGEnableItem (dialogID, LABEL_LAYER_TIMBER);
			//	DGEnableItem (dialogID, USERCONTROL_LAYER_TIMBER);
			//} else {
			//	DGDisableItem (dialogID, LABEL_LAYER_TIMBER);
			//	DGDisableItem (dialogID, USERCONTROL_LAYER_TIMBER);
			//}

			//ucb.itemID	 = USERCONTROL_LAYER_FILLERSP;
			//ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			//DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, 1);
			//if (bLayerInd_Fillersp == true) {
			//	DGEnableItem (dialogID, LABEL_LAYER_FILLERSP);
			//	DGEnableItem (dialogID, USERCONTROL_LAYER_FILLERSP);
			//} else {
			//	DGDisableItem (dialogID, LABEL_LAYER_FILLERSP);
			//	DGDisableItem (dialogID, USERCONTROL_LAYER_FILLERSP);
			//}

			//ucb.itemID	 = USERCONTROL_LAYER_OUTCORNER_ANGLE;
			//ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			//DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, 1);
			//if (bLayerInd_OutcornerAngle == true) {
			//	DGEnableItem (dialogID, LABEL_LAYER_OUTCORNER_ANGLE);
			//	DGEnableItem (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE);
			//} else {
			//	DGDisableItem (dialogID, LABEL_LAYER_OUTCORNER_ANGLE);
			//	DGDisableItem (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE);
			//}

			//ucb.itemID	 = USERCONTROL_LAYER_OUTCORNER_PANEL;
			//ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			//DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, 1);
			//if (bLayerInd_OutcornerPanel == true) {
			//	DGEnableItem (dialogID, LABEL_LAYER_OUTCORNER_PANEL);
			//	DGEnableItem (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL);
			//} else {
			//	DGDisableItem (dialogID, LABEL_LAYER_OUTCORNER_PANEL);
			//	DGDisableItem (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL);
			//}

			//ucb.itemID	 = USERCONTROL_LAYER_INCORNER_PANEL;
			//ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			//DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, 1);
			//if (bLayerInd_IncornerPanel == true) {
			//	DGEnableItem (dialogID, LABEL_LAYER_INCORNER_PANEL);
			//	DGEnableItem (dialogID, USERCONTROL_LAYER_INCORNER_PANEL);
			//} else {
			//	DGDisableItem (dialogID, LABEL_LAYER_INCORNER_PANEL);
			//	DGDisableItem (dialogID, USERCONTROL_LAYER_INCORNER_PANEL);
			//}

			//ucb.itemID	 = USERCONTROL_LAYER_RECTPIPE_HANGER;
			//ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			//DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, 1);
			//if (bLayerInd_RectpipeHanger == true) {
			//	DGEnableItem (dialogID, LABEL_LAYER_RECTPIPE_HANGER);
			//	DGEnableItem (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER);
			//} else {
			//	DGDisableItem (dialogID, LABEL_LAYER_RECTPIPE_HANGER);
			//	DGDisableItem (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER);
			//}

			//ucb.itemID	 = USERCONTROL_LAYER_EUROFORM_HOOK;
			//ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			//DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, 1);
			//if (bLayerInd_EuroformHook == true) {
			//	DGEnableItem (dialogID, LABEL_LAYER_EUROFORM_HOOK);
			//	DGEnableItem (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK);
			//} else {
			//	DGDisableItem (dialogID, LABEL_LAYER_EUROFORM_HOOK);
			//	DGDisableItem (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK);
			//}

			//ucb.itemID	 = USERCONTROL_LAYER_CROSS_JOINT_BAR;
			//ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			//DGSetItemValLong (dialogID, USERCONTROL_LAYER_CROSS_JOINT_BAR, 1);
			//if (bLayerInd_CrossJointBar == true) {
			//	DGEnableItem (dialogID, LABEL_LAYER_CROSS_JOINT_BAR);
			//	DGEnableItem (dialogID, USERCONTROL_LAYER_CROSS_JOINT_BAR);
			//} else {
			//	DGDisableItem (dialogID, LABEL_LAYER_CROSS_JOINT_BAR);
			//	DGDisableItem (dialogID, USERCONTROL_LAYER_CROSS_JOINT_BAR);
			//}

			//ucb.itemID	 = USERCONTROL_LAYER_BLUE_CLAMP;
			//ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			//DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_CLAMP, 1);
			//if (bLayerInd_BlueClamp == true) {
			//	DGEnableItem (dialogID, LABEL_LAYER_BLUE_CLAMP);
			//	DGEnableItem (dialogID, USERCONTROL_LAYER_BLUE_CLAMP);
			//} else {
			//	DGDisableItem (dialogID, LABEL_LAYER_BLUE_CLAMP);
			//	DGDisableItem (dialogID, USERCONTROL_LAYER_BLUE_CLAMP);
			//}

			//ucb.itemID	 = USERCONTROL_LAYER_BLUE_TIMBER_RAIL;
			//ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			//DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL, 1);
			//if (bLayerInd_BlueTimberRail == true) {
			//	DGEnableItem (dialogID, LABEL_LAYER_BLUE_TIMBER_RAIL);
			//	DGEnableItem (dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL);
			//} else {
			//	DGDisableItem (dialogID, LABEL_LAYER_BLUE_TIMBER_RAIL);
			//	DGDisableItem (dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL);
			//}

			//ucb.itemID	 = USERCONTROL_LAYER_HIDDEN;
			//ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			//DGSetItemValLong (dialogID, USERCONTROL_LAYER_HIDDEN, 1);
			//if (bLayerInd_Hidden == true) {
			//	DGEnableItem (dialogID, LABEL_LAYER_HIDDEN);
			//	DGEnableItem (dialogID, USERCONTROL_LAYER_HIDDEN);
			//} else {
			//	DGDisableItem (dialogID, LABEL_LAYER_HIDDEN);
			//	DGDisableItem (dialogID, USERCONTROL_LAYER_HIDDEN);
			//}
			break;

		case DG_MSG_CHANGE:
			// ���̾� ���� �ٲ�
			//if (DGGetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING) == 1) {
			//	long selectedLayer;

			//	selectedLayer = DGGetItemValLong (dialogID, item);

			//	for (xx = USERCONTROL_LAYER_SLABTABLEFORM ; xx <= USERCONTROL_LAYER_BLUE_TIMBER_RAIL ; ++xx)
			//		DGSetItemValLong (dialogID, xx, selectedLayer);
			//}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// ���̾� ��ȣ ����
				//	if (bLayerInd_SlabTableform == true)	layerInd_SlabTableform	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM);
				//	if (bLayerInd_Profile == true)			layerInd_Profile		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE);
				//	if (bLayerInd_Euroform == true)			layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM);
				//	if (bLayerInd_RectPipe == true)			layerInd_RectPipe		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE);
				//	if (bLayerInd_PinBolt == true)			layerInd_PinBolt		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT);
				//	if (bLayerInd_WallTie == true)			layerInd_WallTie		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE);
				//	if (bLayerInd_Join == true)				layerInd_Join			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN);
				//	if (bLayerInd_HeadPiece == true)		layerInd_HeadPiece		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE);
				//	if (bLayerInd_Steelform == true)		layerInd_Steelform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM);
				//	if (bLayerInd_Plywood == true)			layerInd_Plywood		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD);
				//	if (bLayerInd_Timber == true)			layerInd_Timber			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER);
				//	if (bLayerInd_Fillersp == true)			layerInd_Fillersp		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP);
				//	if (bLayerInd_OutcornerAngle == true)	layerInd_OutcornerAngle	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE);
				//	if (bLayerInd_OutcornerPanel == true)	layerInd_OutcornerPanel	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL);
				//	if (bLayerInd_IncornerPanel == true)	layerInd_IncornerPanel	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL);
				//	if (bLayerInd_RectpipeHanger == true)	layerInd_RectpipeHanger	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER);
				//	if (bLayerInd_EuroformHook == true)		layerInd_EuroformHook	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK);
				//	if (bLayerInd_CrossJointBar == true)	layerInd_CrossJointBar	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_CROSS_JOINT_BAR);
				//	if (bLayerInd_BlueClamp == true)		layerInd_BlueClamp		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_CLAMP);
				//	if (bLayerInd_BlueTimberRail == true)	layerInd_BlueTimberRail	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL);
				//	if (bLayerInd_Hidden == true)			layerInd_Hidden			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_HIDDEN);

				//	break;

				//case BUTTON_AUTOSET:
				//	item = 0;

				//	DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, FALSE);

				//	if (placingZone.tableformType == 1) {
				//		layerInd_Euroform		= makeTemporaryLayer (structuralObject_forTableformWall, "UFOM", NULL);
				//		layerInd_RectPipe		= makeTemporaryLayer (structuralObject_forTableformWall, "SPIP", NULL);
				//		layerInd_PinBolt		= makeTemporaryLayer (structuralObject_forTableformWall, "PINB", NULL);
				//		layerInd_Join			= makeTemporaryLayer (structuralObject_forTableformWall, "CLAM", NULL);
				//		layerInd_HeadPiece		= makeTemporaryLayer (structuralObject_forTableformWall, "HEAD", NULL);
				//		layerInd_Plywood		= makeTemporaryLayer (structuralObject_forTableformWall, "PLYW", NULL);
				//		layerInd_Timber			= makeTemporaryLayer (structuralObject_forTableformWall, "TIMB", NULL);
				//		layerInd_BlueClamp		= makeTemporaryLayer (structuralObject_forTableformWall, "UFCL", NULL);
				//		layerInd_BlueTimberRail	= makeTemporaryLayer (structuralObject_forTableformWall, "RAIL", NULL);

				//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, layerInd_Euroform);
				//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, layerInd_RectPipe);
				//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, layerInd_PinBolt);
				//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, layerInd_Join);
				//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, layerInd_HeadPiece);
				//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, layerInd_Plywood);
				//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, layerInd_Timber);
				//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_CLAMP, layerInd_BlueClamp);
				//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL, layerInd_BlueTimberRail);

				//	} else if (placingZone.tableformType == 2) {
				//		layerInd_Euroform		= makeTemporaryLayer (structuralObject_forTableformWall, "UFOM", NULL);
				//		layerInd_RectPipe		= makeTemporaryLayer (structuralObject_forTableformWall, "SPIP", NULL);
				//		layerInd_PinBolt		= makeTemporaryLayer (structuralObject_forTableformWall, "PINB", NULL);
				//		layerInd_Join			= makeTemporaryLayer (structuralObject_forTableformWall, "CLAM", NULL);
				//		layerInd_HeadPiece		= makeTemporaryLayer (structuralObject_forTableformWall, "HEAD", NULL);
				//		layerInd_Plywood		= makeTemporaryLayer (structuralObject_forTableformWall, "PLYW", NULL);
				//		layerInd_Timber			= makeTemporaryLayer (structuralObject_forTableformWall, "TIMB", NULL);
				//		layerInd_BlueClamp		= makeTemporaryLayer (structuralObject_forTableformWall, "UFCL", NULL);
				//		layerInd_BlueTimberRail	= makeTemporaryLayer (structuralObject_forTableformWall, "RAIL", NULL);

				//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, layerInd_Euroform);
				//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, layerInd_RectPipe);
				//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, layerInd_PinBolt);
				//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, layerInd_Join);
				//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, layerInd_HeadPiece);
				//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, layerInd_Plywood);
				//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, layerInd_Timber);
				//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_CLAMP, layerInd_BlueClamp);
				//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL, layerInd_BlueTimberRail);

				//	} else if (placingZone.tableformType == 3) {
				//		layerInd_Euroform		= makeTemporaryLayer (structuralObject_forTableformWall, "UFOM", NULL);
				//		layerInd_RectPipe		= makeTemporaryLayer (structuralObject_forTableformWall, "SPIP", NULL);
				//		layerInd_PinBolt		= makeTemporaryLayer (structuralObject_forTableformWall, "PINB", NULL);
				//		layerInd_Join			= makeTemporaryLayer (structuralObject_forTableformWall, "CLAM", NULL);
				//		layerInd_HeadPiece		= makeTemporaryLayer (structuralObject_forTableformWall, "HEAD", NULL);
				//		layerInd_CrossJointBar	= makeTemporaryLayer (structuralObject_forTableformWall, "CROS", NULL);
				//		layerInd_Plywood		= makeTemporaryLayer (structuralObject_forTableformWall, "PLYW", NULL);
				//		layerInd_Timber			= makeTemporaryLayer (structuralObject_forTableformWall, "TIMB", NULL);
				//		layerInd_BlueClamp		= makeTemporaryLayer (structuralObject_forTableformWall, "UFCL", NULL);
				//		layerInd_BlueTimberRail	= makeTemporaryLayer (structuralObject_forTableformWall, "RAIL", NULL);

				//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, layerInd_Euroform);
				//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, layerInd_RectPipe);
				//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, layerInd_PinBolt);
				//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, layerInd_Join);
				//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, layerInd_HeadPiece);
				//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_CROSS_JOINT_BAR, layerInd_CrossJointBar);
				//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, layerInd_Plywood);
				//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, layerInd_Timber);
				//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_CLAMP, layerInd_BlueClamp);
				//		DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL, layerInd_BlueTimberRail);
				//	}

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
			//DGSetDialogTitle (dialogID, "���̺��� (���ι���) �迭 ����");

			////////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			//// ���� ��ư
			//DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 310, 140, 70, 25);
			//DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, DG_OK, "Ȯ ��");
			//DGShowItem (dialogID, DG_OK);

			//// ���� ��ư
			//DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 390, 140, 70, 25);
			//DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, DG_CANCEL, "�� ��");
			//DGShowItem (dialogID, DG_CANCEL);

			//// ���� �ʺ� (��)
			//accumLength = 0;
			//for (xx = 0 ; xx < sizeof (placingZone.cells [clickedIndex].tableInHor) / sizeof (int) ; ++xx)
			//	accumLength += placingZone.cells [clickedIndex].tableInHor [xx];
			//sprintf (buffer, "���� �ʺ�: %d", accumLength);
			//itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 270, 20, 100, 23);
			//DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, itmIdx, buffer);
			//DGShowItem (dialogID, itmIdx);

			//// ����� �ʺ� (��)
			//sprintf (buffer, "����� �ʺ�: %d", 0);
			//placingZone.LABEL_TOTAL_WIDTH = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 400, 20, 100, 23);
			//DGSetItemFont (dialogID, placingZone.LABEL_TOTAL_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, placingZone.LABEL_TOTAL_WIDTH, buffer);
			//DGShowItem (dialogID, placingZone.LABEL_TOTAL_WIDTH);

			//itmPosX = 35;
			//itmPosY = 55;

			//for (xx = 0 ; xx < 10 ; ++xx) {
			//	// ������
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 70, 70);
			//	DGShowItem (dialogID, itmIdx);

			//	// �ؽ�Ʈ(������)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX + 10, itmPosY + 10, 50, 23);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "������");
			//	DGShowItem (dialogID, itmIdx);

			//	// �޺��ڽ�
			//	placingZone.POPUP_WIDTH_IN_TABLE [xx] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX + 5, itmPosY + 40, 60, 25);
			//	DGSetItemFont (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DG_IS_LARGE | DG_IS_PLAIN);
			//	for (yy = 0 ; yy < sizeof (placingZone.presetWidth_euroform) / sizeof (int) ; ++yy) {
			//		DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DG_POPUP_BOTTOM);
			//		_itoa (placingZone.presetWidth_euroform [yy], numbuf, 10);
			//		DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DG_POPUP_BOTTOM, numbuf);
			//	}
			//	for (yy = 1 ; yy <= DGPopUpGetItemCount (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx]) ; ++yy) {
			//		if (placingZone.cells [clickedIndex].tableInHor [xx] == atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], yy).ToCStr ().Get ())) {
			//			DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], yy);
			//			break;
			//		}
			//	}
			//	DGShowItem (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx]);

			//	itmPosX += 70;
			//}

			//// ����� �ʺ� (��) ������Ʈ
			//accumLength = 0;
			//for (xx = 0 ; xx < 10 ; ++xx) {
			//	accumLength += atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
			//}
			//sprintf (buffer, "����� �ʺ�: %d", accumLength);
			//DGSetItemText (dialogID, placingZone.LABEL_TOTAL_WIDTH, buffer);

			break;

		case DG_MSG_CHANGE:

			// ����� �ʺ� (��) ������Ʈ
			//accumLength = 0;
			//for (xx = 0 ; xx < 10 ; ++xx) {
			//	accumLength += atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
			//}
			//sprintf (buffer, "����� �ʺ�: %d", accumLength);
			//DGSetItemText (dialogID, placingZone.LABEL_TOTAL_WIDTH, buffer);

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// ������ �޺��ڽ����� ���� ������� ����ü ���� ������
					//for (xx = 0 ; xx < 10 ; ++xx) {
					//	placingZone.cells [clickedIndex].tableInHor [xx] = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
					//}

					//accumLength = 0;
					//for (xx = 0 ; xx < 10 ; ++xx) {
					//	accumLength += atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
					//}
					//placingZone.cells [clickedIndex].horLen = accumLength;
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
			//DGSetDialogTitle (dialogID, "���̺��� (���ι���) �迭 ����");

			////////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			//// ���� ��ư
			//DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 310, 140, 70, 25);
			//DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, DG_OK, "Ȯ ��");
			//DGShowItem (dialogID, DG_OK);

			//// ���� ��ư
			//DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 390, 140, 70, 25);
			//DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, DG_CANCEL, "�� ��");
			//DGShowItem (dialogID, DG_CANCEL);

			//// ���� �ʺ� (��)
			//accumLength = 0;
			//for (xx = 0 ; xx < sizeof (placingZone.cells [clickedIndex].tableInHor) / sizeof (int) ; ++xx)
			//	accumLength += placingZone.cells [clickedIndex].tableInHor [xx];
			//sprintf (buffer, "���� �ʺ�: %d", accumLength);
			//itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 270, 20, 100, 23);
			//DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, itmIdx, buffer);
			//DGShowItem (dialogID, itmIdx);

			//// ����� �ʺ� (��)
			//sprintf (buffer, "����� �ʺ�: %d", 0);
			//placingZone.LABEL_TOTAL_WIDTH = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 400, 20, 100, 23);
			//DGSetItemFont (dialogID, placingZone.LABEL_TOTAL_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, placingZone.LABEL_TOTAL_WIDTH, buffer);
			//DGShowItem (dialogID, placingZone.LABEL_TOTAL_WIDTH);

			//itmPosX = 35;
			//itmPosY = 55;

			//for (xx = 0 ; xx < 10 ; ++xx) {
			//	// ������
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 70, 70);
			//	DGShowItem (dialogID, itmIdx);

			//	// �ؽ�Ʈ(������)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX + 10, itmPosY + 10, 50, 23);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "������");
			//	DGShowItem (dialogID, itmIdx);

			//	// �޺��ڽ�
			//	placingZone.POPUP_WIDTH_IN_TABLE [xx] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX + 5, itmPosY + 40, 60, 25);
			//	DGSetItemFont (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DG_IS_LARGE | DG_IS_PLAIN);
			//	for (yy = 0 ; yy < sizeof (placingZone.presetHeight_euroform) / sizeof (int) ; ++yy) {
			//		DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DG_POPUP_BOTTOM);
			//		_itoa (placingZone.presetHeight_euroform [yy], numbuf, 10);
			//		DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DG_POPUP_BOTTOM, numbuf);
			//	}
			//	for (yy = 1 ; yy <= DGPopUpGetItemCount (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx]) ; ++yy) {
			//		if (placingZone.cells [clickedIndex].tableInHor [xx] == atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], yy).ToCStr ().Get ())) {
			//			DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], yy);
			//			break;
			//		}
			//	}
			//	DGShowItem (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx]);

			//	itmPosX += 70;
			//}

			//// ����� �ʺ� (��) ������Ʈ
			//accumLength = 0;
			//for (xx = 0 ; xx < 10 ; ++xx) {
			//	accumLength += atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
			//}
			//sprintf (buffer, "����� �ʺ�: %d", accumLength);
			//DGSetItemText (dialogID, placingZone.LABEL_TOTAL_WIDTH, buffer);

			break;

		case DG_MSG_CHANGE:

			// ����� �ʺ� (��) ������Ʈ
			//accumLength = 0;
			//for (xx = 0 ; xx < 10 ; ++xx) {
			//	accumLength += atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
			//}
			//sprintf (buffer, "����� �ʺ�: %d", accumLength);
			//DGSetItemText (dialogID, placingZone.LABEL_TOTAL_WIDTH, buffer);

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// ������ �޺��ڽ����� ���� ������� ����ü ���� ������
					//for (xx = 0 ; xx < 10 ; ++xx) {
					//	placingZone.cells [clickedIndex].tableInHor [xx] = atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
					//}

					//accumLength = 0;
					//for (xx = 0 ; xx < 10 ; ++xx) {
					//	accumLength += atoi (DGPopUpGetItemText (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx], DGPopUpGetSelected (dialogID, placingZone.POPUP_WIDTH_IN_TABLE [xx])).ToCStr ().Get ());
					//}
					//placingZone.cells [clickedIndex].horLen = accumLength;
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
