#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "SlabEuroformPlacer.hpp"

using namespace slabBottomPlacerDG;

static SlabPlacingZone	placingZone;			// �⺻ ������ �Ϻ� ���� ����
static short			layerInd_Euroform;		// ���̾� ��ȣ: ������
static short			layerInd_Plywood;		// ���̾� ��ȣ: ����
static short			layerInd_Wood;			// ���̾� ��ȣ: ����


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

	// ���� ��ü ����
	InfoMorphForSlab		infoMorph;

	// �� �Է�
	API_GetPointType		pointInfo;
	API_Coord3D				point1, point2;
	API_Coord3D				tempPoint, resultPoint;
	
	// ������ ���� �迭�� �����ϰ� ������� ��ǥ ���� ��
	API_Coord3D		nodes_random [20];
	API_Coord3D		nodes_sequential [20];
	long			nNodes;		// ���� �������� ���� ��ǥ ����
	long			nEntered;
	bool			bFindPoint;
	API_Coord3D		bufPoint;
	short			result;

	// �۾� �� ����
	API_StoryInfo	storyInfo;
	double			workLevel_morph;

	// �ڳ� ��ǥ�� ���ϱ� ���� �ֿܰ� ��ǥ �ӽ� ����
	API_Coord3D		outer_leftTop;
	API_Coord3D		outer_leftBottom;
	API_Coord3D		outer_rightTop;
	API_Coord3D		outer_rightBottom;


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
			if ( (abs (trCoord.z - elem.morph.level) < EPS) && (abs (elem.morph.level - trCoord.z) < EPS) ) {
				coords.Push (trCoord);
			}
		}
	}
	nNodes = coords.GetSize ();

	// �ϴ� �� 2���� Ŭ��
	BNZeroMemory (&pointInfo, sizeof (API_GetPointType));
	CHCopyC ("Left Bottom ���� Ŭ���Ͻʽÿ�.", pointInfo.prompt);
	pointInfo.enableQuickSelection = true;
	err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
	point1 = pointInfo.pos;

	BNZeroMemory (&pointInfo, sizeof (API_GetPointType));
	CHCopyC ("Right Bottom ���� Ŭ���Ͻʽÿ�.", pointInfo.prompt);
	pointInfo.enableQuickSelection = true;
	err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
	point2 = pointInfo.pos;
	
	// �� �� ���� ������ ����
	dx = point2.x - point1.x;
	dy = point2.y - point1.y;
	ang = RadToDegree (atan2 (dy, dx));

	// �������� ȸ�������� ����
	if ((ang > 0.0) && (ang < 90.0))	ang = ang;			// 0�� �ʰ� 90�� �̸��̸�,
	if ((ang > 90.0) && (ang < 180.0))	ang = ang - 90.0;	// 90�� �ʰ� 180�� �̸��̸�,
	if ((ang > 180.0) && (ang < 270.0))	ang = ang - 180.0;	// 180�� �ʰ� 270�� �̸��̸�,
	if (ang > 270.0)					ang = ang - 270.0;	// 270�� �ʰ��̸�,

	// ȸ������ 0�� ���� ��ǥ�� ����ϰ�, �������� ������ ������
	for (xx = 0 ; xx < nNodes ; ++xx) {
		tempPoint = coords.Pop ();
		resultPoint.x = point1.x + ((tempPoint.x - point1.x)*cos(DegreeToRad (-ang)) - (tempPoint.y - point1.y)*sin(DegreeToRad (-ang)));
		resultPoint.y = point1.y + ((tempPoint.x - point1.x)*sin(DegreeToRad (-ang)) + (tempPoint.y - point1.y)*cos(DegreeToRad (-ang)));
		resultPoint.z = tempPoint.z;

		nodes_random [xx] = resultPoint;
	}

	// ����ڰ� Ŭ���� 1, 2�� ���� ���� ����
	tempPoint = point2;
	resultPoint.x = point1.x + ((tempPoint.x - point1.x)*cos(DegreeToRad (-ang)) - (tempPoint.y - point1.y)*sin(DegreeToRad (-ang)));
	resultPoint.y = point1.y + ((tempPoint.x - point1.x)*sin(DegreeToRad (-ang)) + (tempPoint.y - point1.y)*cos(DegreeToRad (-ang)));
	resultPoint.z = tempPoint.z;

	nodes_sequential [0] = point1;
	nodes_sequential [1] = resultPoint;

	// ������ �� ������� ������ ��
	nEntered = 1;
	for (xx = 1 ; xx < (nNodes-1) ; ++xx) {
		
		bFindPoint = false;		// ���� ���� ã�Ҵ��� ���� (����� ��, �� ���� �����ϱ� ����)

		for (yy = 0 ; yy < nNodes ; ++yy) {

			// �̹� ����� ���� �ƴ� ���
			if (!isAlreadyStored (nodes_random [yy], nodes_sequential, 0, xx)) {
				// ���� ���� �½��ϱ�?
				if (isNextPoint (nodes_sequential [xx-1], nodes_sequential [xx], nodes_random [yy])) {

					// ó�� ã�� ���
					if (bFindPoint == false) {
						bFindPoint = true;
						bufPoint = nodes_random [yy];
					}
				
					// �� ã�� ���
					if (bFindPoint == true) {
						result = moreCloserPoint (nodes_sequential [xx], bufPoint, nodes_random [yy]);	// nodes_sequential [xx]�� ����� ���� � ���Դϱ�?

						if (result == 2)
							bufPoint = nodes_random [yy];
					}
				}
			}
		}

		// ã�� ���� ���� ������ �߰�
		if (bFindPoint) {
			nodes_sequential [xx+1] = bufPoint;
			++nEntered;
		}
	}

	// �۾� �� ���� �ݿ�
	// ...
	
	// ���� ������ �� ���� ����
	// ...

	// [DIALOG] 1��° ���̾�α׿��� ������ ���� �Է� ����
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32511, ACAPI_GetOwnResModule (), slabBottomPlacerHandler1, 0);

	// �� �� ���� ������ ������
	placingZone.ang = DegreeToRad (ang);
	placingZone.level = nodes_sequential [0].z;

	// �ֿܰ� ��ǥ�� ����
	placingZone.outerLeft	= nodes_sequential [0].x;
	placingZone.outerRight	= nodes_sequential [0].x;
	placingZone.outerTop	= nodes_sequential [0].y;
	placingZone.outerBottom	= nodes_sequential [0].y;

	for (xx = 1 ; xx < nNodes ; ++xx) {
		if (nodes_sequential [xx].x < placingZone.outerLeft)
			placingZone.outerLeft = nodes_sequential [xx].x;
		if (nodes_sequential [xx].x > placingZone.outerRight)
			placingZone.outerRight = nodes_sequential [xx].x;
		if (nodes_sequential [xx].y > placingZone.outerTop)
			placingZone.outerTop = nodes_sequential [xx].y;
		if (nodes_sequential [xx].y < placingZone.outerBottom)
			placingZone.outerBottom = nodes_sequential [xx].y;
	}

	// ���� �κ� �ڳ� ��ǥ�� ����
	outer_leftTop.x		= placingZone.outerLeft;	outer_leftTop.y		= placingZone.outerTop;		outer_leftTop.z		= placingZone.level;
	outer_leftBottom.x	= placingZone.outerLeft;	outer_leftBottom.y	= placingZone.outerBottom;	outer_leftBottom.z	= placingZone.level;
	outer_rightTop.x	= placingZone.outerRight;	outer_rightTop.y	= placingZone.outerTop;		outer_rightTop.z	= placingZone.level;
	outer_rightBottom.x	= placingZone.outerRight;	outer_rightBottom.y	= placingZone.outerBottom;	outer_rightBottom.z	= placingZone.level;

	placingZone.corner_leftTop = NULL;		placingZone.corner_leftBottom = NULL;
	placingZone.corner_rightTop = NULL;		placingZone.corner_rightBottom = NULL;

	// !!! ���� ����Ʈ (���� ���� ã�ƾ� �ϴµ�...)
	for (xx = 1 ; xx < nNodes ; ++xx) {
		// �»�� �ڳ�
		if ( (moreCloserPoint (nodes_sequential [xx], outer_leftTop, outer_rightTop) == 1) && (moreCloserPoint (nodes_sequential [xx], outer_leftTop, outer_leftBottom) == 1) )
			placingZone.corner_leftTop = &nodes_sequential [xx];

		// ���ϴ� �ڳ�
		if ( (moreCloserPoint (nodes_sequential [xx], outer_leftBottom, outer_rightBottom) == 1) && (moreCloserPoint (nodes_sequential [xx], outer_leftTop, outer_leftBottom) == 2) )
			placingZone.corner_leftBottom = &nodes_sequential [xx];

		// ���� �ڳ�
		if ( (moreCloserPoint (nodes_sequential [xx], outer_leftTop, outer_rightTop) == 2) && (moreCloserPoint (nodes_sequential [xx], outer_rightTop, outer_rightBottom) == 1) )
			placingZone.corner_rightTop = &nodes_sequential [xx];

		// ���ϴ� �ڳ�
		if ( (moreCloserPoint (nodes_sequential [xx], outer_leftBottom, outer_rightBottom) == 2) && (moreCloserPoint (nodes_sequential [xx], outer_rightTop, outer_rightBottom) == 2) )
			placingZone.corner_rightBottom = &nodes_sequential [xx];
	}

	// !!! ���� ����Ʈ
	err = ACAPI_CallUndoableCommand ("�׽�Ʈ", [&] () -> GSErrCode {
		placeCoordinateLabel (placingZone.corner_leftTop->x, placingZone.corner_leftTop->y, placingZone.corner_leftTop->z, true, "�ڳ� leftTop", 1, 0);
		placeCoordinateLabel (placingZone.corner_leftBottom->x, placingZone.corner_leftBottom->y, placingZone.corner_leftBottom->z, true, "�ڳ� leftBottom", 1, 0);
		placeCoordinateLabel (placingZone.corner_rightTop->x, placingZone.corner_rightTop->y, placingZone.corner_rightTop->z, true, "�ڳ� rightTop", 1, 0);
		placeCoordinateLabel (placingZone.corner_rightBottom->x, placingZone.corner_rightBottom->y, placingZone.corner_rightBottom->z, true, "�ڳ� rightBottom", 1, 0);

		return NoError;
	});

	// 1. �������� ����/���� ������ ����Ѵ�. (�����ڸ� �ʺ�� 150 �̻� 300 �̸�)
	//	- �߽��� �������� inner ��ǥ������ �� ��ġ ����
	// 2. ���� ��ġ: ����� �ִ� ���� 1800 ����
	// 3. ���� ���� ��ġ (�β� 50, �ʺ� 80): ����� �ִ� ���� 1800 ����
	// 4. �ٱ��� ���� ��ġ (�β� 40, �ʺ� 50): ����� �ִ� ���� 1800 ����

	/*
		*** ��� ����(3����): ������(ȸ��X: 0��, �������� ����), ����(����: 90��), ����(��ġ����: �ٴڴ�����)
		1. ����� �Է� (2��)
			- ������ ���� ���� �پ� �־ ��
			- ���� ������ ��ġ �� ��ư�� �־�� ��
			- Ÿ��Ʋ: ����/���� ä���
			- ��ư: 1. ���� ���� Ȯ�� // 2. ��  ġ // 3. ������
			- ��ġ ��ư�� ������ (x���� y���� ��ư�� �� ũ�⸦ �����ϸ�? - �ʺ� �ٲٸ� y�� ��ü�� ������ ��, ���̸� �ٲٸ� x�� ��ü�� ������ ��)
			- ���� ����/���� ����: ǥ�ø� �� (���� �� ��ģ ��): ���� ���� ���� ǥ�� (150~300mm), ���� �������� �ƴ��� �۲÷� ǥ��
			- ��� ���� �ʺ� ������ �� �־�� �� (��/�Ʒ� ���, ����/������ ��� - ����ڰ� ����.. �׿� ���� ������ ��ü �迭�� �̵���)
			- ��ġ ��ư ��濡�� ��ư ���̸��� ���� ���縦 ���� ���θ� ������ �� �־�� �� (��ư ���)
		2. ���� ��ġ
			- (1) �������� ���� ���͸� �������� ��ġ�� ��
			- (2) ����(11.5T, ����Ʋ Off)�� ������ �°� ��ġ�ϵ�, ��ġ�� �κ��� ���Ⱑ �ʿ���! (���̵� ����)
			- (3) ���ʿ� ���縦 �� �� (Z���� �β��� 50�̾�� ��, �ʺ�� 80)
			- (4) ���� �����絵 �� �� (���� ����� �β�, �ʺ�� ���� - ���̴� ����� ������ ����)
	*/

	return	err;
}

