#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "SupportingPostPlacer.hpp"

using namespace SupportingPostPlacerDG;

InfoMorphForSupportingPost			infoMorph;				// ���� ����
PERISupportingPostPlacementInfo		placementInfoForPERI;	// PERI ���ٸ� ��ġ ����
short HPOST_CENTER [5];
short HPOST_UP [5];
short HPOST_DOWN [5];
static short	layerInd_vPost;			// ���̾� ��ȣ: ������
static short	layerInd_hPost;			// ���̾� ��ȣ: ������
static short	layerInd_SuppPost;		// ���̾� ��ȣ: ����Ʈ(���ٸ�)
static short	layerInd_Timber;		// ���̾� ��ȣ: ����(��°�/�����)
static short	layerInd_GT24Girder;	// ���̾� ��ȣ: GT24 �Ŵ�
static short	layerInd_BeamBracket;	// ���̾� ��ȣ: �� �����
static short	layerInd_Yoke;			// ���̾� ��ȣ: �� �ۿ���

static GS::Array<API_Guid>	elemList;	// �׷�ȭ�� ���� ������ ��������� GUID�� ���� ������

// ������ ������ü ������ ������� PERI ���ٸ��� ��ġ��
GSErrCode	placePERIPost (void)
{
	GSErrCode	err = NoError;
	short		result;
	long		nSel;
	short		xx, yy;

	// Selection Manager ���� ����
	API_SelectionInfo		selectionInfo;
	API_Element				tElem;
	API_Neig				**selNeigs;
	GS::Array<API_Guid>&	morphs = GS::Array<API_Guid> ();
	long					nMorphs = 0;

	// ��ü ���� ��������
	API_Element				elem;
	API_ElemInfo3D			info3D;

	// ���� 3D ������� ��������
	API_Component3D			component;
	API_Tranmat				tm;
	Int32					nVert, nEdge, nPgon;
	Int32					elemIdx, bodyIdx;
	API_Coord3D				trCoord;
	GS::Array<API_Coord3D>&	coords = GS::Array<API_Coord3D> ();
	long					nNodes;

	// �� �Է�
	API_GetPointType		pointInfo;
	API_Coord3D				point1, point2;
	double					morphHorLen, morphVerLen;
	bool					bPassed1, bPassed2;

	// �۾� �� ����
	//API_StoryInfo	storyInfo;
	//double			workLevel_morph;	// ������ �۾� �� ����


	// ������ ��� ��������
	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err == APIERR_NOPLAN) {
		ACAPI_WriteReport ("���� ������Ʈ â�� �����ϴ�.", true);
	}
	if (err == APIERR_NOSEL) {
		ACAPI_WriteReport ("�ƹ� �͵� �������� �ʾҽ��ϴ�.\n�ʼ� ����: ���� ������ü (1��)", true);
		//ACAPI_WriteReport ("�ƹ� �͵� �������� �ʾҽ��ϴ�.\n�ʼ� ����: ���ٸ� �Ϻο� ��ġ�ϴ� ������ �Ǵ� �� (1��), ���� ������ü (1��)", true);
	}
	if (err != NoError) {
		BMKillHandle ((GSHandle *) &selNeigs);
		return err;
	}

	// ���� 1�� �����ؾ� ��
	if (selectionInfo.typeID != API_SelEmpty) {
		nSel = BMGetHandleSize ((GSHandle) selNeigs) / sizeof (API_Neig);
		for (xx = 0 ; xx < nSel && err == NoError ; ++xx) {
			tElem.header.typeID = Neig_To_ElemID ((*selNeigs)[xx].neigID);

			tElem.header.guid = (*selNeigs)[xx].guid;
			if (ACAPI_Element_Get (&tElem) != NoError)	// ������ �� �ִ� ����ΰ�?
				continue;

			if (tElem.header.typeID == API_MorphID)		// �����ΰ�?
				morphs.Push (tElem.header.guid);
		}
	}
	BMKillHandle ((GSHandle *) &selNeigs);
	nMorphs = morphs.GetSize ();

	// ������ 1���ΰ�?
	if (nMorphs != 1) {
		ACAPI_WriteReport ("������ü ������ 1�� �����ϼž� �մϴ�.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// ���� ������ ������
	BNZeroMemory (&elem, sizeof (API_Element));
	elem.header.guid = morphs.Pop ();
	err = ACAPI_Element_Get (&elem);
	err = ACAPI_Element_Get3DInfo (elem.header, &info3D);

	// ���� ������ ���� ���̰� 0�̸� �ߴ�
	if (abs (info3D.bounds.zMax - info3D.bounds.zMin) < EPS) {
		ACAPI_WriteReport ("������ü ������ ���� ���̰� �����ϴ�.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// ������ GUID ����
	infoMorph.guid = elem.header.guid;

	// ������ �� �ε��� ����
	infoMorph.floorInd = elem.header.floorInd;

	// ������ 3D �ٵ� ������
	BNZeroMemory (&component, sizeof (API_Component3D));
	component.header.typeID = API_BodyID;
	component.header.index = info3D.fbody;
	err = ACAPI_3D_GetComponent (&component);

	nVert = component.body.nVert;
	nEdge = component.body.nEdge;
	nPgon = component.body.nPgon;
	tm = component.body.tranmat;
	elemIdx = component.body.head.elemIndex - 1;
	bodyIdx = component.body.head.bodyIndex - 1;
	
	yy = 0;

	// ������ 8�� ������ ���ϱ�
	for (xx = 1 ; xx <= nVert ; ++xx) {
		component.header.typeID	= API_VertID;
		component.header.index	= xx;
		err = ACAPI_3D_GetComponent (&component);
		if (err == NoError) {
			trCoord.x = tm.tmx[0]*component.vert.x + tm.tmx[1]*component.vert.y + tm.tmx[2]*component.vert.z + tm.tmx[3];
			trCoord.y = tm.tmx[4]*component.vert.x + tm.tmx[5]*component.vert.y + tm.tmx[6]*component.vert.z + tm.tmx[7];
			trCoord.z = tm.tmx[8]*component.vert.x + tm.tmx[9]*component.vert.y + tm.tmx[10]*component.vert.z + tm.tmx[11];
			// ���⼭ ���� ������ ����Ʈ�� �߰����� �ʴ´�.
			if ((abs (trCoord.x) < EPS) && (abs (trCoord.y) < EPS) && (abs (trCoord.z - 1.0) < EPS))		// 1�� (0, 0, 1)
				; // pass
			else if ((abs (trCoord.x) < EPS) && (abs (trCoord.y - 1.0) < EPS) && (abs (trCoord.z) < EPS))	// 2�� (0, 1, 0)
				; // pass
			else if ((abs (trCoord.x - 1.0) < EPS) && (abs (trCoord.y) < EPS) && (abs (trCoord.z) < EPS))	// 3�� (1, 0, 0)
				; // pass
			else if ((abs (trCoord.x) < EPS) && (abs (trCoord.y) < EPS) && (abs (trCoord.z) < EPS))			// 4�� (0, 0, 0)
				; // pass
			else {
				//placeCoordinateLabel (trCoord.x, trCoord.y, trCoord.z);
				coords.Push (trCoord);
				if (yy < 8) infoMorph.points [yy++] = trCoord;
			}
		}
	}
	nNodes = coords.GetSize ();

	// ������ ���� ���� ���ϱ�
	infoMorph.leftBottomX = infoMorph.points [0].x;
	infoMorph.leftBottomY = infoMorph.points [0].y;
	infoMorph.leftBottomZ = infoMorph.points [0].z;
	infoMorph.rightTopX = infoMorph.points [0].x;
	infoMorph.rightTopY = infoMorph.points [0].y;
	infoMorph.rightTopZ = infoMorph.points [0].z;

	for (xx = 0 ; xx < 8 ; ++xx) {
		if (infoMorph.leftBottomX > infoMorph.points [xx].x)	infoMorph.leftBottomX = infoMorph.points [xx].x;
		if (infoMorph.leftBottomY > infoMorph.points [xx].y)	infoMorph.leftBottomY = infoMorph.points [xx].y;
		if (infoMorph.leftBottomZ > infoMorph.points [xx].z)	infoMorph.leftBottomZ = infoMorph.points [xx].z;

		if (infoMorph.rightTopX < infoMorph.points [xx].x)		infoMorph.rightTopX = infoMorph.points [xx].x;
		if (infoMorph.rightTopY < infoMorph.points [xx].y)		infoMorph.rightTopY = infoMorph.points [xx].y;
		if (infoMorph.rightTopZ < infoMorph.points [xx].z)		infoMorph.rightTopZ = infoMorph.points [xx].z;
	}
	infoMorph.ang = 0.0;
	infoMorph.width = GetDistance (infoMorph.leftBottomX, infoMorph.leftBottomY, infoMorph.rightTopX, infoMorph.leftBottomY);
	infoMorph.depth = GetDistance (infoMorph.leftBottomX, infoMorph.leftBottomY, infoMorph.leftBottomX, infoMorph.rightTopY);
	infoMorph.height = abs (info3D.bounds.zMax - info3D.bounds.zMin);

	// ������ ����, ���� ���̸� ����
	morphHorLen = infoMorph.width;
	morphVerLen = infoMorph.depth;

	// ����ڷκ��� ������ü ������ �� ���� �Է� ���� (���� ����ϴ� ����� ��ġ�ϴ� �� ���� Ŭ���Ͻʽÿ�)
	BNZeroMemory (&pointInfo, sizeof (API_GetPointType));
	CHCopyC ("���� ����ϴ� ����� ��ġ�ϴ� �� �� �� ���� ���� Ŭ���Ͻʽÿ�.", pointInfo.prompt);
	pointInfo.enableQuickSelection = true;
	err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
	point1 = pointInfo.pos;

	BNZeroMemory (&pointInfo, sizeof (API_GetPointType));
	CHCopyC ("���� ����ϴ� ����� ��ġ�ϴ� �� �� �� ������ ���� Ŭ���Ͻʽÿ�.", pointInfo.prompt);
	pointInfo.enableQuickSelection = true;
	err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
	point2 = pointInfo.pos;

	// ���� point1, point2�� ������ �������� �ƴϸ� ���� �޽��� ��� �� ����
	bPassed1 = false;
	for (xx = 0 ; xx < 8 ; ++xx) {
		if (GetDistance (point1, infoMorph.points [xx]) < EPS)
			bPassed1 = true;
	}

	bPassed2 = false;
	for (xx = 0 ; xx < 8 ; ++xx) {
		if (GetDistance (point2, infoMorph.points [xx]) < EPS)
			bPassed2 = true;
	}

	if ( !(bPassed1 && bPassed2) ) {
		ACAPI_WriteReport ("������ �������� ������ ���� ���� Ŭ���߽��ϴ�.", true);
		return err;
	}

	// ���� point1�� point2 ���� �Ÿ��� ���� �Ǵ� ���� ���̰� �ƴϸ� ���� �޽��� ��� �� ����
	if ( !((abs (GetDistance (point1, point2) - morphHorLen) < EPS) || (abs (GetDistance (point1, point2) - morphVerLen) < EPS)) ) {
		ACAPI_WriteReport ("�� �� ���� �Ÿ��� ������ ���� �Ǵ� ���� ���̿� ��ġ���� �ʽ��ϴ�.", true);
		return err;
	}

	// ���� point1�� point2 ���� �Ÿ��� ���� ������ ���,
	if (abs (GetDistance (point1, point2) - morphHorLen) < EPS) {
		placementInfoForPERI.width = morphHorLen;
		placementInfoForPERI.depth = morphVerLen;
		placementInfoForPERI.bFlipped = false;

	// ���� point1�� point2 ���� �Ÿ��� ���� ������ ���,
	} else if (abs (GetDistance (point1, point2) - morphVerLen) < EPS) {
		placementInfoForPERI.width = morphVerLen;
		placementInfoForPERI.depth = morphHorLen;
		placementInfoForPERI.bFlipped = true;
	}
	
	// �ʺ� ������ ������ ���� �ʱ� 1��
	placementInfoForPERI.nColVPost = 1;

	// [���̾�α�] ���ٸ� ��ġ �ɼ��� ������
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32521, ACAPI_GetOwnResModule (), PERISupportingPostPlacerHandler1, 0);

	// ���� ���� ����
	//API_Elem_Head* headList = new API_Elem_Head [1];
	//headList [0] = elem.header;
	//err = ACAPI_Element_Delete (&headList, 1);
	//delete headList;

	// �Է��� �����͸� ������� ������, ������ ��ġ
	//if (result == DG_OK) {
	//	PERI_VPost vpost;
	//	PERI_HPost hpost;

	//	// �����簡 ���� ���, �������� �԰ݿ� �°� ������
	//	if (placementInfoForPERI.bHPost == true) {
	//		// ����/������ ������ �԰ݿ� ���� ���� ������ ���� ���̰� �޶���
	//		if (my_strcmp (placementInfoForPERI.nomHPost_South, "296 cm") == 0)			infoMorph.width = 2.960;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_South, "266 cm") == 0)	infoMorph.width = 2.660;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_South, "237 cm") == 0)	infoMorph.width = 2.370;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_South, "230 cm") == 0)	infoMorph.width = 2.300;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_South, "225 cm") == 0)	infoMorph.width = 2.250;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_South, "201.5 cm") == 0)	infoMorph.width = 2.015;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_South, "150 cm") == 0)	infoMorph.width = 1.500;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_South, "137.5 cm") == 0)	infoMorph.width = 1.375;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_South, "120 cm") == 0)	infoMorph.width = 1.200;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_South, "90 cm") == 0)		infoMorph.width = 0.900;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_South, "75 cm") == 0)		infoMorph.width = 0.750;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_South, "62.5 cm") == 0)	infoMorph.width = 0.625;

	//		// ����/������ ������ �԰ݿ� ���� ���� ������ ���� ���̰� �޶���
	//		if (my_strcmp (placementInfoForPERI.nomHPost_West, "296 cm") == 0)			infoMorph.depth = 2.960;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_West, "266 cm") == 0)		infoMorph.depth = 2.660;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_West, "237 cm") == 0)		infoMorph.depth = 2.370;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_West, "230 cm") == 0)		infoMorph.depth = 2.300;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_West, "225 cm") == 0)		infoMorph.depth = 2.250;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_West, "201.5 cm") == 0)	infoMorph.depth = 2.015;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_West, "150 cm") == 0)		infoMorph.depth = 1.500;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_West, "137.5 cm") == 0)	infoMorph.depth = 1.375;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_West, "120 cm") == 0)		infoMorph.depth = 1.200;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_West, "90 cm") == 0)		infoMorph.depth = 0.900;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_West, "75 cm") == 0)		infoMorph.depth = 0.750;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_West, "62.5 cm") == 0)	infoMorph.depth = 0.625;
	//	}

	//	// ������ 1�� ��ġ
	//	// ȸ��Y(180), ũ�ν���� ��ġ(�ϴ�), ���� len_current��ŭ �̵��ؾ� ��
	//	vpost.leftBottomX = infoMorph.leftBottomX;
	//	vpost.leftBottomY = infoMorph.leftBottomY;
	//	vpost.leftBottomZ = infoMorph.leftBottomZ + placementInfoForPERI.heightVPost1;
	//	vpost.ang = infoMorph.ang;
	//	if (placementInfoForPERI.bVPost2 == false)
	//		vpost.bCrosshead = placementInfoForPERI.bCrosshead;
	//	else
	//		vpost.bCrosshead = false;
	//	sprintf (vpost.stType, "%s", placementInfoForPERI.nomVPost1);
	//	sprintf (vpost.crossheadType, "PERI");
	//	sprintf (vpost.posCrosshead, "�ϴ�");
	//	vpost.angCrosshead = 0.0;
	//	vpost.angY = DegreeToRad (180.0);
	//	vpost.len_current = placementInfoForPERI.heightVPost1;
	//	vpost.text2_onoff = true;
	//	vpost.text_onoff = true;
	//	vpost.bShowCoords = true;

	//	elemList.Push (placementInfoForPERI.placeVPost (vpost));	// ���ϴ�
	//	moveIn3D ('x', vpost.ang, infoMorph.width, &vpost.leftBottomX, &vpost.leftBottomY, &vpost.leftBottomZ);
	//	elemList.Push (placementInfoForPERI.placeVPost (vpost));	// ���ϴ�
	//	moveIn3D ('y', vpost.ang, infoMorph.depth, &vpost.leftBottomX, &vpost.leftBottomY, &vpost.leftBottomZ);
	//	elemList.Push (placementInfoForPERI.placeVPost (vpost));	// ����
	//	moveIn3D ('x', vpost.ang, -infoMorph.width, &vpost.leftBottomX, &vpost.leftBottomY, &vpost.leftBottomZ);
	//	elemList.Push (placementInfoForPERI.placeVPost (vpost));	// �»��

	//	// ������ 2���� ���� ���
	//	if (placementInfoForPERI.bVPost2 == true) {
	//		// ȸ��Y(0), ũ�ν���� ��ġ(���)
	//		vpost.leftBottomX = infoMorph.leftBottomX;
	//		vpost.leftBottomY = infoMorph.leftBottomY;
	//		vpost.leftBottomZ = infoMorph.leftBottomZ + placementInfoForPERI.heightVPost1;
	//		vpost.ang = infoMorph.ang;
	//		vpost.bCrosshead = placementInfoForPERI.bCrosshead;
	//		sprintf (vpost.stType, "%s", placementInfoForPERI.nomVPost2);
	//		sprintf (vpost.crossheadType, "PERI");
	//		sprintf (vpost.posCrosshead, "���");
	//		vpost.angCrosshead = 0.0;
	//		vpost.angY = 0.0;
	//		vpost.len_current = placementInfoForPERI.heightVPost2;
	//		vpost.text2_onoff = true;
	//		vpost.text_onoff = true;
	//		vpost.bShowCoords = true;

	//		elemList.Push (placementInfoForPERI.placeVPost (vpost));	// ���ϴ�
	//		moveIn3D ('x', vpost.ang, infoMorph.width, &vpost.leftBottomX, &vpost.leftBottomY, &vpost.leftBottomZ);
	//		elemList.Push (placementInfoForPERI.placeVPost (vpost));	// ���ϴ�
	//		moveIn3D ('y', vpost.ang, infoMorph.depth, &vpost.leftBottomX, &vpost.leftBottomY, &vpost.leftBottomZ);
	//		elemList.Push (placementInfoForPERI.placeVPost (vpost));	// ����
	//		moveIn3D ('x', vpost.ang, -infoMorph.width, &vpost.leftBottomX, &vpost.leftBottomY, &vpost.leftBottomZ);
	//		elemList.Push (placementInfoForPERI.placeVPost (vpost));	// �»��
	//	}

	//	// �����簡 ���� ���
	//	if (placementInfoForPERI.bHPost == true) {
	//		hpost.leftBottomX = infoMorph.leftBottomX;
	//		hpost.leftBottomY = infoMorph.leftBottomY;
	//		hpost.leftBottomZ = infoMorph.leftBottomZ;
	//		hpost.ang = infoMorph.ang;
	//		hpost.angX = 0.0;
	//		hpost.angY = 0.0;
	//		
	//		// 1�� ----------------------------------------------------------------------------------------------------
	//		// ����
	//		moveIn3D ('z', hpost.ang, 2.000, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
	//		moveIn3D ('x', hpost.ang, 0.050, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
	//		sprintf (hpost.stType, "%s", placementInfoForPERI.nomHPost_South);	// �� ���ڿ��̸� ��ġ���� ����
	//		if (my_strcmp (placementInfoForPERI.nomHPost_South, "") != 0)	elemList.Push (placementInfoForPERI.placeHPost (hpost));

	//		// ����
	//		moveIn3D ('x', hpost.ang, infoMorph.width - 0.050, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
	//		moveIn3D ('y', hpost.ang, 0.050, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
	//		hpost.ang = infoMorph.ang + DegreeToRad (90.0);
	//		sprintf (hpost.stType, "%s", placementInfoForPERI.nomHPost_East);	// �� ���ڿ��̸� ��ġ���� ����
	//		if (my_strcmp (placementInfoForPERI.nomHPost_East, "") != 0)	elemList.Push (placementInfoForPERI.placeHPost (hpost));
	//		hpost.ang = infoMorph.ang;

	//		// ����
	//		moveIn3D ('x', hpost.ang, -0.050, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
	//		moveIn3D ('y', hpost.ang, infoMorph.depth - 0.050, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
	//		hpost.ang = infoMorph.ang + DegreeToRad (180.0);
	//		sprintf (hpost.stType, "%s", placementInfoForPERI.nomHPost_North);	// �� ���ڿ��̸� ��ġ���� ����
	//		if (my_strcmp (placementInfoForPERI.nomHPost_North, "") != 0)	elemList.Push (placementInfoForPERI.placeHPost (hpost));
	//		hpost.ang = infoMorph.ang;

	//		// ����
	//		moveIn3D ('x', hpost.ang, 0.050 - infoMorph.width, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
	//		moveIn3D ('y', hpost.ang, -0.050, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
	//		hpost.ang = infoMorph.ang + DegreeToRad (270.0);
	//		sprintf (hpost.stType, "%s", placementInfoForPERI.nomHPost_West);	// �� ���ڿ��̸� ��ġ���� ����
	//		if (my_strcmp (placementInfoForPERI.nomHPost_West, "") != 0)	elemList.Push (placementInfoForPERI.placeHPost (hpost));
	//		hpost.ang = infoMorph.ang;

	//		// 2�� ----------------------------------------------------------------------------------------------------
	//		hpost.leftBottomX = infoMorph.leftBottomX;
	//		hpost.leftBottomY = infoMorph.leftBottomY;
	//		hpost.leftBottomZ = infoMorph.leftBottomZ;
	//		hpost.ang = infoMorph.ang;

	//		// ������ 1��/2�� ������ ���� 4000 �̻��� ���
	//		if (((placementInfoForPERI.bVPost1 * placementInfoForPERI.heightVPost1) + (placementInfoForPERI.bVPost2 * placementInfoForPERI.heightVPost2)) > 4.000) {
	//			// ����
	//			moveIn3D ('z', hpost.ang, 4.000, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
	//			moveIn3D ('x', hpost.ang, 0.050, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
	//			sprintf (hpost.stType, "%s", placementInfoForPERI.nomHPost_South);	// �� ���ڿ��̸� ��ġ���� ����
	//			if (my_strcmp (placementInfoForPERI.nomHPost_South, "") != 0)	elemList.Push (placementInfoForPERI.placeHPost (hpost));

	//			// ����
	//			moveIn3D ('x', hpost.ang, infoMorph.width - 0.050, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
	//			moveIn3D ('y', hpost.ang, 0.050, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
	//			hpost.ang = infoMorph.ang + DegreeToRad (90.0);
	//			sprintf (hpost.stType, "%s", placementInfoForPERI.nomHPost_East);	// �� ���ڿ��̸� ��ġ���� ����
	//			if (my_strcmp (placementInfoForPERI.nomHPost_East, "") != 0)	elemList.Push (placementInfoForPERI.placeHPost (hpost));
	//			hpost.ang = infoMorph.ang;

	//			// ����
	//			moveIn3D ('x', hpost.ang, -0.050, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
	//			moveIn3D ('y', hpost.ang, infoMorph.depth - 0.050, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
	//			hpost.ang = infoMorph.ang + DegreeToRad (180.0);
	//			sprintf (hpost.stType, "%s", placementInfoForPERI.nomHPost_North);	// �� ���ڿ��̸� ��ġ���� ����
	//			if (my_strcmp (placementInfoForPERI.nomHPost_North, "") != 0)	elemList.Push (placementInfoForPERI.placeHPost (hpost));
	//			hpost.ang = infoMorph.ang;

	//			// ����
	//			moveIn3D ('x', hpost.ang, 0.050 - infoMorph.width, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
	//			moveIn3D ('y', hpost.ang, -0.050, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
	//			hpost.ang = infoMorph.ang + DegreeToRad (270.0);
	//			sprintf (hpost.stType, "%s", placementInfoForPERI.nomHPost_West);	// �� ���ڿ��̸� ��ġ���� ����
	//			if (my_strcmp (placementInfoForPERI.nomHPost_West, "") != 0)	elemList.Push (placementInfoForPERI.placeHPost (hpost));
	//			hpost.ang = infoMorph.ang;
	//		}
	//	}

	//	// �׷�ȭ �ϱ�
	//	if (!elemList.IsEmpty ()) {
	//		GSSize nElems = elemList.GetSize ();
	//		API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
	//		if (elemHead != NULL) {
	//			for (GSIndex i = 0; i < nElems; i++)
	//				(*elemHead)[i].guid = elemList[i];

	//			ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

	//			BMKillHandle ((GSHandle *) &elemHead);
	//		}
	//	}
	//	elemList.Clear (false);
	//}

	// ȭ�� ���ΰ�ħ
	ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	bool	regenerate = true;
	ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

	return	err;
}

// ������ ��ġ
API_Guid	PERISupportingPostPlacementInfo::placeVPost (PERI_VPost params)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = L("PERI���ٸ� ������ v0.1.gsm");
	double				aParam;
	double				bParam;
	Int32				addParNum;

	// ��ü �ε�
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return elem.header.guid;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	elem.header.typeID = API_ObjectID;
	elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&elem, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	// ���̺귯���� �Ķ���� �� �Է�
	elem.object.libInd = libPart.index;
	elem.object.pos.x = params.leftBottomX;
	elem.object.pos.y = params.leftBottomY;
	elem.object.level = params.leftBottomZ;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.object.angle = params.ang;
	elem.header.floorInd = infoMorph.floorInd;

	// ���̾�
	elem.header.layer = layerInd_vPost;

	setParameterByName (&memo, "stType", params.stType);				// �԰�
	setParameterByName (&memo, "bCrosshead", params.bCrosshead);		// ũ�ν���� On/Off
	setParameterByName (&memo, "posCrosshead", params.posCrosshead);	// ũ�ν���� ��ġ (���, �ϴ�)
	setParameterByName (&memo, "crossheadType", params.crossheadType);	// ũ�ν���� Ÿ�� (PERI, ��� ����ǰ)
	setParameterByName (&memo, "angCrosshead", params.angCrosshead);	// ũ�ν���� ȸ������
	setParameterByName (&memo, "len_current", params.len_current);		// ���� ����
	setParameterByName (&memo, "angY", params.angY);					// ȸ�� Y

	setParameterByName (&memo, "text2_onoff", params.text2_onoff);		// 2D �ؽ�Ʈ On/Off
	setParameterByName (&memo, "text_onoff", params.text_onoff);		// 3D �ؽ�Ʈ On/Off
	setParameterByName (&memo, "bShowCoords", params.bShowCoords);		// ��ǥ�� ǥ�� On/Off

	// �԰ݿ� ���� �ּ� ����, �ִ� ���̸� �ڵ����� ����
	if (my_strcmp (params.stType, "MP 120") == 0) {
		setParameterByName (&memo, "pos_lever", 0.600);
		setParameterByName (&memo, "len_min", 0.800);
		setParameterByName (&memo, "len_max", 1.200);
	} else if (my_strcmp (params.stType, "MP 250") == 0) {
		setParameterByName (&memo, "pos_lever", 1.250);
		setParameterByName (&memo, "len_min", 1.450);
		setParameterByName (&memo, "len_max", 2.500);
	} else if (my_strcmp (params.stType, "MP 350") == 0) {
		setParameterByName (&memo, "pos_lever", 1.750);
		setParameterByName (&memo, "len_min", 1.950);
		setParameterByName (&memo, "len_max", 3.500);
	} else if (my_strcmp (params.stType, "MP 480") == 0) {
		setParameterByName (&memo, "pos_lever", 2.400);
		setParameterByName (&memo, "len_min", 2.600);
		setParameterByName (&memo, "len_max", 4.800);
	} else if (my_strcmp (params.stType, "MP 625") == 0) {
		setParameterByName (&memo, "pos_lever", 4.100);
		setParameterByName (&memo, "len_min", 4.300);
		setParameterByName (&memo, "len_max", 6.250);
	}

	// ��ü ��ġ
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	// �Ķ���� ��ũ��Ʈ�� ������ �����Ŵ
	ACAPI_Goodies (APIAny_RunGDLParScriptID, &elem.header, 0);

	return	elem.header.guid;
}

// ������ ��ġ
API_Guid	PERISupportingPostPlacementInfo::placeHPost (PERI_HPost params)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = L("PERI���ٸ� ������ v0.2.gsm");
	double				aParam;
	double				bParam;
	Int32				addParNum;

	// ��ü �ε�
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return elem.header.guid;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	elem.header.typeID = API_ObjectID;
	elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&elem, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	if (my_strcmp (params.stType, "296 cm") == 0)			{ setParameterByName (&memo, "A", 2.960);	aParam = 2.960;	}
	else if (my_strcmp (params.stType, "266 cm") == 0)		{ setParameterByName (&memo, "A", 2.660);	aParam = 2.660;	}
	else if (my_strcmp (params.stType, "237 cm") == 0)		{ setParameterByName (&memo, "A", 2.370);	aParam = 2.370;	}
	else if (my_strcmp (params.stType, "230 cm") == 0)		{ setParameterByName (&memo, "A", 2.300);	aParam = 2.300;	}
	else if (my_strcmp (params.stType, "225 cm") == 0)		{ setParameterByName (&memo, "A", 2.250);	aParam = 2.250;	}
	else if (my_strcmp (params.stType, "201.5 cm") == 0)	{ setParameterByName (&memo, "A", 2.015);	aParam = 2.015;	}
	else if (my_strcmp (params.stType, "150 cm") == 0)		{ setParameterByName (&memo, "A", 1.500);	aParam = 1.500;	}
	else if (my_strcmp (params.stType, "137.5 cm") == 0)	{ setParameterByName (&memo, "A", 1.375);	aParam = 1.375;	}
	else if (my_strcmp (params.stType, "120 cm") == 0)		{ setParameterByName (&memo, "A", 1.200);	aParam = 1.200;	}
	else if (my_strcmp (params.stType, "90 cm") == 0)		{ setParameterByName (&memo, "A", 0.900);	aParam = 0.900;	}
	else if (my_strcmp (params.stType, "75 cm") == 0)		{ setParameterByName (&memo, "A", 0.750);	aParam = 0.750;	}
	else if (my_strcmp (params.stType, "62.5 cm") == 0)		{ setParameterByName (&memo, "A", 0.625);	aParam = 0.625;	}
	else if (my_strcmp (params.stType, "Custom") == 0)		{ setParameterByName (&memo, "A", params.customLength);	aParam = params.customLength;	}
	
	setParameterByName (&memo, "lenFrame", aParam - 0.100);

	// ���̺귯���� �Ķ���� �� �Է�
	elem.object.libInd = libPart.index;
	elem.object.pos.x = params.leftBottomX;
	elem.object.pos.y = params.leftBottomY;
	elem.object.level = params.leftBottomZ;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.object.angle = params.ang;
	elem.header.floorInd = infoMorph.floorInd;

	// ���̾�
	elem.header.layer = layerInd_hPost;

	setParameterByName (&memo, "stType", params.stType);	// �԰�
	setParameterByName (&memo, "angX", params.angX);		// ȸ�� X
	setParameterByName (&memo, "angY", params.angY);		// ȸ�� Y

	// ��ü ��ġ
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	// �Ķ���� ��ũ��Ʈ�� ������ �����Ŵ
	ACAPI_Goodies (APIAny_RunGDLParScriptID, &elem.header, 0);

	return	elem.header.guid;
}

