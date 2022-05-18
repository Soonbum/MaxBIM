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
	API_ElementMemo			memo;
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
	if (nWall + nBeam + nSlab + nMesh == 1) {
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
	}

	// �� ������ ������
	if (nBeam == 1) {
	}

	// ������ ������ ������
	if (nSlab == 1) {
	}

	// �޽� ������ ������
	if (nMesh == 1) {
	}

	//BNZeroMemory (&elem, sizeof (API_Element));
	//BNZeroMemory (&memo, sizeof (API_ElementMemo));
	//elem.header.guid = walls.Pop ();
	//structuralObject_forTableformWall = elem.header.guid;
	//err = ACAPI_Element_Get (&elem);						// elem.wall.poly.nCoords : ������ ���� ������ �� ����
	//err = ACAPI_Element_GetMemo (elem.header.guid, &memo);	// memo.coords : ������ ��ǥ�� ������ �� ����
	//
	//if (elem.wall.thickness != elem.wall.thickness1) {
	//	WriteReport_Alert ("���� �β��� �����ؾ� �մϴ�.");
	//	err = APIERR_GENERAL;
	//	return err;
	//}
	//infoWall.wallThk		= elem.wall.thickness;
	//infoWall.floorInd		= elem.header.floorInd;
	//infoWall.bottomOffset	= elem.wall.bottomOffset;
	//infoWall.begX			= elem.wall.begC.x;
	//infoWall.begY			= elem.wall.begC.y;
	//infoWall.endX			= elem.wall.endC.x;
	//infoWall.endY			= elem.wall.endC.y;

	//ACAPI_DisposeElemMemoHdls (&memo);

	//// (2) ���� ������ ������
	//for (xx = 0 ; xx < nMorphs ; ++xx) {
	//	BNZeroMemory (&elem, sizeof (API_Element));
	//	elem.header.guid = morphs.Pop ();
	//	err = ACAPI_Element_Get (&elem);
	//	err = ACAPI_Element_Get3DInfo (elem.header, &info3D);

	//	// ���� ������ ���� ������(������ ���� ������) �ߴ�
	//	if (abs (info3D.bounds.zMax - info3D.bounds.zMin) < EPS) {
	//		WriteReport_Alert ("������ ������ ���� �ʽ��ϴ�.");
	//		err = APIERR_GENERAL;
	//		return err;
	//	}

	//	// ������ GUID ����
	//	infoMorph [xx].guid = elem.header.guid;

	//	// ������ ���ϴ�, ���� �� ����
	//	if (abs (elem.morph.tranmat.tmx [11] - info3D.bounds.zMin) < EPS) {
	//		// ���ϴ� ��ǥ ����
	//		infoMorph [xx].leftBottomX = elem.morph.tranmat.tmx [3];
	//		infoMorph [xx].leftBottomY = elem.morph.tranmat.tmx [7];
	//		infoMorph [xx].leftBottomZ = elem.morph.tranmat.tmx [11];

	//		// ���� ��ǥ��?
	//		if (abs (infoMorph [xx].leftBottomX - info3D.bounds.xMin) < EPS)
	//			infoMorph [xx].rightTopX = info3D.bounds.xMax;
	//		else
	//			infoMorph [xx].rightTopX = info3D.bounds.xMin;
	//		if (abs (infoMorph [xx].leftBottomY - info3D.bounds.yMin) < EPS)
	//			infoMorph [xx].rightTopY = info3D.bounds.yMax;
	//		else
	//			infoMorph [xx].rightTopY = info3D.bounds.yMin;
	//		if (abs (infoMorph [xx].leftBottomZ - info3D.bounds.zMin) < EPS)
	//			infoMorph [xx].rightTopZ = info3D.bounds.zMax;
	//		else
	//			infoMorph [xx].rightTopZ = info3D.bounds.zMin;
	//	} else {
	//		// ���� ��ǥ ����
	//		infoMorph [xx].rightTopX = elem.morph.tranmat.tmx [3];
	//		infoMorph [xx].rightTopY = elem.morph.tranmat.tmx [7];
	//		infoMorph [xx].rightTopZ = elem.morph.tranmat.tmx [11];

	//		// ���ϴ� ��ǥ��?
	//		if (abs (infoMorph [xx].rightTopX - info3D.bounds.xMin) < EPS)
	//			infoMorph [xx].leftBottomX = info3D.bounds.xMax;
	//		else
	//			infoMorph [xx].leftBottomX = info3D.bounds.xMin;
	//		if (abs (infoMorph [xx].rightTopY - info3D.bounds.yMin) < EPS)
	//			infoMorph [xx].leftBottomY = info3D.bounds.yMax;
	//		else
	//			infoMorph [xx].leftBottomY = info3D.bounds.yMin;
	//		if (abs (infoMorph [xx].rightTopZ - info3D.bounds.zMin) < EPS)
	//			infoMorph [xx].leftBottomZ = info3D.bounds.zMax;
	//		else
	//			infoMorph [xx].leftBottomZ = info3D.bounds.zMin;
	//	}

	//	// ������ Z�� ȸ�� ���� (���� ��ġ ����)
	//	dx = infoMorph [xx].rightTopX - infoMorph [xx].leftBottomX;
	//	dy = infoMorph [xx].rightTopY - infoMorph [xx].leftBottomY;
	//	infoMorph [xx].ang = RadToDegree (atan2 (dy, dx));

	//	// ������ ���� ����
	//	infoMorph [xx].horLen = GetDistance (info3D.bounds.xMin, info3D.bounds.yMin, info3D.bounds.xMax, info3D.bounds.yMax);

	//	// ������ ���� ����
	//	infoMorph [xx].verLen = abs (info3D.bounds.zMax - info3D.bounds.zMin);

	//	// ���� ���� ����
	//	API_Elem_Head* headList = new API_Elem_Head [1];
	//	headList [0] = elem.header;
	//	err = ACAPI_Element_Delete (&headList, 1);
	//	delete headList;
	//}

	// ���� ����� ���� �ľ���
	// ���� ������ �ʺ�� ���̸� ����
		// ���̰� 600 �̸��̸� ���� -> ����
		// ���̰� 600 �̻��̸� ���� -> ����
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
	// 1�� UI
		// ���̺��� ����: ����, ����
		// ���̺��� Ÿ��: Ÿ��A
		// �߰� / ���� ��ư
		// ���� �ʺ�
		// ��: ���ڳ�, �ƿ��ڳʾޱ�, �ƿ��ڳ��ǳ�, ���̺���(������ ����: ���� �����̸� 3��, ���� �����̸� 6�� - �ִ� �ʺ� 3600), ������, �ٷ������̼�, ����, ����
	// 2�� UI : ���̾� �����ϱ�
		// ������, ��� ������, �ɺ�Ʈ ��Ʈ, ����ö��, ����ǽ�, ����, ����, �ٷ������̼�, �ƿ��ڳʾޱ�, �ƿ��ڳ��ǳ�, ���ڳ�

	// �������� ����
		// �Ϻ� ����� ����: 150
		// ��� ����� ���� (������ ��쿡��): ������ 300
		// ������ ��ġ/����: �� ���� 50�� ������

	return	err;
}