// aPoint�� bPoint�� ���� ������ Ȯ����
bool	isSamePoint (API_Coord3D aPoint, API_Coord3D bPoint)
{
	if ( (abs (aPoint.x - bPoint.x) < EPS) && (abs (aPoint.y - bPoint.y) < EPS) && (abs (aPoint.z - bPoint.z) < EPS) &&
		(abs (bPoint.x - aPoint.x) < EPS) && (abs (bPoint.y - aPoint.y) < EPS) && (abs (bPoint.z - aPoint.z) < EPS) ) {
		return true;
	} else
		return false;
}

// aPoint�� pointList�� ������ �Ǿ����� Ȯ����
bool	isAlreadyStored (API_Coord3D aPoint, API_Coord3D pointList [], short startInd, short endInd)
{
	short	xx;

	for (xx = startInd ; xx <= endInd ; ++xx) {
		// ��� ��ǥ ���� ��ġ�� ���, �̹� ���Ե� ��ǥ ���̶�� ������
		if ( (abs (aPoint.x - pointList [xx].x) < EPS) && (abs (aPoint.y - pointList [xx].y) < EPS) && (abs (aPoint.z - pointList [xx].z) < EPS) &&
			(abs (pointList [xx].x - aPoint.x) < EPS) && (abs (pointList [xx].y - aPoint.y) < EPS) && (abs (pointList [xx].z - aPoint.z) < EPS) ) {
			return true;
		}
	}

	return false;
}