// ����Ʈ(���ٸ�) ��ġ
API_Guid	PERISupportingPostPlacementInfo::placeSupport (SuppPost params)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = L("����Ʈv1.0.gsm");
	double				aParam;
	double				bParam;
	Int32				addParNum;

	// ��ü �ε�
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return elem.header.guid;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	elem.header.typeID = API_ObjectID;
	elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&elem, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	// ���̺귯���� �Ķ���� �� �Է�
	elem.object.libInd = libPart.index;
	elem.object.pos.x = params.leftBottomX;
	elem.object.pos.y = params.leftBottomY;
	elem.object.level = params.leftBottomZ;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.object.angle = params.ang;
	elem.header.floorInd = infoMorph.floorInd;

	// ���̾�
	elem.header.layer = layerInd_SuppPost;

	setParameterByName (&memo, "s_stan", params.s_stan);	// �԰�
	setParameterByName (&memo, "s_leng", params.s_leng);	// ����
	setParameterByName (&memo, "s_ang", params.s_ang);		// ����

	// ��ü ��ġ
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	// �Ķ���� ��ũ��Ʈ�� ������ �����Ŵ
	ACAPI_Goodies (APIAny_RunGDLParScriptID, &elem.header, 0);

	return	elem.header.guid;
}

// ����(��°�/�����) ��ġ
API_Guid	PERISupportingPostPlacementInfo::placeTimber (Wood params)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = L("����v1.0.gsm");
	double				aParam;
	double				bParam;
	Int32				addParNum;

	// ��ü �ε�
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return elem.header.guid;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	elem.header.typeID = API_ObjectID;
	elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&elem, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	// ���̺귯���� �Ķ���� �� �Է�
	elem.object.libInd = libPart.index;
	elem.object.pos.x = params.leftBottomX;
	elem.object.pos.y = params.leftBottomY;
	elem.object.level = params.leftBottomZ;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.object.angle = params.ang;
	elem.header.floorInd = infoMorph.floorInd;

	// ���̾�
	elem.header.layer = layerInd_Timber;

	setParameterByName (&memo, "w_ins", "�ٴڴ�����");		// ��ġ����
	setParameterByName (&memo, "w_w", params.w_w);			// �β� (��°� 80, ����� 240)
	setParameterByName (&memo, "w_h", params.w_h);			// �ʺ� (��°�/����� 80)
	setParameterByName (&memo, "w_leng", params.w_leng);	// ����
	setParameterByName (&memo, "w_ang", params.w_ang);		// ����

	// ��ü ��ġ
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	// �Ķ���� ��ũ��Ʈ�� ������ �����Ŵ
	ACAPI_Goodies (APIAny_RunGDLParScriptID, &elem.header, 0);

	return	elem.header.guid;
}

