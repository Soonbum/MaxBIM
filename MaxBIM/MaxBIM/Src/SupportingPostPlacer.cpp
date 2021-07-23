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
static short	layerInd_vPost;		// ���̾� ��ȣ: ������
static short	layerInd_hPost;		// ���̾� ��ȣ: ������

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

	// [���̾�α�] ������ �ܼ� (1/2��, ���̰� 6���� �ʰ��Ǹ� 2�� ������ ��?), �������� �԰�/����, ������ ����(��, ���̰� 3500 �̻��̸� �߰��� ���� ������ ��?), ������ �ʺ�, ũ�ν���� ����, ������/������ ���̾�
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32521, ACAPI_GetOwnResModule (), PERISupportingPostPlacerHandler1, 0);

	// ���� ���� ����
	API_Elem_Head* headList = new API_Elem_Head [1];
	headList [0] = elem.header;
	err = ACAPI_Element_Delete (&headList, 1);
	delete headList;

	// �Է��� �����͸� ������� ������, ������ ��ġ
	if (result == DG_OK) {
		PERI_VPost vpost;
		PERI_HPost hpost;

		// �����簡 ���� ���, �������� �԰ݿ� �°� ������
		if (placementInfoForPERI.bHPost == true) {
			// ����/������ ������ �԰ݿ� ���� ���� ������ ���� ���̰� �޶���
			if (my_strcmp (placementInfoForPERI.nomHPost_South, "296 cm") == 0)			infoMorph.width = 2.960;
			else if (my_strcmp (placementInfoForPERI.nomHPost_South, "266 cm") == 0)	infoMorph.width = 2.660;
			else if (my_strcmp (placementInfoForPERI.nomHPost_South, "237 cm") == 0)	infoMorph.width = 2.370;
			else if (my_strcmp (placementInfoForPERI.nomHPost_South, "230 cm") == 0)	infoMorph.width = 2.300;
			else if (my_strcmp (placementInfoForPERI.nomHPost_South, "225 cm") == 0)	infoMorph.width = 2.250;
			else if (my_strcmp (placementInfoForPERI.nomHPost_South, "201.5 cm") == 0)	infoMorph.width = 2.015;
			else if (my_strcmp (placementInfoForPERI.nomHPost_South, "150 cm") == 0)	infoMorph.width = 1.500;
			else if (my_strcmp (placementInfoForPERI.nomHPost_South, "137.5 cm") == 0)	infoMorph.width = 1.375;
			else if (my_strcmp (placementInfoForPERI.nomHPost_South, "120 cm") == 0)	infoMorph.width = 1.200;
			else if (my_strcmp (placementInfoForPERI.nomHPost_South, "90 cm") == 0)		infoMorph.width = 0.900;
			else if (my_strcmp (placementInfoForPERI.nomHPost_South, "75 cm") == 0)		infoMorph.width = 0.750;
			else if (my_strcmp (placementInfoForPERI.nomHPost_South, "62.5 cm") == 0)	infoMorph.width = 0.625;

			// ����/������ ������ �԰ݿ� ���� ���� ������ ���� ���̰� �޶���
			if (my_strcmp (placementInfoForPERI.nomHPost_West, "296 cm") == 0)			infoMorph.depth = 2.960;
			else if (my_strcmp (placementInfoForPERI.nomHPost_West, "266 cm") == 0)		infoMorph.depth = 2.660;
			else if (my_strcmp (placementInfoForPERI.nomHPost_West, "237 cm") == 0)		infoMorph.depth = 2.370;
			else if (my_strcmp (placementInfoForPERI.nomHPost_West, "230 cm") == 0)		infoMorph.depth = 2.300;
			else if (my_strcmp (placementInfoForPERI.nomHPost_West, "225 cm") == 0)		infoMorph.depth = 2.250;
			else if (my_strcmp (placementInfoForPERI.nomHPost_West, "201.5 cm") == 0)	infoMorph.depth = 2.015;
			else if (my_strcmp (placementInfoForPERI.nomHPost_West, "150 cm") == 0)		infoMorph.depth = 1.500;
			else if (my_strcmp (placementInfoForPERI.nomHPost_West, "137.5 cm") == 0)	infoMorph.depth = 1.375;
			else if (my_strcmp (placementInfoForPERI.nomHPost_West, "120 cm") == 0)		infoMorph.depth = 1.200;
			else if (my_strcmp (placementInfoForPERI.nomHPost_West, "90 cm") == 0)		infoMorph.depth = 0.900;
			else if (my_strcmp (placementInfoForPERI.nomHPost_West, "75 cm") == 0)		infoMorph.depth = 0.750;
			else if (my_strcmp (placementInfoForPERI.nomHPost_West, "62.5 cm") == 0)	infoMorph.depth = 0.625;
		}

		// ������ 1�� ��ġ
		// ȸ��Y(180), ũ�ν���� ��ġ(�ϴ�), ���� len_current��ŭ �̵��ؾ� ��
		vpost.leftBottomX = infoMorph.leftBottomX;
		vpost.leftBottomY = infoMorph.leftBottomY;
		vpost.leftBottomZ = infoMorph.leftBottomZ + placementInfoForPERI.heightVPost1;
		vpost.ang = infoMorph.ang;
		if (placementInfoForPERI.bVPost2 == false)
			vpost.bCrosshead = placementInfoForPERI.bCrosshead;
		else
			vpost.bCrosshead = false;
		sprintf (vpost.stType, "%s", placementInfoForPERI.nomVPost1);
		sprintf (vpost.crossheadType, "PERI");
		sprintf (vpost.posCrosshead, "�ϴ�");
		vpost.angCrosshead = 0.0;
		vpost.angY = DegreeToRad (180.0);
		vpost.len_current = placementInfoForPERI.heightVPost1;
		vpost.text2_onoff = true;
		vpost.text_onoff = true;
		vpost.bShowCoords = true;

		elemList.Push (placementInfoForPERI.placeVPost (vpost));	// ���ϴ�
		moveIn3D ('x', vpost.ang, infoMorph.width, &vpost.leftBottomX, &vpost.leftBottomY, &vpost.leftBottomZ);
		elemList.Push (placementInfoForPERI.placeVPost (vpost));	// ���ϴ�
		moveIn3D ('y', vpost.ang, infoMorph.depth, &vpost.leftBottomX, &vpost.leftBottomY, &vpost.leftBottomZ);
		elemList.Push (placementInfoForPERI.placeVPost (vpost));	// ����
		moveIn3D ('x', vpost.ang, -infoMorph.width, &vpost.leftBottomX, &vpost.leftBottomY, &vpost.leftBottomZ);
		elemList.Push (placementInfoForPERI.placeVPost (vpost));	// �»��

		// ������ 2���� ���� ���
		if (placementInfoForPERI.bVPost2 == true) {
			// ȸ��Y(0), ũ�ν���� ��ġ(���)
			vpost.leftBottomX = infoMorph.leftBottomX;
			vpost.leftBottomY = infoMorph.leftBottomY;
			vpost.leftBottomZ = infoMorph.leftBottomZ + placementInfoForPERI.heightVPost1;
			vpost.ang = infoMorph.ang;
			vpost.bCrosshead = placementInfoForPERI.bCrosshead;
			sprintf (vpost.stType, "%s", placementInfoForPERI.nomVPost2);
			sprintf (vpost.crossheadType, "PERI");
			sprintf (vpost.posCrosshead, "���");
			vpost.angCrosshead = 0.0;
			vpost.angY = 0.0;
			vpost.len_current = placementInfoForPERI.heightVPost2;
			vpost.text2_onoff = true;
			vpost.text_onoff = true;
			vpost.bShowCoords = true;

			elemList.Push (placementInfoForPERI.placeVPost (vpost));	// ���ϴ�
			moveIn3D ('x', vpost.ang, infoMorph.width, &vpost.leftBottomX, &vpost.leftBottomY, &vpost.leftBottomZ);
			elemList.Push (placementInfoForPERI.placeVPost (vpost));	// ���ϴ�
			moveIn3D ('y', vpost.ang, infoMorph.depth, &vpost.leftBottomX, &vpost.leftBottomY, &vpost.leftBottomZ);
			elemList.Push (placementInfoForPERI.placeVPost (vpost));	// ����
			moveIn3D ('x', vpost.ang, -infoMorph.width, &vpost.leftBottomX, &vpost.leftBottomY, &vpost.leftBottomZ);
			elemList.Push (placementInfoForPERI.placeVPost (vpost));	// �»��
		}

		// �����簡 ���� ���
		if (placementInfoForPERI.bHPost == true) {
			hpost.leftBottomX = infoMorph.leftBottomX;
			hpost.leftBottomY = infoMorph.leftBottomY;
			hpost.leftBottomZ = infoMorph.leftBottomZ;
			hpost.ang = infoMorph.ang;
			hpost.angX = 0.0;
			hpost.angY = 0.0;
			
			// 1�� ----------------------------------------------------------------------------------------------------
			// ����
			moveIn3D ('z', hpost.ang, 2.000, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
			moveIn3D ('x', hpost.ang, 0.050, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
			sprintf (hpost.stType, "%s", placementInfoForPERI.nomHPost_South);	// �� ���ڿ��̸� ��ġ���� ����
			if (my_strcmp (placementInfoForPERI.nomHPost_South, "") != 0)	elemList.Push (placementInfoForPERI.placeHPost (hpost));

			// ����
			moveIn3D ('x', hpost.ang, infoMorph.width - 0.050, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
			moveIn3D ('y', hpost.ang, 0.050, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
			hpost.ang = infoMorph.ang + DegreeToRad (90.0);
			sprintf (hpost.stType, "%s", placementInfoForPERI.nomHPost_East);	// �� ���ڿ��̸� ��ġ���� ����
			if (my_strcmp (placementInfoForPERI.nomHPost_East, "") != 0)	elemList.Push (placementInfoForPERI.placeHPost (hpost));
			hpost.ang = infoMorph.ang;

			// ����
			moveIn3D ('x', hpost.ang, -0.050, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
			moveIn3D ('y', hpost.ang, infoMorph.depth - 0.050, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
			hpost.ang = infoMorph.ang + DegreeToRad (180.0);
			sprintf (hpost.stType, "%s", placementInfoForPERI.nomHPost_North);	// �� ���ڿ��̸� ��ġ���� ����
			if (my_strcmp (placementInfoForPERI.nomHPost_North, "") != 0)	elemList.Push (placementInfoForPERI.placeHPost (hpost));
			hpost.ang = infoMorph.ang;

			// ����
			moveIn3D ('x', hpost.ang, 0.050 - infoMorph.width, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
			moveIn3D ('y', hpost.ang, -0.050, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
			hpost.ang = infoMorph.ang + DegreeToRad (270.0);
			sprintf (hpost.stType, "%s", placementInfoForPERI.nomHPost_West);	// �� ���ڿ��̸� ��ġ���� ����
			if (my_strcmp (placementInfoForPERI.nomHPost_West, "") != 0)	elemList.Push (placementInfoForPERI.placeHPost (hpost));
			hpost.ang = infoMorph.ang;

			// 2�� ----------------------------------------------------------------------------------------------------
			hpost.leftBottomX = infoMorph.leftBottomX;
			hpost.leftBottomY = infoMorph.leftBottomY;
			hpost.leftBottomZ = infoMorph.leftBottomZ;
			hpost.ang = infoMorph.ang;

			// ������ 1��/2�� ������ ���� 4000 �̻��� ���
			if (((placementInfoForPERI.bVPost1 * placementInfoForPERI.heightVPost1) + (placementInfoForPERI.bVPost2 * placementInfoForPERI.heightVPost2)) > 4.000) {
				// ����
				moveIn3D ('z', hpost.ang, 4.000, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
				moveIn3D ('x', hpost.ang, 0.050, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
				sprintf (hpost.stType, "%s", placementInfoForPERI.nomHPost_South);	// �� ���ڿ��̸� ��ġ���� ����
				if (my_strcmp (placementInfoForPERI.nomHPost_South, "") != 0)	elemList.Push (placementInfoForPERI.placeHPost (hpost));

				// ����
				moveIn3D ('x', hpost.ang, infoMorph.width - 0.050, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
				moveIn3D ('y', hpost.ang, 0.050, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
				hpost.ang = infoMorph.ang + DegreeToRad (90.0);
				sprintf (hpost.stType, "%s", placementInfoForPERI.nomHPost_East);	// �� ���ڿ��̸� ��ġ���� ����
				if (my_strcmp (placementInfoForPERI.nomHPost_East, "") != 0)	elemList.Push (placementInfoForPERI.placeHPost (hpost));
				hpost.ang = infoMorph.ang;

				// ����
				moveIn3D ('x', hpost.ang, -0.050, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
				moveIn3D ('y', hpost.ang, infoMorph.depth - 0.050, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
				hpost.ang = infoMorph.ang + DegreeToRad (180.0);
				sprintf (hpost.stType, "%s", placementInfoForPERI.nomHPost_North);	// �� ���ڿ��̸� ��ġ���� ����
				if (my_strcmp (placementInfoForPERI.nomHPost_North, "") != 0)	elemList.Push (placementInfoForPERI.placeHPost (hpost));
				hpost.ang = infoMorph.ang;

				// ����
				moveIn3D ('x', hpost.ang, 0.050 - infoMorph.width, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
				moveIn3D ('y', hpost.ang, -0.050, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
				hpost.ang = infoMorph.ang + DegreeToRad (270.0);
				sprintf (hpost.stType, "%s", placementInfoForPERI.nomHPost_West);	// �� ���ڿ��̸� ��ġ���� ����
				if (my_strcmp (placementInfoForPERI.nomHPost_West, "") != 0)	elemList.Push (placementInfoForPERI.placeHPost (hpost));
				hpost.ang = infoMorph.ang;
			}
		}

		// �׷�ȭ �ϱ�
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
	}

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

// ������ �ܼ� (1/2��, ���̰� 6���� �ʰ��Ǹ� 2�� ������ ��), �������� �԰�/����, ������ ����(��, ���̰� 3500 �̻��̸� �߰��� ���� ������ ��), ������ �ʺ�, ũ�ν���� ����, ������/������ ���̾ ����
short DGCALLBACK PERISupportingPostPlacerHandler1 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	char	tempStr [16];
	API_UCCallbackType	ucb;

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, "PERI ���ٸ� �ڵ� ��ġ");

			//////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			// ���� ��ư
			DGSetItemText (dialogID, DG_OK, "Ȯ ��");

			// ���� ��ư
			DGSetItemText (dialogID, DG_CANCEL, "�� ��");

			//////////////////////////////////////////////////////////// ������ ��ġ (������)
			DGSetItemText (dialogID, LABEL_VPOST, "������");				// ��: ������
			DGSetItemText (dialogID, LABEL_HPOST, "������");				// ��: ������

			DGSetItemText (dialogID, LABEL_TOTAL_HEIGHT, "�� ����");		// ��: �� ����
			DGSetItemText (dialogID, LABEL_REMAIN_HEIGHT, "���� ����");		// ��: ���� ����

			DGSetItemText (dialogID, CHECKBOX_CROSSHEAD, "ũ�ν����");		// üũ�ڽ�: ũ�ν����

			DGSetItemText (dialogID, CHECKBOX_VPOST1, "1��");				// üũ�ڽ�: 1��
			DGSetItemText (dialogID, LABEL_VPOST1_NOMINAL, "�԰�");			// ��: �԰�
			DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "����");			// ��: ����
			DGSetItemText (dialogID, CHECKBOX_VPOST2, "2��");				// üũ�ڽ�: 2��
			DGSetItemText (dialogID, LABEL_VPOST2_NOMINAL, "�԰�");			// ��: �԰�
			DGSetItemText (dialogID, LABEL_VPOST2_HEIGHT, "����");			// ��: ����

			DGSetItemText (dialogID, CHECKBOX_HPOST, "������");				// üũ�ڽ�: ������

			DGSetItemText (dialogID, LABEL_PLAN_WIDTH, "����");				// ��: ����
			DGSetItemText (dialogID, LABEL_PLAN_DEPTH, "����");				// ��: ����

			DGSetItemText (dialogID, LABEL_WIDTH_NORTH, "�ʺ�(��)");		// ��: �ʺ�(��)
			DGSetItemText (dialogID, LABEL_WIDTH_WEST, "�ʺ�(��)");			// ��: �ʺ�(��)
			DGSetItemText (dialogID, LABEL_WIDTH_EAST, "�ʺ�(��)");			// ��: �ʺ�(��)
			DGSetItemText (dialogID, LABEL_WIDTH_SOUTH, "�ʺ�(��)");		// ��: �ʺ�(��)

			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, "���纰 ���̾� ����");
			DGSetItemText (dialogID, LABEL_LAYER_VPOST, "������");
			DGSetItemText (dialogID, LABEL_LAYER_HPOST, "������");

			// üũ�ڽ�: ���̾� ����
			DGSetItemText (dialogID, CHECKBOX_LAYER_COUPLING, "���̾� ����");
			DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, TRUE);

			// ���� ��Ʈ�� �ʱ�ȭ
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_LAYER_VPOST;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_VPOST, 1);

			ucb.itemID	 = USERCONTROL_LAYER_HPOST;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_HPOST, 1);

			// �ʱ� ����
			// 1. ������ �԰� �˾� �߰� - MP 120, MP 250, MP 350, MP 480, MP 625
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
			
			// 2. ������ ���� �� ���� ���� ǥ�� - MP 120 (800~1200), MP 250 (1450~2500), MP 350 (1950~3500), MP 480 (2600~4800), MP 625 (4300~6250)
			DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 800~1200");
			DGSetItemText (dialogID, LABEL_VPOST2_HEIGHT, "H. 800~1200");

			// 3. ������ 1�� On, 2�� Off
			DGSetItemValLong (dialogID, CHECKBOX_VPOST1, TRUE);
			DGDisableItem (dialogID, CHECKBOX_VPOST1);
			DGSetItemValLong (dialogID, CHECKBOX_VPOST2, FALSE);
			DGDisableItem (dialogID, POPUP_VPOST2_NOMINAL);
			DGDisableItem (dialogID, EDITCONTROL_VPOST2_HEIGHT);

			// 4. �������� �ʺ� 4�� ��Ȱ��ȭ
			DGSetItemValLong (dialogID, CHECKBOX_HPOST, FALSE);
			DGDisableItem (dialogID, LABEL_WIDTH_NORTH);
			DGDisableItem (dialogID, LABEL_WIDTH_SOUTH);
			DGDisableItem (dialogID, LABEL_WIDTH_WEST);
			DGDisableItem (dialogID, LABEL_WIDTH_EAST);
			DGDisableItem (dialogID, POPUP_WIDTH_NORTH);
			DGDisableItem (dialogID, POPUP_WIDTH_SOUTH);
			DGDisableItem (dialogID, POPUP_WIDTH_WEST);
			DGDisableItem (dialogID, POPUP_WIDTH_EAST);

			// 5. �������� �ʺ� �˾� �߰� - ����, 625, 750, 900, 1200, 1375, 1500, 2015, 2250, 2300, 2370, 2660, 2960
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "����");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "625");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "750");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "900");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "1200");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "1375");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "1500");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "2015");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "2250");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "2300");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "2370");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "2660");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "2960");

			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "����");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "625");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "750");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "900");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "1200");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "1375");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "1500");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "2015");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "2250");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "2300");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "2370");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "2660");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "2960");

			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "����");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "625");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "750");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "900");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "1200");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "1375");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "1500");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "2015");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "2250");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "2300");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "2370");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "2660");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "2960");

			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "����");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "625");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "750");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "900");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "1200");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "1375");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "1500");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "2015");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "2250");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "2300");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "2370");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "2660");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "2960");

			// 6. ������ ���� �Է� ���� ����
			DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 0.800);
			DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 1.200);
			DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 0.800);
			DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 1.200);

			// 7. �� ����, ���� ���� ��Ȱ��ȭ �� �� ���
			DGDisableItem (dialogID, EDITCONTROL_TOTAL_HEIGHT);
			DGDisableItem (dialogID, EDITCONTROL_REMAIN_HEIGHT);
			DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_HEIGHT, infoMorph.height);
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HEIGHT, infoMorph.height - (DGGetItemValDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT) * DGGetItemValLong (dialogID, CHECKBOX_VPOST1) + DGGetItemValDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT) * DGGetItemValLong (dialogID, CHECKBOX_VPOST2)) - 0.003 * DGGetItemValLong (dialogID, CHECKBOX_CROSSHEAD));

			// 8. ������ ����, ���� �� ��Ȱ��ȭ �� �� ���
			DGDisableItem (dialogID, EDITCONTROL_PLAN_WIDTH);
			DGDisableItem (dialogID, EDITCONTROL_PLAN_DEPTH);
			DGSetItemValDouble (dialogID, EDITCONTROL_PLAN_WIDTH, infoMorph.width);
			DGSetItemValDouble (dialogID, EDITCONTROL_PLAN_DEPTH, infoMorph.depth);

			break;

		case DG_MSG_CHANGE:
			// 1. ���̾� ���� üũ�ڽ� On/Off�� ���� �̺�Ʈ ó��
			if (DGGetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING) == 1) {
				switch (item) {
					case USERCONTROL_LAYER_VPOST:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HPOST, DGGetItemValLong (dialogID, USERCONTROL_LAYER_VPOST));
						break;
					case USERCONTROL_LAYER_HPOST:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_VPOST, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HPOST));
						break;
				}
			}

			// 2. ������ 2���� �Ѱ� �� ������ ������ 2���� �԰�, ���� �Է� ��Ʈ���� Ȱ��ȭ/��Ȱ��ȭ
			if (DGGetItemValLong (dialogID, CHECKBOX_VPOST2) == TRUE) {
				DGEnableItem (dialogID, POPUP_VPOST2_NOMINAL);
				DGEnableItem (dialogID, EDITCONTROL_VPOST2_HEIGHT);
			} else {
				DGDisableItem (dialogID, POPUP_VPOST2_NOMINAL);
				DGDisableItem (dialogID, EDITCONTROL_VPOST2_HEIGHT);
			}

			// 3. ������ �԰��� �ٲ� ������ ���� ���� ���ڿ��� ����ǰ�, ������ ���� ���� �ּ�/�ִ밪 ����� - MP 120 (800~1200), MP 250 (1450~2500), MP 350 (1950~3500), MP 480 (2600~4800), MP 625 (4300~6250)
			if (DGPopUpGetSelected (dialogID, POPUP_VPOST1_NOMINAL) == 1) {
				DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 800~1200");
				DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 0.800);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 1.200);
			} else if (DGPopUpGetSelected (dialogID, POPUP_VPOST1_NOMINAL) == 2) {
				DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 1450~2500");
				DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 1.450);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 2.500);
			} else if (DGPopUpGetSelected (dialogID, POPUP_VPOST1_NOMINAL) == 3) {
				DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 1950~3500");
				DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 1.950);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 3.500);
			} else if (DGPopUpGetSelected (dialogID, POPUP_VPOST1_NOMINAL) == 4) {
				DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 2600~4800");
				DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 2.600);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 4.800);
			} else if (DGPopUpGetSelected (dialogID, POPUP_VPOST1_NOMINAL) == 5) {
				DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 4300~6250");
				DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 4.300);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 6.250);
			}

			if (DGPopUpGetSelected (dialogID, POPUP_VPOST2_NOMINAL) == 1) {
				DGSetItemText (dialogID, LABEL_VPOST2_HEIGHT, "H. 800~1200");
				DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 0.800);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 1.200);
			} else if (DGPopUpGetSelected (dialogID, POPUP_VPOST2_NOMINAL) == 2) {
				DGSetItemText (dialogID, LABEL_VPOST2_HEIGHT, "H. 1450~2500");
				DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 1.450);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 2.500);
			} else if (DGPopUpGetSelected (dialogID, POPUP_VPOST2_NOMINAL) == 3) {
				DGSetItemText (dialogID, LABEL_VPOST2_HEIGHT, "H. 1950~3500");
				DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 1.950);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 3.500);
			} else if (DGPopUpGetSelected (dialogID, POPUP_VPOST2_NOMINAL) == 4) {
				DGSetItemText (dialogID, LABEL_VPOST2_HEIGHT, "H. 2600~4800");
				DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 2.600);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 4.800);
			} else if (DGPopUpGetSelected (dialogID, POPUP_VPOST2_NOMINAL) == 5) {
				DGSetItemText (dialogID, LABEL_VPOST2_HEIGHT, "H. 4300~6250");
				DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 4.300);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 6.250);
			}

			// 4. ������ �԰��� �ٲ�ų�, ������ 1��/2�� üũ�ڽ� ���°� �ٲ�ų�, ������ ���̰� �ٲ�ų�, ũ�ν���� üũ ���¿� ���� ���� ���� �����
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HEIGHT, infoMorph.height - (DGGetItemValDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT) * DGGetItemValLong (dialogID, CHECKBOX_VPOST1) + DGGetItemValDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT) * DGGetItemValLong (dialogID, CHECKBOX_VPOST2)) - 0.003 * DGGetItemValLong (dialogID, CHECKBOX_CROSSHEAD));

			// 5. ������ üũ�ڽ��� ���� ������ UI�� �ʺ� ���� ���� (����, ����) Ȱ��ȭ/��Ȱ��ȭ
			if (DGGetItemValLong (dialogID, CHECKBOX_HPOST) == TRUE) {
				DGEnableItem (dialogID, LABEL_WIDTH_WEST);
				DGEnableItem (dialogID, POPUP_WIDTH_WEST);
				DGEnableItem (dialogID, LABEL_WIDTH_SOUTH);
				DGEnableItem (dialogID, POPUP_WIDTH_SOUTH);
			} else {
				DGDisableItem (dialogID, LABEL_WIDTH_WEST);
				DGDisableItem (dialogID, POPUP_WIDTH_WEST);
				DGDisableItem (dialogID, LABEL_WIDTH_SOUTH);
				DGDisableItem (dialogID, POPUP_WIDTH_SOUTH);
			}

			// 6. ������ ����/���� �ʺ� �ٲ�� ����/���� �ʺ� �����ϰ� �����
			DGPopUpSelectItem (dialogID, POPUP_WIDTH_EAST, DGPopUpGetSelected (dialogID, POPUP_WIDTH_WEST));
			DGPopUpSelectItem (dialogID, POPUP_WIDTH_NORTH, DGPopUpGetSelected (dialogID, POPUP_WIDTH_SOUTH));

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:

					if (DGGetItemValLong (dialogID, CHECKBOX_VPOST1) == TRUE)
						placementInfoForPERI.bVPost1 = true;
					else
						placementInfoForPERI.bVPost1 = false;
					sprintf (placementInfoForPERI.nomVPost1, "%s", DGPopUpGetItemText (dialogID, POPUP_VPOST1_NOMINAL, DGPopUpGetSelected (dialogID, POPUP_VPOST1_NOMINAL)).ToCStr ().Get ());
					placementInfoForPERI.heightVPost1 = DGGetItemValDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT);
					
					if (DGGetItemValLong (dialogID, CHECKBOX_VPOST2) == TRUE)
						placementInfoForPERI.bVPost2 = true;
					else
						placementInfoForPERI.bVPost2 = false;
					sprintf (placementInfoForPERI.nomVPost2, "%s", DGPopUpGetItemText (dialogID, POPUP_VPOST2_NOMINAL, DGPopUpGetSelected (dialogID, POPUP_VPOST2_NOMINAL)).ToCStr ().Get ());
					placementInfoForPERI.heightVPost2 = DGGetItemValDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT);

					if (DGGetItemValLong (dialogID, CHECKBOX_CROSSHEAD) == TRUE)
						placementInfoForPERI.bCrosshead = true;
					else
						placementInfoForPERI.bCrosshead = false;

					if (DGGetItemValLong (dialogID, CHECKBOX_HPOST) == TRUE)
						placementInfoForPERI.bHPost = true;
					else
						placementInfoForPERI.bHPost = false;

					sprintf (tempStr, "%s", DGPopUpGetItemText (dialogID, POPUP_WIDTH_NORTH, DGPopUpGetSelected (dialogID, POPUP_WIDTH_NORTH)).ToCStr ().Get ());
					if (my_strcmp (tempStr, "����") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "");
					if (my_strcmp (tempStr, "625") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "62.5 cm");
					if (my_strcmp (tempStr, "750") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "75 cm");
					if (my_strcmp (tempStr, "900") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "90 cm");
					if (my_strcmp (tempStr, "1200") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "120 cm");
					if (my_strcmp (tempStr, "1375") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "137.5 cm");
					if (my_strcmp (tempStr, "1500") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "150 cm");
					if (my_strcmp (tempStr, "2015") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "201.5 cm");
					if (my_strcmp (tempStr, "2250") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "225 cm");
					if (my_strcmp (tempStr, "2300") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "230 cm");
					if (my_strcmp (tempStr, "2370") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "237 cm");
					if (my_strcmp (tempStr, "2660") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "266 cm");
					if (my_strcmp (tempStr, "2960") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "296 cm");

					sprintf (tempStr, "%s", DGPopUpGetItemText (dialogID, POPUP_WIDTH_WEST, DGPopUpGetSelected (dialogID, POPUP_WIDTH_WEST)).ToCStr ().Get ());
					if (my_strcmp (tempStr, "����") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "");
					if (my_strcmp (tempStr, "625") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "62.5 cm");
					if (my_strcmp (tempStr, "750") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "75 cm");
					if (my_strcmp (tempStr, "900") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "90 cm");
					if (my_strcmp (tempStr, "1200") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "120 cm");
					if (my_strcmp (tempStr, "1375") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "137.5 cm");
					if (my_strcmp (tempStr, "1500") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "150 cm");
					if (my_strcmp (tempStr, "2015") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "201.5 cm");
					if (my_strcmp (tempStr, "2250") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "225 cm");
					if (my_strcmp (tempStr, "2300") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "230 cm");
					if (my_strcmp (tempStr, "2370") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "237 cm");
					if (my_strcmp (tempStr, "2660") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "266 cm");
					if (my_strcmp (tempStr, "2960") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "296 cm");

					sprintf (tempStr, "%s", DGPopUpGetItemText (dialogID, POPUP_WIDTH_EAST, DGPopUpGetSelected (dialogID, POPUP_WIDTH_EAST)).ToCStr ().Get ());
					if (my_strcmp (tempStr, "����") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "");
					if (my_strcmp (tempStr, "625") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "62.5 cm");
					if (my_strcmp (tempStr, "750") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "75 cm");
					if (my_strcmp (tempStr, "900") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "90 cm");
					if (my_strcmp (tempStr, "1200") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "120 cm");
					if (my_strcmp (tempStr, "1375") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "137.5 cm");
					if (my_strcmp (tempStr, "1500") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "150 cm");
					if (my_strcmp (tempStr, "2015") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "201.5 cm");
					if (my_strcmp (tempStr, "2250") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "225 cm");
					if (my_strcmp (tempStr, "2300") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "230 cm");
					if (my_strcmp (tempStr, "2370") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "237 cm");
					if (my_strcmp (tempStr, "2660") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "266 cm");
					if (my_strcmp (tempStr, "2960") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "296 cm");

					sprintf (tempStr, "%s", DGPopUpGetItemText (dialogID, POPUP_WIDTH_SOUTH, DGPopUpGetSelected (dialogID, POPUP_WIDTH_SOUTH)).ToCStr ().Get ());
					if (my_strcmp (tempStr, "����") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "");
					if (my_strcmp (tempStr, "625") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "62.5 cm");
					if (my_strcmp (tempStr, "750") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "75 cm");
					if (my_strcmp (tempStr, "900") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "90 cm");
					if (my_strcmp (tempStr, "1200") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "120 cm");
					if (my_strcmp (tempStr, "1375") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "137.5 cm");
					if (my_strcmp (tempStr, "1500") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "150 cm");
					if (my_strcmp (tempStr, "2015") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "201.5 cm");
					if (my_strcmp (tempStr, "2250") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "225 cm");
					if (my_strcmp (tempStr, "2300") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "230 cm");
					if (my_strcmp (tempStr, "2370") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "237 cm");
					if (my_strcmp (tempStr, "2660") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "266 cm");
					if (my_strcmp (tempStr, "2960") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "296 cm");

					layerInd_vPost	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_VPOST);
					layerInd_hPost	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_HPOST);

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