// nextPoint�� curPoint�� ���� ���Դϱ�?
bool	isNextPoint (API_Coord3D prevPoint, API_Coord3D curPoint, API_Coord3D nextPoint)
{
	bool	cond1 = false;
	bool	cond2_1 = false;
	bool	cond2_2 = false;

	// curPoint�� nextPoint�� ���� Z���� ���°�?
	if ( (abs (curPoint.z - nextPoint.z) < EPS) && (abs (nextPoint.z - curPoint.z) < EPS) )
		cond1 = true;

	// ���� ���� ���� ���� Y�� �� ���� ���, ���� ���� ���� ���� X�� �� �־�� �ϰ�, ���� ���� ���� �� ������ X���� ���̰� �־�� ��
	if ((abs (curPoint.x - prevPoint.x) < EPS) && (abs (prevPoint.x - curPoint.x) < EPS) &&
		(abs (curPoint.y - nextPoint.y) < EPS) && (abs (nextPoint.y - curPoint.y) < EPS) &&
		((abs (curPoint.x - nextPoint.x) > EPS) || (abs (nextPoint.x - curPoint.x) > EPS)))
		cond2_1 = true;

	// ���� ���� ���� ���� X�� �� ���� ���, ���� ���� ���� ���� Y�� �� �־�� �ϰ�, ���� ���� ���� �� ������ Y���� ���̰� �־�� ��
	if ((abs (curPoint.y - prevPoint.y) < EPS) && (abs (prevPoint.y - curPoint.y) < EPS) &&
		(abs (curPoint.x - nextPoint.x) < EPS) && (abs (nextPoint.x - curPoint.x) < EPS) &&
		((abs (curPoint.y - nextPoint.y) > EPS) || (abs (nextPoint.y - curPoint.y) > EPS)))
		cond2_2 = true;

	// ���� Z���̸鼭 ���� �� ���� ������ �Ÿ��� ���� ���
	if (cond1 && (cond2_1 || cond2_2))
		return true;
	else
		return false;
}

