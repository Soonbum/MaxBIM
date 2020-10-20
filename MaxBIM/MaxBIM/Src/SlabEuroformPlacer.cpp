#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "SlabEuroformPlacer.hpp"

using namespace slabBottomPlacerDG;

static SlabPlacingZone	placingZone;			// �⺻ ������ �Ϻ� ���� ����
static InfoSlab			infoSlab;				// ������ ��ü ����
static short			clickedBtnItemIdx;		// �׸��� ��ư���� Ŭ���� ��ư�� �ε��� ��ȣ�� ����
static bool				clickedOKButton;		// OK ��ư�� �������ϱ�?
static short			layerInd_Euroform;		// ���̾� ��ȣ: ������
static short			layerInd_Plywood;		// ���̾� ��ȣ: ����
static short			layerInd_Wood;			// ���̾� ��ȣ: ����
static short			itemInitIdx = GRIDBUTTON_IDX_START;		// �׸��� ��ư �׸� �ε��� ���۹�ȣ


// 2�� �޴�: ������ �Ϻο� �������� ��ġ�ϴ� ���� ��ƾ
GSErrCode	placeEuroformOnSlabBottom (void)
{
	GSErrCode	err = NoError;
	long		nSel;
	short		xx, yy;
	double		dx, dy, ang;
	API_Coord3D	rotatedPoint, unrotatedPoint;

	// Selection Manager ���� ����
	API_SelectionInfo		selectionInfo;
	API_Element				tElem;
	API_Neig				**selNeigs;
	GS::Array<API_Guid>&	morphs = GS::Array<API_Guid> ();
	GS::Array<API_Guid>&	slabs = GS::Array<API_Guid> ();
	long					nMorphs = 0;
	long					nSlabs = 0;

	// ��ü ���� ��������
	API_Element				elem;
	API_ElementMemo			memo;
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
	bool					bIsInPolygon1, bIsInPolygon2;
	
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

	API_Coord3D		outer_leftTopBelow;
	API_Coord3D		outer_leftTopSide;
	API_Coord3D		outer_leftBottomOver;
	API_Coord3D		outer_leftBottomSide;
	API_Coord3D		outer_rightTopBelow;
	API_Coord3D		outer_rightTopSide;
	API_Coord3D		outer_rightBottomOver;
	API_Coord3D		outer_rightBottomSide;


	// ������ ��� ��������
	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err == APIERR_NOPLAN) {
		ACAPI_WriteReport ("���� ������Ʈ â�� �����ϴ�.", true);
		return err;
	}
	if (err == APIERR_NOSEL) {
		ACAPI_WriteReport ("�ƹ� �͵� �������� �ʾҽ��ϴ�.\n�ʼ� ����: ������ (1��), ������ �Ϻθ� ���� ���� (1��)", true);
		return err;
	}
	if (err != NoError) {
		BMKillHandle ((GSHandle *) &selNeigs);
		return err;
	}

	// ������ 1��, ���� 1�� �����ؾ� ��
	if (selectionInfo.typeID != API_SelEmpty) {
		nSel = BMGetHandleSize ((GSHandle) selNeigs) / sizeof (API_Neig);
		for (xx = 0 ; xx < nSel && err == NoError ; ++xx) {
			tElem.header.typeID = Neig_To_ElemID ((*selNeigs)[xx].neigID);

			tElem.header.guid = (*selNeigs)[xx].guid;
			if (ACAPI_Element_Get (&tElem) != NoError)	// ������ �� �ִ� ����ΰ�?
				continue;

			if (tElem.header.typeID == API_MorphID)		// �����ΰ�?
				morphs.Push (tElem.header.guid);

			if (tElem.header.typeID == API_SlabID)		// �������ΰ�?
				slabs.Push (tElem.header.guid);
		}
	}
	BMKillHandle ((GSHandle *) &selNeigs);
	nMorphs = morphs.GetSize ();
	nSlabs = slabs.GetSize ();

	// �����갡 1���ΰ�?
	if (nSlabs != 1) {
		ACAPI_WriteReport ("�����긦 1�� �����ؾ� �մϴ�.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// ������ 1���ΰ�?
	if (nMorphs != 1) {
		ACAPI_WriteReport ("������ �Ϻθ� ���� ������ 1�� �����ϼž� �մϴ�.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// (1) ������ ������ ������
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	elem.header.guid = slabs.Pop ();
	err = ACAPI_Element_Get (&elem);
	err = ACAPI_Element_GetMemo (elem.header.guid, &memo);
	
	infoSlab.floorInd		= elem.header.floorInd;
	infoSlab.offsetFromTop	= elem.slab.offsetFromTop;
	infoSlab.thickness		= elem.slab.thickness;

	ACAPI_DisposeElemMemoHdls (&memo);

	// (2) ���� ������ ������
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

	// ���� ������ �� ���� �����￡ ���� ���� �ƴϸ� ����
	bIsInPolygon1 = false;
	bIsInPolygon2 = false;
	for (xx = 0 ; xx < nNodes ; ++xx) {
		if (isSamePoint (point1, nodes_random [xx]))
			bIsInPolygon1 = true;
		if (isSamePoint (resultPoint, nodes_random [xx]))
			bIsInPolygon2 = true;
	}

	if ( !(bIsInPolygon1 && bIsInPolygon2) ) {
		ACAPI_WriteReport ("�����￡ ������ ���� ���� Ŭ���߽��ϴ�.", true);
		return err;
	}

	// ���� ���� ����
	//err = ACAPI_CallUndoableCommand ("���� ���� ����", [&] () -> GSErrCode {
	//	API_Elem_Head* headList = new API_Elem_Head [1];
	//	headList [0] = elem.header;
	//	err = ACAPI_Element_Delete (&headList, 1);
	//	delete headList;

	//	return err;
	//});

	// ������ �� ������� ������ ��
	nEntered = 2;	// �̹� 2���� ���� �ԷµǾ� ����
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

	// �� �� ���� ������ ������
	placingZone.ang = DegreeToRad (ang);
	placingZone.level = nodes_sequential [0].z;

	// �۾� �� ���� �ݿ�
	BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	workLevel_morph = 0.0;
	ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
	for (xx = 0 ; xx < (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
		if (storyInfo.data [0][xx].index == infoSlab.floorInd) {
			workLevel_morph = storyInfo.data [0][xx].level;
			break;
		}
	}
	BMKillHandle ((GSHandle *) &storyInfo.data);
	
	// ���� ������ �� ���� ����
	placingZone.level = infoSlab.offsetFromTop - infoSlab.thickness;

	// [DIALOG] 1��° ���̾�α׿��� ������ ���� �Է� ����
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32511, ACAPI_GetOwnResModule (), slabBottomPlacerHandler1, 0);

	// ���ڿ��� �� �������� �ʺ�/���̸� �Ǽ������ε� ����
	placingZone.eu_wid_numeric = atof (placingZone.eu_wid.c_str ()) / 1000.0;
	placingZone.eu_hei_numeric = atof (placingZone.eu_hei.c_str ()) / 1000.0;

	// �ֿܰ� ��ǥ�� ����
	placingZone.outerLeft	= nodes_sequential [0].x;
	placingZone.outerRight	= nodes_sequential [0].x;
	placingZone.outerTop	= nodes_sequential [0].y;
	placingZone.outerBottom	= nodes_sequential [0].y;

	for (xx = 1 ; xx < nEntered ; ++xx) {
		if (nodes_sequential [xx].x < placingZone.outerLeft)
			placingZone.outerLeft = nodes_sequential [xx].x;
		if (nodes_sequential [xx].x > placingZone.outerRight)
			placingZone.outerRight = nodes_sequential [xx].x;
		if (nodes_sequential [xx].y > placingZone.outerTop)
			placingZone.outerTop = nodes_sequential [xx].y;
		if (nodes_sequential [xx].y < placingZone.outerBottom)
			placingZone.outerBottom = nodes_sequential [xx].y;
	}

	// ���� ������ ��ǥ�� �ӽ÷� ������
	outer_leftTop.x		= placingZone.outerLeft;	outer_leftTop.y		= placingZone.outerTop;		outer_leftTop.z		= placingZone.level;
	outer_leftBottom.x	= placingZone.outerLeft;	outer_leftBottom.y	= placingZone.outerBottom;	outer_leftBottom.z	= placingZone.level;
	outer_rightTop.x	= placingZone.outerRight;	outer_rightTop.y	= placingZone.outerTop;		outer_rightTop.z	= placingZone.level;
	outer_rightBottom.x	= placingZone.outerRight;	outer_rightBottom.y	= placingZone.outerBottom;	outer_rightBottom.z	= placingZone.level;

	API_Coord3D	tPoint;
	tPoint.x = (placingZone.outerLeft + placingZone.outerRight) / 2;
	tPoint.y = (placingZone.outerTop + placingZone.outerBottom) / 2;
	tPoint.z = placingZone.level;

	outer_leftTopBelow		= tPoint;
	outer_leftTopSide		= tPoint;
	outer_leftBottomOver	= tPoint;
	outer_leftBottomSide	= tPoint;
	outer_rightTopBelow		= tPoint;
	outer_rightTopSide		= tPoint;
	outer_rightBottomOver	= tPoint;
	outer_rightBottomSide	= tPoint;

	// ���� ������ ��ǥ�� �ٷ� ���ʿ� �ִ� ���� ��ǥ�� ã�Ƴ�
	for (xx = 0 ; xx < nEntered ; ++xx) {
		if ( (!isSamePoint (outer_leftTop, nodes_sequential [xx])) && (moreCloserPoint (nodes_sequential [xx], outer_leftTop, outer_leftBottom) == 1) && (abs (nodes_sequential [xx].x - outer_leftTop.x) < EPS) )
			outer_leftTopBelow = nodes_sequential [xx];

		if ( (!isSamePoint (outer_leftTop, nodes_sequential [xx])) && (moreCloserPoint (nodes_sequential [xx], outer_leftTop, outer_rightTop) == 1) && (abs (nodes_sequential [xx].y - outer_leftTop.y) < EPS) )
			outer_leftTopSide = nodes_sequential [xx];

		if ( (!isSamePoint (outer_leftBottom, nodes_sequential [xx])) && (moreCloserPoint (nodes_sequential [xx], outer_leftTop, outer_leftBottom) == 2) && (abs (nodes_sequential [xx].x - outer_leftBottom.x) < EPS) )
			outer_leftBottomOver = nodes_sequential [xx];

		if ( (!isSamePoint (outer_leftBottom, nodes_sequential [xx])) && (moreCloserPoint (nodes_sequential [xx], outer_leftBottom, outer_rightBottom) == 1) && (abs (nodes_sequential [xx].y - outer_leftBottom.y) < EPS) )
			outer_leftBottomSide = nodes_sequential [xx];

		if ( (!isSamePoint (outer_rightTop, nodes_sequential [xx])) && (moreCloserPoint (nodes_sequential [xx], outer_rightTop, outer_rightBottom) == 1) && (abs (nodes_sequential [xx].x - outer_rightTop.x) < EPS) )
			outer_rightTopBelow = nodes_sequential [xx];

		if ( (!isSamePoint (outer_rightTop, nodes_sequential [xx])) && (moreCloserPoint (nodes_sequential [xx], outer_leftTop, outer_rightTop) == 2) && (abs (nodes_sequential [xx].y - outer_rightTop.y) < EPS) )
			outer_rightTopSide = nodes_sequential [xx];

		if ( (!isSamePoint (outer_rightBottom, nodes_sequential [xx])) && (moreCloserPoint (nodes_sequential [xx], outer_rightTop, outer_rightBottom) == 2) && (abs (nodes_sequential [xx].x - outer_rightBottom.x) < EPS) )
			outer_rightBottomOver = nodes_sequential [xx];

		if ( (!isSamePoint (outer_rightBottom, nodes_sequential [xx])) && (moreCloserPoint (nodes_sequential [xx], outer_leftBottom, outer_rightBottom) == 2) && (abs (nodes_sequential [xx].y - outer_rightBottom.y) < EPS) )
			outer_rightBottomSide = nodes_sequential [xx];
	}

	// ���� �ڳ� Ȥ�� �Ϲ� �ڳ��� ��ǥ�� ã�Ƴ�
	for (xx = 0 ; xx < nEntered ; ++xx) {
		// �»�� �ڳ�
		if ( (isSamePoint (outer_leftTopBelow, tPoint)) && (isSamePoint (outer_leftTopSide, tPoint)) )
			placingZone.corner_leftTop = outer_leftTop;
		else {
			placingZone.corner_leftTop.x = outer_leftTopSide.x;
			placingZone.corner_leftTop.y = outer_leftTopBelow.y;
			placingZone.corner_leftTop.z = placingZone.level;
		}
		
		// ���ϴ� �ڳ�
		if ( (isSamePoint (outer_leftBottomOver, tPoint)) && (isSamePoint (outer_leftBottomSide, tPoint)) )
			placingZone.corner_leftBottom = outer_leftBottom;
		else {
			placingZone.corner_leftBottom.x = outer_leftBottomSide.x;
			placingZone.corner_leftBottom.y = outer_leftBottomOver.y;
			placingZone.corner_leftBottom.z = placingZone.level;
		}

		// ���� �ڳ�
		if ( (isSamePoint (outer_rightTopBelow, tPoint)) && (isSamePoint (outer_rightTopSide, tPoint)) )
			placingZone.corner_rightTop = outer_rightTop;
		else {
			placingZone.corner_rightTop.x = outer_rightTopSide.x;
			placingZone.corner_rightTop.y = outer_rightTopBelow.y;
			placingZone.corner_rightTop.z = placingZone.level;
		}

		// ���ϴ� �ڳ�
		if ( (isSamePoint (outer_rightBottomOver, tPoint)) && (isSamePoint (outer_rightBottomSide, tPoint)) )
			placingZone.corner_rightBottom = outer_rightBottom;
		else {
			placingZone.corner_rightBottom.x = outer_rightBottomSide.x;
			placingZone.corner_rightBottom.y = outer_rightBottomOver.y;
			placingZone.corner_rightBottom.z = placingZone.level;
		}
	}

	// �ڳ� ������ ��ǥ�� ����
	if (placingZone.corner_leftTop.x < placingZone.corner_leftBottom.x)
		placingZone.innerLeft = placingZone.corner_leftBottom.x;
	else
		placingZone.innerLeft = placingZone.corner_leftTop.x;

	if (placingZone.corner_rightTop.x < placingZone.corner_rightBottom.x)
		placingZone.innerRight = placingZone.corner_rightTop.x;
	else
		placingZone.innerRight = placingZone.corner_rightBottom.x;

	if (placingZone.corner_leftTop.y < placingZone.corner_rightTop.y)
		placingZone.innerTop = placingZone.corner_leftTop.y;
	else
		placingZone.innerTop = placingZone.corner_rightTop.y;

	if (placingZone.corner_leftBottom.y < placingZone.corner_rightBottom.y)
		placingZone.innerBottom = placingZone.corner_rightBottom.y;
	else
		placingZone.innerBottom = placingZone.corner_leftBottom.y;

	// ���� ������ �ʺ�� ���̸� ����
	placingZone.innerWidth = placingZone.innerRight - placingZone.innerLeft;
	placingZone.innerHeight = placingZone.innerTop - placingZone.innerBottom;

	// ���� ���� �ʱ�ȭ
	placingZone.remain_hor = placingZone.outerRight - placingZone.outerLeft;
	placingZone.remain_ver = placingZone.outerTop - placingZone.outerBottom;

	// ������ ����/���� ���� ���� ����
	placingZone.eu_count_hor = 0;
	placingZone.eu_count_ver = 0;

	if (placingZone.eu_ori.compare (std::string ("�������")) == 0) {
		placingZone.eu_count_hor = static_cast<short>(placingZone.remain_hor / placingZone.eu_wid_numeric);				// ���� ���� ����
		placingZone.remain_hor = placingZone.remain_hor - (placingZone.eu_count_hor * placingZone.eu_wid_numeric);		// ���� ���� ������
		placingZone.eu_count_ver = static_cast<short>(placingZone.remain_ver / placingZone.eu_hei_numeric);				// ���� ���� ����
		placingZone.remain_ver = placingZone.remain_ver - (placingZone.eu_count_ver * placingZone.eu_hei_numeric);		// ���� ���� ������
	} else {
		placingZone.eu_count_hor = static_cast<short>(placingZone.remain_hor / placingZone.eu_hei_numeric);				// ���� ���� ����
		placingZone.remain_hor = placingZone.remain_hor - (placingZone.eu_count_hor * placingZone.eu_hei_numeric);		// ���� ���� ������
		placingZone.eu_count_ver = static_cast<short>(placingZone.remain_ver / placingZone.eu_wid_numeric);				// ���� ���� ����
		placingZone.remain_ver = placingZone.remain_ver - (placingZone.eu_count_ver * placingZone.eu_wid_numeric);		// ���� ���� ������
	}

	placingZone.remain_hor_updated = placingZone.remain_hor;
	placingZone.remain_ver_updated = placingZone.remain_ver;

	// ������ ���� ��ǥ ����
	if (placingZone.eu_ori.compare (std::string ("�������")) == 0) {
		placingZone.formArrayWidth = (placingZone.eu_count_hor * placingZone.eu_wid_numeric);
		placingZone.formArrayHeight = (placingZone.eu_count_ver * placingZone.eu_hei_numeric);
	} else {
		placingZone.formArrayWidth = (placingZone.eu_count_hor * placingZone.eu_hei_numeric);
		placingZone.formArrayHeight = (placingZone.eu_count_ver * placingZone.eu_wid_numeric);
	}
	placingZone.leftBottomX = (placingZone.outerLeft + placingZone.outerRight) / 2 - (placingZone.formArrayWidth / 2);
	placingZone.leftBottomY = (placingZone.outerTop + placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2);
	placingZone.leftBottomZ = placingZone.level;
	
	// ���� ���� unrotated ��ġ�� ������Ʈ
	rotatedPoint.x = placingZone.leftBottomX;
	rotatedPoint.y = placingZone.leftBottomY;
	rotatedPoint.z = placingZone.leftBottomZ;
	unrotatedPoint = getUnrotatedPoint (rotatedPoint, point1, ang);
	placingZone.leftBottomX = unrotatedPoint.x;
	placingZone.leftBottomY = unrotatedPoint.y;
	placingZone.leftBottomZ = unrotatedPoint.z;

	// placingZone�� Cell ���� �ʱ�ȭ
	initCellsForSlabBottom (&placingZone);

	// ��ġ�� ���� ���� �Է�
	firstPlacingSettingsForSlabBottom (&placingZone);

	err = ACAPI_CallUndoableCommand ("�׽�Ʈ", [&] () -> GSErrCode {
		for (xx = 0 ; xx < placingZone.eu_count_ver ; ++xx) {
			for (yy = 0 ; yy < placingZone.eu_count_hor ; ++yy) {
				placeLibPartForSlabBottom (placingZone.cells [xx][yy]);
			}
		}

		return NoError;
	});

	// [DIALOG] 2��° ���̾�α׿��� ������ ��ġ�� �����ϰų� ���� ���縦 �����մϴ�.
	clickedOKButton = false;
	result = DGBlankModalDialog (185, 250, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, slabBottomPlacerHandler2, 0);

	if (clickedOKButton == false)
		return err;

	// 1. �������� ����/���� ������ ����Ѵ�. (�����ڸ� �ʺ�� 150 �̻� 300 �̸�)
	// 2. ���� ��ġ: ����� �ִ� ���� 1800 ����
	// 3. ���� ���� ��ġ (�β� 50, �ʺ� 80): ����� �ִ� ���� 1800 ����
	// 4. �ٱ��� ���� ��ġ (�β� 40, �ʺ� 50): ����� �ִ� ���� 1800 ����

	/*
		1. ����� �Է� (2��)
			- ���� ���� ��ġ �� ��ư�� �־�� ��
			- ��ġ ��ư�� ������ (x���� y���� ��ư�� �� ũ�⸦ �����ϸ�? - �ʺ� �ٲٸ� y�� ��ü�� ������ ��, ���̸� �ٲٸ� x�� ��ü�� ������ ��)
			- ���� ����/���� ����: ǥ�ø� �� (���� �� ��ģ ��): ���� ���� ���� ǥ�� (150~300mm), ���� �������� �ƴ��� �۲÷� ǥ��
			- ��� ���� �ʺ� ������ �� �־�� �� (��/�Ʒ� ���, ����/������ ��� - ����ڰ� ����.. �׿� ���� ������ ��ü �迭�� �̵���)
			- ��ġ ��ư ��濡�� ��ư ���̸��� ���� ���縦 ���� ���θ� ������ �� �־�� �� (üũ�ڽ�)
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

// ȸ���� ������� �ʾ��� ���� ��ġ (��ġ�Ǿ�� �� ���� ��ġ�� ����), ������ Degree
API_Coord3D		getUnrotatedPoint (API_Coord3D rotatedPoint, API_Coord3D axisPoint, double ang)
{
	API_Coord3D		unrotatedPoint;

	unrotatedPoint.x = axisPoint.x + ((rotatedPoint.x - axisPoint.x)*cos(DegreeToRad (ang)) - (rotatedPoint.y - axisPoint.y)*sin(DegreeToRad (ang)));
	unrotatedPoint.y = axisPoint.y + ((rotatedPoint.x - axisPoint.x)*sin(DegreeToRad (ang)) + (rotatedPoint.y - axisPoint.y)*cos(DegreeToRad (ang)));
	unrotatedPoint.z = rotatedPoint.z;

	return unrotatedPoint;
}

// Cell �迭�� �ʱ�ȭ��
void	initCellsForSlabBottom (SlabPlacingZone* placingZone)
{
	short xx, yy;

	for (xx = 0 ; xx < 50 ; ++xx)
		for (yy = 0 ; yy < 50 ; ++yy) {
			placingZone->cells [xx][yy].objType = NONE;
			placingZone->cells [xx][yy].ang = 0.0;
			placingZone->cells [xx][yy].horLen = 0.0;
			placingZone->cells [xx][yy].verLen = 0.0;
			placingZone->cells [xx][yy].leftBottomX = 0.0;
			placingZone->cells [xx][yy].leftBottomY = 0.0;
			placingZone->cells [xx][yy].leftBottomZ = 0.0;
		}
}

// 1�� ��ġ: ������
void	firstPlacingSettingsForSlabBottom (SlabPlacingZone* placingZone)
{
	short			xx, yy;
	API_Coord3D		axisPoint, rotatedPoint, unrotatedPoint;

	// ������ ����
	for (xx = 0 ; xx < placingZone->eu_count_ver ; ++xx) {
		for (yy = 0 ; yy < placingZone->eu_count_hor ; ++yy) {

			placingZone->cells [xx][yy].objType = EUROFORM;
			placingZone->cells [xx][yy].ang = placingZone->ang;

			if (placingZone->eu_ori.compare (std::string ("�������")) == 0) {
				placingZone->cells [xx][yy].libPart.form.u_ins_wall = true;
				placingZone->cells [xx][yy].horLen = placingZone->eu_wid_numeric;
				placingZone->cells [xx][yy].verLen = placingZone->eu_hei_numeric;
			} else {
				placingZone->cells [xx][yy].libPart.form.u_ins_wall = false;
				placingZone->cells [xx][yy].horLen = placingZone->eu_hei_numeric;
				placingZone->cells [xx][yy].verLen = placingZone->eu_wid_numeric;
			}

			placingZone->cells [xx][yy].libPart.form.eu_stan_onoff = true;
			placingZone->cells [xx][yy].libPart.form.eu_wid = placingZone->eu_wid_numeric;
			placingZone->cells [xx][yy].libPart.form.eu_hei = placingZone->eu_hei_numeric;

			// ������ʹ� �� ��ġ ���� ��ƾ
			placingZone->cells [xx][yy].leftBottomX = placingZone->leftBottomX + (placingZone->cells [xx][yy].horLen * yy);
			placingZone->cells [xx][yy].leftBottomY = placingZone->leftBottomY - (placingZone->cells [xx][yy].verLen * xx);
			placingZone->cells [xx][yy].leftBottomZ = placingZone->leftBottomZ;

			axisPoint.x = placingZone->leftBottomX;
			axisPoint.y = placingZone->leftBottomY;
			axisPoint.z = placingZone->leftBottomZ;

			rotatedPoint.x = placingZone->cells [xx][yy].leftBottomX;
			rotatedPoint.y = placingZone->cells [xx][yy].leftBottomY;
			rotatedPoint.z = placingZone->cells [xx][yy].leftBottomZ;
			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

			placingZone->cells [xx][yy].leftBottomX = unrotatedPoint.x;
			placingZone->cells [xx][yy].leftBottomY = unrotatedPoint.y;
			placingZone->cells [xx][yy].leftBottomZ = unrotatedPoint.z;
		}
	}
}

// �ش� ���� LeftBottom ��ġ�� ����
void	setCellPositionForSlabBottom (SlabPlacingZone *target_zone, CellForSlab objInfo, short ver, short hor)
{
	// ... alignPlacingZoneForSlabBottom �Լ��� ���� ���ʿ��� ���� ����
	// ... ���� ��(ver-1, hor-1����)���� ��ġ �� ũ�⸦ �����ؼ� ������ų ��
	//	placingZone->cells [xx][yy].leftBottomX = placingZone->leftBottomX + (getCellPositionLeftBottomXForWall (placingZone, xx, zz) * cos(placingZone->ang));
	//	placingZone->cells [xx][yy].leftBottomY = placingZone->leftBottomY + (getCellPositionLeftBottomXForWall (placingZone, xx, zz) * sin(placingZone->ang));
	//	placingZone->cells [xx][yy].leftBottomZ = placingZone->leftBottomZ + (xx * placingZone->cells [0][zz].verLen);
}

// Cell ������ ����ʿ� ���� ����ȭ�� ��ġ�� ��������
void	alignPlacingZoneForSlabBottom (SlabPlacingZone* target_zone)
{
	// ...
}

// �ش� �� ������ ������� ���̺귯�� ��ġ
API_Guid	placeLibPartForSlabBottom (CellForSlab objInfo)
{
	GSErrCode	err = NoError;

	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const	GS::uchar_t* gsmName = NULL;
	double	aParam;
	double	bParam;
	Int32	addParNum;

	std::string		tempString;

	// GUID ���� �ʱ�ȭ
	element.header.guid.clock_seq_hi_and_reserved = 0;
	element.header.guid.clock_seq_low = 0;
	element.header.guid.node[0] = 0;
	element.header.guid.node[1] = 0;
	element.header.guid.node[2] = 0;
	element.header.guid.node[3] = 0;
	element.header.guid.node[4] = 0;
	element.header.guid.node[5] = 0;
	element.header.guid.time_hi_and_version = 0;
	element.header.guid.time_low = 0;
	element.header.guid.time_mid = 0;

	// ���̺귯�� �̸� ����
	if (objInfo.objType == NONE)			return element.header.guid;
	if (objInfo.objType == EUROFORM)		gsmName = L("������v2.0.gsm");
	if (objInfo.objType == PLYWOOD)			gsmName = L("����v1.0.gsm");
	if (objInfo.objType == WOOD)			gsmName = L("����v1.0.gsm");

	// ��ü �ε�
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return element.header.guid;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

	element.header.typeID = API_ObjectID;
	element.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&element, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	// ���̺귯���� �Ķ���� �� �Է�
	element.object.libInd = libPart.index;
	element.object.pos.x = objInfo.leftBottomX;
	element.object.pos.y = objInfo.leftBottomY;
	element.object.level = objInfo.leftBottomZ;
	element.object.xRatio = aParam;
	element.object.yRatio = bParam;
	element.object.angle = objInfo.ang;
	element.header.floorInd = infoSlab.floorInd;

	if (objInfo.objType == EUROFORM) {
		element.header.layer = layerInd_Euroform;

		// �԰�ǰ�� ���,
		if (objInfo.libPart.form.eu_stan_onoff == true) {
			// �԰��� On/Off
			memo.params [0][27].value.real = TRUE;

			// �ʺ�
			tempString = format_string ("%.0f", objInfo.libPart.form.eu_wid * 1000);
			GS::ucscpy (memo.params [0][28].value.uStr, GS::UniString (tempString.c_str ()).ToUStr ().Get ());

			// ����
			tempString = format_string ("%.0f", objInfo.libPart.form.eu_hei * 1000);
			GS::ucscpy (memo.params [0][29].value.uStr, GS::UniString (tempString.c_str ()).ToUStr ().Get ());

		// ��԰�ǰ�� ���,
		} else {
			// �԰��� On/Off
			memo.params [0][27].value.real = FALSE;

			// �ʺ�
			memo.params [0][30].value.real = objInfo.libPart.form.eu_wid2;

			// ����
			memo.params [0][31].value.real = objInfo.libPart.form.eu_hei2;
		}

		// ��ġ����
		if (objInfo.libPart.form.u_ins_wall == true) {
			tempString = "�������";
		} else {
			tempString = "��������";
			element.object.pos.x += ( objInfo.horLen * cos(objInfo.ang) );
			element.object.pos.y += ( objInfo.horLen * sin(objInfo.ang) );
		}
		GS::ucscpy (memo.params [0][32].value.uStr, GS::UniString (tempString.c_str ()).ToUStr ().Get ());
		
		// ȸ��X
		memo.params [0][33].value.real = DegreeToRad (0.0);

		// ������ �β���ŭ ����
		//element.object.level -= 0.064;

	} else if (objInfo.objType == PLYWOOD) {
		element.header.layer = layerInd_Plywood;
		GS::ucscpy (memo.params [0][32].value.uStr, L("��԰�"));
		GS::ucscpy (memo.params [0][33].value.uStr, L("�ٴڱ��"));
		memo.params [0][35].value.real = objInfo.libPart.plywood.p_wid;		// ����
		memo.params [0][36].value.real = objInfo.libPart.plywood.p_leng;	// ����
		memo.params [0][38].value.real = FALSE;		// ����Ʋ OFF
		
		// ��ġ����
		if (objInfo.libPart.plywood.w_dir_wall == true)
			tempString = "�������";
		else
			tempString = "��������";
		GS::ucscpy (memo.params [0][33].value.uStr, GS::UniString (tempString.c_str ()).ToUStr ().Get ());

	} else if (objInfo.objType == WOOD) {
		element.header.layer = layerInd_Wood;
		GS::ucscpy (memo.params [0][27].value.uStr, L("�ٴڴ�����"));	// ��ġ����
		memo.params [0][28].value.real = objInfo.libPart.wood.w_w;		// �β�
		memo.params [0][29].value.real = objInfo.libPart.wood.w_h;		// �ʺ�
		memo.params [0][30].value.real = objInfo.libPart.wood.w_leng;	// ����
		memo.params [0][31].value.real = objInfo.libPart.wood.w_ang;	// ����
	}

	// ��ü ��ġ
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return element.header.guid;
}

// �������� ä�� �� ������ ���� ä���
GSErrCode	fillRestAreasForSlabBottom (void)
{
	// ...

	return NoError;
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

// 1�� ��ġ �� ������ ��û�ϴ� 2�� ���̾�α�
short DGCALLBACK slabBottomPlacerHandler2 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	btnSizeX = 50, btnSizeY = 50;
	short	dialogSizeX, dialogSizeY;
	short	groupboxSizeX, groupboxSizeY;
	short	btnInitPosX = 220 + 25;
	short	btnPosX, btnPosY;
	short	xx, yy;
	short	idxBtn;
	short	lastIdxBtn;
	short	idxCell;
	std::string		txtButton = "";
	API_Element		elem;
	GSErrCode		err;

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, "������ �Ϻο� ��ġ - ������ ��ġ ����");

			//////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			// ������Ʈ ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 130, 130, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "2. ��  ġ");
			DGShowItem (dialogID, DG_OK);

			// ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 170, 130, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "3. ������ ä���");
			DGShowItem (dialogID, DG_CANCEL);

			// ��: ���� ���� ����
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 30, 20, 90, 23);
			DGSetItemFont (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH, "�¿� ���� �ʺ�");
			DGShowItem (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH);

			// ��: ���� ���� ����
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 30, 50, 90, 23);
			DGSetItemFont (dialogID, LABEL_REMAIN_VERTICAL_LENGTH, DG_IS_LARGE | DG_IS_BOLD);
			DGSetItemText (dialogID, LABEL_REMAIN_VERTICAL_LENGTH, "���� ���� �ʺ�");
			DGShowItem (dialogID, LABEL_REMAIN_VERTICAL_LENGTH);

			// Edit ��Ʈ��: ���� ���� ����
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 130, 20-7, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH, DG_IS_LARGE | DG_IS_BOLD);
			DGShowItem (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH);
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH, placingZone.remain_hor_updated);

			// Edit ��Ʈ��: ���� ���� ����
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 130, 50-7, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH, DG_IS_LARGE | DG_IS_BOLD);
			DGShowItem (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH);
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH, placingZone.remain_ver_updated);

			// �׷�ڽ�: ������/�ٷ������̼� ��ġ ����
			groupboxSizeX = 40 + (btnSizeX * placingZone.eu_count_hor) + 50;
			groupboxSizeY = 70 + (btnSizeY * placingZone.eu_count_ver) + 50;
			DGAppendDialogItem (dialogID, DG_ITM_GROUPBOX, DG_GT_PRIMARY, 0, 200, 10, groupboxSizeX, groupboxSizeY);
			DGSetItemFont (dialogID, GROUPBOX_GRID_EUROFORM_WOOD, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, GROUPBOX_GRID_EUROFORM_WOOD, "������/���� ���� ��ġ ����");
			DGShowItem (dialogID, GROUPBOX_GRID_EUROFORM_WOOD);

			// ���� �Ÿ� Ȯ�� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 90, 130, 25);
			DGSetItemFont (dialogID, PUSHBUTTON_CONFIRM_REMAIN_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, PUSHBUTTON_CONFIRM_REMAIN_LENGTH, "1. ���� ���� Ȯ��");
			DGShowItem (dialogID, PUSHBUTTON_CONFIRM_REMAIN_LENGTH);

			// ���� â ũ�⸦ ����
			dialogSizeX = 270 + (btnSizeX * placingZone.eu_count_hor) + 50;
			dialogSizeY = max<short>(300, 150 + (btnSizeY * placingZone.eu_count_ver) + 50);
			DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_CENTER, true);

			// �׸��� ����ü�� ���� ��ư�� �������� ��ġ��
			btnPosX = 220 + 25, btnPosY = (btnSizeY * placingZone.eu_count_ver) + 25;
			for (xx = 0 ; xx < placingZone.eu_count_ver ; ++xx) {
				for (yy = 0 ; yy < placingZone.eu_count_hor ; ++yy) {
					idxBtn = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
					lastIdxBtn = idxBtn;
					DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL);

					txtButton = "";
					if (placingZone.cells [xx][yy].objType == NONE) {
						txtButton = "NONE";
					} else if (placingZone.cells [xx][yy].objType == EUROFORM) {
						if (placingZone.cells [xx][yy].libPart.form.u_ins_wall)
							txtButton = format_string ("������\n(����)\n��%.0f\n��%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
						else
							txtButton = format_string ("������\n(����)\n��%.0f\n��%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
					} else if (placingZone.cells [xx][yy].objType == PLYWOOD) {
						if (placingZone.cells [xx][yy].libPart.plywood.w_dir_wall)
							txtButton = format_string ("����\n(����)\n��%.0f\n��%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
						else
							txtButton = format_string ("����\n(����)\n��%.0f\n��%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
					} else if (placingZone.cells [xx][yy].objType == WOOD) {
						txtButton = format_string ("����\n��%.0f\n��%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
					}
					DGSetItemText (dialogID, idxBtn, txtButton.c_str ());		// �׸��� ��ư �ؽ�Ʈ ����
					DGShowItem (dialogID, idxBtn);
					btnPosX += btnSizeX;
				}
				btnPosX = btnInitPosX;
				btnPosY -= btnSizeY;
			}

			// ���� ���� ���� üũ �ڽ�
			btnPosX = 270 + 12, btnPosY = 48;
			for (xx = 0 ; xx < placingZone.eu_count_hor - 1 ; ++xx) {
				idxBtn = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, btnPosX, btnPosY, 25, 25);
				lastIdxBtn = idxBtn;
				DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL);

				txtButton = "T";
				DGSetItemText (dialogID, idxBtn, txtButton.c_str ());
				DGShowItem (dialogID, idxBtn);
				btnPosX += 50;
			}

			// �Ʒ��� ���� ���� üũ �ڽ�
			btnPosX = 270 + 12, btnPosY = (btnSizeY * (placingZone.eu_count_ver + 1)) + 27;
			for (xx = 0 ; xx < placingZone.eu_count_hor - 1 ; ++xx) {
				idxBtn = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, btnPosX, btnPosY, 25, 25);
				lastIdxBtn = idxBtn;
				DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL);

				txtButton = "B";
				DGSetItemText (dialogID, idxBtn, txtButton.c_str ());
				DGShowItem (dialogID, idxBtn);
				btnPosX += 50;
			}

			// ���� ���� ���� üũ �ڽ�
			btnPosX = 219, btnPosY = 114;
			for (xx = 0 ; xx < placingZone.eu_count_ver - 1 ; ++xx) {
				idxBtn = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, btnPosX, btnPosY, 25, 25);
				lastIdxBtn = idxBtn;
				DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL);

				txtButton = "L";
				DGSetItemText (dialogID, idxBtn, txtButton.c_str ());
				DGShowItem (dialogID, idxBtn);
				btnPosY += 50;
			}

			// ������ ���� ���� üũ �ڽ�
			btnPosX = 220 + 25 + (btnSizeX * placingZone.eu_count_hor), btnPosY = 114;
			for (xx = 0 ; xx < placingZone.eu_count_ver - 1 ; ++xx) {
				idxBtn = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, btnPosX, btnPosY, 25, 25);
				lastIdxBtn = idxBtn;
				DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL);

				txtButton = "R";
				DGSetItemText (dialogID, idxBtn, txtButton.c_str ());
				DGShowItem (dialogID, idxBtn);
				btnPosY += 50;
			}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case PUSHBUTTON_CONFIRM_REMAIN_LENGTH:
					// �������� �ʰ� ���� ���� �Ÿ��� �׸��� ��ư �Ӽ��� ������
					item = 0;

					//// �� ����(Ÿ�� �� ũ��) ���� �߻�, ��� ���� ��ġ ���� ������Ʈ
					//alignPlacingZoneForWall (&placingZone);

					//// ��ư �ε��� iteration �غ�
					//idxBtn = itemInitIdx;
					//
					//// �׸��� ��ư �ؽ�Ʈ ������Ʈ
					//for (xx = 0 ; xx < placingZone.eu_count_ver ; ++xx) {
					//	for (yy = 0 ; yy < placingZone.nCells ; yy += 2) {

					//		// �� �ε����� ��ư �ε����� ����
					//		idxCell = ((idxBtn - itemInitIdx) * 2) - (xx * (placingZone.eu_count_hor + 2)) * 2;		// ��ư �ε����� �� �ε����� ����

					//		txtButton = "";
					//		if (placingZone.cells [0][yy].objType == NONE) {
					//			txtButton = "NONE";
					//		} else if (placingZone.cells [0][yy].objType == INCORNER) {
					//			txtButton = format_string ("���ڳ�\n��%.0f\n��%.0f", placingZone.cells [0][yy].horLen * 1000, placingZone.cells [0][yy].verLen * 1000);
					//		} else if (placingZone.cells [0][yy].objType == EUROFORM) {
					//			if (placingZone.cells [0][yy].libPart.form.u_ins_wall)
					//				txtButton = format_string ("������\n(����)\n��%.0f\n��%.0f", placingZone.cells [0][yy].horLen * 1000, placingZone.cells [0][yy].verLen * 1000);
					//			else
					//				txtButton = format_string ("������\n(����)\n��%.0f\n��%.0f", placingZone.cells [0][yy].horLen * 1000, placingZone.cells [0][yy].verLen * 1000);
					//		} else if (placingZone.cells [0][yy].objType == FILLERSPACER) {
					//			txtButton = format_string ("�ٷ�\n��%.0f\n��%.0f", placingZone.cells [0][yy].horLen * 1000, placingZone.cells [0][yy].verLen * 1000);
					//		} else if (placingZone.cells [0][yy].objType == PLYWOOD) {
					//			if (placingZone.cells [0][yy].libPart.plywood.w_dir_wall)
					//				txtButton = format_string ("����\n(����)\n��%.0f\n��%.0f", placingZone.cells [0][yy].horLen * 1000, placingZone.cells [0][yy].verLen * 1000);
					//			else
					//				txtButton = format_string ("����\n(����)\n��%.0f\n��%.0f", placingZone.cells [0][yy].horLen * 1000, placingZone.cells [0][yy].verLen * 1000);
					//		} else if (placingZone.cells [0][yy].objType == WOOD) {
					//			txtButton = format_string ("����\n��%.0f\n��%.0f", placingZone.cells [0][yy].horLen * 1000, placingZone.cells [0][yy].verLen * 1000);
					//		}
					//		DGSetItemText (dialogID, idxBtn, txtButton.c_str ());		// �׸��� ��ư �ؽ�Ʈ ����
					//		
					//		// ���� ��ư ���� ���� '����'�� �ƴ϶�� �ش� ���� �۲��� ������
					//		if ( (idxCell > 0) && (idxCell < (placingZone.nCells - 1)) ) {
					//			idxCell_prev = idxCell - 1;
					//			idxCell_next = idxCell + 1;
					//		} else if (idxCell == 0) {
					//			idxCell_prev = -1;
					//			idxCell_next = idxCell + 1;
					//		} else if (idxCell == (placingZone.nCells - 1)) {
					//			idxCell_prev = idxCell - 1;
					//			idxCell_next = -1;
					//		}

					//		// ���� ���� ��ü ������ NONE�� �ƴϸ� ��ư �۲� ����
					//		DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL | DG_IS_PLAIN);
					//		if (yy == 0) {
					//			if (placingZone.cells [0][yy+1].objType != NONE)
					//				DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL | DG_IS_BOLD);
					//		} else if ( (yy > 0) && (yy < (placingZone.nCells - 2)) ) {
					//			if ( (placingZone.cells [0][yy-1].objType != NONE) || (placingZone.cells [0][yy+1].objType != NONE) )
					//				DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL | DG_IS_BOLD);
					//		} else if ( yy == (placingZone.nCells - 1) ) {
					//			if (placingZone.cells [0][yy-1].objType != NONE)
					//				DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL | DG_IS_BOLD);
					//		}

					//		++idxBtn;
					//	}
					//}

					//// ���� ���� ���� ������Ʈ
					//DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH, placingZone.remain_hor_updated);
					//DGSetItemFont (dialogID, PUSHBUTTON_CONFIRM_REMAIN_LENGTH, DG_IS_LARGE | DG_IS_BOLD);

				case DG_OK:
					// �������� �ʰ� ��ġ�� ��ü�� ���� �� ���ġ�ϰ� �׸��� ��ư �Ӽ��� ������
					item = 0;

					//// �� ����(Ÿ�� �� ũ��) ���� �߻�, ��� ���� ��ġ ���� ������Ʈ
					//alignPlacingZoneForWall (&placingZone);

					//// ��ư �ε��� iteration �غ�
					//idxBtn = itemInitIdx;

					//// �׸��� ��ư �ؽ�Ʈ ������Ʈ
					//for (xx = 0 ; xx < placingZone.eu_count_ver ; ++xx) {
					//	for (yy = 0 ; yy < placingZone.nCells ; yy += 2) {

					//		// �� �ε����� ��ư �ε����� ����
					//		idxCell = ((idxBtn - itemInitIdx) * 2) - (xx * (placingZone.eu_count_hor + 2)) * 2;		// ��ư �ε����� �� �ε����� ����

					//		txtButton = "";
					//		if (placingZone.cells [0][yy].objType == NONE) {
					//			txtButton = "NONE";
					//		} else if (placingZone.cells [0][yy].objType == INCORNER) {
					//			txtButton = format_string ("���ڳ�\n��%.0f\n��%.0f", placingZone.cells [0][yy].horLen * 1000, placingZone.cells [0][yy].verLen * 1000);
					//		} else if (placingZone.cells [0][yy].objType == EUROFORM) {
					//			if (placingZone.cells [0][yy].libPart.form.u_ins_wall)
					//				txtButton = format_string ("������\n(����)\n��%.0f\n��%.0f", placingZone.cells [0][yy].horLen * 1000, placingZone.cells [0][yy].verLen * 1000);
					//			else
					//				txtButton = format_string ("������\n(����)\n��%.0f\n��%.0f", placingZone.cells [0][yy].horLen * 1000, placingZone.cells [0][yy].verLen * 1000);
					//		} else if (placingZone.cells [0][yy].objType == FILLERSPACER) {
					//			txtButton = format_string ("�ٷ�\n��%.0f\n��%.0f", placingZone.cells [0][yy].horLen * 1000, placingZone.cells [0][yy].verLen * 1000);
					//		} else if (placingZone.cells [0][yy].objType == PLYWOOD) {
					//			if (placingZone.cells [0][yy].libPart.plywood.w_dir_wall)
					//				txtButton = format_string ("����\n(����)\n��%.0f\n��%.0f", placingZone.cells [0][yy].horLen * 1000, placingZone.cells [0][yy].verLen * 1000);
					//			else
					//				txtButton = format_string ("����\n(����)\n��%.0f\n��%.0f", placingZone.cells [0][yy].horLen * 1000, placingZone.cells [0][yy].verLen * 1000);
					//		} else if (placingZone.cells [0][yy].objType == WOOD) {
					//			txtButton = format_string ("����\n��%.0f\n��%.0f", placingZone.cells [0][yy].horLen * 1000, placingZone.cells [0][yy].verLen * 1000);
					//		}
					//		DGSetItemText (dialogID, idxBtn, txtButton.c_str ());		// �׸��� ��ư �ؽ�Ʈ ����

					//		// ���� ��ư ���� ���� '����'�� �ƴ϶�� �ش� ���� �۲��� ������
					//		if ( (idxCell > 0) && (idxCell < (placingZone.nCells - 1)) ) {
					//			idxCell_prev = idxCell - 1;
					//			idxCell_next = idxCell + 1;
					//		} else if (idxCell == 0) {
					//			idxCell_prev = -1;
					//			idxCell_next = idxCell + 1;
					//		} else if (idxCell == (placingZone.nCells - 1)) {
					//			idxCell_prev = idxCell - 1;
					//			idxCell_next = -1;
					//		}

					//		// ���� ���� ��ü ������ NONE�� �ƴϸ� ��ư �۲� ����
					//		DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL | DG_IS_PLAIN);
					//		if (yy == 0) {
					//			if (placingZone.cells [0][yy+1].objType != NONE)
					//				DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL | DG_IS_BOLD);
					//		} else if ( (yy > 0) && (yy < (placingZone.nCells - 2)) ) {
					//			if ( (placingZone.cells [0][yy-1].objType != NONE) || (placingZone.cells [0][yy+1].objType != NONE) )
					//				DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL | DG_IS_BOLD);
					//		} else if ( yy == (placingZone.nCells - 1) ) {
					//			if (placingZone.cells [0][yy-1].objType != NONE)
					//				DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL | DG_IS_BOLD);
					//		}

					//		++idxBtn;
					//	}
					//}

					//// ���� ���� ���� ������Ʈ
					//DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH, placingZone.remain_hor_updated);
					//DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_BOLD);

					//err = ACAPI_CallUndoableCommand ("������ ���ġ", [&] () -> GSErrCode {
					//	// ���� ��ġ�� ��ü ���� ����
					//	for (xx = 0 ; xx < placingZone.eu_count_ver ; ++xx) {
					//		for (yy = 0 ; yy < placingZone.nCells ; ++ yy) {
					//			elem.header.guid = placingZone.cells [xx][yy].guid;
					//			if (ACAPI_Element_Get (&elem) != NoError)
					//				continue;

					//			API_Elem_Head* headList = new API_Elem_Head [1];
					//			headList [0] = elem.header;
					//			err = ACAPI_Element_Delete (&headList, 1);
					//			delete headList;
					//		}
					//	}

					//	for (xx = 0 ; xx < placingZoneBackside.eu_count_ver ; ++xx) {
					//		for (yy = 0 ; yy < placingZoneBackside.nCells ; ++ yy) {
					//			elem.header.guid = placingZoneBackside.cells [xx][yy].guid;
					//			if (ACAPI_Element_Get (&elem) != NoError)
					//				continue;

					//			API_Elem_Head* headList = new API_Elem_Head [1];
					//			headList [0] = elem.header;
					//			err = ACAPI_Element_Delete (&headList, 1);
					//			delete headList;
					//		}
					//	}

					//	// ������Ʈ�� �� ������� ��ü ���ġ
					//	//////////////////////////////////////////////////////////// �� ����
					//	for (xx = 0 ; xx < placingZone.eu_count_ver ; ++xx)
					//		for (yy = 0 ; yy < placingZone.nCells ; ++yy)
					//			placingZone.cells [xx][yy].guid = placeLibPartForWall (placingZone.cells [xx][yy]);

					//	//////////////////////////////////////////////////////////// �� ����
					//	for (xx = 0 ; xx < placingZoneBackside.eu_count_ver ; ++xx)
					//		for (yy = 0 ; yy < placingZoneBackside.nCells ; ++yy)
					//			placingZoneBackside.cells [xx][yy].guid = placeLibPartForWall (placingZoneBackside.cells [xx][yy]);

					//	return err;
					//});

					clickedOKButton = true;

					break;
				case DG_CANCEL:
					break;

				default:
					// [DIALOG] �׸��� ��ư�� ������ Cell�� �����ϱ� ���� ���� â(3��° ���̾�α�)�� ����
					clickedBtnItemIdx = item;
					result = DGBlankModalDialog (240, 260, DG_DLG_NOGROW, 0, DG_DLG_NORMALFRAME, slabBottomPlacerHandler3, 0);

					item = 0;	// �׸��� ��ư�� ������ �� â�� ������ �ʰ� ��

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

// 2�� ���̾�α׿��� �� ���� ��ü Ÿ���� �����ϱ� ���� 3�� ���̾�α�
short DGCALLBACK slabBottomPlacerHandler3 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	idxItem;
	short	idxCell;
	short	popupSelectedIdx = 0;
	short	xx;
	double	temp;
	short	rIdx, cIdx;		// �� ��ȣ, �� ��ȣ

	switch (message) {
		case DG_MSG_INIT:

			// slabBottomPlacerHandler2 ���� Ŭ���� �׸��� ��ư�� �ε��� ���� �̿��Ͽ� �� �ε��� �� �ε�
			idxCell = (clickedBtnItemIdx - itemInitIdx);
			rIdx = 0;
			while (idxCell >= (placingZone.eu_count_hor)) {
				idxCell -= ((placingZone.eu_count_hor));
				++rIdx;
			}
			cIdx = idxCell;
			
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, "Cell �� ����");

			//////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			// ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 215, 70, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "����");
			DGShowItem (dialogID, DG_OK);

			// ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 130, 215, 70, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "���");
			DGShowItem (dialogID, DG_CANCEL);

			////////////////////////////////////////////////////////////// �ʵ� ���� (Ŭ���� ��)
			//// ��: ��ü Ÿ��
			//idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+240, 15, 70, 23);
			//DGSetItemFont (dialogID, LABEL_OBJ_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, LABEL_OBJ_TYPE, "��ü Ÿ��");
			//DGShowItem (dialogID, LABEL_OBJ_TYPE);

			//// �˾���Ʈ��: ��ü Ÿ���� �ٲ� �� �ִ� �޺��ڽ��� �� ���� ����
			//DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100+240, 20-7, 120, 25);
			//DGSetItemFont (dialogID, POPUP_OBJ_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			//DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			//DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, "����");
			//DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			//DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, "���ڳ��ǳ�");
			//DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			//DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, "������");
			//DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			//DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, "�ٷ������̼�");
			//DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			//DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, "����");
			//DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			//DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, "����");
			//DGShowItem (dialogID, POPUP_OBJ_TYPE);

			//// ��: �ʺ�
			//idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+240, 50, 70, 23);
			//DGSetItemFont (dialogID, LABEL_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, LABEL_WIDTH, "�ʺ�");

			//// Edit ��Ʈ��: �ʺ�
			//DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100+240, 50-6, 50, 25);
			//DGSetItemFont (dialogID, EDITCONTROL_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);

			//// ��: ����
			//idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+240, 80, 70, 23);
			//DGSetItemFont (dialogID, LABEL_HEIGHT, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, LABEL_HEIGHT, "����");

			//// Edit ��Ʈ��: ����
			//DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100+240, 80-6, 50, 25);
			//DGSetItemFont (dialogID, EDITCONTROL_HEIGHT, DG_IS_LARGE | DG_IS_PLAIN);

			//// ��: �β�
			//idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+240, 110, 70, 23);
			//DGSetItemFont (dialogID, LABEL_THK, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, LABEL_THK, "�β�");

			//// Edit ��Ʈ��: �β�
			//DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100+240, 110-6, 50, 25);
			//DGSetItemFont (dialogID, EDITCONTROL_THK, DG_IS_LARGE | DG_IS_PLAIN);

			//// ��: ��ġ����
			//idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+240, 140, 70, 23);
			//DGSetItemFont (dialogID, LABEL_ORIENTATION, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, LABEL_ORIENTATION, "��ġ����");
			//	
			//// ���� ��ư: ��ġ���� (�������)
			//idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 777, 100+240, 140-6, 70, 25);
			//DGSetItemFont (dialogID, RADIO_ORIENTATION_1_PLYWOOD, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, RADIO_ORIENTATION_1_PLYWOOD, "�������");
			//// ���� ��ư: ��ġ���� (��������)
			//idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 777, 100+240, 170-6, 70, 25);
			//DGSetItemFont (dialogID, RADIO_ORIENTATION_2_PLYWOOD, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, RADIO_ORIENTATION_2_PLYWOOD, "��������");

			//// üũ�ڽ�: �԰���
			//DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 20+240, 50, 70, 25-5);
			//DGSetItemFont (dialogID, CHECKBOX_SET_STANDARD, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, CHECKBOX_SET_STANDARD, "�԰���");

			//// ��: �ʺ�
			//idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+240, 80, 70, 23);
			//DGSetItemFont (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS, "�ʺ�");

			//// �˾� ��Ʈ��: �ʺ�
			//DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100+240, 80-7, 100, 25);
			//DGSetItemFont (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			//DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM);
			//DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM, "600");
			//DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM);
			//DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM, "500");
			//DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM);
			//DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM, "450");
			//DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM);
			//DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM, "400");
			//DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM);
			//DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM, "300");
			//DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM);
			//DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM, "200");

			//// Edit ��Ʈ��: �ʺ�
			//DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100+240, 80-6, 50, 25);
			//DGSetItemFont (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);

			//// ��: ����
			//idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+240, 110, 70, 23);
			//DGSetItemFont (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS, "����");

			//// �˾� ��Ʈ��: ����
			//DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100+240, 110-7, 100, 25);
			//DGSetItemFont (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			//DGPopUpInsertItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM);
			//DGPopUpSetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM, "1200");
			//DGPopUpInsertItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM);
			//DGPopUpSetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM, "900");
			//DGPopUpInsertItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM);
			//DGPopUpSetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM, "600");

			//// Edit ��Ʈ��: ����
			//DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100+240, 110-6, 50, 25);
			//DGSetItemFont (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			//
			//// ��: ��ġ����
			//idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20+240, 140, 70, 23);
			//DGSetItemFont (dialogID, LABEL_EUROFORM_ORIENTATION_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, LABEL_EUROFORM_ORIENTATION_OPTIONS, "��ġ����");
			//
			//// ���� ��ư: ��ġ���� (�������)
			//idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 778, 100+240, 140-6, 70, 25);
			//DGSetItemFont (dialogID, RADIO_ORIENTATION_1_EUROFORM, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, RADIO_ORIENTATION_1_EUROFORM, "�������");
			//// ���� ��ư: ��ġ���� (��������)
			//idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 778, 100+240, 170-6, 70, 25);
			//DGSetItemFont (dialogID, RADIO_ORIENTATION_2_EUROFORM, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, RADIO_ORIENTATION_2_EUROFORM, "��������");

			//// �ʱ� �Է� �ʵ� ǥ��
			//if (placingZone.cells [0][idxCell].objType == INCORNER) {
			//	DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, INCORNER + 1);

			//	// ��: �ʺ�
			//	DGShowItem (dialogID, LABEL_WIDTH);

			//	// Edit ��Ʈ��: �ʺ�
			//	DGShowItem (dialogID, EDITCONTROL_WIDTH);
			//	if (idxCell == 0)
			//		DGSetItemValDouble (dialogID, EDITCONTROL_WIDTH, placingZone.cells [0][idxCell].libPart.incorner.leng_s);
			//	else if (idxCell > 0)
			//		DGSetItemValDouble (dialogID, EDITCONTROL_WIDTH, placingZone.cells [0][idxCell].libPart.incorner.wid_s);
			//	DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH, 0.080);
			//	DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH, 0.500);

			//	// �� ����
			//	DGShowItem (dialogID, LABEL_HEIGHT);

			//	// Edit ��Ʈ��: ����
			//	DGShowItem (dialogID, EDITCONTROL_HEIGHT);
			//	DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT, placingZone.cells [0][idxCell].libPart.incorner.hei_s);
			//	DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT, 0.050);
			//	DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT, 1.500);

			//} else if (placingZone.cells [0][idxCell].objType == EUROFORM) {
			//	DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, EUROFORM + 1);

			//	// üũ�ڽ�: �԰���
			//	DGShowItem (dialogID, CHECKBOX_SET_STANDARD);
			//	DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD, placingZone.cells [0][idxCell].libPart.form.eu_stan_onoff);

			//	if (placingZone.cells [0][idxCell].libPart.form.eu_stan_onoff == true) {
			//		// ��: �ʺ�
			//		DGShowItem (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS);

			//		// �˾� ��Ʈ��: �ʺ�
			//		DGShowItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS);
			//		if (abs(placingZone.cells [0][idxCell].libPart.form.eu_wid - 0.600) < EPS)		popupSelectedIdx = 1;
			//		if (abs(placingZone.cells [0][idxCell].libPart.form.eu_wid - 0.500) < EPS)		popupSelectedIdx = 2;
			//		if (abs(placingZone.cells [0][idxCell].libPart.form.eu_wid - 0.450) < EPS)		popupSelectedIdx = 3;
			//		if (abs(placingZone.cells [0][idxCell].libPart.form.eu_wid - 0.400) < EPS)		popupSelectedIdx = 4;
			//		if (abs(placingZone.cells [0][idxCell].libPart.form.eu_wid - 0.300) < EPS)		popupSelectedIdx = 5;
			//		if (abs(placingZone.cells [0][idxCell].libPart.form.eu_wid - 0.200) < EPS)		popupSelectedIdx = 6;
			//		DGPopUpSelectItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, popupSelectedIdx);

			//		// ��: ����
			//		DGShowItem (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS);

			//		// �˾� ��Ʈ��: ����
			//		DGShowItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS);
			//		if (abs(placingZone.cells [0][idxCell].libPart.form.eu_hei - 1.200) < EPS)		popupSelectedIdx = 1;
			//		if (abs(placingZone.cells [0][idxCell].libPart.form.eu_hei - 0.900) < EPS)		popupSelectedIdx = 2;
			//		if (abs(placingZone.cells [0][idxCell].libPart.form.eu_hei - 0.600) < EPS)		popupSelectedIdx = 3;
			//		DGPopUpSelectItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, popupSelectedIdx);
			//	} else if (placingZone.cells [0][idxCell].libPart.form.eu_stan_onoff == false) {
			//		// ��: �ʺ�
			//		DGShowItem (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS);

			//		// Edit ��Ʈ��: �ʺ�
			//		DGShowItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS);
			//		DGSetItemValDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS, placingZone.cells [0][idxCell].libPart.form.eu_wid2);
			//		DGSetItemMinDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS, 0.050);
			//		DGSetItemMaxDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS, 0.900);

			//		// ��: ����
			//		DGShowItem (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS);

			//		// Edit ��Ʈ��: ����
			//		DGShowItem (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS);
			//		DGSetItemValDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS, placingZone.cells [0][idxCell].libPart.form.eu_hei2);
			//		DGSetItemMinDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS, 0.050);
			//		DGSetItemMaxDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS, 1.500);
			//	}

			//	// ��: ��ġ����
			//	DGShowItem (dialogID, LABEL_EUROFORM_ORIENTATION_OPTIONS);
			//	
			//	// ���� ��ư: ��ġ���� (�������)
			//	DGShowItem (dialogID, RADIO_ORIENTATION_1_EUROFORM);
			//	// ���� ��ư: ��ġ���� (��������)
			//	DGShowItem (dialogID, RADIO_ORIENTATION_2_EUROFORM);

			//	if (placingZone.cells [0][idxCell].libPart.form.u_ins_wall == true) {
			//		DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM, true);
			//		DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_EUROFORM, false);
			//	} else if (placingZone.cells [0][idxCell].libPart.form.u_ins_wall == false) {
			//		DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM, false);
			//		DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_EUROFORM, true);
			//	}
			//} else if (placingZone.cells [0][idxCell].objType == FILLERSPACER) {
			//	DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, FILLERSPACER + 1);

			//	// ��: �ʺ�
			//	DGShowItem (dialogID, LABEL_WIDTH);

			//	// Edit ��Ʈ��: �ʺ�
			//	DGShowItem (dialogID, EDITCONTROL_WIDTH);
			//	DGSetItemValDouble (dialogID, EDITCONTROL_WIDTH, placingZone.cells [0][idxCell].libPart.fillersp.f_thk);
			//	DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH, 0.010);
			//	DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH, 0.050);

			//	// �� ����
			//	DGShowItem (dialogID, LABEL_HEIGHT);

			//	// Edit ��Ʈ��: ����
			//	DGShowItem (dialogID, EDITCONTROL_HEIGHT);
			//	DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT, placingZone.cells [0][idxCell].libPart.fillersp.f_leng);
			//	DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT, 0.150);
			//	DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT, 2.400);

			//} else if (placingZone.cells [0][idxCell].objType == PLYWOOD) {
			//	DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, PLYWOOD + 1);

			//	// ��: �ʺ�
			//	DGShowItem (dialogID, LABEL_WIDTH);

			//	// Edit ��Ʈ��: �ʺ�
			//	DGShowItem (dialogID, EDITCONTROL_WIDTH);
			//	DGSetItemValDouble (dialogID, EDITCONTROL_WIDTH, placingZone.cells [0][idxCell].libPart.plywood.p_wid);
			//	DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH, 0.110);
			//	DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH, 1.220);

			//	// ��: ����
			//	DGShowItem (dialogID, LABEL_HEIGHT);

			//	// Edit ��Ʈ��: ����
			//	DGShowItem (dialogID, EDITCONTROL_HEIGHT);
			//	DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT, placingZone.cells [0][idxCell].libPart.plywood.p_leng);
			//	DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT, 0.110);
			//	DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT, 2.440);

			//	// ��: ��ġ����
			//	DGShowItem (dialogID, LABEL_ORIENTATION);
			//	
			//	// ���� ��ư: ��ġ���� (�������)
			//	DGShowItem (dialogID, RADIO_ORIENTATION_1_PLYWOOD);
			//	// ���� ��ư: ��ġ���� (��������)
			//	DGShowItem (dialogID, RADIO_ORIENTATION_2_PLYWOOD);

			//	if (placingZone.cells [0][idxCell].libPart.plywood.w_dir_wall == true) {
			//		DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_PLYWOOD, true);
			//		DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_PLYWOOD, false);
			//	} else if (placingZone.cells [0][idxCell].libPart.plywood.w_dir_wall == false) {
			//		DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_PLYWOOD, false);
			//		DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_PLYWOOD, true);
			//	}
			//} else if (placingZone.cells [0][idxCell].objType == WOOD) {
			//	DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, WOOD + 1);

			//	// ��: �ʺ�
			//	DGShowItem (dialogID, LABEL_WIDTH);

			//	// Edit ��Ʈ��: �ʺ�
			//	DGShowItem (dialogID, EDITCONTROL_WIDTH);
			//	DGSetItemValDouble (dialogID, EDITCONTROL_WIDTH, placingZone.cells [0][idxCell].libPart.wood.w_h);
			//	DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH, 0.005);
			//	DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH, 1.000);

			//	// ��: ����
			//	DGShowItem (dialogID, LABEL_HEIGHT);

			//	// Edit ��Ʈ��: ����
			//	DGShowItem (dialogID, EDITCONTROL_HEIGHT);
			//	DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT, placingZone.cells [0][idxCell].libPart.wood.w_leng);
			//	DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT, 0.010);
			//	DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT, 3.600);

			//	// ��: �β�
			//	DGShowItem (dialogID, LABEL_THK);

			//	// Edit ��Ʈ��: �β�
			//	DGShowItem (dialogID, EDITCONTROL_THK);
			//	DGSetItemValDouble (dialogID, EDITCONTROL_THK, placingZone.cells [0][idxCell].libPart.wood.w_w);
			//	DGSetItemMinDouble (dialogID, EDITCONTROL_THK, 0.005);
			//	DGSetItemMaxDouble (dialogID, EDITCONTROL_THK, 1.000);
			//}

			break;

		case DG_MSG_CHANGE:
			switch (item) {
				//case POPUP_OBJ_TYPE:	// ��ü Ÿ�� �޺��ڽ� ���� ������ ������ �Է� �ʵ尡 �޶��� (�����ؾ� �ϹǷ� Cell ���� �ҷ����� ����)
				//	//////////////////////////////////////////////////////////// �ʵ� ���� (Ŭ���� ��)
				//	// �ϴ� �׸��� �����, ��ü Ÿ�� ���� �׸� ǥ����
				//	DGHideItem (dialogID, LABEL_WIDTH);
				//	DGHideItem (dialogID, EDITCONTROL_WIDTH);
				//	DGHideItem (dialogID, LABEL_HEIGHT);
				//	DGHideItem (dialogID, EDITCONTROL_HEIGHT);
				//	DGHideItem (dialogID, LABEL_THK);
				//	DGHideItem (dialogID, EDITCONTROL_THK);
				//	DGHideItem (dialogID, LABEL_ORIENTATION);
				//	DGHideItem (dialogID, RADIO_ORIENTATION_1_PLYWOOD);
				//	DGHideItem (dialogID, RADIO_ORIENTATION_2_PLYWOOD);
				//	DGHideItem (dialogID, CHECKBOX_SET_STANDARD);
				//	DGHideItem (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS);
				//	DGHideItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS);
				//	DGHideItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS);
				//	DGHideItem (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS);
				//	DGHideItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS);
				//	DGHideItem (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS);
				//	DGHideItem (dialogID, LABEL_EUROFORM_ORIENTATION_OPTIONS);
				//	DGHideItem (dialogID, RADIO_ORIENTATION_1_EUROFORM);
				//	DGHideItem (dialogID, RADIO_ORIENTATION_2_EUROFORM);

				//	DGShowItem (dialogID, LABEL_OBJ_TYPE);
				//	DGShowItem (dialogID, POPUP_OBJ_TYPE);

				//	if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == INCORNER + 1) {
				//		// ��: �ʺ�
				//		DGShowItem (dialogID, LABEL_WIDTH);

				//		// Edit ��Ʈ��: �ʺ�
				//		DGShowItem (dialogID, EDITCONTROL_WIDTH);
				//		DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH, 0.080);
				//		DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH, 0.500);

				//		// �� ����
				//		DGShowItem (dialogID, LABEL_HEIGHT);

				//		// Edit ��Ʈ��: ����
				//		DGShowItem (dialogID, EDITCONTROL_HEIGHT);
				//		DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT, 0.050);
				//		DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT, 1.500);

				//	} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == EUROFORM + 1) {
				//		// üũ�ڽ�: �԰���
				//		DGShowItem (dialogID, CHECKBOX_SET_STANDARD);
				//		DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD, true);

				//		// ��: �ʺ�
				//		DGShowItem (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS);

				//		// ��: ����
				//		DGShowItem (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS);

				//		// �԰����̸�,
				//		if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == TRUE) {
				//			// �˾� ��Ʈ��: �ʺ�
				//			DGShowItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS);
				//			DGHideItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS);

				//			// �˾� ��Ʈ��: ����
				//			DGShowItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS);
				//			DGHideItem (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS);
				//		} else if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == FALSE) {
				//			// Edit ��Ʈ��: �ʺ�
				//			DGHideItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS);
				//			DGShowItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS);
				//			DGSetItemMinDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS, 0.050);
				//			DGSetItemMaxDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS, 0.900);

				//			// Edit ��Ʈ��: ����
				//			DGHideItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS);
				//			DGShowItem (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS);
				//			DGSetItemMinDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS, 0.050);
				//			DGSetItemMaxDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS, 1.500);
				//		}

				//		// ��: ��ġ����
				//		DGShowItem (dialogID, LABEL_EUROFORM_ORIENTATION_OPTIONS);
				//
				//		// ���� ��ư: ��ġ���� (�������)
				//		DGShowItem (dialogID, RADIO_ORIENTATION_1_EUROFORM);
				//		// ���� ��ư: ��ġ���� (��������)
				//		DGShowItem (dialogID, RADIO_ORIENTATION_2_EUROFORM);

				//		DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM, true);
				//		DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_EUROFORM, false);

				//	} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == FILLERSPACER + 1) {
				//		// ��: �ʺ�
				//		DGShowItem (dialogID, LABEL_WIDTH);

				//		// Edit ��Ʈ��: �ʺ�
				//		DGShowItem (dialogID, EDITCONTROL_WIDTH);
				//		DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH, 0.010);
				//		DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH, 0.050);

				//		// �� ����
				//		DGShowItem (dialogID, LABEL_HEIGHT);

				//		// Edit ��Ʈ��: ����
				//		DGShowItem (dialogID, EDITCONTROL_HEIGHT);
				//		DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT, 0.150);
				//		DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT, 2.400);

				//	} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == PLYWOOD + 1) {
				//		// ��: �ʺ�
				//		DGShowItem (dialogID, LABEL_WIDTH);

				//		// Edit ��Ʈ��: �ʺ�
				//		DGShowItem (dialogID, EDITCONTROL_WIDTH);
				//		DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH, 0.110);
				//		DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH, 1.220);

				//		// ��: ����
				//		DGShowItem (dialogID, LABEL_HEIGHT);

				//		// Edit ��Ʈ��: ����
				//		DGShowItem (dialogID, EDITCONTROL_HEIGHT);
				//		DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT, 0.110);
				//		DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT, 2.440);

				//		// ��: ��ġ����
				//		DGShowItem (dialogID, LABEL_ORIENTATION);
				//
				//		// ���� ��ư: ��ġ���� (�������)
				//		DGShowItem (dialogID, RADIO_ORIENTATION_1_PLYWOOD);
				//		// ���� ��ư: ��ġ���� (��������)
				//		DGShowItem (dialogID, RADIO_ORIENTATION_2_PLYWOOD);

				//		DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_PLYWOOD, true);
				//		DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_PLYWOOD, false);
				//	} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == WOOD + 1) {
				//		// ��: �ʺ�
				//		DGShowItem (dialogID, LABEL_WIDTH);

				//		// Edit ��Ʈ��: �ʺ�
				//		DGShowItem (dialogID, EDITCONTROL_WIDTH);
				//		DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH, 0.005);
				//		DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH, 1.000);

				//		// ��: ����
				//		DGShowItem (dialogID, LABEL_HEIGHT);

				//		// Edit ��Ʈ��: ����
				//		DGShowItem (dialogID, EDITCONTROL_HEIGHT);
				//		DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT, 0.010);
				//		DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT, 3.600);

				//		// ��: �β�
				//		DGShowItem (dialogID, LABEL_THK);

				//		// Edit ��Ʈ��: �β�
				//		DGShowItem (dialogID, EDITCONTROL_THK);
				//		DGSetItemMinDouble (dialogID, EDITCONTROL_THK, 0.005);
				//		DGSetItemMaxDouble (dialogID, EDITCONTROL_THK, 1.000);
				//	}

				//	break;

				//case CHECKBOX_SET_STANDARD:	// �������� ���, �԰��� üũ�ڽ� ���� �ٲ� ������ �ʺ�, ���� �Է� �ʵ� Ÿ���� �ٲ�
				//	//////////////////////////////////////////////////////////// �ʵ� ���� (Ŭ���� ��)
				//	if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == TRUE) {
				//		// �˾� ��Ʈ��: �ʺ�
				//		DGShowItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS);
				//		DGHideItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS);
				//		// �˾� ��Ʈ��: ����
				//		DGShowItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS);
				//		DGHideItem (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS);
				//	} else if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == FALSE) {
				//		// Edit ��Ʈ��: �ʺ�
				//		DGHideItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS);
				//		DGShowItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS);
				//		DGSetItemMinDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS, 0.050);
				//		DGSetItemMaxDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS, 0.900);
				//		// Edit ��Ʈ��: ����
				//		DGHideItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS);
				//		DGShowItem (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS);
				//		DGSetItemMinDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS, 0.050);
				//		DGSetItemMaxDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS, 1.500);
				//	}

				//	break;
			}

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:

					//// wallPlacerHandler2 ���� Ŭ���� �׸��� ��ư�� �ε��� ���� �̿��Ͽ� �� �ε��� �� �ε�
					//idxCell = (clickedBtnItemIdx - itemInitIdx) * 2;
					//while (idxCell >= ((placingZone.eu_count_hor + 2) * 2))
					//	idxCell -= ((placingZone.eu_count_hor + 2) * 2);

					//// ���� ���� �߰� ���̸�,
					//if ( (idxCell > 0) && (idxCell < (placingZone.nCells - 1)) ) {
					//	idxCell_prev = idxCell - 1;
					//	idxCell_next = idxCell + 1;
					//// ���� ���� �� ó�� ���̸�,
					//} else if (idxCell == 0) {
					//	idxCell_prev = -1;
					//	idxCell_next = idxCell + 1;
					//// ���� ���� �� �� ���̸�,
					//} else if (idxCell == (placingZone.nCells - 1)) {
					//	idxCell_prev = idxCell - 1;
					//	idxCell_next = -1;
					//}

					//for (xx = 0 ; xx < placingZone.eu_count_ver ; ++xx) {

					//	//////////////////////////////////////////////////////////// �ʵ� ���� (Ŭ���� ��)
					//	// �Է��� ���� �ٽ� ���� ����
					//	if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == NONE + 1) {
					//		placingZone.cells [xx][idxCell].objType = NONE;

					//	} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == INCORNER + 1) {
					//		placingZone.cells [xx][idxCell].objType = INCORNER;

					//		// �ʺ�
					//		if (idxCell == 0) {
					//			placingZone.cells [xx][idxCell].libPart.incorner.leng_s = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH);
					//			placingZone.cells [xx][idxCell].libPart.incorner.wid_s = 0.100;
					//		} else if (idxCell > 0) {
					//			placingZone.cells [xx][idxCell].libPart.incorner.leng_s = 0.100;
					//			placingZone.cells [xx][idxCell].libPart.incorner.wid_s = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH);
					//		}
					//		placingZone.cells [xx][idxCell].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH);

					//		// ����
					//		placingZone.cells [xx][idxCell].libPart.incorner.hei_s = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT);
					//		placingZone.cells [xx][idxCell].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT);
					//	} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == EUROFORM + 1) {
					//		placingZone.cells [xx][idxCell].objType = EUROFORM;

					//		// �԰���
					//		if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == TRUE)
					//			placingZone.cells [xx][idxCell].libPart.form.eu_stan_onoff = true;
					//		else if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == FALSE)
					//			placingZone.cells [xx][idxCell].libPart.form.eu_stan_onoff = false;

					//		if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == TRUE) {
					//			// �ʺ�
					//			placingZone.cells [xx][idxCell].libPart.form.eu_wid = atof (DGPopUpGetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DGPopUpGetSelected (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS)).ToCStr ()) / 1000.0;
					//			placingZone.cells [xx][idxCell].horLen = placingZone.cells [xx][idxCell].libPart.form.eu_wid;
					//			// ����
					//			placingZone.cells [xx][idxCell].libPart.form.eu_hei = atof (DGPopUpGetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DGPopUpGetSelected (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS)).ToCStr ()) / 1000.0;
					//			placingZone.cells [xx][idxCell].verLen = placingZone.cells [xx][idxCell].libPart.form.eu_hei;
					//		} else if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == FALSE) {
					//			// �ʺ�
					//			placingZone.cells [xx][idxCell].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS);
					//			placingZone.cells [xx][idxCell].horLen = placingZone.cells [xx][idxCell].libPart.form.eu_wid2;
					//			// ����
					//			placingZone.cells [xx][idxCell].libPart.form.eu_hei2 = DGGetItemValDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS);
					//			placingZone.cells [xx][idxCell].verLen = placingZone.cells [xx][idxCell].libPart.form.eu_hei2;
					//		}

					//		// ��ġ����
					//		if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM) == TRUE)
					//			placingZone.cells [xx][idxCell].libPart.form.u_ins_wall = true;
					//		else if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM) == FALSE) {
					//			placingZone.cells [xx][idxCell].libPart.form.u_ins_wall = false;
					//			// ����, ���� ���� ��ȯ
					//			temp = placingZone.cells [xx][idxCell].horLen;
					//			placingZone.cells [xx][idxCell].horLen = placingZone.cells [xx][idxCell].verLen;
					//			placingZone.cells [xx][idxCell].verLen = temp;
					//		}
					//	} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == FILLERSPACER + 1) {
					//		placingZone.cells [xx][idxCell].objType = FILLERSPACER;

					//		// �ʺ�
					//		placingZone.cells [xx][idxCell].libPart.fillersp.f_thk = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH);
					//		placingZone.cells [xx][idxCell].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH);

					//		// ����
					//		placingZone.cells [xx][idxCell].libPart.fillersp.f_leng = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT);
					//		placingZone.cells [xx][idxCell].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT);
					//	} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == PLYWOOD + 1) {
					//		placingZone.cells [xx][idxCell].objType = PLYWOOD;

					//		// �ʺ�
					//		placingZone.cells [xx][idxCell].libPart.plywood.p_wid = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH);
					//		placingZone.cells [xx][idxCell].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH);

					//		// ����
					//		placingZone.cells [xx][idxCell].libPart.plywood.p_leng = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT);
					//		placingZone.cells [xx][idxCell].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT);

					//		// ��ġ����
					//		if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_PLYWOOD) == TRUE)
					//			placingZone.cells [xx][idxCell].libPart.plywood.w_dir_wall = true;
					//		else if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_PLYWOOD) == FALSE) {
					//			placingZone.cells [xx][idxCell].libPart.plywood.w_dir_wall = false;
					//			// ����, ���� ���� ��ȯ
					//			temp = placingZone.cells [xx][idxCell].horLen;
					//			placingZone.cells [xx][idxCell].horLen = placingZone.cells [xx][idxCell].verLen;
					//			placingZone.cells [xx][idxCell].verLen = temp;
					//		}
					//	} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == WOOD + 1) {
					//		placingZone.cells [xx][idxCell].objType = WOOD;

					//		// �ʺ�
					//		placingZone.cells [xx][idxCell].libPart.wood.w_h = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH);
					//		placingZone.cells [xx][idxCell].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH);

					//		// ����
					//		placingZone.cells [xx][idxCell].libPart.wood.w_leng = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT);
					//		placingZone.cells [xx][idxCell].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT);

					//		// �β�
					//		placingZone.cells [xx][idxCell].libPart.wood.w_w = DGGetItemValDouble (dialogID, EDITCONTROL_THK);

					//		// ����: 90��
					//		placingZone.cells [xx][idxCell].libPart.wood.w_ang = DegreeToRad (90.0);
					//	}
					//}

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