// GT24 �Ŵ� ��ġ
API_Guid	PERISupportingPostPlacementInfo::placeGT24Girder (GT24Girder params)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = L("GT24 �Ŵ� v1.0.gsm");
	double				aParam;
	double				bParam;
	Int32				addParNum;

	// ��ü �ε�
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return elem.header.guid;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	elem.header.typeID = API_ObjectID;
	elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&elem, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	// ���̺귯���� �Ķ���� �� �Է�
	elem.object.libInd = libPart.index;
	elem.object.pos.x = params.leftBottomX;
	elem.object.pos.y = params.leftBottomY;
	elem.object.level = params.leftBottomZ;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.object.angle = params.ang;
	elem.header.floorInd = infoMorph.floorInd;

	// ���̾�
	elem.header.layer = layerInd_GT24Girder;

	setParameterByName (&memo, "type", params.type);	// �԰�
	setParameterByName (&memo, "angX", params.angX);	// ȸ��X
	setParameterByName (&memo, "angY", params.angY);	// ȸ��Y

	// ��ü ��ġ
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	// �Ķ���� ��ũ��Ʈ�� ������ �����Ŵ
	ACAPI_Goodies (APIAny_RunGDLParScriptID, &elem.header, 0);

	return	elem.header.guid;
}

// ��� �� ����� ��ġ
API_Guid	PERISupportingPostPlacementInfo::placeBeamBracket (BlueBeamBracket params)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = L("��� �� ����� v1.0.gsm");
	double				aParam;
	double				bParam;
	Int32				addParNum;

	// ��ü �ε�
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return elem.header.guid;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	elem.header.typeID = API_ObjectID;
	elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&elem, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	// ���̺귯���� �Ķ���� �� �Է�
	elem.object.libInd = libPart.index;
	elem.object.pos.x = params.leftBottomX;
	elem.object.pos.y = params.leftBottomY;
	elem.object.level = params.leftBottomZ;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.object.angle = params.ang;
	elem.header.floorInd = infoMorph.floorInd;

	// ���̾�
	elem.header.layer = layerInd_BeamBracket;

	setParameterByName (&memo, "type", params.type);						// Ÿ��
	setParameterByName (&memo, "verticalHeight", params.verticalHeight);	// ����

	// ��ü ��ġ
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	// �Ķ���� ��ũ��Ʈ�� ������ �����Ŵ
	ACAPI_Goodies (APIAny_RunGDLParScriptID, &elem.header, 0);

	return	elem.header.guid;
}

