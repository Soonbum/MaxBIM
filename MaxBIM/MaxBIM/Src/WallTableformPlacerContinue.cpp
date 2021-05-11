#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "WallTableformPlacerContinue.hpp"

//namespace wallTableformPlacerContinueDG;

static InfoWall						infoWall;		// �� ��ü ����

// ���� �������� ���̺����� ��ġ�ϴ� ���� ��ƾ
GSErrCode	placeTableformOnWallContinually (void)
{
	GSErrCode	err = NoError;
	short		xx, yy;
	double		dx, dy;

	// Selection Manager ���� ����
	long		nSel;
	API_SelectionInfo		selectionInfo;
	API_Element				tElem;
	API_Neig				**selNeigs;
	GS::Array<API_Guid>&	walls = GS::Array<API_Guid> ();
	GS::Array<API_Guid>&	morphs = GS::Array<API_Guid> ();
	GS::Array<API_Guid>&	polylines = GS::Array<API_Guid> ();
	long					nWalls = 0;
	long					nMorphs = 0;
	long					nPolylines = 0;

	// ��ü ���� ��������
	API_Element				elem;
	API_ElementMemo			memo;
	API_ElemInfo3D			info3D;

	// ���� ��ü ����
	InfoMorphForWallTableformContinue	infoMorph;

	// �۾� �� ����
	API_StoryInfo	storyInfo;
	double			workLevel_wall;		// ���� �۾� �� ����

	// ���� 3D ������� ��������
	API_Component3D			component;
	API_Tranmat				tm;
	Int32					nVert, nEdge, nPgon;
	Int32					elemIdx, bodyIdx;
	API_Coord3D				trCoord;
	GS::Array<API_Coord3D>&	coords = GS::Array<API_Coord3D> ();

	// �������� ���ܵǴ� �� 4��
	API_Coord3D		excludeP1;
	API_Coord3D		excludeP2;
	API_Coord3D		excludeP3;
	API_Coord3D		excludeP4;

	excludeP1.x = 0;	excludeP1.y = 0;	excludeP1.z = 0;
	excludeP2.x = 1;	excludeP2.y = 0;	excludeP2.z = 0;
	excludeP3.x = 0;	excludeP3.y = 1;	excludeP3.z = 0;
	excludeP4.x = 0;	excludeP4.y = 0;	excludeP4.z = 1;

	// ��Ÿ
	char	buffer [256];


	// ������ ��� ��������
	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err == APIERR_NOPLAN) {
		ACAPI_WriteReport ("���� ������Ʈ â�� �����ϴ�.", true);
	}
	if (err == APIERR_NOSEL) {
		ACAPI_WriteReport ("�ƹ� �͵� �������� �ʾҽ��ϴ�.\n�ʼ� ����: �� (1��), �� �Ϻθ� ���� ���� (1��), ���� ���δ� �������� (������)", true);
	}
	if (err != NoError) {
		BMKillHandle ((GSHandle *) &selNeigs);
		return err;
	}

	if (selectionInfo.typeID != API_SelEmpty) {
		nSel = BMGetHandleSize ((GSHandle) selNeigs) / sizeof (API_Neig);
		for (xx = 0 ; xx < nSel && err == NoError ; ++xx) {
			tElem.header.typeID = Neig_To_ElemID ((*selNeigs)[xx].neigID);

			tElem.header.guid = (*selNeigs)[xx].guid;
			if (ACAPI_Element_Get (&tElem) != NoError)	// ������ �� �ִ� ����ΰ�?
				continue;

			if (tElem.header.typeID == API_WallID)		// ���ΰ�?
				walls.Push (tElem.header.guid);

			if (tElem.header.typeID == API_MorphID)		// �����ΰ�?
				morphs.Push (tElem.header.guid);

			if (tElem.header.typeID == API_PolyLineID)	// ���������ΰ�?
				polylines.Push (tElem.header.guid);
		}
	}
	BMKillHandle ((GSHandle *) &selNeigs);
	nWalls = walls.GetSize ();
	nMorphs = morphs.GetSize ();
	nPolylines = polylines.GetSize ();

	// ���� 1���ΰ�?
	if (nWalls != 1) {
		ACAPI_WriteReport ("���� 1�� �����ؾ� �մϴ�.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// ������ 1���ΰ�?
	if (nMorphs != 1) {
		ACAPI_WriteReport ("���� �Ϻθ� ���� ������ 1�� �����ؾ� �մϴ�.\n������ ���̺���/�������� ���� ���� ������ ǥ���ؾ� �մϴ�.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// ���������� 2�� �̻��ΰ�?
	if ( !(nPolylines >= 2) ) {
		ACAPI_WriteReport ("���� ���δ� ���������� �ּ� 2�� �̻� �����ؾ� �մϴ�.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// (1) �� ������ ������
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	elem.header.guid = walls.Pop ();
	err = ACAPI_Element_Get (&elem);						// elem.wall.poly.nCoords : ������ ���� ������ �� ����
	err = ACAPI_Element_GetMemo (elem.header.guid, &memo);	// memo.coords : ������ ��ǥ�� ������ �� ����
	
	if (elem.wall.thickness != elem.wall.thickness1) {
		ACAPI_WriteReport ("���� �β��� �����ؾ� �մϴ�.", true);
		err = APIERR_GENERAL;
		return err;
	}
	infoWall.wallThk		= elem.wall.thickness;
	infoWall.floorInd		= elem.header.floorInd;
	infoWall.bottomOffset	= elem.wall.bottomOffset;
	infoWall.begX			= elem.wall.begC.x;
	infoWall.begY			= elem.wall.begC.y;
	infoWall.endX			= elem.wall.endC.x;
	infoWall.endY			= elem.wall.endC.y;

	ACAPI_DisposeElemMemoHdls (&memo);

	// (2) ���� ������ ������
	BNZeroMemory (&elem, sizeof (API_Element));
	elem.header.guid = morphs.Pop ();
	err = ACAPI_Element_Get (&elem);
	err = ACAPI_Element_Get3DInfo (elem.header, &info3D);

	// ���� ������ ���� ������(������ ���� ������) �ߴ�
	if (abs (info3D.bounds.zMax - info3D.bounds.zMin) < EPS) {
		ACAPI_WriteReport ("������ ������ ���� �ʽ��ϴ�.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// ������ GUID ����
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

	// ���� ������ ���� ���� ���� ������Ʈ
	//placingZone.leftBottomX		= infoMorph.leftBottomX;
	//placingZone.leftBottomY		= infoMorph.leftBottomY;
	//placingZone.leftBottomZ		= infoMorph.leftBottomZ;
	//placingZone.horLen			= infoMorph.horLen;
	//placingZone.verLen			= infoMorph.verLen;
	//placingZone.ang				= DegreeToRad (infoMorph.ang);
	
	// �۾� �� ���� �ݿ� -- ����
	BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	workLevel_wall = 0.0;
	ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
	for (xx = 0 ; xx < (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
		if (storyInfo.data [0][xx].index == infoWall.floorInd) {
			workLevel_wall = storyInfo.data [0][xx].level;
			break;
		}
	}
	BMKillHandle ((GSHandle *) &storyInfo.data);

	// ���� ������ �� ������ ����
	//placingZone.leftBottomZ = infoWall.bottomOffset;

	// ���� ���� ����
	API_Elem_Head* headList = new API_Elem_Head [1];
	headList [0] = elem.header;
	err = ACAPI_Element_Delete (&headList, 1);
	delete headList;


	// [�Ϸ�] ������ �̿��� ��/�� ���� ���� ������ -> ����ü InfoMorphForWallTableform Ȱ��

	// ���� ���� ������ ����
	// 1. ���������� ȸ����Ŵ (X, Y���� ȸ������ �ʾҴٰ� �����ϵ���) -> �Լ� getUnrotatedPoint Ȱ��
		// ȸ������ �Ǵ� ���ϴ� ���� ��� ã�°�?
	// 2. ��� ���������� ��ĵ�ϸ鼭 x�� vector, y�� vector�� �ϴ� �����ϰ�
	// 3. vector�� ������ ��, �ߺ��� ���� ��� �Ұ�
	// 4. ������ x, y���� API_Coord�� ���� -> �� ����

	// ���� �߿��� �� �� ���� ���� �� �Ұ�
	// ... ���� �Լ� �ʿ�
	// 1. �� ���� -> �������ε� ���� ���� : �ش� ���� �� �� �������ο� ���ߴ��� ã�Ƽ� �������� ��ȣ ������ ��

	// �������� ���� �� �� �Ÿ� ����, �� �β��� ������ ���� ã�Ƽ� �����ϰ� x�� ��ġ ������������ ����
	// 1. ���� �ݺ����� ���� ����i, ����j ���� �� �Ÿ� ���� -> �� �β��� ������ ���� ã�´�.
	// 2. �� ���� ���� (�� ������ ���� �پ� �ִ� ����)
	// 3. ������ ���� x�� ��ġ �������� �������� ����, �ߺ��� ���� ��� �Ұ�

	// ��-�������� ��ȸ
	// ���� ���� ���� ���������� ���� ���� ���� ��, ���� ���� ���� ���������� ���� ���� ������ --> �� ���� O (���� ���� flag�� true�� set)
	// ���� ���� ���� ���������� ���� ���� ���� ��, ���� ���� ���� ���������� ���� ���� �ٸ��� --> �� ���� X (���� ���� flag�� false�� set)

	// �� ���� ����
	// �� ��ȸ: ���������� ������ �� ������ ��� ��ü�� �ϳ��� ������ ����

	//if (nMorphs > 0) {
	//	for (xx = 0 ; xx < nMorphs ; ++xx) {
	//		// ������ ������ ������
	//		BNZeroMemory (&elem, sizeof (API_Element));
	//		elem.header.guid = morphs.Pop ();
	//		err = ACAPI_Element_Get (&elem);
	//		err = ACAPI_Element_Get3DInfo (elem.header, &info3D);

	//		// ������ �� ��ǥ���� ������
	//		BNZeroMemory (&component, sizeof (API_Component3D));
	//		component.header.typeID = API_BodyID;
	//		component.header.index = info3D.fbody;
	//		err = ACAPI_3D_GetComponent (&component);

	//		nVert = component.body.nVert;
	//		nEdge = component.body.nEdge;
	//		nPgon = component.body.nPgon;
	//		tm = component.body.tranmat;
	//		elemIdx = component.body.head.elemIndex - 1;
	//		bodyIdx = component.body.head.bodyIndex - 1;
	//
	//		// ���� ��ǥ�� ������
	//		for (yy = 1 ; yy <= nVert ; ++yy) {
	//			component.header.typeID	= API_VertID;
	//			component.header.index	= yy;
	//			err = ACAPI_3D_GetComponent (&component);
	//			if (err == NoError) {
	//				trCoord.x = tm.tmx[0]*component.vert.x + tm.tmx[1]*component.vert.y + tm.tmx[2]*component.vert.z + tm.tmx[3];
	//				trCoord.y = tm.tmx[4]*component.vert.x + tm.tmx[5]*component.vert.y + tm.tmx[6]*component.vert.z + tm.tmx[7];
	//				trCoord.z = tm.tmx[8]*component.vert.x + tm.tmx[9]*component.vert.y + tm.tmx[10]*component.vert.z + tm.tmx[11];
	//				coords.Push (trCoord);

	//				sprintf (buffer, "%d ", yy);

	//				if ( !(isSamePoint (excludeP1, trCoord) || isSamePoint (excludeP2, trCoord) || isSamePoint (excludeP3, trCoord) || isSamePoint (excludeP4, trCoord)) ) {
	//					placeCoordinateLabel (trCoord.x, trCoord.y, trCoord.z, true, buffer);
	//				}
	//			}
	//		}
	//	}
	//}

	//if (nPolylines > 0) {
	//	for (xx = 0 ; xx < nPolylines ; ++xx) {
	//		// ���������� ������ ������
	//		BNZeroMemory (&elem, sizeof (API_Element));
	//		BNZeroMemory (&memo, sizeof (API_ElementMemo));
	//		elem.header.guid = polylines.Pop ();
	//		err = ACAPI_Element_Get (&elem);
	//		err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

	//		// ���� ��ǥ�� ������
	//		for (yy = 1 ; yy <= elem.polyLine.poly.nCoords ; ++yy) {
	//			sprintf (buffer, "%d ", yy);
	//			err = placeCoordinateLabel (memo.coords [0][yy].x, memo.coords [0][yy].y, 0, true, buffer);
	//		}
	//	}
	//}

	return err;
}
