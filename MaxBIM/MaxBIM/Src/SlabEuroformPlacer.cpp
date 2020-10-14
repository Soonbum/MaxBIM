#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "SlabEuroformPlacer.hpp"


// 2�� �޴�: ������ �Ϻο� �������� ��ġ�ϴ� ���� ��ƾ
GSErrCode	placeEuroformOnSlabBottom (void)
{
	GSErrCode	err = NoError;
	long		nSel;
	short		xx, yy;
	double		dx, dy, ang;

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

	// �� �Է�
	API_GetPointType		pointInfo;
	API_Coord3D				point1, point2;
	API_Coord3D				tempPoint, resultPoint;
	GS::Array<API_Coord3D>&	coordsRotated = GS::Array<API_Coord3D> ();

	// ���� ��ü ����
	InfoMorphForSlab		infoMorph;

	// �۾� �� ����
	API_StoryInfo			storyInfo;
	double					plusLevel;


	// ������ ��� ��������
	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err == APIERR_NOPLAN) {
		ACAPI_WriteReport ("���� ������Ʈ â�� �����ϴ�.", true);
		return err;
	}
	if (err == APIERR_NOSEL) {
		ACAPI_WriteReport ("�ƹ� �͵� �������� �ʾҽ��ϴ�.\n�ʼ� ����: ������ �Ϻθ� ���� ���� (1��)", true);
		return err;
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
		ACAPI_WriteReport ("������ �Ϻθ� ���� ������ 1�� �����ϼž� �մϴ�.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// ���� ������ ������
	BNZeroMemory (&elem, sizeof (API_Element));
	elem.header.guid = morphs.Pop ();
	err = ACAPI_Element_Get (&elem);
	err = ACAPI_Element_Get3DInfo (elem.header, &info3D);

	// ���� ������ ���� �־�� ��
	if (abs (info3D.bounds.zMax - info3D.bounds.zMin) > EPS) {
		ACAPI_WriteReport ("������ ���� ���� �ʽ��ϴ�.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// ������ GUID ����
	infoMorph.guid = elem.header.guid;

	// ������ 3D �ٵ� ������
	BNZeroMemory (&component, sizeof (API_Component3D));
	component.header.typeID = API_BodyID;
	component.header.index = info3D.fbody;
	err = ACAPI_3D_GetComponent (&component);

	// ������ 3D ���� �������� ���ϸ� ����
	if (err != NoError) {
		ACAPI_WriteReport ("������ 3D ���� �������� ���߽��ϴ�.", true);
		return err;
	}

	nVert = component.body.nVert;
	nEdge = component.body.nEdge;
	nPgon = component.body.nPgon;
	tm = component.body.tranmat;
	elemIdx = component.body.head.elemIndex - 1;
	bodyIdx = component.body.head.bodyIndex - 1;
	
	// ���� ��ǥ�� ���� ������� ������
	for (xx = 1 ; xx <= nVert ; ++xx) {
		component.header.typeID	= API_VertID;
		component.header.index	= xx;
		err = ACAPI_3D_GetComponent (&component);
		if (err == NoError) {
			trCoord.x = tm.tmx[0]*component.vert.x + tm.tmx[1]*component.vert.y + tm.tmx[2]*component.vert.z + tm.tmx[3];
			trCoord.y = tm.tmx[4]*component.vert.x + tm.tmx[5]*component.vert.y + tm.tmx[6]*component.vert.z + tm.tmx[7];
			trCoord.z = tm.tmx[8]*component.vert.x + tm.tmx[9]*component.vert.y + tm.tmx[10]*component.vert.z + tm.tmx[11];
			if (abs (trCoord.z - elem.morph.level) < EPS)
				coords.Push (trCoord);
		}
	}

	// �ϴ� �� 2���� Ŭ��
	BNZeroMemory (&pointInfo, sizeof (API_GetPointType));
	CHCopyC ("���ϴ� ���� Ŭ���Ͻʽÿ�.", pointInfo.prompt);
	pointInfo.enableQuickSelection = true;
	err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
	point1 = pointInfo.pos;

	BNZeroMemory (&pointInfo, sizeof (API_GetPointType));
	CHCopyC ("���ϴ� ���� Ŭ���Ͻʽÿ�.", pointInfo.prompt);
	pointInfo.enableQuickSelection = true;
	err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
	point2 = pointInfo.pos;
	
	// ������ ȸ�������� ����
	dx = point2.x - point1.x;
	dy = point2.y - point1.y;
	ang = RadToDegree (atan2 (dy, dx));

	// ȸ������ 0�� ���� ��ǥ�� �����
	for (xx = 0 ; coords.GetSize () ; ++xx) {
		tempPoint = coords.Pop ();
		resultPoint.x = point1.x + ((tempPoint.x - point1.x)*cos(DegreeToRad (-ang)) - (tempPoint.y - point1.y)*sin(DegreeToRad (-ang)));
		resultPoint.y = point1.y + ((tempPoint.x - point1.x)*sin(DegreeToRad (-ang)) + (tempPoint.y - point1.y)*cos(DegreeToRad (-ang)));
		resultPoint.z = tempPoint.z;

		coordsRotated.Push (resultPoint);
	}

	// ������ ���� �迭�� ������
	API_Coord3D	*nodeCoords = new API_Coord3D [coordsRotated.GetSize ()];

	for (xx = 0 ; xx < coordsRotated.GetSize () ; ++xx)
		nodeCoords [xx] = coordsRotated.Pop ();

	// ������ �� ������� ������ ��
	// ...

	delete nodeCoords;

	// ... ������ �� ������� ������ �� (1,2�� ���� �ٷ� ����)

	// ... ������ �ʺ�, ���̸� ���ؾ� �Ѵ�.




	/*
		* ��� ����(3����): ������(ȸ��X: 0��, �������� ����), ����(����: 90��), ����(��ġ����: �ٴڴ�����)
		
		3. �۾� �� ���� �ݿ�
		4. ��� ���� ������ Z ������ ����
		5. ����� �Է� (1��)
			- �⺻ ��ġ �� ����
				: �԰��� ���� -- �ʺ�, ���� (������ ����ڰ� ���� ���� ���ϴ� ���� �������� ��)
				: ��ġ������ � �������� ���� ���ΰ�? - ������ ó�� ���� ��(TMX �� �ν�)�� ���ϴ� ���̶�� ������
				: ���ϴ� ���� ����̳� ���� ������ ������ ��� �� ���ΰ�? -> ������ ���̸� �����ؼ� ó�� �� ���� �Ʒ��� �����̶�� ����. -> �Ʒ��� �� ������ ����/�� ���� �������� ������ ȹ��
			- ���纰 ���̾� ���� (������, ����, ����)
			- ������ ���� ���� �پ� �־ ��
			- ���� ������ ��ġ �� ��ư�� �־�� ��
		6. ����� �Է� (2��)
			- Ÿ��Ʋ: ����/���� ä���
			- ��ư: 1. ���� ���� Ȯ�� // 2. ��  ġ // 3. ������
			- ��ġ ��ư�� ������ (x���� y���� ��ư�� �� ũ�⸦ �����ϸ�? - �ʺ� �ٲٸ� y�� ��ü�� ������ ��, ���̸� �ٲٸ� x�� ��ü�� ������ ��)
			- ���� ����/���� ����: ǥ�ø� �� (���� �� ��ģ ��): ���� ���� ���� ǥ�� (150~300mm), ���� �������� �ƴ��� �۲÷� ǥ��
			- ��� ���� �ʺ� ������ �� �־�� �� (��/�Ʒ� ���, ����/������ ��� - ����ڰ� ����.. �׿� ���� ������ ��ü �迭�� �̵���)
			- ��ġ ��ư ��濡�� ��ư ���̸��� ���� ���縦 ���� ���θ� ������ �� �־�� �� (��ư ���)
		7. ���� ��ġ
			- (1) �������� ���� ���͸� �������� ��ġ�� ��
			- (2) ����(11.5T, ����Ʋ Off)�� ������ �°� ��ġ�ϵ�, ��ġ�� �κ��� ���Ⱑ �ʿ���! (���̵� ����)
			- (3) ���ʿ� ���縦 �� �� (Z���� �β��� 50�̾�� ��, �ʺ�� 80)
			- (4) ���� �����絵 �� �� (���� ����� �β�, �ʺ�� ���� - ���̴� ����� ������ ����)
	*/

	return	err;
}