// �� �ۿ��� ��ġ
API_Guid	PERISupportingPostPlacementInfo::placeYoke (Yoke params)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = L("�� �ۿ���v1.0.gsm");
	double				aParam;
	double				bParam;
	Int32				addParNum;

	// ��ü �ε�
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return elem.header.guid;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	elem.header.typeID = API_ObjectID;
	elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&elem, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	// ���̺귯���� �Ķ���� �� �Է�
	elem.object.libInd = libPart.index;
	elem.object.pos.x = params.leftBottomX;
	elem.object.pos.y = params.leftBottomY;
	elem.object.level = params.leftBottomZ;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.object.angle = params.ang;
	elem.header.floorInd = infoMorph.floorInd;

	// ���̾�
	elem.header.layer = layerInd_Yoke;

	setParameterByName (&memo, "beamLength", params.beamLength);				// �� ����
	setParameterByName (&memo, "verticalGap", params.verticalGap);				// ������ ����
	setParameterByName (&memo, "innerVerticalLen", params.innerVerticalLen);	// ���� ������ ����
	setParameterByName (&memo, "bScale", params.bScale);						// ���� �� ���� ǥ��
	setParameterByName (&memo, "bLrod", params.bLrod);							// ���� ȯ��
	setParameterByName (&memo, "bLstrut", params.bLstrut);						// ���� ��ħ��
	setParameterByName (&memo, "bRrod", params.bRrod);							// ���� ȯ��
	setParameterByName (&memo, "bRstrut", params.bRstrut);						// ���� ��ħ��
	setParameterByName (&memo, "LsupVoffset", params.LsupVoffset);				// ���� ����Ʈ ������
	setParameterByName (&memo, "LsupVdist", params.LsupVdist);					// ���� ����Ʈ �����Ÿ�
	setParameterByName (&memo, "LnutVdist", params.LnutVdist);					// ���� ��Ʈ �����Ÿ�
	setParameterByName (&memo, "RsupVoffset", params.RsupVoffset);				// ���� ����Ʈ ������
	setParameterByName (&memo, "RsupVdist", params.RsupVdist);					// ���� ����Ʈ �����Ÿ�
	setParameterByName (&memo, "RnutVdist", params.RnutVdist);					// ���� ��Ʈ �����Ÿ�
	setParameterByName (&memo, "LbarHdist", params.LbarHdist);					// ���� �������� ����Ÿ�
	setParameterByName (&memo, "LbarVdist", params.LbarVdist);					// ���� �������� �����Ÿ�
	setParameterByName (&memo, "RbarHdist", params.RbarHdist);					// ���� �������� ����Ÿ�
	setParameterByName (&memo, "RbarVdist", params.RbarVdist);					// ���� �������� �����Ÿ�

	// ��ü ��ġ
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	// �Ķ���� ��ũ��Ʈ�� ������ �����Ŵ
	ACAPI_Goodies (APIAny_RunGDLParScriptID, &elem.header, 0);

	return	elem.header.guid;
}