// curPoint�� ����� ���� p1, p2 �� � ���Դϱ�?
short	moreCloserPoint (API_Coord3D curPoint, API_Coord3D p1, API_Coord3D p2)
{
	double dist1, dist2;

	dist1 = GetDistance (curPoint, p1);
	dist2 = GetDistance (curPoint, p2);

	// curPoint�� p1�� �� ������ 1 ����
	if ((dist2 - dist1) > EPS)	return 1;
	
	// curPoint�� p2�� �� ������ 2 ����
	if ((dist1 - dist2) > EPS)	return 2;

	// �� �ܿ��� 0 ����
	return 0;
}

// 1�� ��ġ�� ���� ���Ǹ� ��û�ϴ� 1�� ���̾�α�
short DGCALLBACK slabBottomPlacerHandler1 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short		result;
	API_UCCallbackType	ucb;

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, "������ �Ϻο� ��ġ");

			//////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			// ���� ��ư
			DGSetItemText (dialogID, DG_OK, "Ȯ ��");

			// ���� ��ư
			DGSetItemText (dialogID, DG_CANCEL, "�� ��");

			//////////////////////////////////////////////////////////// ������ ��ġ (������)
			// ��: ������ ��ġ ����
			DGSetItemText (dialogID, LABEL_PLACING_EUROFORM, "������ ��ġ ����");

			// ��: �ʺ�
			DGSetItemText (dialogID, LABEL_EUROFORM_WIDTH, "�ʺ�");

			// ��: ����
			DGSetItemText (dialogID, LABEL_EUROFORM_HEIGHT, "����");

			// ��: ��ġ����
			DGSetItemText (dialogID, LABEL_EUROFORM_ORIENTATION, "��ġ����");

			// ��: ���̾� ����
			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, "���纰 ���̾� ����");

			// ��: ���̾� - ������
			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM, "������");

			// ��: ���̾� - ����
			DGSetItemText (dialogID, LABEL_LAYER_PLYWOOD, "����");

			// ��: ���̾� - ����
			DGSetItemText (dialogID, LABEL_LAYER_WOOD, "����");

			// ���� ��Ʈ�� �ʱ�ȭ
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_LAYER_EUROFORM;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, 1);

			ucb.itemID	 = USERCONTROL_LAYER_PLYWOOD;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, 1);

			ucb.itemID	 = USERCONTROL_LAYER_WOOD;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, 1);

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					//////////////////////////////////////// ���̾�α� â ������ �Է� ����
					// ������ �ʺ�, ����, ����
					placingZone.eu_wid = DGPopUpGetItemText (dialogID, POPUP_EUROFORM_WIDTH, static_cast<short>(DGGetItemValLong (dialogID, POPUP_EUROFORM_WIDTH))).ToCStr ().Get ();
					placingZone.eu_hei = DGPopUpGetItemText (dialogID, POPUP_EUROFORM_HEIGHT, static_cast<short>(DGGetItemValLong (dialogID, POPUP_EUROFORM_HEIGHT))).ToCStr ().Get ();
					placingZone.eu_ori = DGPopUpGetItemText (dialogID, POPUP_EUROFORM_ORIENTATION, static_cast<short>(DGGetItemValLong (dialogID, POPUP_EUROFORM_ORIENTATION))).ToCStr ().Get ();

					// ���̾� ��ȣ ����
					layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM);
					layerInd_Plywood		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD);
					layerInd_Wood			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD);

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