// ������ �ܼ� (1/2��, ���̰� 6���� �ʰ��Ǹ� 2�� ������ ��), �������� �԰�/����, ������ ����(��, ���̰� 3500 �̻��̸� �߰��� ���� ������ ��), ������ �ʺ�, ũ�ν���� ����, ������/������ ���̾ ����
short DGCALLBACK PERISupportingPostPlacerHandler1 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	xx;
	short	itmIdx;
	short	result;
	double	remainLength;
	char	tempStr [64];
	char	tempStr2 [64];
	API_UCCallbackType	ucb;

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, "PERI ���ٸ� �ڵ� ��ġ");

			// ���̾�α� ũ�� �ʱ�ȭ
			DGSetDialogSize (dialogID, DG_CLIENT, 700, 770, DG_TOPLEFT, true);

			//////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			// ���� ��ư
			DGSetItemText (dialogID, DG_OK, "Ȯ ��");

			// ���� ��ư
			DGSetItemText (dialogID, DG_CANCEL, "�� ��");

			//////////////////////////////////////////////////////////// ������ ��ġ (������)
			DGSetItemText (dialogID, LABEL_TYPE, "Ÿ��");

			DGSetItemText (dialogID, LABEL_SIDE_VIEW, "���鵵");
			DGSetItemText (dialogID, LABEL_PLAN_VIEW, "��鵵");

			DGSetItemText (dialogID, LABEL_UPWARD, "��/������");
			DGSetItemText (dialogID, LABEL_TIMBER, "����");		// Ÿ�Կ� ���� �ٲ�
			DGSetItemText (dialogID, LABEL_VERTICAL_2ND, "������\n2��");
			DGSetItemText (dialogID, LABEL_VERTICAL_1ST, "������\n1��");
			DGSetItemText (dialogID, LABEL_DOWNWARD, "�ٴ�");

			DGSetItemText (dialogID, LABEL_TOTAL_HEIGHT, "�� ����");
			DGSetItemText (dialogID, LABEL_REMAIN_HEIGHT, "���� ����");

			DGSetItemText (dialogID, CHECKBOX_CROSSHEAD, "ũ�ν����");

			DGSetItemText (dialogID, LABEL_VPOST1, "1��");
			DGSetItemText (dialogID, LABEL_VPOST1_NOMINAL, "�԰�");
			DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "����");
			DGSetItemText (dialogID, CHECKBOX_VPOST2, "2��");
			DGSetItemText (dialogID, LABEL_VPOST2_NOMINAL, "�԰�");
			DGSetItemText (dialogID, LABEL_VPOST2_HEIGHT, "����");

			// ���̾� ����
			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, "���̾� ����");
			DGSetItemText (dialogID, LABEL_LAYER_SUPPORT, "���� ���ٸ�");
			DGSetItemText (dialogID, LABEL_LAYER_VPOST, "PERI ������");
			DGSetItemText (dialogID, LABEL_LAYER_HPOST, "PERI ������");
			DGSetItemText (dialogID, LABEL_LAYER_TIMBER, "��°�/�����");
			DGSetItemText (dialogID, LABEL_LAYER_GIRDER, "GT24 �Ŵ�");
			DGSetItemText (dialogID, LABEL_LAYER_BEAM_BRACKET, "�� �����");
			DGSetItemText (dialogID, LABEL_LAYER_YOKE, "�� �ۿ���");

			// üũ�ڽ�: ���̾� ����
			DGSetItemText (dialogID, CHECKBOX_LAYER_COUPLING, "���̾� ����");
			DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, TRUE);

			// ���� ��Ʈ�� �ʱ�ȭ
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_LAYER_SUPPORT;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_SUPPORT, 1);

			ucb.itemID	 = USERCONTROL_LAYER_VPOST;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_VPOST, 1);

			ucb.itemID	 = USERCONTROL_LAYER_HPOST;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_HPOST, 1);

			ucb.itemID	 = USERCONTROL_LAYER_TIMBER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, 1);
			
			ucb.itemID	 = USERCONTROL_LAYER_GIRDER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_GIRDER, 1);

			ucb.itemID	 = USERCONTROL_LAYER_BEAM_BRACKET;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_BEAM_BRACKET, 1);

			ucb.itemID	 = USERCONTROL_LAYER_YOKE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_YOKE, 1);

			DGSetItemText (dialogID, BUTTON_ADD, "�� �߰�");
			DGSetItemText (dialogID, BUTTON_DEL, "�� ����");

			DGSetItemText (dialogID, LABEL_TOTAL_WIDTH, "��ü �ʺ�");
			DGSetItemText (dialogID, LABEL_EXPLANATION, "�ʺ� ����\n������ ������\n��ü �ʺ񺸴�\n�۾ƾ� ��");
			DGSetItemText (dialogID, LABEL_TOTAL_LENGTH, "��ü ����");
			DGSetItemText (dialogID, LABEL_REMAIN_LENGTH, "���� ����");

			// �ʱ� ����
			// 1. Ÿ�� �߰�
			DGPopUpInsertItem (dialogID, POPUP_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TYPE, DG_POPUP_BOTTOM, "���� ���ٸ�");
			DGPopUpInsertItem (dialogID, POPUP_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TYPE, DG_POPUP_BOTTOM, "PERI ���ٸ�");
			DGPopUpInsertItem (dialogID, POPUP_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TYPE, DG_POPUP_BOTTOM, "���� ���ٸ� + ��°�");
			DGPopUpInsertItem (dialogID, POPUP_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TYPE, DG_POPUP_BOTTOM, "PERI ���ٸ� + �����");
			DGPopUpInsertItem (dialogID, POPUP_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TYPE, DG_POPUP_BOTTOM, "PERI ���ٸ� + GT24 �Ŵ�");
			DGPopUpInsertItem (dialogID, POPUP_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TYPE, DG_POPUP_BOTTOM, "PERI ���ٸ� + �� �ۿ���");

			// 2. ũ�ν���� �� ������ 2�� ��Ȱ��ȭ (���� ���ٸ��� ���)
			DGDisableItem (dialogID, CHECKBOX_CROSSHEAD);
			DGDisableItem (dialogID, CHECKBOX_VPOST2);
			DGSetItemValLong (dialogID, CHECKBOX_VPOST2, FALSE);
			DGDisableItem (dialogID, POPUP_VPOST2_NOMINAL);
			DGDisableItem (dialogID, EDITCONTROL_VPOST2_HEIGHT);

			// 3. ������ �԰� �˾� �߰�
			// ���� ���ٸ��� ��� V0 (2.0m), V1 (3.2m), V2 (3.4m), V3 (3.8m), V4 (4.0m), V5 (5.0m), V6 (5.9m), V7 (0.5~2.0m)
			// PERI ���ٸ��� ��� MP 120, MP 250, MP 350, MP 480, MP 625
			DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "V0 (2.0m)");
			DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "V1 (3.2m)");
			DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "V2 (3.4m)");
			DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "V3 (3.8m)");
			DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "V4 (4.0m)");
			DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "V5 (5.0m)");
			DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "V6 (5.9m)");
			DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "V7 (0.5~2.0m)");

			DGPopUpInsertItem (dialogID, POPUP_VPOST2_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST2_NOMINAL, DG_POPUP_BOTTOM, "MP 120");
			DGPopUpInsertItem (dialogID, POPUP_VPOST2_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST2_NOMINAL, DG_POPUP_BOTTOM, "MP 250");
			DGPopUpInsertItem (dialogID, POPUP_VPOST2_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST2_NOMINAL, DG_POPUP_BOTTOM, "MP 350");
			DGPopUpInsertItem (dialogID, POPUP_VPOST2_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST2_NOMINAL, DG_POPUP_BOTTOM, "MP 480");
			DGPopUpInsertItem (dialogID, POPUP_VPOST2_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST2_NOMINAL, DG_POPUP_BOTTOM, "MP 625");
			
			// 4. ������ ���� �� ���� ���� ǥ��
			// ���� ���ٸ��� ��� V0 (2.0m) (1200~2000), V1 (3.2m) (1800~3200), V2 (3.4m) (2000~3400), V3 (3.8m) (2400~3800), V4 (4.0m) (2600~4000), V5 (5.0m) (3000~5000), V6 (5.9m) (3200~5900), V7 (0.5~2.0m) (500~2000)
			// PERI ���ٸ��� ��� MP 120 (800~1200), MP 250 (1450~2500), MP 350 (1950~3500), MP 480 (2600~4800), MP 625 (4300~6250)
			DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 1200~2000");
			DGSetItemText (dialogID, LABEL_VPOST2_HEIGHT, "H. 800~1200");

			// 5. ������ ���� �Է� ���� ����
			DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 1.200);
			DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 2.000);
			DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 0.800);
			DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 1.200);

			// 6. ����, ũ�ν���� ���� ����
			DGSetItemText (dialogID, LABEL_TIMBER_HEIGHT, "0");
			DGSetItemText (dialogID, LABEL_CROSSHEAD_HEIGHT, "0");

			// 7. �� ����, ���� ���� ��Ȱ��ȭ �� �� ���
			DGDisableItem (dialogID, EDITCONTROL_TOTAL_HEIGHT);
			DGDisableItem (dialogID, EDITCONTROL_REMAIN_HEIGHT);
			DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_HEIGHT, infoMorph.height);
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HEIGHT, infoMorph.height - (DGGetItemValDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT) + DGGetItemValDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT) * DGGetItemValLong (dialogID, CHECKBOX_VPOST2)) - (atof (DGGetItemText (dialogID, LABEL_CROSSHEAD_HEIGHT).ToCStr ().Get ()) / 1000) * DGGetItemValLong (dialogID, CHECKBOX_CROSSHEAD) - atof (DGGetItemText (dialogID, LABEL_TIMBER_HEIGHT).ToCStr ().Get ()) / 1000);

			// 8. ��ü �ʺ�, ��ü ���� �� ���� ����
			DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_WIDTH, placementInfoForPERI.depth);
			DGDisableItem (dialogID, EDITCONTROL_TOTAL_WIDTH);
			DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_LENGTH, placementInfoForPERI.width);
			DGDisableItem (dialogID, EDITCONTROL_TOTAL_LENGTH);
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_LENGTH, placementInfoForPERI.width);
			DGDisableItem (dialogID, EDITCONTROL_REMAIN_LENGTH);

			// 9. 1�� ������ ���� �˾���Ʈ�� �߰�
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 490, 230, 30, 30);
			DGShowItem (dialogID, itmIdx);
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 490, 400, 30, 30);
			DGShowItem (dialogID, itmIdx);
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 505, 261, 1, 138);
			DGShowItem (dialogID, itmIdx);
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 515, 290, 70, 25);
			HPOST_CENTER [0] = itmIdx;
			DGShowItem (dialogID, itmIdx);
			DGSetItemText (dialogID, itmIdx, "������");
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 200, 10, 515, 315, 70, 25);
			DGShowItem (dialogID, itmIdx);
			DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "����");
			DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "625");
			DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "750");
			DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "900");
			DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "1200");
			DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "1375");
			DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "1500");
			DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "2015");
			DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "2250");
			DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "2300");
			DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "2370");
			DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "2660");
			DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "2960");
			DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "����");
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 515, 340, 70, 25);
			DGShowItem (dialogID, itmIdx);

			// 10. ���� ���ٸ��� ���, ������� ��Ȱ��ȭ��
			DGDisableItem (dialogID, HPOST_CENTER [0]);
			DGDisableItem (dialogID, HPOST_CENTER [0]+1);

			break;

		case DG_MSG_CHANGE:
			switch (item) {
				case USERCONTROL_LAYER_SUPPORT:
				case USERCONTROL_LAYER_VPOST:
				case USERCONTROL_LAYER_HPOST:
				case USERCONTROL_LAYER_TIMBER:
				case USERCONTROL_LAYER_GIRDER:
				case USERCONTROL_LAYER_BEAM_BRACKET:
				case USERCONTROL_LAYER_YOKE:
					// ���̾� ���� üũ�ڽ� On/Off�� ���� �̺�Ʈ ó��
					if (DGGetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING) == 1) {
						switch (item) {
							case USERCONTROL_LAYER_SUPPORT:
								for (xx = USERCONTROL_LAYER_SUPPORT ; xx <= USERCONTROL_LAYER_YOKE ; ++xx)	DGSetItemValLong (dialogID, xx, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SUPPORT));
								break;
							case USERCONTROL_LAYER_VPOST:
								for (xx = USERCONTROL_LAYER_SUPPORT ; xx <= USERCONTROL_LAYER_YOKE ; ++xx)	DGSetItemValLong (dialogID, xx, DGGetItemValLong (dialogID, USERCONTROL_LAYER_VPOST));
								break;
							case USERCONTROL_LAYER_HPOST:
								for (xx = USERCONTROL_LAYER_SUPPORT ; xx <= USERCONTROL_LAYER_YOKE ; ++xx)	DGSetItemValLong (dialogID, xx, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HPOST));
								break;
							case USERCONTROL_LAYER_TIMBER:
								for (xx = USERCONTROL_LAYER_SUPPORT ; xx <= USERCONTROL_LAYER_YOKE ; ++xx)	DGSetItemValLong (dialogID, xx, DGGetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER));
								break;
							case USERCONTROL_LAYER_GIRDER:
								for (xx = USERCONTROL_LAYER_SUPPORT ; xx <= USERCONTROL_LAYER_YOKE ; ++xx)	DGSetItemValLong (dialogID, xx, DGGetItemValLong (dialogID, USERCONTROL_LAYER_GIRDER));
								break;
							case USERCONTROL_LAYER_BEAM_BRACKET:
								for (xx = USERCONTROL_LAYER_SUPPORT ; xx <= USERCONTROL_LAYER_YOKE ; ++xx)	DGSetItemValLong (dialogID, xx, DGGetItemValLong (dialogID, USERCONTROL_LAYER_BEAM_BRACKET));
								break;
							case USERCONTROL_LAYER_YOKE:
								for (xx = USERCONTROL_LAYER_SUPPORT ; xx <= USERCONTROL_LAYER_YOKE ; ++xx)	DGSetItemValLong (dialogID, xx, DGGetItemValLong (dialogID, USERCONTROL_LAYER_YOKE));
								break;
						}
					}
					break;

				case POPUP_TYPE:
					// Ÿ�Կ� ���� ������, ������ ������ �ٲ�
					strcpy (tempStr, DGPopUpGetItemText (dialogID, POPUP_TYPE, DGPopUpGetSelected (dialogID, POPUP_TYPE)).ToCStr ().Get ());
					if (my_strcmp (tempStr, "���� ���ٸ�") == 0) {
						// ������ 1��: ���� Ÿ��
						DGPopUpDeleteItem (dialogID, POPUP_VPOST1_NOMINAL, DG_ALL_ITEMS);
						DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "V0 (2.0m)");
						DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "V1 (3.2m)");
						DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "V2 (3.4m)");
						DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "V3 (3.8m)");
						DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "V4 (4.0m)");
						DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "V5 (5.0m)");
						DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "V6 (5.9m)");
						DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "V7 (0.5~2.0m)");

						DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 1200~2000");
						DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 1.200);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 2.000);

						// ��Ȱ��ȭ: ������ 2��, ũ�ν����, ������
						DGDisableItem (dialogID, CHECKBOX_VPOST2);
						DGSetItemValLong (dialogID, CHECKBOX_VPOST2, FALSE);
						DGDisableItem (dialogID, POPUP_VPOST2_NOMINAL);
						DGDisableItem (dialogID, EDITCONTROL_VPOST2_HEIGHT);

						DGDisableItem (dialogID, CHECKBOX_CROSSHEAD);
						DGSetItemValLong (dialogID, CHECKBOX_CROSSHEAD, FALSE);
						DGSetItemText (dialogID, LABEL_CROSSHEAD_HEIGHT, "0");

						for (xx = 0 ; xx < placementInfoForPERI.nColVPost ; ++xx) {
							DGDisableItem (dialogID, HPOST_CENTER [xx]);
							DGDisableItem (dialogID, HPOST_CENTER [xx]+1);
						}
						for (xx = 1 ; xx < placementInfoForPERI.nColVPost ; ++xx) {
							DGDisableItem (dialogID, HPOST_UP [xx]);
							DGDisableItem (dialogID, HPOST_UP [xx]+1);
							DGDisableItem (dialogID, HPOST_DOWN [xx]);
							DGDisableItem (dialogID, HPOST_DOWN [xx]+1);
						}

						// ���� �ؽ�Ʈ: ����
						DGSetItemText (dialogID, LABEL_TIMBER, "����");
						DGSetItemText (dialogID, LABEL_TIMBER_HEIGHT, "0");

					} else if (my_strcmp (tempStr, "PERI ���ٸ�") == 0) {
						// ������ 1,2��: PERI Ÿ��
						DGPopUpDeleteItem (dialogID, POPUP_VPOST1_NOMINAL, DG_ALL_ITEMS);
						DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "MP 120");
						DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "MP 250");
						DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "MP 350");
						DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "MP 480");
						DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "MP 625");

						DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 800~1200");
						DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 0.800);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 1.200);

						// Ȱ��ȭ: ������ 2��, ũ�ν����, ������
						DGEnableItem (dialogID, CHECKBOX_VPOST2);
						DGSetItemValLong (dialogID, CHECKBOX_VPOST2, FALSE);
						DGDisableItem (dialogID, POPUP_VPOST2_NOMINAL);
						DGDisableItem (dialogID, EDITCONTROL_VPOST2_HEIGHT);
						DGEnableItem (dialogID, CHECKBOX_CROSSHEAD);
						DGSetItemValLong (dialogID, CHECKBOX_CROSSHEAD, FALSE);
						DGSetItemText (dialogID, LABEL_CROSSHEAD_HEIGHT, "0");

						for (xx = 0 ; xx < placementInfoForPERI.nColVPost ; ++xx) {
							DGEnableItem (dialogID, HPOST_CENTER [xx]);
							(DGGetItemValLong (dialogID, HPOST_CENTER [xx]) == TRUE) ? DGEnableItem (dialogID, HPOST_CENTER [xx]+1) : DGDisableItem (dialogID, HPOST_CENTER [xx]+1);
						}
						for (xx = 1 ; xx < placementInfoForPERI.nColVPost ; ++xx) {
							DGEnableItem (dialogID, HPOST_UP [xx]);
							(DGGetItemValLong (dialogID, HPOST_UP [xx]) == TRUE) ? DGEnableItem (dialogID, HPOST_UP [xx]+1) : DGDisableItem (dialogID, HPOST_UP [xx]+1);
							DGEnableItem (dialogID, HPOST_DOWN [xx]);
							(DGGetItemValLong (dialogID, HPOST_DOWN [xx]) == TRUE) ? DGEnableItem (dialogID, HPOST_DOWN [xx]+1) : DGDisableItem (dialogID, HPOST_DOWN [xx]+1);
						}

						// ���� �ؽ�Ʈ: ����
						DGSetItemText (dialogID, LABEL_TIMBER, "����");
						DGSetItemText (dialogID, LABEL_TIMBER_HEIGHT, "0");

					} else if (my_strcmp (tempStr, "���� ���ٸ� + ��°�") == 0) {
						// ������ 1��: ���� Ÿ��
						DGPopUpDeleteItem (dialogID, POPUP_VPOST1_NOMINAL, DG_ALL_ITEMS);
						DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "V0 (2.0m)");
						DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "V1 (3.2m)");
						DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "V2 (3.4m)");
						DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "V3 (3.8m)");
						DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "V4 (4.0m)");
						DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "V5 (5.0m)");
						DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "V6 (5.9m)");
						DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "V7 (0.5~2.0m)");

						DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 1200~2000");
						DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 1.200);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 2.000);

						// ��Ȱ��ȭ: ������ 2��, ũ�ν����, ������
						DGDisableItem (dialogID, CHECKBOX_VPOST2);
						DGSetItemValLong (dialogID, CHECKBOX_VPOST2, FALSE);
						DGDisableItem (dialogID, POPUP_VPOST2_NOMINAL);
						DGDisableItem (dialogID, EDITCONTROL_VPOST2_HEIGHT);

						DGDisableItem (dialogID, CHECKBOX_CROSSHEAD);
						DGSetItemValLong (dialogID, CHECKBOX_CROSSHEAD, FALSE);
						DGSetItemText (dialogID, LABEL_CROSSHEAD_HEIGHT, "0");

						for (xx = 0 ; xx < placementInfoForPERI.nColVPost ; ++xx) {
							DGDisableItem (dialogID, HPOST_CENTER [xx]);
							DGDisableItem (dialogID, HPOST_CENTER [xx]+1);
						}
						for (xx = 1 ; xx < placementInfoForPERI.nColVPost ; ++xx) {
							DGDisableItem (dialogID, HPOST_UP [xx]);
							DGDisableItem (dialogID, HPOST_UP [xx]+1);
							DGDisableItem (dialogID, HPOST_DOWN [xx]);
							DGDisableItem (dialogID, HPOST_DOWN [xx]+1);
						}

						// ���� �ؽ�Ʈ: ��°�
						DGSetItemText (dialogID, LABEL_TIMBER, "��°�");
						DGSetItemText (dialogID, LABEL_TIMBER_HEIGHT, "80");

					} else if (my_strcmp (tempStr, "PERI ���ٸ� + �����") == 0) {
						// ������ 1��: PERI Ÿ��
						DGPopUpDeleteItem (dialogID, POPUP_VPOST1_NOMINAL, DG_ALL_ITEMS);
						DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "MP 120");
						DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "MP 250");
						DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "MP 350");
						DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "MP 480");
						DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "MP 625");

						DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 800~1200");
						DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 0.800);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 1.200);

						// Ȱ��ȭ: ������ 2��, ũ�ν����, ������
						DGEnableItem (dialogID, CHECKBOX_VPOST2);
						DGSetItemValLong (dialogID, CHECKBOX_VPOST2, FALSE);
						DGDisableItem (dialogID, POPUP_VPOST2_NOMINAL);
						DGDisableItem (dialogID, EDITCONTROL_VPOST2_HEIGHT);
						DGEnableItem (dialogID, CHECKBOX_CROSSHEAD);
						DGSetItemValLong (dialogID, CHECKBOX_CROSSHEAD, TRUE);
						DGSetItemText (dialogID, LABEL_CROSSHEAD_HEIGHT, "3");

						for (xx = 0 ; xx < placementInfoForPERI.nColVPost ; ++xx) {
							DGEnableItem (dialogID, HPOST_CENTER [xx]);
							(DGGetItemValLong (dialogID, HPOST_CENTER [xx]) == TRUE) ? DGEnableItem (dialogID, HPOST_CENTER [xx]+1) : DGDisableItem (dialogID, HPOST_CENTER [xx]+1);
						}
						for (xx = 1 ; xx < placementInfoForPERI.nColVPost ; ++xx) {
							DGEnableItem (dialogID, HPOST_UP [xx]);
							(DGGetItemValLong (dialogID, HPOST_UP [xx]) == TRUE) ? DGEnableItem (dialogID, HPOST_UP [xx]+1) : DGDisableItem (dialogID, HPOST_UP [xx]+1);
							DGEnableItem (dialogID, HPOST_DOWN [xx]);
							(DGGetItemValLong (dialogID, HPOST_DOWN [xx]) == TRUE) ? DGEnableItem (dialogID, HPOST_DOWN [xx]+1) : DGDisableItem (dialogID, HPOST_DOWN [xx]+1);
						}

						// ���� �ؽ�Ʈ: �����
						DGSetItemText (dialogID, LABEL_TIMBER, "�����");
						DGSetItemText (dialogID, LABEL_TIMBER_HEIGHT, "240");

					} else if (my_strcmp (tempStr, "PERI ���ٸ� + GT24 �Ŵ�") == 0) {
						// ������ 1��: PERI Ÿ��
						DGPopUpDeleteItem (dialogID, POPUP_VPOST1_NOMINAL, DG_ALL_ITEMS);
						DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "MP 120");
						DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "MP 250");
						DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "MP 350");
						DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "MP 480");
						DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "MP 625");

						DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 800~1200");
						DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 0.800);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 1.200);

						// Ȱ��ȭ: ������ 2��, ũ�ν����, ������
						DGEnableItem (dialogID, CHECKBOX_VPOST2);
						DGSetItemValLong (dialogID, CHECKBOX_VPOST2, FALSE);
						DGDisableItem (dialogID, POPUP_VPOST2_NOMINAL);
						DGDisableItem (dialogID, EDITCONTROL_VPOST2_HEIGHT);
						DGEnableItem (dialogID, CHECKBOX_CROSSHEAD);
						DGSetItemValLong (dialogID, CHECKBOX_CROSSHEAD, TRUE);
						DGSetItemText (dialogID, LABEL_CROSSHEAD_HEIGHT, "3");

						for (xx = 0 ; xx < placementInfoForPERI.nColVPost ; ++xx) {
							DGEnableItem (dialogID, HPOST_CENTER [xx]);
							(DGGetItemValLong (dialogID, HPOST_CENTER [xx]) == TRUE) ? DGEnableItem (dialogID, HPOST_CENTER [xx]+1) : DGDisableItem (dialogID, HPOST_CENTER [xx]+1);
						}
						for (xx = 1 ; xx < placementInfoForPERI.nColVPost ; ++xx) {
							DGEnableItem (dialogID, HPOST_UP [xx]);
							(DGGetItemValLong (dialogID, HPOST_UP [xx]) == TRUE) ? DGEnableItem (dialogID, HPOST_UP [xx]+1) : DGDisableItem (dialogID, HPOST_UP [xx]+1);
							DGEnableItem (dialogID, HPOST_DOWN [xx]);
							(DGGetItemValLong (dialogID, HPOST_DOWN [xx]) == TRUE) ? DGEnableItem (dialogID, HPOST_DOWN [xx]+1) : DGDisableItem (dialogID, HPOST_DOWN [xx]+1);
						}

						// ���� �ؽ�Ʈ: GT24 �Ŵ�
						DGSetItemText (dialogID, LABEL_TIMBER, "GT24 �Ŵ�");
						DGSetItemText (dialogID, LABEL_TIMBER_HEIGHT, "240");

					} else if (my_strcmp (tempStr, "PERI ���ٸ� + �� �ۿ���") == 0) {
						// ������ 1��: PERI Ÿ��
						DGPopUpDeleteItem (dialogID, POPUP_VPOST1_NOMINAL, DG_ALL_ITEMS);
						DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "MP 120");
						DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "MP 250");
						DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "MP 350");
						DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "MP 480");
						DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
						DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "MP 625");

						DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 800~1200");
						DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 0.800);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 1.200);

						// Ȱ��ȭ: ������ 2��, ũ�ν����, ������
						DGEnableItem (dialogID, CHECKBOX_VPOST2);
						DGSetItemValLong (dialogID, CHECKBOX_VPOST2, FALSE);
						DGDisableItem (dialogID, POPUP_VPOST2_NOMINAL);
						DGDisableItem (dialogID, EDITCONTROL_VPOST2_HEIGHT);
						DGEnableItem (dialogID, CHECKBOX_CROSSHEAD);
						DGSetItemValLong (dialogID, CHECKBOX_CROSSHEAD, FALSE);
						DGSetItemText (dialogID, LABEL_CROSSHEAD_HEIGHT, "0");

						for (xx = 0 ; xx < placementInfoForPERI.nColVPost ; ++xx) {
							DGEnableItem (dialogID, HPOST_CENTER [xx]);
							(DGGetItemValLong (dialogID, HPOST_CENTER [xx]) == TRUE) ? DGEnableItem (dialogID, HPOST_CENTER [xx]+1) : DGDisableItem (dialogID, HPOST_CENTER [xx]+1);
						}
						for (xx = 1 ; xx < placementInfoForPERI.nColVPost ; ++xx) {
							DGEnableItem (dialogID, HPOST_UP [xx]);
							(DGGetItemValLong (dialogID, HPOST_UP [xx]) == TRUE) ? DGEnableItem (dialogID, HPOST_UP [xx]+1) : DGDisableItem (dialogID, HPOST_UP [xx]+1);
							DGEnableItem (dialogID, HPOST_DOWN [xx]);
							(DGGetItemValLong (dialogID, HPOST_DOWN [xx]) == TRUE) ? DGEnableItem (dialogID, HPOST_DOWN [xx]+1) : DGDisableItem (dialogID, HPOST_DOWN [xx]+1);
						}

						// ���� �ؽ�Ʈ: �� �ۿ���
						DGSetItemText (dialogID, LABEL_TIMBER, "�� �ۿ���");
						DGSetItemText (dialogID, LABEL_TIMBER_HEIGHT, "105");
					}

					// ���� ���� ���
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HEIGHT, infoMorph.height - (DGGetItemValDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT) + DGGetItemValDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT) * DGGetItemValLong (dialogID, CHECKBOX_VPOST2)) - (atof (DGGetItemText (dialogID, LABEL_CROSSHEAD_HEIGHT).ToCStr ().Get ()) / 1000) * DGGetItemValLong (dialogID, CHECKBOX_CROSSHEAD) - atof (DGGetItemText (dialogID, LABEL_TIMBER_HEIGHT).ToCStr ().Get ()) / 1000);

					// ���� ���� ���
					remainLength = placementInfoForPERI.width;
					for (xx = 1 ; xx <= placementInfoForPERI.nColVPost ; ++xx) {
						remainLength -= DGGetItemValDouble (dialogID, HPOST_UP [xx]+2);
					}
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_LENGTH, remainLength);

					break;

				case CHECKBOX_CROSSHEAD:
				case CHECKBOX_VPOST2:
				case POPUP_VPOST1_NOMINAL:
				case POPUP_VPOST2_NOMINAL:
				case EDITCONTROL_VPOST2_HEIGHT:
				case EDITCONTROL_VPOST1_HEIGHT:
					// ������ 2�� On/Off�� ������ 2���� �԰�, ���� �Է� ��Ʈ���� Ȱ��ȭ/��Ȱ��ȭ (PERI Ÿ�Կ� ����)
					if (DGGetItemValLong (dialogID, CHECKBOX_VPOST2) == TRUE) {
						DGEnableItem (dialogID, POPUP_VPOST2_NOMINAL);
						DGEnableItem (dialogID, EDITCONTROL_VPOST2_HEIGHT);
					} else {
						DGDisableItem (dialogID, POPUP_VPOST2_NOMINAL);
						DGDisableItem (dialogID, EDITCONTROL_VPOST2_HEIGHT);
					}

					// ũ�ν���� üũ�ڽ� �����
					if (DGGetItemValLong (dialogID, CHECKBOX_CROSSHEAD) == TRUE)
						DGSetItemText (dialogID, LABEL_CROSSHEAD_HEIGHT, "3");
					else
						DGSetItemText (dialogID, LABEL_CROSSHEAD_HEIGHT, "0");

					// ������ 1�� �԰ݿ� ���� ���� ���� �����
					strcpy (tempStr, DGPopUpGetItemText (dialogID, POPUP_TYPE, DGPopUpGetSelected (dialogID, POPUP_TYPE)).ToCStr ().Get ());
					if (strncmp (tempStr, "���� ���ٸ�", strlen ("���� ���ٸ�")) == 0) {
						// ���� ���ٸ��� ��� V0 (2.0m) (1200~2000), V1 (3.2m) (1800~3200), V2 (3.4m) (2000~3400), V3 (3.8m) (2400~3800), V4 (4.0m) (2600~4000), V5 (5.0m) (3000~5000), V6 (5.9m) (3200~5900), V7 (0.5~2.0m) (500~2000)
						strcpy (tempStr2, DGPopUpGetItemText (dialogID, POPUP_VPOST1_NOMINAL, DGPopUpGetSelected (dialogID, POPUP_VPOST1_NOMINAL)).ToCStr ().Get ());
						if (my_strcmp (tempStr2, "V0 (2.0m)") == 0) {
							DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 1200~2000");
							DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 1.200);
							DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 2.000);
						} else if (my_strcmp (tempStr2, "V1 (3.2m)") == 0) {
							DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 1800~3200");
							DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 1.800);
							DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 3.200);
						} else if (my_strcmp (tempStr2, "V2 (3.4m)") == 0) {
							DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 2000~3400");
							DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 2.000);
							DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 3.400);
						} else if (my_strcmp (tempStr2, "V3 (3.8m)") == 0) {
							DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 2400~3800");
							DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 2.400);
							DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 3.800);
						} else if (my_strcmp (tempStr2, "V4 (4.0m)") == 0) {
							DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 2600~4000");
							DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 2.600);
							DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 4.000);
						} else if (my_strcmp (tempStr2, "V5 (5.0m)") == 0) {
							DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 3000~5000");
							DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 3.000);
							DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 5.000);
						} else if (my_strcmp (tempStr2, "V6 (5.9m)") == 0) {
							DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 3200~5900");
							DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 3.200);
							DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 5.900);
						} else if (my_strcmp (tempStr2, "V7 (0.5~2.0m)") == 0) {
							DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 500~2000");
							DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 0.500);
							DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 2.000);
						}
					} else if (strncmp (tempStr, "PERI ���ٸ�", strlen ("PERI ���ٸ�")) == 0) {
						// PERI ���ٸ��� ��� MP 120 (800~1200), MP 250 (1450~2500), MP 350 (1950~3500), MP 480 (2600~4800), MP 625 (4300~6250)
						strcpy (tempStr2, DGPopUpGetItemText (dialogID, POPUP_VPOST1_NOMINAL, DGPopUpGetSelected (dialogID, POPUP_VPOST1_NOMINAL)).ToCStr ().Get ());
						if (my_strcmp (tempStr2, "MP 120") == 0) {
							DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 800~1200");
							DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 0.800);
							DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 1.200);
						} else if (my_strcmp (tempStr2, "MP 250") == 0) {
							DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 1450~2500");
							DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 1.450);
							DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 2.500);
						} else if (my_strcmp (tempStr2, "MP 350") == 0) {
							DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 1950~3500");
							DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 1.950);
							DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 3.500);
						} else if (my_strcmp (tempStr2, "MP 480") == 0) {
							DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 2600~4800");
							DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 2.600);
							DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 4.800);
						} else if (my_strcmp (tempStr2, "MP 625") == 0) {
							DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 4300~6250");
							DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 4.300);
							DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 6.250);
						}
					}

					// ������ 2�� �԰ݿ� ���� ���� ���� �����
					// PERI ���ٸ��� ��� MP 120 (800~1200), MP 250 (1450~2500), MP 350 (1950~3500), MP 480 (2600~4800), MP 625 (4300~6250)
					strcpy (tempStr2, DGPopUpGetItemText (dialogID, POPUP_VPOST2_NOMINAL, DGPopUpGetSelected (dialogID, POPUP_VPOST2_NOMINAL)).ToCStr ().Get ());
					if (my_strcmp (tempStr, "MP 120") == 0) {
						DGSetItemText (dialogID, LABEL_VPOST2_HEIGHT, "H. 800~1200");
						DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 0.800);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 1.200);
					} else if (my_strcmp (tempStr, "MP 250") == 0) {
						DGSetItemText (dialogID, LABEL_VPOST2_HEIGHT, "H. 1450~2500");
						DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 1.450);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 2.500);
					} else if (my_strcmp (tempStr, "MP 350") == 0) {
						DGSetItemText (dialogID, LABEL_VPOST2_HEIGHT, "H. 1950~3500");
						DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 1.950);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 3.500);
					} else if (my_strcmp (tempStr, "MP 480") == 0) {
						DGSetItemText (dialogID, LABEL_VPOST2_HEIGHT, "H. 2600~4800");
						DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 2.600);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 4.800);
					} else if (my_strcmp (tempStr, "MP 625") == 0) {
						DGSetItemText (dialogID, LABEL_VPOST2_HEIGHT, "H. 4300~6250");
						DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 4.300);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 6.250);
					}

					// ���� ���� ���
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HEIGHT, infoMorph.height - (DGGetItemValDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT) + DGGetItemValDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT) * DGGetItemValLong (dialogID, CHECKBOX_VPOST2)) - (atof (DGGetItemText (dialogID, LABEL_CROSSHEAD_HEIGHT).ToCStr ().Get ()) / 1000) * DGGetItemValLong (dialogID, CHECKBOX_CROSSHEAD) - atof (DGGetItemText (dialogID, LABEL_TIMBER_HEIGHT).ToCStr ().Get ()) / 1000);

					break;

				default:
					// ������ ������ �������� ��
					for (xx = 0 ; xx < placementInfoForPERI.nColVPost ; ++xx)
						(DGGetItemValLong (dialogID, HPOST_CENTER [xx]) == TRUE) ? DGEnableItem (dialogID, HPOST_CENTER [xx]+1) : DGDisableItem (dialogID, HPOST_CENTER [xx]+1);
					for (xx = 1 ; xx < placementInfoForPERI.nColVPost ; ++xx) {
						(DGGetItemValLong (dialogID, HPOST_UP [xx]) == TRUE) ? DGEnableItem (dialogID, HPOST_UP [xx]+1) : DGDisableItem (dialogID, HPOST_UP [xx]+1);
						(DGGetItemValLong (dialogID, HPOST_DOWN [xx]) == TRUE) ? DGEnableItem (dialogID, HPOST_DOWN [xx]+1) : DGDisableItem (dialogID, HPOST_DOWN [xx]+1);
					}
					
					// ������ �˾���Ʈ���� �������� �� (���� ���� ��쿡�� �����)
					for (xx = 0 ; xx < placementInfoForPERI.nColVPost ; ++xx) {
						if (item == HPOST_CENTER [xx]+1) {
							DGSetItemValDouble (dialogID, HPOST_CENTER [xx]+2, atof (DGPopUpGetItemText (dialogID, HPOST_CENTER [xx]+1, DGPopUpGetSelected (dialogID, HPOST_CENTER [xx]+1)).ToCStr ().Get ()) / 1000);
						}
					}
					for (xx = 1 ; xx < placementInfoForPERI.nColVPost ; ++xx) {
						if (item == HPOST_UP [xx]+1) {
							// ��
							DGSetItemValDouble (dialogID, HPOST_UP [xx]+2, atof (DGPopUpGetItemText (dialogID, HPOST_UP [xx]+1, DGPopUpGetSelected (dialogID, HPOST_UP [xx]+1)).ToCStr ().Get ()) / 1000);

							// �Ʒ�
							DGPopUpSelectItem (dialogID, HPOST_DOWN [xx]+1, DGPopUpGetSelected (dialogID, HPOST_UP [xx]+1));
							DGSetItemValDouble (dialogID, HPOST_DOWN [xx]+2, atof (DGPopUpGetItemText (dialogID, HPOST_UP [xx]+1, DGPopUpGetSelected (dialogID, HPOST_UP [xx]+1)).ToCStr ().Get ()) / 1000);
						}
						if (item == HPOST_DOWN [xx]+1) {
							// �Ʒ�
							DGSetItemValDouble (dialogID, HPOST_DOWN [xx]+2, atof (DGPopUpGetItemText (dialogID, HPOST_DOWN [xx]+1, DGPopUpGetSelected (dialogID, HPOST_DOWN [xx]+1)).ToCStr ().Get ()) / 1000);

							// ��
							DGPopUpSelectItem (dialogID, HPOST_UP [xx]+1, DGPopUpGetSelected (dialogID, HPOST_DOWN [xx]+1));
							DGSetItemValDouble (dialogID, HPOST_UP [xx]+2, atof (DGPopUpGetItemText (dialogID, HPOST_DOWN [xx]+1, DGPopUpGetSelected (dialogID, HPOST_DOWN [xx]+1)).ToCStr ().Get ()) / 1000);
						}
					}

					// ������ ũ�⸦ �������� ��
					for (xx = 1 ; xx < placementInfoForPERI.nColVPost ; ++xx) {
						if (item == HPOST_UP [xx]+2)
							DGSetItemValDouble (dialogID, HPOST_DOWN [xx]+2, DGGetItemValDouble (dialogID, HPOST_UP [xx]+2));
						if (item == HPOST_DOWN [xx]+2)
							DGSetItemValDouble (dialogID, HPOST_UP [xx]+2, DGGetItemValDouble (dialogID, HPOST_DOWN [xx]+2));
					}
					
					// ���� ���� ���
					remainLength = placementInfoForPERI.width;
					for (xx = 1 ; xx <= placementInfoForPERI.nColVPost ; ++xx) {
						remainLength -= DGGetItemValDouble (dialogID, HPOST_UP [xx]+2);
					}
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_LENGTH, remainLength);

					break;
			}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// Ÿ�Կ� ���� �з�
					strcpy (tempStr, DGPopUpGetItemText (dialogID, POPUP_TYPE, DGPopUpGetSelected (dialogID, POPUP_TYPE)).ToCStr ().Get ());
					if (my_strcmp (tempStr, "���� ���ٸ�") == 0) {
						// ... nameVPost1 �迭�� "����Ʈv1.0.gsm" ����

					} else if (my_strcmp (tempStr, "PERI ���ٸ�") == 0) {
						// ... nameVPost1 �迭�� "PERI���ٸ� ������ v0.1" ����

					} else if (my_strcmp (tempStr, "���� ���ٸ� + ��°�") == 0) {
						// ... nameVPost1 �迭�� "����Ʈv1.0.gsm" ����

					} else if (my_strcmp (tempStr, "PERI ���ٸ� + �����") == 0) {
						// ... nameVPost1 �迭�� "PERI���ٸ� ������ v0.1" ����

					} else if (my_strcmp (tempStr, "PERI ���ٸ� + GT24 �Ŵ�") == 0) {
						// ... nameVPost1 �迭�� "PERI���ٸ� ������ v0.1" ����

					} else if (my_strcmp (tempStr, "PERI ���ٸ� + �� �ۿ���") == 0) {
						// ... nameVPost1 �迭�� "PERI���ٸ� ������ v0.1" ����
					}

					// ���� ���� ����
					// ... nameVPost2 �迭�� "PERI���ٸ� ������ v0.1.gsm" ����

					// ... nomVPost1 �迭�� POPUP_VPOST1_NOMINAL ���� ������ �ؽ�Ʈ ���� ����
					// ... heightVPost1 �� EDITCONTROL_VPOST1_HEIGHT �� ����

					// ... bVPost2 �� CHECKBOX_VPOST2 �� ����
					// ... nomVPost2 �迭�� POPUP_VPOST2_NOMINAL ���� ������ �ؽ�Ʈ ���� ����
					// ... heightVPost2 �� EDITCONTROL_VPOST2_HEIGHT �� ����

					// ... bCrosshead �� CHECKBOX_CROSSHEAD �� ����
					// ... heightCrosshead �� LABEL_CROSSHEAD_HEIGHT ���� ���ڷ� ��ȯ�ؼ� ����

					// ... nameTimber �迭�� LABEL_TIMBER �� �ؽ�Ʈ ���� ����
					// ... heightTimber �� LABEL_TIMBER_HEIGHT ���� ���ڷ� ��ȯ�ؼ� ����

					// ... (0 ~ nColVPost ����)
					// ... bHPost_center [xx] �� HPOST_CENTER [xx] �� ���� ����
					// ... sizeHPost_center [xx] �迭�� HPOST_CENTER [xx]+1 ���� ������ �ؽ�Ʈ ���� ����
					// ... lengthHPost_center [xx] �� HPOST_CENTER [xx]+2 �� ����

					// ... (1 ~ nColVPost ����)
					// ... bHPost_up [xx] �� HPOST_UP [xx] �� ���� ����
					// ... sizeHPost_up [xx] �迭�� HPOST_UP [xx]+1 ���� ������ �ؽ�Ʈ ���� ����
					// ... lengthHPost_up [xx] �� HPOST_UP [xx]+2 �� ����
					// ... bHPost_down [xx] �� HPOST_DOWN [xx] �� ���� ����
					// ... sizeHPost_down [xx] �迭�� HPOST_DOWN [xx]+1 ���� ������ �ؽ�Ʈ ���� ����
					// ... lengthHPost_down [xx] �� HPOST_DOWN [xx]+2 �� ����



					//if (DGGetItemValLong (dialogID, CHECKBOX_VPOST1) == TRUE)
					//	placementInfoForPERI.bVPost1 = true;
					//else
					//	placementInfoForPERI.bVPost1 = false;
					//sprintf (placementInfoForPERI.nomVPost1, "%s", DGPopUpGetItemText (dialogID, POPUP_VPOST1_NOMINAL, DGPopUpGetSelected (dialogID, POPUP_VPOST1_NOMINAL)).ToCStr ().Get ());
					//placementInfoForPERI.heightVPost1 = DGGetItemValDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT);
					//
					//if (DGGetItemValLong (dialogID, CHECKBOX_VPOST2) == TRUE)
					//	placementInfoForPERI.bVPost2 = true;
					//else
					//	placementInfoForPERI.bVPost2 = false;
					//sprintf (placementInfoForPERI.nomVPost2, "%s", DGPopUpGetItemText (dialogID, POPUP_VPOST2_NOMINAL, DGPopUpGetSelected (dialogID, POPUP_VPOST2_NOMINAL)).ToCStr ().Get ());
					//placementInfoForPERI.heightVPost2 = DGGetItemValDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT);

					//if (DGGetItemValLong (dialogID, CHECKBOX_CROSSHEAD) == TRUE)
					//	placementInfoForPERI.bCrosshead = true;
					//else
					//	placementInfoForPERI.bCrosshead = false;

					//if (DGGetItemValLong (dialogID, CHECKBOX_HPOST) == TRUE)
					//	placementInfoForPERI.bHPost = true;
					//else
					//	placementInfoForPERI.bHPost = false;

					//sprintf (tempStr, "%s", DGPopUpGetItemText (dialogID, POPUP_WIDTH_NORTH, DGPopUpGetSelected (dialogID, POPUP_WIDTH_NORTH)).ToCStr ().Get ());
					//if (my_strcmp (tempStr, "����") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "");
					//if (my_strcmp (tempStr, "625") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "62.5 cm");
					//if (my_strcmp (tempStr, "750") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "75 cm");
					//if (my_strcmp (tempStr, "900") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "90 cm");
					//if (my_strcmp (tempStr, "1200") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "120 cm");
					//if (my_strcmp (tempStr, "1375") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "137.5 cm");
					//if (my_strcmp (tempStr, "1500") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "150 cm");
					//if (my_strcmp (tempStr, "2015") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "201.5 cm");
					//if (my_strcmp (tempStr, "2250") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "225 cm");
					//if (my_strcmp (tempStr, "2300") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "230 cm");
					//if (my_strcmp (tempStr, "2370") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "237 cm");
					//if (my_strcmp (tempStr, "2660") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "266 cm");
					//if (my_strcmp (tempStr, "2960") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "296 cm");

					//sprintf (tempStr, "%s", DGPopUpGetItemText (dialogID, POPUP_WIDTH_WEST, DGPopUpGetSelected (dialogID, POPUP_WIDTH_WEST)).ToCStr ().Get ());
					//if (my_strcmp (tempStr, "����") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "");
					//if (my_strcmp (tempStr, "625") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "62.5 cm");
					//if (my_strcmp (tempStr, "750") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "75 cm");
					//if (my_strcmp (tempStr, "900") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "90 cm");
					//if (my_strcmp (tempStr, "1200") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "120 cm");
					//if (my_strcmp (tempStr, "1375") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "137.5 cm");
					//if (my_strcmp (tempStr, "1500") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "150 cm");
					//if (my_strcmp (tempStr, "2015") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "201.5 cm");
					//if (my_strcmp (tempStr, "2250") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "225 cm");
					//if (my_strcmp (tempStr, "2300") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "230 cm");
					//if (my_strcmp (tempStr, "2370") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "237 cm");
					//if (my_strcmp (tempStr, "2660") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "266 cm");
					//if (my_strcmp (tempStr, "2960") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "296 cm");

					//sprintf (tempStr, "%s", DGPopUpGetItemText (dialogID, POPUP_WIDTH_EAST, DGPopUpGetSelected (dialogID, POPUP_WIDTH_EAST)).ToCStr ().Get ());
					//if (my_strcmp (tempStr, "����") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "");
					//if (my_strcmp (tempStr, "625") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "62.5 cm");
					//if (my_strcmp (tempStr, "750") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "75 cm");
					//if (my_strcmp (tempStr, "900") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "90 cm");
					//if (my_strcmp (tempStr, "1200") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "120 cm");
					//if (my_strcmp (tempStr, "1375") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "137.5 cm");
					//if (my_strcmp (tempStr, "1500") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "150 cm");
					//if (my_strcmp (tempStr, "2015") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "201.5 cm");
					//if (my_strcmp (tempStr, "2250") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "225 cm");
					//if (my_strcmp (tempStr, "2300") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "230 cm");
					//if (my_strcmp (tempStr, "2370") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "237 cm");
					//if (my_strcmp (tempStr, "2660") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "266 cm");
					//if (my_strcmp (tempStr, "2960") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "296 cm");

					//sprintf (tempStr, "%s", DGPopUpGetItemText (dialogID, POPUP_WIDTH_SOUTH, DGPopUpGetSelected (dialogID, POPUP_WIDTH_SOUTH)).ToCStr ().Get ());
					//if (my_strcmp (tempStr, "����") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "");
					//if (my_strcmp (tempStr, "625") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "62.5 cm");
					//if (my_strcmp (tempStr, "750") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "75 cm");
					//if (my_strcmp (tempStr, "900") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "90 cm");
					//if (my_strcmp (tempStr, "1200") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "120 cm");
					//if (my_strcmp (tempStr, "1375") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "137.5 cm");
					//if (my_strcmp (tempStr, "1500") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "150 cm");
					//if (my_strcmp (tempStr, "2015") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "201.5 cm");
					//if (my_strcmp (tempStr, "2250") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "225 cm");
					//if (my_strcmp (tempStr, "2300") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "230 cm");
					//if (my_strcmp (tempStr, "2370") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "237 cm");
					//if (my_strcmp (tempStr, "2660") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "266 cm");
					//if (my_strcmp (tempStr, "2960") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "296 cm");

					layerInd_vPost			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_VPOST);
					layerInd_hPost			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_HPOST);
					layerInd_SuppPost		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_SUPPORT);
					layerInd_Timber			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER);
					layerInd_GT24Girder		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_GIRDER);
					layerInd_BeamBracket	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_BEAM_BRACKET);
					layerInd_Yoke			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_YOKE);

					break;

				default:
					if (item == BUTTON_ADD) {
						if (placementInfoForPERI.nColVPost < 5) {
							placementInfoForPERI.nColVPost ++;
							DGRemoveDialogItems (dialogID, AFTER_ALL + 6);
							DGGrowDialog (dialogID, 130, 0);
						}
					} else if (item == BUTTON_DEL) {
						if (placementInfoForPERI.nColVPost > 1) {
							placementInfoForPERI.nColVPost --;
							DGRemoveDialogItems (dialogID, AFTER_ALL + 6);
							DGGrowDialog (dialogID, -130, 0);
						}
					}

					item = 0;

					if ((placementInfoForPERI.nColVPost >= 1) && (placementInfoForPERI.nColVPost <= 5)) {
						for (xx = 2 ; xx <= placementInfoForPERI.nColVPost ; ++xx) {
							// ������ �� �׸���
							itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 521-260+(130*xx), 245, 100, 1);
							DGShowItem (dialogID, itmIdx);
							itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 521-260+(130*xx), 415, 100, 1);
							DGShowItem (dialogID, itmIdx);

							itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 490-130+(130*xx), 230, 30, 30);
							DGShowItem (dialogID, itmIdx);
							itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 490-130+(130*xx), 400, 30, 30);
							DGShowItem (dialogID, itmIdx);
							itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 505-130+(130*xx), 261, 1, 138);
							DGShowItem (dialogID, itmIdx);

							// ������ ���� �ɼ� [���]
							itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 515-130+(130*xx), 290, 70, 25);
							HPOST_CENTER [xx-1] = itmIdx;
							DGShowItem (dialogID, itmIdx);
							DGSetItemText (dialogID, itmIdx, "������");
							itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 200, 10, 515-130+(130*xx), 315, 70, 25);
							DGShowItem (dialogID, itmIdx);
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "����");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "625");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "750");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "900");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "1200");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "1375");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "1500");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "2015");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "2250");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "2300");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "2370");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "2660");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "2960");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "����");
							itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 515-130+(130*xx), 340, 70, 25);
							DGShowItem (dialogID, itmIdx);

							// ������ ���� �ɼ� [����]
							itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 515-260+(130*xx)+25, 290-125, 70, 25);
							HPOST_UP [xx-1] = itmIdx;
							DGShowItem (dialogID, itmIdx);
							DGSetItemText (dialogID, itmIdx, "������");
							itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 200, 10, 515-260+(130*xx)+25, 315-125, 70, 25);
							DGShowItem (dialogID, itmIdx);
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "����");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "625");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "750");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "900");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "1200");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "1375");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "1500");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "2015");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "2250");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "2300");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "2370");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "2660");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "2960");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "����");
							itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 515-260+(130*xx)+25, 340-125, 70, 25);
							DGShowItem (dialogID, itmIdx);

							// ������ ���� �ɼ� [�Ʒ���]
							itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 515-260+(130*xx)+25, 290+125, 70, 25);
							HPOST_DOWN [xx-1] = itmIdx;
							DGShowItem (dialogID, itmIdx);
							DGSetItemText (dialogID, itmIdx, "������");
							itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 200, 10, 515-260+(130*xx)+25, 315+125, 70, 25);
							DGShowItem (dialogID, itmIdx);
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "����");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "625");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "750");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "900");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "1200");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "1375");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "1500");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "2015");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "2250");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "2300");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "2370");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "2660");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "2960");
							DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "����");
							itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 515-260+(130*xx)+25, 340+125, 70, 25);
							DGShowItem (dialogID, itmIdx);
						}
					}

					strcpy (tempStr, DGPopUpGetItemText (dialogID, POPUP_TYPE, DGPopUpGetSelected (dialogID, POPUP_TYPE)).ToCStr ().Get ());
					if (strncmp (tempStr, "���� ���ٸ�", strlen ("���� ���ٸ�")) == 0) {
						for (xx = 0 ; xx < placementInfoForPERI.nColVPost ; ++xx) {
							DGDisableItem (dialogID, HPOST_CENTER [xx]);
							DGDisableItem (dialogID, HPOST_CENTER [xx]+1);
						}
						for (xx = 1 ; xx < placementInfoForPERI.nColVPost ; ++xx) {
							DGDisableItem (dialogID, HPOST_UP [xx]);
							DGDisableItem (dialogID, HPOST_UP [xx]+1);
							DGDisableItem (dialogID, HPOST_DOWN [xx]);
							DGDisableItem (dialogID, HPOST_DOWN [xx]+1);
						}
					} else if (strncmp (tempStr, "PERI ���ٸ�", strlen ("PERI ���ٸ�")) == 0) {
						for (xx = 0 ; xx < placementInfoForPERI.nColVPost ; ++xx) {
							DGEnableItem (dialogID, HPOST_CENTER [xx]);
							(DGGetItemValLong (dialogID, HPOST_CENTER [xx])) ? DGEnableItem (dialogID, HPOST_CENTER [xx]+1) : DGDisableItem (dialogID, HPOST_CENTER [xx]+1);
						}
						for (xx = 1 ; xx < placementInfoForPERI.nColVPost ; ++xx) {
							DGEnableItem (dialogID, HPOST_UP [xx]);
							(DGGetItemValLong (dialogID, HPOST_UP [xx])) ? DGEnableItem (dialogID, HPOST_UP [xx]+1) : DGDisableItem (dialogID, HPOST_UP [xx]+1);
							DGEnableItem (dialogID, HPOST_DOWN [xx]);
							(DGGetItemValLong (dialogID, HPOST_DOWN [xx])) ? DGEnableItem (dialogID, HPOST_DOWN [xx]+1) : DGDisableItem (dialogID, HPOST_DOWN [xx]+1);
						}
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