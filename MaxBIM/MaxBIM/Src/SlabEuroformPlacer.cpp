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
static bool				clickedPrevButton;		// ���� ��ư�� �������ϱ�?
static short			layerInd_Euroform;		// ���̾� ��ȣ: ������
static short			layerInd_Plywood;		// ���̾� ��ȣ: ����
static short			layerInd_Wood;			// ���̾� ��ȣ: ����
static short			itemInitIdx = GRIDBUTTON_IDX_START;		// �׸��� ��ư �׸� �ε��� ���� ��ȣ
static API_Coord3D		firstClickPoint;		// 1��°�� Ŭ���� ��
static short			TButtonStartIdx = 0;	// T ��ư ���� �ε���
static short			BButtonStartIdx = 0;	// B ��ư ���� �ε���
static short			LButtonStartIdx = 0;	// L ��ư ���� �ε���
static short			RButtonStartIdx = 0;	// R ��ư ���� �ε���
static GS::Array<API_Guid>	elemList;			// �׷�ȭ�� ���� ������ ��������� GUID�� ���� ������


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
	double			workLevel_slab;

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
	}
	if (err == APIERR_NOSEL) {
		ACAPI_WriteReport ("�ƹ� �͵� �������� �ʾҽ��ϴ�.\n�ʼ� ����: ������ (1��), ������ �Ϻθ� ���� ���� (1��)", true);
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
	infoSlab.level			= elem.slab.level;

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

	// ������ ���� ����
	infoMorph.guid		= elem.header.guid;
	infoMorph.floorInd	= elem.header.floorInd;
	infoMorph.level		= info3D.bounds.zMin;

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
			//if ( (abs (trCoord.z - elem.morph.level) < EPS) && (abs (elem.morph.level - trCoord.z) < EPS) )
				coords.Push (trCoord);
		}
	}
	nNodes = coords.GetSize ();

	// �ϴ� �� 2���� Ŭ��
	BNZeroMemory (&pointInfo, sizeof (API_GetPointType));
	CHCopyC ("������ �ϴ� ������ ���� ���� Ŭ���Ͻʽÿ�.", pointInfo.prompt);
	pointInfo.enableQuickSelection = true;
	err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
	point1 = pointInfo.pos;
	firstClickPoint = point1;

	BNZeroMemory (&pointInfo, sizeof (API_GetPointType));
	CHCopyC ("������ �ϴ� ������ ������ ���� Ŭ���Ͻʽÿ�.", pointInfo.prompt);
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
	API_Elem_Head* headList = new API_Elem_Head [1];
	headList [0] = elem.header;
	err = ACAPI_Element_Delete (&headList, 1);
	delete headList;

	bufPoint.x = 0.0;
	bufPoint.y = 0.0;
	bufPoint.z = 0.0;

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

FIRST:

	// [DIALOG] 1��° ���̾�α׿��� ������ ���� �Է� ����
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32512, ACAPI_GetOwnResModule (), slabBottomPlacerHandler1, 0);

	// �۾� �� ���� �ݿ�
	BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	workLevel_slab = 0.0;
	ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
	for (xx = 0 ; xx < (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
		if (storyInfo.data [0][xx].index == infoSlab.floorInd) {
			workLevel_slab = storyInfo.data [0][xx].level;
			break;
		}
	}
	BMKillHandle ((GSHandle *) &storyInfo.data);
	
	// ���� ������ �� ���� ����
	placingZone.level = infoSlab.level + infoSlab.offsetFromTop - infoSlab.thickness - placingZone.gap;

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
	placingZone.initCells (&placingZone);

	// ��ġ�� ���� ���� �Է�
	placingZone.firstPlacingSettings (&placingZone);

	// [DIALOG] 2��° ���̾�α׿��� ������ ��ġ�� �����ϰų� ���� ���縦 �����մϴ�.
	clickedOKButton = false;
	clickedPrevButton = false;
	result = DGBlankModalDialog (185, 290, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, slabBottomPlacerHandler2, 0);

	// ���� ��ư�� ������ 1��° ���̾�α� �ٽ� ����
	if (clickedPrevButton == true)
		goto FIRST;

	// 2��° ���̾�α׿��� OK ��ư�� �����߸� ���� �ܰ�� �Ѿ
	if (clickedOKButton == false)
		return err;

	// ������ ���� ä��� - ����, ����
	err = placingZone.fillRestAreas ();

	// ����� ��ü �׷�ȭ
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

	return	err;
}

// Cell �迭�� �ʱ�ȭ��
void	SlabPlacingZone::initCells (SlabPlacingZone* placingZone)
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
void	SlabPlacingZone::firstPlacingSettings (SlabPlacingZone* placingZone)
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

// �ش� ���� ������ �࿡ �ִ� �ٸ� ������ Ÿ�� �� ���̸� ������ (�ʺ�� �������� ����)
void	SlabPlacingZone::adjustOtherCellsInSameRow (SlabPlacingZone* target_zone, short row, short col)
{
	short	xx;
	bool	bStandardWidth;		// �԰��� �ʺ��ΰ�?
	bool	bStandardHeight;	// �԰��� �����ΰ�?
	
	// ��� ���鿡 ����
	for (xx = 0 ; xx < target_zone->eu_count_hor ; ++xx) {
		if (xx == col) continue;

		bStandardWidth = false;
		bStandardHeight = false;

		// �ش� ���� NONE�̸�, ������ ������ Ÿ�Ե� NONE
		if (target_zone->cells [row][col].objType == NONE) {

			target_zone->cells [row][xx].objType = NONE;

		// �ش� ���� EUROFORM�̸�, ������ ������ Ÿ�Ե� EUROFORM
		} else if (target_zone->cells [row][col].objType == EUROFORM) {

			target_zone->cells [row][xx].objType = EUROFORM;

			if (target_zone->cells [row][col].libPart.form.u_ins_wall == true) {
				// �ش� ���� ����� ���̰� �԰��� �����ΰ�?
				if ( (abs (target_zone->cells [row][col].verLen - 1.200) < EPS) ||
					 (abs (target_zone->cells [row][col].verLen - 0.900) < EPS) ||
					 (abs (target_zone->cells [row][col].verLen - 0.600) < EPS) )
					bStandardHeight = true;

				// ������ ���� �ʺ� �԰��� �ʺ��ΰ�?
				if ( (abs (target_zone->cells [row][xx].horLen - 0.600) < EPS) ||
					 (abs (target_zone->cells [row][xx].horLen - 0.500) < EPS) ||
					 (abs (target_zone->cells [row][xx].horLen - 0.450) < EPS) ||
					 (abs (target_zone->cells [row][xx].horLen - 0.400) < EPS) ||
					 (abs (target_zone->cells [row][xx].horLen - 0.300) < EPS) ||
					 (abs (target_zone->cells [row][xx].horLen - 0.200) < EPS) )
					bStandardWidth = true;

				// �ش� ���� ����� ���̿� ������ ���� �ʺ� ��� �԰� �������̸� �԰���
				if ( bStandardHeight && bStandardWidth ) {
					target_zone->cells [row][xx].libPart.form.eu_stan_onoff = true;
					target_zone->cells [row][xx].verLen = target_zone->cells [row][col].verLen;
					target_zone->cells [row][xx].libPart.form.eu_hei = target_zone->cells [row][col].verLen;
			
				// �� �ܿ��� ��԰���
				} else {
					target_zone->cells [row][xx].libPart.form.eu_stan_onoff = false;
					target_zone->cells [row][xx].verLen = target_zone->cells [row][col].verLen;
					target_zone->cells [row][xx].libPart.form.eu_hei2 = target_zone->cells [row][col].verLen;
				}
			} else {
				// �ش� ���� ����� ���̰� �԰��� �����ΰ�?
				if ( (abs (target_zone->cells [row][col].horLen - 1.200) < EPS) ||
					 (abs (target_zone->cells [row][col].horLen - 0.900) < EPS) ||
					 (abs (target_zone->cells [row][col].horLen - 0.600) < EPS) )
					bStandardHeight = true;

				// ������ ���� �ʺ� �԰��� �ʺ��ΰ�?
				if ( (abs (target_zone->cells [row][xx].verLen - 0.600) < EPS) ||
					 (abs (target_zone->cells [row][xx].verLen - 0.500) < EPS) ||
					 (abs (target_zone->cells [row][xx].verLen - 0.450) < EPS) ||
					 (abs (target_zone->cells [row][xx].verLen - 0.400) < EPS) ||
					 (abs (target_zone->cells [row][xx].verLen - 0.300) < EPS) ||
					 (abs (target_zone->cells [row][xx].verLen - 0.200) < EPS) )
					bStandardWidth = true;

				// �ش� ���� ����� ���̿� ������ ���� �ʺ� ��� �԰� �������̸� �԰���
				if ( bStandardHeight && bStandardWidth ) {
					target_zone->cells [row][xx].libPart.form.eu_stan_onoff = true;
					target_zone->cells [row][xx].verLen = target_zone->cells [row][col].verLen;
					target_zone->cells [row][xx].libPart.form.eu_wid = target_zone->cells [row][col].verLen;
			
				// �� �ܿ��� ��԰���
				} else {
					target_zone->cells [row][xx].libPart.form.eu_stan_onoff = false;
					target_zone->cells [row][xx].verLen = target_zone->cells [row][col].verLen;
					target_zone->cells [row][xx].libPart.form.eu_wid2 = target_zone->cells [row][col].verLen;
				}
			}
		}
	}
}

// �ش� ���� ������ ���� �ִ� �ٸ� ������ Ÿ�� �� �ʺ� ������ (���̴� �������� ����)
void	SlabPlacingZone::adjustOtherCellsInSameCol (SlabPlacingZone* target_zone, short row, short col)
{
	short	xx;
	bool	bStandardWidth;		// �԰��� �ʺ��ΰ�?
	bool	bStandardHeight;	// �԰��� �����ΰ�?

	// ��� ��鿡 ����
	for (xx = 0 ; xx < target_zone->eu_count_ver ; ++xx) {
		if (xx == row) continue;

		bStandardWidth = false;
		bStandardHeight = false;

		// �ش� ���� NONE�̸�, ������ ������ Ÿ�Ե� NONE
		if (target_zone->cells [row][col].objType == NONE) {

			target_zone->cells [xx][col].objType = NONE;

		// �ش� ���� EUROFORM�̸�, ������ ������ Ÿ�Ե� EUROFORM
		} else if (target_zone->cells [row][col].objType == EUROFORM) {

			target_zone->cells [xx][col].objType = EUROFORM;

			if (target_zone->cells [row][col].libPart.form.u_ins_wall == true) {
				// ������ ���� ���̰� �԰��� �����ΰ�?
				if ( (abs (target_zone->cells [xx][col].verLen - 1.200) < EPS) ||
					 (abs (target_zone->cells [xx][col].verLen - 0.900) < EPS) ||
					 (abs (target_zone->cells [xx][col].verLen - 0.600) < EPS) )
					bStandardHeight = true;

				// �ش� ���� ����� �ʺ� �԰��� �ʺ��ΰ�?
				if ( (abs (target_zone->cells [row][col].horLen - 0.600) < EPS) ||
					 (abs (target_zone->cells [row][col].horLen - 0.500) < EPS) ||
					 (abs (target_zone->cells [row][col].horLen - 0.450) < EPS) ||
					 (abs (target_zone->cells [row][col].horLen - 0.400) < EPS) ||
					 (abs (target_zone->cells [row][col].horLen - 0.300) < EPS) ||
					 (abs (target_zone->cells [row][col].horLen - 0.200) < EPS) )
					bStandardWidth = true;

				// �ش� ���� ����� �ʺ�� ������ ���� ���̰� ��� �԰� �������̸� �԰���
				if ( bStandardHeight && bStandardWidth ) {
					target_zone->cells [xx][col].libPart.form.eu_stan_onoff = true;
					target_zone->cells [xx][col].horLen = target_zone->cells [row][col].horLen;
					target_zone->cells [xx][col].libPart.form.eu_wid = target_zone->cells [row][col].horLen;
			
				// �� �ܿ��� ��԰���
				} else {
					target_zone->cells [xx][col].libPart.form.eu_stan_onoff = false;
					target_zone->cells [xx][col].horLen = target_zone->cells [row][col].horLen;
					target_zone->cells [xx][col].libPart.form.eu_wid2 = target_zone->cells [row][col].horLen;
				}
			} else {
				// ������ ���� ���̰� �԰��� �����ΰ�?
				if ( (abs (target_zone->cells [xx][col].horLen - 1.200) < EPS) ||
					 (abs (target_zone->cells [xx][col].horLen - 0.900) < EPS) ||
					 (abs (target_zone->cells [xx][col].horLen - 0.600) < EPS) )
					bStandardHeight = true;

				// �ش� ���� ����� �ʺ� �԰��� �ʺ��ΰ�?
				if ( (abs (target_zone->cells [row][col].verLen - 0.600) < EPS) ||
					 (abs (target_zone->cells [row][col].verLen - 0.500) < EPS) ||
					 (abs (target_zone->cells [row][col].verLen - 0.450) < EPS) ||
					 (abs (target_zone->cells [row][col].verLen - 0.400) < EPS) ||
					 (abs (target_zone->cells [row][col].verLen - 0.300) < EPS) ||
					 (abs (target_zone->cells [row][col].verLen - 0.200) < EPS) )
					bStandardWidth = true;

				// �ش� ���� ����� �ʺ�� ������ ���� ���̰� ��� �԰� �������̸� �԰���
				if ( bStandardHeight && bStandardWidth ) {
					target_zone->cells [xx][col].libPart.form.eu_stan_onoff = true;
					target_zone->cells [xx][col].horLen = target_zone->cells [row][col].horLen;
					target_zone->cells [xx][col].libPart.form.eu_hei = target_zone->cells [row][col].horLen;
			
				// �� �ܿ��� ��԰���
				} else {
					target_zone->cells [xx][col].libPart.form.eu_stan_onoff = false;
					target_zone->cells [xx][col].horLen = target_zone->cells [row][col].horLen;
					target_zone->cells [xx][col].libPart.form.eu_hei2 = target_zone->cells [row][col].horLen;
				}
			}
		}
	}
}

// ���ο� ���� �߰��� (�� �ϳ��� �ø��� �߰��� �࿡ ������ �� ���� ����)
void	SlabPlacingZone::addNewRow (SlabPlacingZone* target_zone)
{
	short	xx;

	// ���ο� �� �߰�
	target_zone->eu_count_ver ++;

	for (xx = 0 ; xx < target_zone->eu_count_hor ; ++xx)
		target_zone->cells [target_zone->eu_count_ver - 1][xx] = target_zone->cells [target_zone->eu_count_ver - 2][xx];
}

// ���ο� ���� �߰��� (�� �ϳ��� �ø��� �߰��� ���� ������ �� ���� ����)
void	SlabPlacingZone::addNewCol (SlabPlacingZone* target_zone)
{
	short	xx;

	// ���ο� �� �߰�
	target_zone->eu_count_hor ++;

	for (xx = 0 ; xx < target_zone->eu_count_ver ; ++xx)
		target_zone->cells [xx][target_zone->eu_count_hor - 1] = target_zone->cells [xx][target_zone->eu_count_hor - 2];
}

// ������ ���� ������
void	SlabPlacingZone::delLastRow (SlabPlacingZone* target_zone)
{
	target_zone->eu_count_ver --;
}

// ������ ���� ������
void	SlabPlacingZone::delLastCol (SlabPlacingZone* target_zone)
{
	target_zone->eu_count_hor --;
}

// Cell ������ ����ʿ� ���� ����ȭ�� ��ġ�� ��������
void	SlabPlacingZone::alignPlacingZone (SlabPlacingZone* target_zone)
{
	short			xx, yy, zz;
	double			dist_horizontal;
	double			dist_vertical;
	double			total_length;
	API_Coord3D		axisPoint, rotatedPoint, unrotatedPoint;

	// ���� �������� ���� �Ÿ� ���� �׸���� ������Ʈ��
	target_zone->remain_hor_updated = (placingZone.outerRight - placingZone.outerLeft);
	target_zone->remain_ver_updated = (placingZone.outerTop - placingZone.outerBottom);

	// ���� ���� ���� ����: �� ���� �ʺ�ŭ ����
	total_length = 0.0;
	for (xx = 0 ; xx < target_zone->eu_count_ver ; ++xx) {
		for (yy = 0 ; yy < target_zone->eu_count_hor ; ++yy)
			if (target_zone->cells [xx][yy].objType != NONE)
				total_length += target_zone->cells [xx][yy].horLen;
		
		// ���� �� ���� �ƴϸ� ���� Ż��
		if (abs (total_length) > EPS)
			xx = target_zone->eu_count_ver;
	}
	target_zone->formArrayWidth = total_length;
	target_zone->remain_hor_updated -= total_length;

	// ���� ���� ���� ����: �� ���� ���̸�ŭ ����
	total_length = 0.0;
	for (xx = 0 ; xx < target_zone->eu_count_hor ; ++xx) {
		for (yy = 0 ; yy < target_zone->eu_count_ver ; ++yy)
			if (target_zone->cells [yy][xx].objType != NONE)
				total_length += target_zone->cells [yy][xx].verLen;

		// ���� �� ���� �ƴϸ� ���� Ż��
		if (abs (total_length) > EPS)
			xx = target_zone->eu_count_hor;
	}
	target_zone->formArrayHeight = total_length;
	target_zone->remain_ver_updated -= total_length;

	// ��ü �� �ʺ�/���̸� Ȯ���� �� ���� ��ǥ�� �ű� ��
	target_zone->leftBottomX = (target_zone->outerLeft + target_zone->outerRight) / 2 - (target_zone->formArrayWidth / 2);
	target_zone->leftBottomY = (target_zone->outerTop + target_zone->outerBottom) / 2 + (target_zone->formArrayHeight / 2);
	target_zone->leftBottomZ = target_zone->level;
	
	// ���� ���� unrotated ��ġ�� ������Ʈ
	rotatedPoint.x = target_zone->leftBottomX;
	rotatedPoint.y = target_zone->leftBottomY;
	rotatedPoint.z = target_zone->leftBottomZ;
	unrotatedPoint = getUnrotatedPoint (rotatedPoint, firstClickPoint, RadToDegree (target_zone->ang));
	target_zone->leftBottomX = unrotatedPoint.x;
	target_zone->leftBottomY = unrotatedPoint.y;
	target_zone->leftBottomZ = unrotatedPoint.z;

	// �� Cell���� ��ġ �� ���� ������ ������Ʈ��
	for (xx = 0 ; xx < target_zone->eu_count_ver ; ++xx) {
		for (yy = 0 ; yy < target_zone->eu_count_hor ; ++yy) {

			dist_horizontal = 0.0;
			dist_vertical = 0.0;

			// ���� X ���� ���� �Ÿ��� ����
			for (zz = 0 ; zz < yy ; ++zz) {
				if (target_zone->cells [xx][zz].objType != NONE)
					dist_horizontal += target_zone->cells [xx][zz].horLen;
			}

			// ���� Y ���� ���� �Ÿ��� ����
			for (zz = 0 ; zz < xx ; ++zz) {
				if (target_zone->cells [zz][yy].objType != NONE)
					dist_vertical += target_zone->cells [zz][yy].verLen;
			}

			// �� ���� ������Ű�� ���� ��ġ ���� ������
			target_zone->cells [xx][yy].leftBottomX = target_zone->leftBottomX + dist_horizontal;
			target_zone->cells [xx][yy].leftBottomY = target_zone->leftBottomY - dist_vertical;
			target_zone->cells [xx][yy].leftBottomZ = target_zone->leftBottomZ;

			axisPoint.x = target_zone->leftBottomX;
			axisPoint.y = target_zone->leftBottomY;
			axisPoint.z = target_zone->leftBottomZ;

			rotatedPoint.x = target_zone->cells [xx][yy].leftBottomX;
			rotatedPoint.y = target_zone->cells [xx][yy].leftBottomY;
			rotatedPoint.z = target_zone->cells [xx][yy].leftBottomZ;
			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (target_zone->ang));

			target_zone->cells [xx][yy].leftBottomX = unrotatedPoint.x;
			target_zone->cells [xx][yy].leftBottomY = unrotatedPoint.y;
			target_zone->cells [xx][yy].leftBottomZ = unrotatedPoint.z;
		}
	}
}

// �ش� �� ������ ������� ���̺귯�� ��ġ
API_Guid	SlabPlacingZone::placeLibPart (CellForSlab objInfo)
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

	} else if (objInfo.objType == PLYWOOD) {
		element.header.layer = layerInd_Plywood;
		GS::ucscpy (memo.params [0][32].value.uStr, L("��԰�"));
		GS::ucscpy (memo.params [0][33].value.uStr, L("�ٴڱ��"));
		GS::ucscpy (memo.params [0][34].value.uStr, L("11.5T"));
		memo.params [0][35].value.real = objInfo.libPart.plywood.p_wid;		// ����
		memo.params [0][36].value.real = objInfo.libPart.plywood.p_leng;	// ����
		memo.params [0][38].value.real = FALSE;		// ����Ʋ OFF
		
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
GSErrCode	SlabPlacingZone::fillRestAreas (void)
{
	GSErrCode	err = NoError;
	short		xx;
	CellForSlab	insCell;
	double		startXPos, startYPos;
	API_Coord3D	axisPoint, rotatedPoint, unrotatedPoint;

	// �ָ��� ������ ���� GUID�� ������
	API_Guid	topAtLeftTop, topAtRightTop;
	API_Guid	bottomAtLeftBottom, bottomAtRightBottom;
	API_Guid	leftAtLeftTop, leftAtLeftBottom;
	API_Guid	rightAtRightTop, rightAtRightBottom;

	// ȸ������ �Ǵ� ��
	axisPoint.x = firstClickPoint.x;
	axisPoint.y = firstClickPoint.y;
	axisPoint.z = firstClickPoint.y;


	// ���� ��ġ (TOP)
	insCell.objType = PLYWOOD;
	insCell.ang = placingZone.ang;
	insCell.leftBottomX = axisPoint.x;
	insCell.leftBottomY = axisPoint.y - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2);
	insCell.leftBottomZ = placingZone.level;
	insCell.libPart.plywood.p_wid = (placingZone.outerTop - placingZone.outerBottom) / 2 - (placingZone.formArrayHeight / 2);
	insCell.libPart.plywood.p_leng = placingZone.cells [0][0].horLen + (placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) - (placingZone.corner_leftTop.x - placingZone.outerLeft);
	insCell.libPart.plywood.w_dir_wall = true;

	// ��ġ ���� ��ȸ�������� ��ȯ
	rotatedPoint.x = insCell.leftBottomX;
	rotatedPoint.y = insCell.leftBottomY;
	rotatedPoint.z = insCell.leftBottomZ;
	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
	insCell.leftBottomX = unrotatedPoint.x;
	insCell.leftBottomY = unrotatedPoint.y;
	insCell.leftBottomZ = unrotatedPoint.z;

	// �� ��ġ
	topAtLeftTop = placingZone.placeLibPart (insCell);
	elemList.Push (topAtLeftTop);

	startXPos = axisPoint.x - placingZone.corner_leftTop.x + placingZone.outerLeft + (placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) + placingZone.cells [0][0].horLen;
	startYPos = axisPoint.y - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2);
	for (xx = 1 ; xx < placingZone.eu_count_hor-1 ; ++xx) {
		insCell.objType = PLYWOOD;
		insCell.ang = placingZone.ang;
		insCell.leftBottomX = startXPos;
		insCell.leftBottomY = startYPos;
		insCell.leftBottomZ = placingZone.level;
		insCell.libPart.plywood.p_wid = (placingZone.outerTop - placingZone.outerBottom) / 2 - (placingZone.formArrayHeight / 2);
		insCell.libPart.plywood.p_leng = placingZone.cells [0][xx].horLen;
		insCell.libPart.plywood.w_dir_wall = true;

		// ��ġ ���� ��ȸ�������� ��ȯ
		rotatedPoint.x = insCell.leftBottomX;
		rotatedPoint.y = insCell.leftBottomY;
		rotatedPoint.z = insCell.leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
		insCell.leftBottomX = unrotatedPoint.x;
		insCell.leftBottomY = unrotatedPoint.y;
		insCell.leftBottomZ = unrotatedPoint.z;

		// �� ��ġ
		elemList.Push (placingZone.placeLibPart (insCell));

		// ���� �� ��ġ�� ���� ���� ��ǥ �̵�
		startXPos += placingZone.cells [0][xx].horLen;
	}

	insCell.objType = PLYWOOD;
	insCell.ang = placingZone.ang;
	insCell.leftBottomX = startXPos;
	insCell.leftBottomY = startYPos;
	insCell.leftBottomZ = placingZone.level;
	insCell.libPart.plywood.p_wid = (placingZone.outerTop - placingZone.outerBottom) / 2 - (placingZone.formArrayHeight / 2);
	insCell.libPart.plywood.p_leng = placingZone.cells [0][xx].horLen + (placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) - (placingZone.outerRight - placingZone.corner_rightTop.x);
	insCell.libPart.plywood.w_dir_wall = true;

	// ��ġ ���� ��ȸ�������� ��ȯ
	rotatedPoint.x = insCell.leftBottomX;
	rotatedPoint.y = insCell.leftBottomY;
	rotatedPoint.z = insCell.leftBottomZ;
	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
	insCell.leftBottomX = unrotatedPoint.x;
	insCell.leftBottomY = unrotatedPoint.y;
	insCell.leftBottomZ = unrotatedPoint.z;

	// �� ��ġ
	topAtRightTop = placingZone.placeLibPart (insCell);
	elemList.Push (topAtRightTop);


	// ���� ��ġ (BOTTOM)
	insCell.objType = PLYWOOD;
	insCell.ang = placingZone.ang;
	insCell.leftBottomX = axisPoint.x - (placingZone.corner_leftTop.x - placingZone.outerLeft) + (placingZone.corner_leftBottom.x - placingZone.outerLeft);
	insCell.leftBottomY = axisPoint.y - (placingZone.outerTop - placingZone.outerBottom);
	insCell.leftBottomZ = placingZone.level;
	insCell.libPart.plywood.p_wid = (placingZone.outerTop - placingZone.outerBottom) / 2 - (placingZone.formArrayHeight / 2);
	insCell.libPart.plywood.p_leng = placingZone.cells [0][0].horLen + (placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) - (placingZone.corner_leftBottom.x - placingZone.outerLeft);
	insCell.libPart.plywood.w_dir_wall = true;

	// ��ġ ���� ��ȸ�������� ��ȯ
	rotatedPoint.x = insCell.leftBottomX;
	rotatedPoint.y = insCell.leftBottomY;
	rotatedPoint.z = insCell.leftBottomZ;
	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
	insCell.leftBottomX = unrotatedPoint.x;
	insCell.leftBottomY = unrotatedPoint.y;
	insCell.leftBottomZ = unrotatedPoint.z;

	// �� ��ġ
	bottomAtLeftBottom = placingZone.placeLibPart (insCell);
	elemList.Push (bottomAtLeftBottom);

	startXPos = axisPoint.x - placingZone.corner_leftTop.x + placingZone.outerLeft + (placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) + placingZone.cells [0][0].horLen;
	startYPos = axisPoint.y - (placingZone.outerTop - placingZone.outerBottom);
	for (xx = 1 ; xx < placingZone.eu_count_hor-1 ; ++xx) {
		insCell.objType = PLYWOOD;
		insCell.ang = placingZone.ang;
		insCell.leftBottomX = startXPos;
		insCell.leftBottomY = startYPos;
		insCell.leftBottomZ = placingZone.level;
		insCell.libPart.plywood.p_wid = (placingZone.outerTop - placingZone.outerBottom) / 2 - (placingZone.formArrayHeight / 2);
		insCell.libPart.plywood.p_leng = placingZone.cells [0][xx].horLen;
		insCell.libPart.plywood.w_dir_wall = true;

		// ��ġ ���� ��ȸ�������� ��ȯ
		rotatedPoint.x = insCell.leftBottomX;
		rotatedPoint.y = insCell.leftBottomY;
		rotatedPoint.z = insCell.leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
		insCell.leftBottomX = unrotatedPoint.x;
		insCell.leftBottomY = unrotatedPoint.y;
		insCell.leftBottomZ = unrotatedPoint.z;

		// �� ��ġ
		elemList.Push (placingZone.placeLibPart (insCell));
			
		// ���� �� ��ġ�� ���� ���� ��ǥ �̵�
		startXPos += placingZone.cells [0][xx].horLen;
	}

	insCell.objType = PLYWOOD;
	insCell.ang = placingZone.ang;
	insCell.leftBottomX = startXPos;
	insCell.leftBottomY = startYPos;
	insCell.leftBottomZ = placingZone.level;
	insCell.libPart.plywood.p_wid = (placingZone.outerTop - placingZone.outerBottom) / 2 - (placingZone.formArrayHeight / 2);
	insCell.libPart.plywood.p_leng = placingZone.cells [0][xx].horLen + (placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) - (placingZone.outerRight - placingZone.corner_rightBottom.x);;
	insCell.libPart.plywood.w_dir_wall = true;

	// ��ġ ���� ��ȸ�������� ��ȯ
	rotatedPoint.x = insCell.leftBottomX;
	rotatedPoint.y = insCell.leftBottomY;
	rotatedPoint.z = insCell.leftBottomZ;
	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
	insCell.leftBottomX = unrotatedPoint.x;
	insCell.leftBottomY = unrotatedPoint.y;
	insCell.leftBottomZ = unrotatedPoint.z;

	// �� ��ġ
	bottomAtRightBottom = placingZone.placeLibPart (insCell);
	elemList.Push (bottomAtRightBottom);


	// ���� ��ġ (LEFT)
	insCell.objType = PLYWOOD;
	insCell.ang = placingZone.ang;
	insCell.leftBottomX = axisPoint.x - placingZone.corner_leftTop.x + placingZone.outerLeft;
	insCell.leftBottomY = axisPoint.y - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) - placingZone.cells [0][0].verLen;
	insCell.leftBottomZ = placingZone.level;
	insCell.libPart.plywood.p_wid = placingZone.cells [0][0].verLen + (placingZone.outerTop - placingZone.outerBottom) / 2 - (placingZone.formArrayHeight / 2) - (placingZone.outerTop - placingZone.corner_leftTop.y);
	insCell.libPart.plywood.p_leng = (placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2);
	insCell.libPart.plywood.w_dir_wall = true;

	// ��ġ ���� ��ȸ�������� ��ȯ
	rotatedPoint.x = insCell.leftBottomX;
	rotatedPoint.y = insCell.leftBottomY;
	rotatedPoint.z = insCell.leftBottomZ;
	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
	insCell.leftBottomX = unrotatedPoint.x;
	insCell.leftBottomY = unrotatedPoint.y;
	insCell.leftBottomZ = unrotatedPoint.z;

	// �� ��ġ
	leftAtLeftTop = placingZone.placeLibPart (insCell);
	elemList.Push (leftAtLeftTop);

	startXPos = axisPoint.x - placingZone.corner_leftTop.x + placingZone.outerLeft;
	startYPos = axisPoint.y - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) - placingZone.cells [0][0].verLen - placingZone.cells [1][0].verLen;
	for (xx = 1 ; xx < placingZone.eu_count_ver-1 ; ++xx) {
		insCell.objType = PLYWOOD;
		insCell.ang = placingZone.ang;
		insCell.leftBottomX = startXPos;
		insCell.leftBottomY = startYPos;
		insCell.leftBottomZ = placingZone.level;
		insCell.libPart.plywood.p_wid = placingZone.cells [xx][0].verLen;
		insCell.libPart.plywood.p_leng = (placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2);
		insCell.libPart.plywood.w_dir_wall = true;

		// ��ġ ���� ��ȸ�������� ��ȯ
		rotatedPoint.x = insCell.leftBottomX;
		rotatedPoint.y = insCell.leftBottomY;
		rotatedPoint.z = insCell.leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
		insCell.leftBottomX = unrotatedPoint.x;
		insCell.leftBottomY = unrotatedPoint.y;
		insCell.leftBottomZ = unrotatedPoint.z;

		// �� ��ġ
		elemList.Push (placingZone.placeLibPart (insCell));

		// ���� �� ��ġ�� ���� ���� ��ǥ �̵�
		startYPos -= placingZone.cells [xx+1][0].verLen;
	}

	insCell.objType = PLYWOOD;
	insCell.ang = placingZone.ang;
	insCell.leftBottomX = startXPos;
	insCell.leftBottomY = startYPos - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) + (placingZone.corner_leftBottom.y - placingZone.outerBottom);
	insCell.leftBottomZ = placingZone.level;
	insCell.libPart.plywood.p_wid = placingZone.cells [xx][0].verLen + (placingZone.outerTop - placingZone.outerBottom) / 2 - (placingZone.formArrayHeight / 2) - (placingZone.corner_leftBottom.y - placingZone.outerBottom);
	insCell.libPart.plywood.p_leng = (placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2);
	insCell.libPart.plywood.w_dir_wall = true;

	// ��ġ ���� ��ȸ�������� ��ȯ
	rotatedPoint.x = insCell.leftBottomX;
	rotatedPoint.y = insCell.leftBottomY;
	rotatedPoint.z = insCell.leftBottomZ;
	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
	insCell.leftBottomX = unrotatedPoint.x;
	insCell.leftBottomY = unrotatedPoint.y;
	insCell.leftBottomZ = unrotatedPoint.z;

	// �� ��ġ
	leftAtLeftBottom = placingZone.placeLibPart (insCell);
	elemList.Push (leftAtLeftBottom);


	// ���� ��ġ (RIGHT)
	insCell.objType = PLYWOOD;
	insCell.ang = placingZone.ang;
	insCell.leftBottomX = axisPoint.x - placingZone.corner_leftTop.x + placingZone.outerLeft + (placingZone.outerRight - placingZone.outerLeft) - (placingZone.outerRight - placingZone.outerLeft) / 2 + (placingZone.formArrayWidth / 2);
	insCell.leftBottomY = axisPoint.y - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) - placingZone.cells [0][0].verLen;
	insCell.leftBottomZ = placingZone.level;
	insCell.libPart.plywood.p_wid = placingZone.cells [0][0].verLen + (placingZone.outerTop - placingZone.outerBottom) / 2 - (placingZone.formArrayHeight / 2) - (placingZone.outerTop - placingZone.corner_rightTop.y);
	insCell.libPart.plywood.p_leng = (placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2);
	insCell.libPart.plywood.w_dir_wall = true;

	// ��ġ ���� ��ȸ�������� ��ȯ
	rotatedPoint.x = insCell.leftBottomX;
	rotatedPoint.y = insCell.leftBottomY;
	rotatedPoint.z = insCell.leftBottomZ;
	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
	insCell.leftBottomX = unrotatedPoint.x;
	insCell.leftBottomY = unrotatedPoint.y;
	insCell.leftBottomZ = unrotatedPoint.z;

	// �� ��ġ
	rightAtRightTop = placingZone.placeLibPart (insCell);
	elemList.Push (rightAtRightTop);

	startXPos = axisPoint.x - placingZone.corner_leftTop.x + placingZone.outerLeft + (placingZone.outerRight - placingZone.outerLeft) - (placingZone.outerRight - placingZone.outerLeft) / 2 + (placingZone.formArrayWidth / 2);
	startYPos = axisPoint.y - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) - placingZone.cells [0][0].verLen - placingZone.cells [1][0].verLen;
	for (xx = 1 ; xx < placingZone.eu_count_ver-1 ; ++xx) {
		insCell.objType = PLYWOOD;
		insCell.ang = placingZone.ang;
		insCell.leftBottomX = startXPos;
		insCell.leftBottomY = startYPos;
		insCell.leftBottomZ = placingZone.level;
		insCell.libPart.plywood.p_wid = placingZone.cells [xx][0].verLen;
		insCell.libPart.plywood.p_leng = (placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2);
		insCell.libPart.plywood.w_dir_wall = true;

		// ��ġ ���� ��ȸ�������� ��ȯ
		rotatedPoint.x = insCell.leftBottomX;
		rotatedPoint.y = insCell.leftBottomY;
		rotatedPoint.z = insCell.leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
		insCell.leftBottomX = unrotatedPoint.x;
		insCell.leftBottomY = unrotatedPoint.y;
		insCell.leftBottomZ = unrotatedPoint.z;

		// �� ��ġ
		elemList.Push (placingZone.placeLibPart (insCell));

		// ���� �� ��ġ�� ���� ���� ��ǥ �̵�
		startYPos -= placingZone.cells [xx+1][0].verLen;
	}

	insCell.objType = PLYWOOD;
	insCell.ang = placingZone.ang;
	insCell.leftBottomX = startXPos;
	insCell.leftBottomY = startYPos - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) + (placingZone.corner_rightBottom.y - placingZone.outerBottom);
	insCell.leftBottomZ = placingZone.level;
	insCell.libPart.plywood.p_wid = placingZone.cells [xx][0].verLen + (placingZone.outerTop - placingZone.outerBottom) / 2 - (placingZone.formArrayHeight / 2) - (placingZone.corner_rightBottom.y - placingZone.outerBottom);
	insCell.libPart.plywood.p_leng = (placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2);
	insCell.libPart.plywood.w_dir_wall = true;

	// ��ġ ���� ��ȸ�������� ��ȯ
	rotatedPoint.x = insCell.leftBottomX;
	rotatedPoint.y = insCell.leftBottomY;
	rotatedPoint.z = insCell.leftBottomZ;
	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
	insCell.leftBottomX = unrotatedPoint.x;
	insCell.leftBottomY = unrotatedPoint.y;
	insCell.leftBottomZ = unrotatedPoint.z;

	// �� ��ġ
	rightAtRightBottom = placingZone.placeLibPart (insCell);
	elemList.Push (rightAtRightBottom);

	// ���� �ڳ� ��ġ�� �κ��� �ָ��� �������� ����
	err = ACAPI_Element_SolidLink_Create (topAtLeftTop,			leftAtLeftTop, APISolid_Substract, APISolidFlag_OperatorAttrib);
	err = ACAPI_Element_SolidLink_Create (topAtRightTop,		rightAtRightTop, APISolid_Substract, APISolidFlag_OperatorAttrib);
	err = ACAPI_Element_SolidLink_Create (bottomAtLeftBottom,	leftAtLeftBottom, APISolid_Substract, APISolidFlag_OperatorAttrib);
	err = ACAPI_Element_SolidLink_Create (bottomAtRightBottom,	rightAtRightBottom, APISolid_Substract, APISolidFlag_OperatorAttrib);


	// ������ �ѷ� ���� ��ġ (TOP)
	startXPos = axisPoint.x - placingZone.corner_leftTop.x + placingZone.outerLeft + (placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2);
	startYPos = axisPoint.y - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) + 0.080;
	for (xx = 0 ; xx < placingZone.eu_count_hor ; ++xx) {
		insCell.objType = WOOD;
		insCell.ang = placingZone.ang;
		insCell.leftBottomX = startXPos;
		insCell.leftBottomY = startYPos;
		insCell.leftBottomZ = placingZone.level - 0.0115;
		insCell.libPart.wood.w_ang = 0.0;
		insCell.libPart.wood.w_h = 0.080;
		insCell.libPart.wood.w_leng = placingZone.cells [0][xx].horLen;
		insCell.libPart.wood.w_w = 0.050;

		// ��ġ ���� ��ȸ�������� ��ȯ
		rotatedPoint.x = insCell.leftBottomX;
		rotatedPoint.y = insCell.leftBottomY;
		rotatedPoint.z = insCell.leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
		insCell.leftBottomX = unrotatedPoint.x;
		insCell.leftBottomY = unrotatedPoint.y;
		insCell.leftBottomZ = unrotatedPoint.z;

		// �� ��ġ
		elemList.Push (placingZone.placeLibPart (insCell));

		// ���� �� ��ġ�� ���� ���� ��ǥ �̵�
		startXPos += placingZone.cells [0][xx].horLen;
	}


	// ������ �ѷ� ���� ��ġ (BOTTOM)
	startXPos = axisPoint.x - placingZone.corner_leftTop.x + placingZone.outerLeft + (placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2);
	startYPos = axisPoint.y - (placingZone.outerTop - placingZone.outerBottom) + (placingZone.outerTop - placingZone.outerBottom) / 2 - (placingZone.formArrayHeight / 2);
	for (xx = 0 ; xx < placingZone.eu_count_hor ; ++xx) {
		insCell.objType = WOOD;
		insCell.ang = placingZone.ang;
		insCell.leftBottomX = startXPos;
		insCell.leftBottomY = startYPos;
		insCell.leftBottomZ = placingZone.level - 0.0115;
		insCell.libPart.wood.w_ang = 0.0;
		insCell.libPart.wood.w_h = 0.080;
		insCell.libPart.wood.w_leng = placingZone.cells [0][xx].horLen;
		insCell.libPart.wood.w_w = 0.050;

		// ��ġ ���� ��ȸ�������� ��ȯ
		rotatedPoint.x = insCell.leftBottomX;
		rotatedPoint.y = insCell.leftBottomY;
		rotatedPoint.z = insCell.leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
		insCell.leftBottomX = unrotatedPoint.x;
		insCell.leftBottomY = unrotatedPoint.y;
		insCell.leftBottomZ = unrotatedPoint.z;

		// �� ��ġ
		elemList.Push (placingZone.placeLibPart (insCell));

		// ���� �� ��ġ�� ���� ���� ��ǥ �̵�
		startXPos += placingZone.cells [0][xx].horLen;
	}

	
	// ������ �ѷ� ���� ��ġ (LEFT) : �� ������ 90 ȸ���Ͽ� ���� ��ǥ �޺κ��� X,Y���� �ٲ��� ��
	insCell.objType = WOOD;
	insCell.ang = placingZone.ang + DegreeToRad (90.0);
	insCell.leftBottomX = axisPoint.x - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) - placingZone.cells [0][0].verLen;
	insCell.leftBottomY = axisPoint.y - (placingZone.outerRight - placingZone.outerLeft) / 2 + (placingZone.formArrayWidth / 2) + (placingZone.corner_leftTop.x - placingZone.outerLeft) + 0.080;
	insCell.leftBottomZ = placingZone.level - 0.0115;
	insCell.libPart.wood.w_ang = 0.0;
	insCell.libPart.wood.w_h = 0.080;
	insCell.libPart.wood.w_leng = placingZone.cells [0][0].verLen + 0.080;
	insCell.libPart.wood.w_w = 0.050;

	// ��ġ ���� ��ȸ�������� ��ȯ
	rotatedPoint.x = insCell.leftBottomX;
	rotatedPoint.y = insCell.leftBottomY;
	rotatedPoint.z = insCell.leftBottomZ;
	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
	insCell.leftBottomX = unrotatedPoint.x;
	insCell.leftBottomY = unrotatedPoint.y;
	insCell.leftBottomZ = unrotatedPoint.z;

	// �� ��ġ
	elemList.Push (placingZone.placeLibPart (insCell));

	startXPos = axisPoint.x - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) - placingZone.cells [0][0].verLen - placingZone.cells [1][0].verLen;
	startYPos = axisPoint.y - (placingZone.outerRight - placingZone.outerLeft) / 2 + (placingZone.formArrayWidth / 2) + (placingZone.corner_leftTop.x - placingZone.outerLeft) + 0.080;
	for (xx = 1 ; xx < placingZone.eu_count_ver-1 ; ++xx) {
		insCell.objType = WOOD;
		insCell.ang = placingZone.ang + DegreeToRad (90.0);
		insCell.leftBottomX = startXPos;
		insCell.leftBottomY = startYPos;
		insCell.leftBottomZ = placingZone.level - 0.0115;
		insCell.libPart.wood.w_ang = 0.0;
		insCell.libPart.wood.w_h = 0.080;
		insCell.libPart.wood.w_leng = placingZone.cells [xx][0].verLen;
		insCell.libPart.wood.w_w = 0.050;

		// ��ġ ���� ��ȸ�������� ��ȯ
		rotatedPoint.x = insCell.leftBottomX;
		rotatedPoint.y = insCell.leftBottomY;
		rotatedPoint.z = insCell.leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
		insCell.leftBottomX = unrotatedPoint.x;
		insCell.leftBottomY = unrotatedPoint.y;
		insCell.leftBottomZ = unrotatedPoint.z;

		// �� ��ġ
		elemList.Push (placingZone.placeLibPart (insCell));

		// ���� �� ��ġ�� ���� ���� ��ǥ �̵�
		startXPos -= placingZone.cells [xx+1][0].verLen;
	}

	insCell.objType = WOOD;
	insCell.ang = placingZone.ang + DegreeToRad (90.0);
	insCell.leftBottomX = startXPos - 0.080;
	insCell.leftBottomY = startYPos;
	insCell.leftBottomZ = placingZone.level - 0.0115;
	insCell.libPart.wood.w_ang = 0.0;
	insCell.libPart.wood.w_h = 0.080;
	insCell.libPart.wood.w_leng = placingZone.cells [xx][0].verLen + 0.080;
	insCell.libPart.wood.w_w = 0.050;

	// ��ġ ���� ��ȸ�������� ��ȯ
	rotatedPoint.x = insCell.leftBottomX;
	rotatedPoint.y = insCell.leftBottomY;
	rotatedPoint.z = insCell.leftBottomZ;
	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
	insCell.leftBottomX = unrotatedPoint.x;
	insCell.leftBottomY = unrotatedPoint.y;
	insCell.leftBottomZ = unrotatedPoint.z;

	// �� ��ġ
	elemList.Push (placingZone.placeLibPart (insCell));


	// ������ �ѷ� ���� ��ġ (RIGHT) : �� ������ 90 ȸ���Ͽ� ���� ��ǥ �޺κ��� X,Y���� �ٲ��� ��
	insCell.objType = WOOD;
	insCell.ang = placingZone.ang + DegreeToRad (90.0);
	insCell.leftBottomX = axisPoint.x - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) - placingZone.cells [0][0].verLen;
	insCell.leftBottomY = axisPoint.y - (- placingZone.corner_leftTop.x + placingZone.outerLeft + (placingZone.outerRight - placingZone.outerLeft) - (placingZone.outerRight - placingZone.outerLeft) / 2 + (placingZone.formArrayWidth / 2));
	insCell.leftBottomZ = placingZone.level - 0.0115;
	insCell.libPart.wood.w_ang = 0.0;
	insCell.libPart.wood.w_h = 0.080;
	insCell.libPart.wood.w_leng = placingZone.cells [0][0].verLen + 0.080;
	insCell.libPart.wood.w_w = 0.050;

	// ��ġ ���� ��ȸ�������� ��ȯ
	rotatedPoint.x = insCell.leftBottomX;
	rotatedPoint.y = insCell.leftBottomY;
	rotatedPoint.z = insCell.leftBottomZ;
	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
	insCell.leftBottomX = unrotatedPoint.x;
	insCell.leftBottomY = unrotatedPoint.y;
	insCell.leftBottomZ = unrotatedPoint.z;

	// �� ��ġ
	elemList.Push (placingZone.placeLibPart (insCell));

	startXPos = axisPoint.x - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) - placingZone.cells [0][0].verLen - placingZone.cells [1][0].verLen;
	startYPos = axisPoint.y - (- placingZone.corner_leftTop.x + placingZone.outerLeft + (placingZone.outerRight - placingZone.outerLeft) - (placingZone.outerRight - placingZone.outerLeft) / 2 + (placingZone.formArrayWidth / 2));
	for (xx = 1 ; xx < placingZone.eu_count_ver-1 ; ++xx) {
		insCell.objType = WOOD;
		insCell.ang = placingZone.ang + DegreeToRad (90.0);
		insCell.leftBottomX = startXPos;
		insCell.leftBottomY = startYPos;
		insCell.leftBottomZ = placingZone.level - 0.0115;
		insCell.libPart.wood.w_ang = 0.0;
		insCell.libPart.wood.w_h = 0.080;
		insCell.libPart.wood.w_leng = placingZone.cells [xx][0].verLen;
		insCell.libPart.wood.w_w = 0.050;

		// ��ġ ���� ��ȸ�������� ��ȯ
		rotatedPoint.x = insCell.leftBottomX;
		rotatedPoint.y = insCell.leftBottomY;
		rotatedPoint.z = insCell.leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
		insCell.leftBottomX = unrotatedPoint.x;
		insCell.leftBottomY = unrotatedPoint.y;
		insCell.leftBottomZ = unrotatedPoint.z;

		// �� ��ġ
		elemList.Push (placingZone.placeLibPart (insCell));

		// ���� �� ��ġ�� ���� ���� ��ǥ �̵�
		startXPos -= placingZone.cells [xx+1][0].verLen;
	}

	insCell.objType = WOOD;
	insCell.ang = placingZone.ang + DegreeToRad (90.0);
	insCell.leftBottomX = startXPos - 0.080;
	insCell.leftBottomY = startYPos;
	insCell.leftBottomZ = placingZone.level - 0.0115;
	insCell.libPart.wood.w_ang = 0.0;
	insCell.libPart.wood.w_h = 0.080;
	insCell.libPart.wood.w_leng = placingZone.cells [xx][0].verLen + 0.080;
	insCell.libPart.wood.w_w = 0.050;

	// ��ġ ���� ��ȸ�������� ��ȯ
	rotatedPoint.x = insCell.leftBottomX;
	rotatedPoint.y = insCell.leftBottomY;
	rotatedPoint.z = insCell.leftBottomZ;
	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
	insCell.leftBottomX = unrotatedPoint.x;
	insCell.leftBottomY = unrotatedPoint.y;
	insCell.leftBottomZ = unrotatedPoint.z;

	// �� ��ġ
	elemList.Push (placingZone.placeLibPart (insCell));


	// �ڳ��� ���� ��ġ (LEFT-TOP)
	// ...


	// �ڳ��� ���� ��ġ (RIGHT-TOP)
	// ...


	// �ڳ��� ���� ��ġ (LEFT-BOTTOM)
	// ...


	// �ڳ��� ���� ��ġ (RIGHT-BOTTOM)
	// ...


	// ���� ���� ��ġ : T ��ư�� �ش� (���ʺ��� ����, 0���� eu_count_hor-2����) : LeftBottom���� RightBottom����
	startXPos = axisPoint.x - (placingZone.outerTop - placingZone.outerBottom) + 0.064;
	startYPos = axisPoint.y + placingZone.corner_leftTop.x - placingZone.outerLeft - (placingZone.outerRight - placingZone.outerLeft) / 2 + (placingZone.formArrayWidth / 2) - placingZone.cells [0][0].horLen + 0.080;
	for (xx = 0 ; xx < placingZone.eu_count_hor-1 ; ++xx) {
		// 1��
		insCell.objType = WOOD;
		insCell.ang = placingZone.ang + DegreeToRad (90.0);
		insCell.leftBottomX = startXPos;
		insCell.leftBottomY = startYPos;
		insCell.leftBottomZ = placingZone.level - 0.0115;
		insCell.libPart.wood.w_ang = 0.0;
		insCell.libPart.wood.w_h = 0.080;
		insCell.libPart.wood.w_leng = (placingZone.outerTop - placingZone.outerBottom) / 2 - (placingZone.formArrayHeight / 2) - 0.080 - 0.064;
		insCell.libPart.wood.w_w = 0.050;

		// ��ġ ���� ��ȸ�������� ��ȯ
		rotatedPoint.x = insCell.leftBottomX;
		rotatedPoint.y = insCell.leftBottomY;
		rotatedPoint.z = insCell.leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
		insCell.leftBottomX = unrotatedPoint.x;
		insCell.leftBottomY = unrotatedPoint.y;
		insCell.leftBottomZ = unrotatedPoint.z;

		// �� ��ġ
		if (placingZone.topBoundsCells [xx] == true)
			elemList.Push (placingZone.placeLibPart (insCell));

		// 2��
		insCell.leftBottomX = startXPos;
		insCell.leftBottomY = startYPos - 0.080;

		// ��ġ ���� ��ȸ�������� ��ȯ
		rotatedPoint.x = insCell.leftBottomX;
		rotatedPoint.y = insCell.leftBottomY;
		rotatedPoint.z = insCell.leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
		insCell.leftBottomX = unrotatedPoint.x;
		insCell.leftBottomY = unrotatedPoint.y;
		insCell.leftBottomZ = unrotatedPoint.z;

		// �� ��ġ
		if (placingZone.topBoundsCells [xx] == true)
			elemList.Push (placingZone.placeLibPart (insCell));

		// ���� �� ��ġ�� ���� ���� ��ǥ �̵�
		startYPos -= placingZone.cells [0][xx+1].horLen;
	}
	
	// ���� ���� ��ġ : B ��ư�� �ش� (���ʺ��� ����, 0���� eu_count_hor-2����) : LeftTop���� RightTop����
	startXPos = axisPoint.x - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) + 0.080;
	startYPos = axisPoint.y + placingZone.corner_leftTop.x - placingZone.outerLeft - (placingZone.outerRight - placingZone.outerLeft) / 2 + (placingZone.formArrayWidth / 2) - placingZone.cells [0][0].horLen + 0.080;
	for (xx = 0 ; xx < placingZone.eu_count_hor-1 ; ++xx) {
		// 1��
		insCell.objType = WOOD;
		insCell.ang = placingZone.ang + DegreeToRad (90.0);
		insCell.leftBottomX = startXPos;
		insCell.leftBottomY = startYPos;
		insCell.leftBottomZ = placingZone.level - 0.0115;
		insCell.libPart.wood.w_ang = 0.0;
		insCell.libPart.wood.w_h = 0.080;
		insCell.libPart.wood.w_leng = (placingZone.outerTop - placingZone.outerBottom) / 2 - (placingZone.formArrayHeight / 2) - 0.080 - 0.064;
		insCell.libPart.wood.w_w = 0.050;

		// ��ġ ���� ��ȸ�������� ��ȯ
		rotatedPoint.x = insCell.leftBottomX;
		rotatedPoint.y = insCell.leftBottomY;
		rotatedPoint.z = insCell.leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
		insCell.leftBottomX = unrotatedPoint.x;
		insCell.leftBottomY = unrotatedPoint.y;
		insCell.leftBottomZ = unrotatedPoint.z;

		// �� ��ġ
		if (placingZone.bottomBoundsCells [xx] == true)
			elemList.Push (placingZone.placeLibPart (insCell));

		// 2��
		insCell.leftBottomX = startXPos;
		insCell.leftBottomY = startYPos - 0.080;

		// ��ġ ���� ��ȸ�������� ��ȯ
		rotatedPoint.x = insCell.leftBottomX;
		rotatedPoint.y = insCell.leftBottomY;
		rotatedPoint.z = insCell.leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
		insCell.leftBottomX = unrotatedPoint.x;
		insCell.leftBottomY = unrotatedPoint.y;
		insCell.leftBottomZ = unrotatedPoint.z;

		// �� ��ġ
		if (placingZone.bottomBoundsCells [xx] == true)
			elemList.Push (placingZone.placeLibPart (insCell));

		// ���� �� ��ġ�� ���� ���� ��ǥ �̵�
		startYPos -= placingZone.cells [0][xx+1].horLen;
	}

	// ���� ���� ��ġ : L ��ư�� �ش� (������ ����, 0���� eu_count_ver-2����) : LeftBottom���� LeftTop����
	startXPos = axisPoint.x - placingZone.corner_leftTop.x + placingZone.outerLeft + 0.064;
	startYPos = axisPoint.y - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) - placingZone.cells [0][0].verLen;
	for (xx = placingZone.eu_count_ver-2 ; xx >= 0 ; --xx) {
		// 1��
		insCell.objType = WOOD;
		insCell.ang = placingZone.ang;
		insCell.leftBottomX = startXPos;
		insCell.leftBottomY = startYPos;
		insCell.leftBottomZ = placingZone.level - 0.0115;
		insCell.libPart.wood.w_ang = 0.0;
		insCell.libPart.wood.w_h = 0.080;
		insCell.libPart.wood.w_leng = (placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) - 0.080 - 0.064;
		insCell.libPart.wood.w_w = 0.050;

		// ��ġ ���� ��ȸ�������� ��ȯ
		rotatedPoint.x = insCell.leftBottomX;
		rotatedPoint.y = insCell.leftBottomY;
		rotatedPoint.z = insCell.leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
		insCell.leftBottomX = unrotatedPoint.x;
		insCell.leftBottomY = unrotatedPoint.y;
		insCell.leftBottomZ = unrotatedPoint.z;

		// �� ��ġ
		if (placingZone.leftBoundsCells [xx] == true)
			elemList.Push (placingZone.placeLibPart (insCell));

		// 2��
		insCell.leftBottomX = startXPos;
		insCell.leftBottomY = startYPos + 0.080;

		// ��ġ ���� ��ȸ�������� ��ȯ
		rotatedPoint.x = insCell.leftBottomX;
		rotatedPoint.y = insCell.leftBottomY;
		rotatedPoint.z = insCell.leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
		insCell.leftBottomX = unrotatedPoint.x;
		insCell.leftBottomY = unrotatedPoint.y;
		insCell.leftBottomZ = unrotatedPoint.z;

		// �� ��ġ
		if (placingZone.leftBoundsCells [xx] == true)
			elemList.Push (placingZone.placeLibPart (insCell));

		// ���� �� ��ġ�� ���� ���� ��ǥ �̵�
		startYPos -= placingZone.cells [xx][0].verLen;
	}

	// ���� ���� ��ġ : R ��ư�� �ش� (������ ����, 0���� eu_count_ver-2����) : RightBottom���� RightTop����
	startXPos = axisPoint.x - placingZone.corner_leftTop.x + placingZone.outerLeft + (placingZone.outerRight - placingZone.outerLeft) - ((placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) - 0.080);
	startYPos = axisPoint.y - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) - placingZone.cells [0][0].verLen;
	for (xx = placingZone.eu_count_ver-2 ; xx >= 0 ; --xx) {
		// 1��
		insCell.objType = WOOD;
		insCell.ang = placingZone.ang;
		insCell.leftBottomX = startXPos;
		insCell.leftBottomY = startYPos;
		insCell.leftBottomZ = placingZone.level - 0.0115;
		insCell.libPart.wood.w_ang = 0.0;
		insCell.libPart.wood.w_h = 0.080;
		insCell.libPart.wood.w_leng = (placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) - 0.080 - 0.064;
		insCell.libPart.wood.w_w = 0.050;

		// ��ġ ���� ��ȸ�������� ��ȯ
		rotatedPoint.x = insCell.leftBottomX;
		rotatedPoint.y = insCell.leftBottomY;
		rotatedPoint.z = insCell.leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
		insCell.leftBottomX = unrotatedPoint.x;
		insCell.leftBottomY = unrotatedPoint.y;
		insCell.leftBottomZ = unrotatedPoint.z;

		// �� ��ġ
		if (placingZone.rightBoundsCells [xx] == true)
			elemList.Push (placingZone.placeLibPart (insCell));

		// 2��
		insCell.leftBottomX = startXPos;
		insCell.leftBottomY = startYPos + 0.080;

		// ��ġ ���� ��ȸ�������� ��ȯ
		rotatedPoint.x = insCell.leftBottomX;
		rotatedPoint.y = insCell.leftBottomY;
		rotatedPoint.z = insCell.leftBottomZ;
		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
		insCell.leftBottomX = unrotatedPoint.x;
		insCell.leftBottomY = unrotatedPoint.y;
		insCell.leftBottomZ = unrotatedPoint.z;

		// �� ��ġ
		if (placingZone.rightBoundsCells [xx] == true)
			elemList.Push (placingZone.placeLibPart (insCell));

		// ���� �� ��ġ�� ���� ���� ��ǥ �̵�
		startYPos -= placingZone.cells [xx][0].verLen;
	}

	return err;
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

			// ��: ��������� ����
			DGSetItemText (dialogID, LABEL_GAP_LENGTH, "��������� ����");

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

					// ��������� ����
					placingZone.gap = DGGetItemValDouble (dialogID, EDITCONTROL_GAP_LENGTH);

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
	short	btnInitPosX = 220 + 25;
	short	btnPosX, btnPosY;
	short	xx, yy;
	short	idxBtn;
	short	lastIdxBtn = 0;
	std::string		txtButton = "";
	API_Element		elem;
	GSErrCode		err;

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, "������ �Ϻο� ��ġ - ������ ��ġ ����");

			//////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			// ��ġ ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 210, 130, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "2. ��  ġ");
			DGShowItem (dialogID, DG_OK);

			// ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 250, 130, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "3. ������ ä���");
			DGShowItem (dialogID, DG_CANCEL);

			// ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 290, 130, 25);
			DGSetItemFont (dialogID, DG_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_PREV, "����");
			DGShowItem (dialogID, DG_PREV);

			// ��: ���� ���� ����
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 30, 20, 90, 23);
			if ( ((placingZone.remain_hor_updated / 2) >= 0.150) && ((placingZone.remain_hor_updated / 2) <= 0.300) )
				DGSetItemFont (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH, DG_IS_LARGE | DG_IS_BOLD);
			else
				DGSetItemFont (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH, "�¿� ����");
			DGShowItem (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH);

			// ��: ���� ���� ����
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 30, 50, 90, 23);
			if ( ((placingZone.remain_ver_updated / 2) >= 0.150) && ((placingZone.remain_ver_updated / 2) <= 0.300) )
				DGSetItemFont (dialogID, LABEL_REMAIN_VERTICAL_LENGTH, DG_IS_LARGE | DG_IS_BOLD);
			else
				DGSetItemFont (dialogID, LABEL_REMAIN_VERTICAL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_REMAIN_VERTICAL_LENGTH, "���� ����");
			DGShowItem (dialogID, LABEL_REMAIN_VERTICAL_LENGTH);

			// Edit ��Ʈ��: ���� ���� ����
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 130, 20-7, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH);
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH, placingZone.remain_hor_updated / 2);

			// Edit ��Ʈ��: ���� ���� ����
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 130, 50-7, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH);
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH, placingZone.remain_ver_updated / 2);

			// ��: ������/�ٷ������̼� ��ġ ����
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 200, 10, 200, 23);
			DGSetItemFont (dialogID, LABEL_GRID_EUROFORM_WOOD, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_GRID_EUROFORM_WOOD, "������/���� ���� ��ġ ����");
			DGShowItem (dialogID, LABEL_GRID_EUROFORM_WOOD);

			// ���� �Ÿ� Ȯ�� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 90, 130, 25);
			DGSetItemFont (dialogID, PUSHBUTTON_CONFIRM_REMAIN_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, PUSHBUTTON_CONFIRM_REMAIN_LENGTH, "1. ���� ���� Ȯ��");
			DGShowItem (dialogID, PUSHBUTTON_CONFIRM_REMAIN_LENGTH);
			DGDisableItem (dialogID, PUSHBUTTON_CONFIRM_REMAIN_LENGTH);

			// �� �߰� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 130, 65, 25);
			DGSetItemFont (dialogID, PUSHBUTTON_ADD_ROW, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, PUSHBUTTON_ADD_ROW, "�� �߰�");
			DGShowItem (dialogID, PUSHBUTTON_ADD_ROW);

			// �� ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 105, 130, 65, 25);
			DGSetItemFont (dialogID, PUSHBUTTON_DEL_ROW, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, PUSHBUTTON_DEL_ROW, "�� ����");
			DGShowItem (dialogID, PUSHBUTTON_DEL_ROW);

			// �� �߰� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 170, 65, 25);
			DGSetItemFont (dialogID, PUSHBUTTON_ADD_COL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, PUSHBUTTON_ADD_COL, "�� �߰�");
			DGShowItem (dialogID, PUSHBUTTON_ADD_COL);
		
			// �� ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 105, 170, 65, 25);
			DGSetItemFont (dialogID, PUSHBUTTON_DEL_COL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, PUSHBUTTON_DEL_COL, "�� ����");
			DGShowItem (dialogID, PUSHBUTTON_DEL_COL);

			// ���� â ũ�⸦ ����
			dialogSizeX = 270 + (btnSizeX * placingZone.eu_count_hor) + 50;
			dialogSizeY = max<short>(300, 150 + (btnSizeY * placingZone.eu_count_ver) + 50);
			DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);

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

			TButtonStartIdx = lastIdxBtn - (placingZone.eu_count_hor - 2);

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

			BButtonStartIdx = lastIdxBtn - (placingZone.eu_count_hor - 2);

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

			LButtonStartIdx = lastIdxBtn - (placingZone.eu_count_ver - 2);

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

			RButtonStartIdx = lastIdxBtn - (placingZone.eu_count_ver - 2);

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case PUSHBUTTON_CONFIRM_REMAIN_LENGTH:
					// �������� �ʰ� ���� ���� �Ÿ��� �׸��� ��ư �Ӽ��� ������
					item = 0;

					// �� ����(Ÿ�� �� ũ��) ���� �߻�, ��� ���� ��ġ ���� ������Ʈ
					placingZone.alignPlacingZone (&placingZone);

					// ��ư �ε��� iteration �غ�
					idxBtn = itemInitIdx;
					
					// �׸��� ��ư �ؽ�Ʈ ������Ʈ
					for (xx = 0 ; xx < placingZone.eu_count_ver ; ++xx) {
						for (yy = 0 ; yy < placingZone.eu_count_hor ; ++yy) {

							txtButton = "";
							if (placingZone.cells [xx][yy].objType == NONE) {
								txtButton = "NONE";
							} else if (placingZone.cells [xx][yy].objType == EUROFORM) {
								if (placingZone.cells [xx][yy].libPart.form.u_ins_wall)
									txtButton = format_string ("������\n(����)\n��%.0f\n��%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
								else
									txtButton = format_string ("������\n(����)\n��%.0f\n��%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
							}

							DGSetItemText (dialogID, idxBtn, txtButton.c_str ());		// �׸��� ��ư �ؽ�Ʈ ����
							++idxBtn;
						}
					}

					// ���� ����/���� ���� ������Ʈ
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH, placingZone.remain_hor_updated / 2);
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH, placingZone.remain_ver_updated / 2);
					DGSetItemFont (dialogID, PUSHBUTTON_CONFIRM_REMAIN_LENGTH, DG_IS_LARGE | DG_IS_BOLD);

					if ( ((placingZone.remain_hor_updated / 2) >= 0.150) && ((placingZone.remain_hor_updated / 2) <= 0.300) )
						DGSetItemFont (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH, DG_IS_LARGE | DG_IS_BOLD);
					else
						DGSetItemFont (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);


					if ( ((placingZone.remain_ver_updated / 2) >= 0.150) && ((placingZone.remain_ver_updated / 2) <= 0.300) )
						DGSetItemFont (dialogID, LABEL_REMAIN_VERTICAL_LENGTH, DG_IS_LARGE | DG_IS_BOLD);
					else
						DGSetItemFont (dialogID, LABEL_REMAIN_VERTICAL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);

					break;

				case DG_OK:
					// �������� �ʰ� ��ġ�� ��ü�� ���� �� ���ġ�ϰ� �׸��� ��ư �Ӽ��� ������
					item = 0;

					// �� ����(Ÿ�� �� ũ��) ���� �߻�, ��� ���� ��ġ ���� ������Ʈ
					placingZone.alignPlacingZone (&placingZone);

					// ��ư �ε��� iteration �غ�
					idxBtn = itemInitIdx;

					// �׸��� ��ư �ؽ�Ʈ ������Ʈ
					for (xx = 0 ; xx < placingZone.eu_count_ver ; ++xx) {
						for (yy = 0 ; yy < placingZone.eu_count_hor ; ++yy) {

							txtButton = "";
							if (placingZone.cells [xx][yy].objType == NONE) {
								txtButton = "NONE";
							} else if (placingZone.cells [xx][yy].objType == EUROFORM) {
								if (placingZone.cells [xx][yy].libPart.form.u_ins_wall)
									txtButton = format_string ("������\n(����)\n��%.0f\n��%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
								else
									txtButton = format_string ("������\n(����)\n��%.0f\n��%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
							}

							DGSetItemText (dialogID, idxBtn, txtButton.c_str ());		// �׸��� ��ư �ؽ�Ʈ ����
							++idxBtn;
						}
					}

					// ���� ����/���� ���� ������Ʈ
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH, placingZone.remain_hor_updated / 2);
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH, placingZone.remain_ver_updated / 2);
					DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_BOLD);

					if ( ((placingZone.remain_hor_updated / 2) >= 0.150) && ((placingZone.remain_hor_updated / 2) <= 0.300) )
						DGSetItemFont (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH, DG_IS_LARGE | DG_IS_BOLD);
					else
						DGSetItemFont (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);


					if ( ((placingZone.remain_ver_updated / 2) >= 0.150) && ((placingZone.remain_ver_updated / 2) <= 0.300) )
						DGSetItemFont (dialogID, LABEL_REMAIN_VERTICAL_LENGTH, DG_IS_LARGE | DG_IS_BOLD);
					else
						DGSetItemFont (dialogID, LABEL_REMAIN_VERTICAL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);

					// ���� ��ġ�� ��ü ���� ����
					for (xx = 0 ; xx < placingZone.eu_count_ver ; ++xx) {
						for (yy = 0 ; yy < placingZone.eu_count_hor ; ++yy) {
							elem.header.guid = placingZone.cells [xx][yy].guid;
							if (ACAPI_Element_Get (&elem) != NoError)
								continue;

							API_Elem_Head* headList = new API_Elem_Head [1];
							headList [0] = elem.header;
							err = ACAPI_Element_Delete (&headList, 1);
							delete headList;
						}
					}

					// ������Ʈ�� �� ������� ��ü ���ġ
					for (xx = 0 ; xx < placingZone.eu_count_ver ; ++xx)
						for (yy = 0 ; yy < placingZone.eu_count_hor ; ++yy)
							placingZone.cells [xx][yy].guid = placingZone.placeLibPart (placingZone.cells [xx][yy]);

					// T, B, L, R ��ư �迭�� Ǫ�� ���θ� ������
					for (xx = 0 ; xx < placingZone.eu_count_hor-1 ; ++xx) {
						if (DGGetItemValLong (dialogID, TButtonStartIdx) == TRUE)
							placingZone.topBoundsCells [xx] = true;
						else
							placingZone.topBoundsCells [xx] = false;
						++TButtonStartIdx;
					}
					for (xx = 0 ; xx < placingZone.eu_count_hor-1 ; ++xx) {
						if (DGGetItemValLong (dialogID, BButtonStartIdx) == TRUE)
							placingZone.bottomBoundsCells [xx] = true;
						else
							placingZone.bottomBoundsCells [xx] = false;
						++BButtonStartIdx;
					}
					for (xx = 0 ; xx < placingZone.eu_count_ver-1 ; ++xx) {
						if (DGGetItemValLong (dialogID, LButtonStartIdx) == TRUE)
							placingZone.leftBoundsCells [xx] = true;
						else
							placingZone.leftBoundsCells [xx] = false;
						++LButtonStartIdx;
					}
					for (xx = 0 ; xx < placingZone.eu_count_ver-1 ; ++xx) {
						if (DGGetItemValLong (dialogID, RButtonStartIdx) == TRUE)
							placingZone.rightBoundsCells [xx] = true;
						else
							placingZone.rightBoundsCells [xx] = false;
						++RButtonStartIdx;
					}

					clickedOKButton = true;

					break;

				case DG_CANCEL:
					// ������ ä���� �Ѿ �� ��ġ�� ���������� ��� GUID�� ������
					for (xx = 0 ; xx < placingZone.eu_count_ver ; ++xx)
						for (yy = 0 ; yy < placingZone.eu_count_hor ; ++yy)
							elemList.Push (placingZone.cells [xx][yy].guid);

					break;

				case DG_PREV:
					clickedPrevButton = true;
					break;

				case PUSHBUTTON_ADD_ROW:
					// �������� �ʰ� ���� ���� �Ÿ��� �׸��� ��ư �Ӽ��� ������
					item = 0;

					// ���ο� �� �߰� (�� �ϳ��� �ø��� �߰��� �࿡ ������ �� ���� ����)
					placingZone.addNewRow (&placingZone);

					// �� ����(Ÿ�� �� ũ��) ���� �߻�, ��� ���� ��ġ ���� ������Ʈ
					placingZone.alignPlacingZone (&placingZone);

					// ��ư �ε��� iteration �غ�
					idxBtn = itemInitIdx;

					// ������ ��ġ�� �׸��� ��ư ��� ���� (itemInitIdx����)
					DGRemoveDialogItems (dialogID, itemInitIdx);
					
					// ���� â ũ�⸦ ����
					dialogSizeX = 270 + (btnSizeX * placingZone.eu_count_hor) + 50;
					dialogSizeY = max<short>(300, 150 + (btnSizeY * placingZone.eu_count_ver) + 50);
					DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);

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
						DGSetItemValLong (dialogID, idxBtn, FALSE);
						DGShowItem (dialogID, idxBtn);
						btnPosX += 50;
					}

					TButtonStartIdx = lastIdxBtn - (placingZone.eu_count_hor - 2);

					// �Ʒ��� ���� ���� üũ �ڽ�
					btnPosX = 270 + 12, btnPosY = (btnSizeY * (placingZone.eu_count_ver + 1)) + 27;
					for (xx = 0 ; xx < placingZone.eu_count_hor - 1 ; ++xx) {
						idxBtn = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, btnPosX, btnPosY, 25, 25);
						lastIdxBtn = idxBtn;
						DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL);

						txtButton = "B";
						DGSetItemText (dialogID, idxBtn, txtButton.c_str ());
						DGSetItemValLong (dialogID, idxBtn, FALSE);
						DGShowItem (dialogID, idxBtn);
						btnPosX += 50;
					}

					BButtonStartIdx = lastIdxBtn - (placingZone.eu_count_hor - 2);

					// ���� ���� ���� üũ �ڽ�
					btnPosX = 219, btnPosY = 114;
					for (xx = 0 ; xx < placingZone.eu_count_ver - 1 ; ++xx) {
						idxBtn = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, btnPosX, btnPosY, 25, 25);
						lastIdxBtn = idxBtn;
						DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL);

						txtButton = "L";
						DGSetItemText (dialogID, idxBtn, txtButton.c_str ());
						DGSetItemValLong (dialogID, idxBtn, FALSE);
						DGShowItem (dialogID, idxBtn);
						btnPosY += 50;
					}

					LButtonStartIdx = lastIdxBtn - (placingZone.eu_count_ver - 2);

					// ������ ���� ���� üũ �ڽ�
					btnPosX = 220 + 25 + (btnSizeX * placingZone.eu_count_hor), btnPosY = 114;
					for (xx = 0 ; xx < placingZone.eu_count_ver - 1 ; ++xx) {
						idxBtn = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, btnPosX, btnPosY, 25, 25);
						lastIdxBtn = idxBtn;
						DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL); 

						txtButton = "R";
						DGSetItemText (dialogID, idxBtn, txtButton.c_str ());
						DGSetItemValLong (dialogID, idxBtn, FALSE);
						DGShowItem (dialogID, idxBtn);
						btnPosY += 50;
					}

					RButtonStartIdx = lastIdxBtn - (placingZone.eu_count_ver - 2);

					// ���� ����/���� ���� ������Ʈ
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH, placingZone.remain_hor_updated / 2);
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH, placingZone.remain_ver_updated / 2);

					if ( ((placingZone.remain_hor_updated / 2) >= 0.150) && ((placingZone.remain_hor_updated / 2) <= 0.300) )
						DGSetItemFont (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH, DG_IS_LARGE | DG_IS_BOLD);
					else
						DGSetItemFont (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);


					if ( ((placingZone.remain_ver_updated / 2) >= 0.150) && ((placingZone.remain_ver_updated / 2) <= 0.300) )
						DGSetItemFont (dialogID, LABEL_REMAIN_VERTICAL_LENGTH, DG_IS_LARGE | DG_IS_BOLD);
					else
						DGSetItemFont (dialogID, LABEL_REMAIN_VERTICAL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);

					break;

				case PUSHBUTTON_DEL_ROW:
					// �������� �ʰ� ���� ���� �Ÿ��� �׸��� ��ư �Ӽ��� ������
					item = 0;

					// ������ �� ����
					placingZone.delLastRow (&placingZone);

					// �� ����(Ÿ�� �� ũ��) ���� �߻�, ��� ���� ��ġ ���� ������Ʈ
					placingZone.alignPlacingZone (&placingZone);

					// ��ư �ε��� iteration �غ�
					idxBtn = itemInitIdx;

					// ������ ��ġ�� �׸��� ��ư ��� ���� (itemInitIdx����)
					DGRemoveDialogItems (dialogID, itemInitIdx);
					
					// ���� â ũ�⸦ ����
					dialogSizeX = 270 + (btnSizeX * placingZone.eu_count_hor) + 50;
					dialogSizeY = max<short>(300, 150 + (btnSizeY * placingZone.eu_count_ver) + 50);
					DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);

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
						DGSetItemValLong (dialogID, idxBtn, FALSE);
						DGShowItem (dialogID, idxBtn);
						btnPosX += 50;
					}

					TButtonStartIdx = lastIdxBtn - (placingZone.eu_count_hor - 2);

					// �Ʒ��� ���� ���� üũ �ڽ�
					btnPosX = 270 + 12, btnPosY = (btnSizeY * (placingZone.eu_count_ver + 1)) + 27;
					for (xx = 0 ; xx < placingZone.eu_count_hor - 1 ; ++xx) {
						idxBtn = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, btnPosX, btnPosY, 25, 25);
						lastIdxBtn = idxBtn;
						DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL);

						txtButton = "B";
						DGSetItemText (dialogID, idxBtn, txtButton.c_str ());
						DGSetItemValLong (dialogID, idxBtn, FALSE);
						DGShowItem (dialogID, idxBtn);
						btnPosX += 50;
					}

					BButtonStartIdx = lastIdxBtn - (placingZone.eu_count_hor - 2);

					// ���� ���� ���� üũ �ڽ�
					btnPosX = 219, btnPosY = 114;
					for (xx = 0 ; xx < placingZone.eu_count_ver - 1 ; ++xx) {
						idxBtn = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, btnPosX, btnPosY, 25, 25);
						lastIdxBtn = idxBtn;
						DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL);

						txtButton = "L";
						DGSetItemText (dialogID, idxBtn, txtButton.c_str ());
						DGSetItemValLong (dialogID, idxBtn, FALSE);
						DGShowItem (dialogID, idxBtn);
						btnPosY += 50;
					}

					LButtonStartIdx = lastIdxBtn - (placingZone.eu_count_ver - 2);

					// ������ ���� ���� üũ �ڽ�
					btnPosX = 220 + 25 + (btnSizeX * placingZone.eu_count_hor), btnPosY = 114;
					for (xx = 0 ; xx < placingZone.eu_count_ver - 1 ; ++xx) {
						idxBtn = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, btnPosX, btnPosY, 25, 25);
						lastIdxBtn = idxBtn;
						DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL); 

						txtButton = "R";
						DGSetItemText (dialogID, idxBtn, txtButton.c_str ());
						DGSetItemValLong (dialogID, idxBtn, FALSE);
						DGShowItem (dialogID, idxBtn);
						btnPosY += 50;
					}

					RButtonStartIdx = lastIdxBtn - (placingZone.eu_count_ver - 2);

					// ���� ����/���� ���� ������Ʈ
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH, placingZone.remain_hor_updated / 2);
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH, placingZone.remain_ver_updated / 2);

					if ( ((placingZone.remain_hor_updated / 2) >= 0.150) && ((placingZone.remain_hor_updated / 2) <= 0.300) )
						DGSetItemFont (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH, DG_IS_LARGE | DG_IS_BOLD);
					else
						DGSetItemFont (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);


					if ( ((placingZone.remain_ver_updated / 2) >= 0.150) && ((placingZone.remain_ver_updated / 2) <= 0.300) )
						DGSetItemFont (dialogID, LABEL_REMAIN_VERTICAL_LENGTH, DG_IS_LARGE | DG_IS_BOLD);
					else
						DGSetItemFont (dialogID, LABEL_REMAIN_VERTICAL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);

					break;

				case PUSHBUTTON_ADD_COL:
					// �������� �ʰ� ���� ���� �Ÿ��� �׸��� ��ư �Ӽ��� ������
					item = 0;

					// ���ο� �� �߰� (�� �ϳ��� �ø��� �߰��� ���� ������ �� ���� ����)
					placingZone.addNewCol (&placingZone);

					// �� ����(Ÿ�� �� ũ��) ���� �߻�, ��� ���� ��ġ ���� ������Ʈ
					placingZone.alignPlacingZone (&placingZone);

					// ��ư �ε��� iteration �غ�
					idxBtn = itemInitIdx;

					// ������ ��ġ�� �׸��� ��ư ��� ���� (itemInitIdx����)
					DGRemoveDialogItems (dialogID, itemInitIdx);
					
					// ���� â ũ�⸦ ����
					dialogSizeX = 270 + (btnSizeX * placingZone.eu_count_hor) + 50;
					dialogSizeY = max<short>(300, 150 + (btnSizeY * placingZone.eu_count_ver) + 50);
					DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);

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
						DGSetItemValLong (dialogID, idxBtn, FALSE);
						DGShowItem (dialogID, idxBtn);
						btnPosX += 50;
					}

					TButtonStartIdx = lastIdxBtn - (placingZone.eu_count_hor - 2);

					// �Ʒ��� ���� ���� üũ �ڽ�
					btnPosX = 270 + 12, btnPosY = (btnSizeY * (placingZone.eu_count_ver + 1)) + 27;
					for (xx = 0 ; xx < placingZone.eu_count_hor - 1 ; ++xx) {
						idxBtn = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, btnPosX, btnPosY, 25, 25);
						lastIdxBtn = idxBtn;
						DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL);

						txtButton = "B";
						DGSetItemText (dialogID, idxBtn, txtButton.c_str ());
						DGSetItemValLong (dialogID, idxBtn, FALSE);
						DGShowItem (dialogID, idxBtn);
						btnPosX += 50;
					}

					BButtonStartIdx = lastIdxBtn - (placingZone.eu_count_hor - 2);

					// ���� ���� ���� üũ �ڽ�
					btnPosX = 219, btnPosY = 114;
					for (xx = 0 ; xx < placingZone.eu_count_ver - 1 ; ++xx) {
						idxBtn = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, btnPosX, btnPosY, 25, 25);
						lastIdxBtn = idxBtn;
						DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL);

						txtButton = "L";
						DGSetItemText (dialogID, idxBtn, txtButton.c_str ());
						DGSetItemValLong (dialogID, idxBtn, FALSE);
						DGShowItem (dialogID, idxBtn);
						btnPosY += 50;
					}

					LButtonStartIdx = lastIdxBtn - (placingZone.eu_count_ver - 2);

					// ������ ���� ���� üũ �ڽ�
					btnPosX = 220 + 25 + (btnSizeX * placingZone.eu_count_hor), btnPosY = 114;
					for (xx = 0 ; xx < placingZone.eu_count_ver - 1 ; ++xx) {
						idxBtn = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, btnPosX, btnPosY, 25, 25);
						lastIdxBtn = idxBtn;
						DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL); 

						txtButton = "R";
						DGSetItemText (dialogID, idxBtn, txtButton.c_str ());
						DGSetItemValLong (dialogID, idxBtn, FALSE);
						DGShowItem (dialogID, idxBtn);
						btnPosY += 50;
					}

					RButtonStartIdx = lastIdxBtn - (placingZone.eu_count_ver - 2);

					// ���� ����/���� ���� ������Ʈ
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH, placingZone.remain_hor_updated / 2);
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH, placingZone.remain_ver_updated / 2);

					if ( ((placingZone.remain_hor_updated / 2) >= 0.150) && ((placingZone.remain_hor_updated / 2) <= 0.300) )
						DGSetItemFont (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH, DG_IS_LARGE | DG_IS_BOLD);
					else
						DGSetItemFont (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);


					if ( ((placingZone.remain_ver_updated / 2) >= 0.150) && ((placingZone.remain_ver_updated / 2) <= 0.300) )
						DGSetItemFont (dialogID, LABEL_REMAIN_VERTICAL_LENGTH, DG_IS_LARGE | DG_IS_BOLD);
					else
						DGSetItemFont (dialogID, LABEL_REMAIN_VERTICAL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);

					break;

				case PUSHBUTTON_DEL_COL:
					// �������� �ʰ� ���� ���� �Ÿ��� �׸��� ��ư �Ӽ��� ������
					item = 0;

					// ������ �� ����
					placingZone.delLastCol (&placingZone);

					// �� ����(Ÿ�� �� ũ��) ���� �߻�, ��� ���� ��ġ ���� ������Ʈ
					placingZone.alignPlacingZone (&placingZone);

					// ��ư �ε��� iteration �غ�
					idxBtn = itemInitIdx;

					// ������ ��ġ�� �׸��� ��ư ��� ���� (itemInitIdx����)
					DGRemoveDialogItems (dialogID, itemInitIdx);
					
					// ���� â ũ�⸦ ����
					dialogSizeX = 270 + (btnSizeX * placingZone.eu_count_hor) + 50;
					dialogSizeY = max<short>(300, 150 + (btnSizeY * placingZone.eu_count_ver) + 50);
					DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);

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
						DGSetItemValLong (dialogID, idxBtn, FALSE);
						DGShowItem (dialogID, idxBtn);
						btnPosX += 50;
					}

					TButtonStartIdx = lastIdxBtn - (placingZone.eu_count_hor - 2);

					// �Ʒ��� ���� ���� üũ �ڽ�
					btnPosX = 270 + 12, btnPosY = (btnSizeY * (placingZone.eu_count_ver + 1)) + 27;
					for (xx = 0 ; xx < placingZone.eu_count_hor - 1 ; ++xx) {
						idxBtn = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, btnPosX, btnPosY, 25, 25);
						lastIdxBtn = idxBtn;
						DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL);

						txtButton = "B";
						DGSetItemText (dialogID, idxBtn, txtButton.c_str ());
						DGSetItemValLong (dialogID, idxBtn, FALSE);
						DGShowItem (dialogID, idxBtn);
						btnPosX += 50;
					}

					BButtonStartIdx = lastIdxBtn - (placingZone.eu_count_hor - 2);

					// ���� ���� ���� üũ �ڽ�
					btnPosX = 219, btnPosY = 114;
					for (xx = 0 ; xx < placingZone.eu_count_ver - 1 ; ++xx) {
						idxBtn = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, btnPosX, btnPosY, 25, 25);
						lastIdxBtn = idxBtn;
						DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL);

						txtButton = "L";
						DGSetItemText (dialogID, idxBtn, txtButton.c_str ());
						DGSetItemValLong (dialogID, idxBtn, FALSE);
						DGShowItem (dialogID, idxBtn);
						btnPosY += 50;
					}

					LButtonStartIdx = lastIdxBtn - (placingZone.eu_count_ver - 2);

					// ������ ���� ���� üũ �ڽ�
					btnPosX = 220 + 25 + (btnSizeX * placingZone.eu_count_hor), btnPosY = 114;
					for (xx = 0 ; xx < placingZone.eu_count_ver - 1 ; ++xx) {
						idxBtn = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, btnPosX, btnPosY, 25, 25);
						lastIdxBtn = idxBtn;
						DGSetItemFont (dialogID, idxBtn, DG_IS_SMALL); 

						txtButton = "R";
						DGSetItemText (dialogID, idxBtn, txtButton.c_str ());
						DGSetItemValLong (dialogID, idxBtn, FALSE);
						DGShowItem (dialogID, idxBtn);
						btnPosY += 50;
					}

					RButtonStartIdx = lastIdxBtn - (placingZone.eu_count_ver - 2);

					// ���� ����/���� ���� ������Ʈ
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH, placingZone.remain_hor_updated / 2);
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH, placingZone.remain_ver_updated / 2);

					if ( ((placingZone.remain_hor_updated / 2) >= 0.150) && ((placingZone.remain_hor_updated / 2) <= 0.300) )
						DGSetItemFont (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH, DG_IS_LARGE | DG_IS_BOLD);
					else
						DGSetItemFont (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);


					if ( ((placingZone.remain_ver_updated / 2) >= 0.150) && ((placingZone.remain_ver_updated / 2) <= 0.300) )
						DGSetItemFont (dialogID, LABEL_REMAIN_VERTICAL_LENGTH, DG_IS_LARGE | DG_IS_BOLD);
					else
						DGSetItemFont (dialogID, LABEL_REMAIN_VERTICAL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);

					break;

				default:
					// [DIALOG] �׸��� ��ư�� ������ Cell�� �����ϱ� ���� ���� â(3��° ���̾�α�)�� ����
					clickedBtnItemIdx = item;
					result = DGBlankModalDialog (240, 260, DG_DLG_NOGROW, 0, DG_DLG_NORMALFRAME, slabBottomPlacerHandler3, 0);

					item = 0;	// �׸��� ��ư�� ������ �� â�� ������ �ʰ� ��

					// �� ����(Ÿ�� �� ũ��) ���� �߻�, ��� ���� ��ġ ���� ������Ʈ
					placingZone.alignPlacingZone (&placingZone);

					// ��ư �ε��� iteration �غ�
					idxBtn = itemInitIdx;
					
					// �׸��� ��ư �ؽ�Ʈ ������Ʈ
					for (xx = 0 ; xx < placingZone.eu_count_ver ; ++xx) {
						for (yy = 0 ; yy < placingZone.eu_count_hor ; ++yy) {

							txtButton = "";
							if (placingZone.cells [xx][yy].objType == NONE) {
								txtButton = "NONE";
							} else if (placingZone.cells [xx][yy].objType == EUROFORM) {
								if (placingZone.cells [xx][yy].libPart.form.u_ins_wall)
									txtButton = format_string ("������\n(����)\n��%.0f\n��%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
								else
									txtButton = format_string ("������\n(����)\n��%.0f\n��%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
							}

							DGSetItemText (dialogID, idxBtn, txtButton.c_str ());		// �׸��� ��ư �ؽ�Ʈ ����
							++idxBtn;
						}
					}

					// ���� ����/���� ���� ������Ʈ
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HORIZONTAL_LENGTH, placingZone.remain_hor_updated / 2);
					DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_VERTICAL_LENGTH, placingZone.remain_ver_updated / 2);
					DGSetItemFont (dialogID, PUSHBUTTON_CONFIRM_REMAIN_LENGTH, DG_IS_LARGE | DG_IS_BOLD);

					if ( ((placingZone.remain_hor_updated / 2) >= 0.150) && ((placingZone.remain_hor_updated / 2) <= 0.300) )
						DGSetItemFont (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH, DG_IS_LARGE | DG_IS_BOLD);
					else
						DGSetItemFont (dialogID, LABEL_REMAIN_HORIZONTAL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);


					if ( ((placingZone.remain_ver_updated / 2) >= 0.150) && ((placingZone.remain_ver_updated / 2) <= 0.300) )
						DGSetItemFont (dialogID, LABEL_REMAIN_VERTICAL_LENGTH, DG_IS_LARGE | DG_IS_BOLD);
					else
						DGSetItemFont (dialogID, LABEL_REMAIN_VERTICAL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);

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

			//////////////////////////////////////////////////////////// �ʵ� ���� (Ŭ���� ��)
			// ��: ��ü Ÿ��
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 20, 70, 23);
			DGSetItemFont (dialogID, LABEL_OBJ_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_OBJ_TYPE, "��ü Ÿ��");
			DGShowItem (dialogID, LABEL_OBJ_TYPE);
			DGDisableItem (dialogID, LABEL_OBJ_TYPE);

			// �˾���Ʈ��: ��ü Ÿ���� �ٲ� �� �ִ� �޺��ڽ��� �� ���� ����
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100, 20-7, 120, 25);
			DGSetItemFont (dialogID, POPUP_OBJ_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, "����");
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, "������");
			DGShowItem (dialogID, POPUP_OBJ_TYPE);
			DGDisableItem (dialogID, POPUP_OBJ_TYPE);

			// ��: �ʺ�
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 50, 70, 23);
			DGSetItemFont (dialogID, LABEL_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_WIDTH, "�ʺ�");

			// Edit ��Ʈ��: �ʺ�
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100, 50-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);

			// ��: ����
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 80, 70, 23);
			DGSetItemFont (dialogID, LABEL_HEIGHT, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_HEIGHT, "����");

			// Edit ��Ʈ��: ����
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100, 80-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_HEIGHT, DG_IS_LARGE | DG_IS_PLAIN);

			// ��: ��ġ����
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 140, 70, 23);
			DGSetItemFont (dialogID, LABEL_ORIENTATION, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_ORIENTATION, "��ġ����");
				
			// ���� ��ư: ��ġ���� (�������)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 777, 100, 140-6, 70, 25);
			DGSetItemFont (dialogID, RADIO_ORIENTATION_1_PLYWOOD, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, RADIO_ORIENTATION_1_PLYWOOD, "�������");
			// ���� ��ư: ��ġ���� (��������)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 777, 100, 170-6, 70, 25);
			DGSetItemFont (dialogID, RADIO_ORIENTATION_2_PLYWOOD, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, RADIO_ORIENTATION_2_PLYWOOD, "��������");

			// üũ�ڽ�: �԰���
			DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 20, 50, 70, 25-5);
			DGSetItemFont (dialogID, CHECKBOX_SET_STANDARD, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, CHECKBOX_SET_STANDARD, "�԰���");

			// ��: �ʺ�
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 80, 70, 23);
			DGSetItemFont (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS, "�ʺ�");

			// �˾� ��Ʈ��: �ʺ�
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100, 80-7, 100, 25);
			DGSetItemFont (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM, "600");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM, "500");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM, "450");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM, "400");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM, "300");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DG_POPUP_BOTTOM, "200");

			// Edit ��Ʈ��: �ʺ�
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100, 80-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);

			// ��: ����
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 110, 70, 23);
			DGSetItemFont (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS, "����");

			// �˾� ��Ʈ��: ����
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100, 110-7, 100, 25);
			DGSetItemFont (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM, "1200");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM, "900");
			DGPopUpInsertItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DG_POPUP_BOTTOM, "600");

			// Edit ��Ʈ��: ����
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100, 110-6, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			
			// ��: ��ġ����
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 140, 70, 23);
			DGSetItemFont (dialogID, LABEL_EUROFORM_ORIENTATION_OPTIONS, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_EUROFORM_ORIENTATION_OPTIONS, "��ġ����");
			
			// ���� ��ư: ��ġ���� (�������)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 778, 100, 140-6, 70, 25);
			DGSetItemFont (dialogID, RADIO_ORIENTATION_1_EUROFORM, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, RADIO_ORIENTATION_1_EUROFORM, "�������");
			// ���� ��ư: ��ġ���� (��������)
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 778, 100, 170-6, 70, 25);
			DGSetItemFont (dialogID, RADIO_ORIENTATION_2_EUROFORM, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, RADIO_ORIENTATION_2_EUROFORM, "��������");

			// �ʱ� �Է� �ʵ� ǥ��
			if (placingZone.cells [rIdx][cIdx].objType == EUROFORM) {
				DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, EUROFORM + 1);

				// üũ�ڽ�: �԰���
				DGShowItem (dialogID, CHECKBOX_SET_STANDARD);
				DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD, placingZone.cells [rIdx][cIdx].libPart.form.eu_stan_onoff);

				if (placingZone.cells [rIdx][cIdx].libPart.form.eu_stan_onoff == true) {
					// ��: �ʺ�
					DGShowItem (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS);

					// �˾� ��Ʈ��: �ʺ�
					DGShowItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS);
					if (abs(placingZone.cells [rIdx][cIdx].libPart.form.eu_wid - 0.600) < EPS)		popupSelectedIdx = 1;
					if (abs(placingZone.cells [rIdx][cIdx].libPart.form.eu_wid - 0.500) < EPS)		popupSelectedIdx = 2;
					if (abs(placingZone.cells [rIdx][cIdx].libPart.form.eu_wid - 0.450) < EPS)		popupSelectedIdx = 3;
					if (abs(placingZone.cells [rIdx][cIdx].libPart.form.eu_wid - 0.400) < EPS)		popupSelectedIdx = 4;
					if (abs(placingZone.cells [rIdx][cIdx].libPart.form.eu_wid - 0.300) < EPS)		popupSelectedIdx = 5;
					if (abs(placingZone.cells [rIdx][cIdx].libPart.form.eu_wid - 0.200) < EPS)		popupSelectedIdx = 6;
					DGPopUpSelectItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, popupSelectedIdx);

					// ��: ����
					DGShowItem (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS);

					// �˾� ��Ʈ��: ����
					DGShowItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS);
					if (abs(placingZone.cells [rIdx][cIdx].libPart.form.eu_hei - 1.200) < EPS)		popupSelectedIdx = 1;
					if (abs(placingZone.cells [rIdx][cIdx].libPart.form.eu_hei - 0.900) < EPS)		popupSelectedIdx = 2;
					if (abs(placingZone.cells [rIdx][cIdx].libPart.form.eu_hei - 0.600) < EPS)		popupSelectedIdx = 3;
					DGPopUpSelectItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, popupSelectedIdx);
				} else if (placingZone.cells [rIdx][cIdx].libPart.form.eu_stan_onoff == false) {
					// ��: �ʺ�
					DGShowItem (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS);

					// Edit ��Ʈ��: �ʺ�
					DGShowItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS);
					DGSetItemValDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS, placingZone.cells [rIdx][cIdx].libPart.form.eu_wid2);
					DGSetItemMinDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS, 0.050);
					DGSetItemMaxDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS, 0.900);

					// ��: ����
					DGShowItem (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS);

					// Edit ��Ʈ��: ����
					DGShowItem (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS);
					DGSetItemValDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS, placingZone.cells[rIdx][cIdx].libPart.form.eu_hei2);
					DGSetItemMinDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS, 0.050);
					DGSetItemMaxDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS, 1.500);
				}

				// ��: ��ġ����
				DGShowItem (dialogID, LABEL_EUROFORM_ORIENTATION_OPTIONS);
				
				// ���� ��ư: ��ġ���� (�������)
				DGShowItem (dialogID, RADIO_ORIENTATION_1_EUROFORM);
				// ���� ��ư: ��ġ���� (��������)
				DGShowItem (dialogID, RADIO_ORIENTATION_2_EUROFORM);

				if (placingZone.cells [rIdx][cIdx].libPart.form.u_ins_wall == true) {
					DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM, true);
					DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_EUROFORM, false);
				} else if (placingZone.cells [rIdx][cIdx].libPart.form.u_ins_wall == false) {
					DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM, false);
					DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_EUROFORM, true);
				}

				DGDisableItem (dialogID, RADIO_ORIENTATION_1_EUROFORM);
				DGDisableItem (dialogID, RADIO_ORIENTATION_2_EUROFORM);
			}

			break;

		case DG_MSG_CHANGE:
			switch (item) {
				case POPUP_OBJ_TYPE:	// ��ü Ÿ�� �޺��ڽ� ���� ������ ������ �Է� �ʵ尡 �޶��� (�����ؾ� �ϹǷ� Cell ���� �ҷ����� ����)
					//////////////////////////////////////////////////////////// �ʵ� ���� (Ŭ���� ��)
					// �ϴ� �׸��� �����, ��ü Ÿ�� ���� �׸� ǥ����
					DGHideItem (dialogID, LABEL_WIDTH);
					DGHideItem (dialogID, EDITCONTROL_WIDTH);
					DGHideItem (dialogID, LABEL_HEIGHT);
					DGHideItem (dialogID, EDITCONTROL_HEIGHT);
					DGHideItem (dialogID, LABEL_ORIENTATION);
					DGHideItem (dialogID, RADIO_ORIENTATION_1_PLYWOOD);
					DGHideItem (dialogID, RADIO_ORIENTATION_2_PLYWOOD);
					DGHideItem (dialogID, CHECKBOX_SET_STANDARD);
					DGHideItem (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS);
					DGHideItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS);
					DGHideItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS);
					DGHideItem (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS);
					DGHideItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS);
					DGHideItem (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS);
					DGHideItem (dialogID, LABEL_EUROFORM_ORIENTATION_OPTIONS);
					DGHideItem (dialogID, RADIO_ORIENTATION_1_EUROFORM);
					DGHideItem (dialogID, RADIO_ORIENTATION_2_EUROFORM);

					DGShowItem (dialogID, LABEL_OBJ_TYPE);
					DGShowItem (dialogID, POPUP_OBJ_TYPE);

					if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == EUROFORM + 1) {
						// üũ�ڽ�: �԰���
						DGShowItem (dialogID, CHECKBOX_SET_STANDARD);
						DGSetItemValLong (dialogID, CHECKBOX_SET_STANDARD, true);

						// ��: �ʺ�
						DGShowItem (dialogID, LABEL_EUROFORM_WIDTH_OPTIONS);

						// ��: ����
						DGShowItem (dialogID, LABEL_EUROFORM_HEIGHT_OPTIONS);

						// �԰����̸�,
						if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == TRUE) {
							// �˾� ��Ʈ��: �ʺ�
							DGShowItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS);
							DGHideItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS);

							// �˾� ��Ʈ��: ����
							DGShowItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS);
							DGHideItem (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS);
						} else if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == FALSE) {
							// Edit ��Ʈ��: �ʺ�
							DGHideItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS);
							DGShowItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS);
							DGSetItemMinDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS, 0.050);
							DGSetItemMaxDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS, 0.900);

							// Edit ��Ʈ��: ����
							DGHideItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS);
							DGShowItem (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS);
							DGSetItemMinDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS, 0.050);
							DGSetItemMaxDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS, 1.500);
						}

						// ��: ��ġ����
						DGShowItem (dialogID, LABEL_EUROFORM_ORIENTATION_OPTIONS);
				
						// ���� ��ư: ��ġ���� (�������)
						DGShowItem (dialogID, RADIO_ORIENTATION_1_EUROFORM);
						// ���� ��ư: ��ġ���� (��������)
						DGShowItem (dialogID, RADIO_ORIENTATION_2_EUROFORM);

						DGSetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM, true);
						DGSetItemValLong (dialogID, RADIO_ORIENTATION_2_EUROFORM, false);

						DGDisableItem (dialogID, RADIO_ORIENTATION_1_EUROFORM);
						DGDisableItem (dialogID, RADIO_ORIENTATION_2_EUROFORM);
					}

					break;

				case CHECKBOX_SET_STANDARD:	// �������� ���, �԰��� üũ�ڽ� ���� �ٲ� ������ �ʺ�, ���� �Է� �ʵ� Ÿ���� �ٲ�
					//////////////////////////////////////////////////////////// �ʵ� ���� (Ŭ���� ��)
					if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == TRUE) {
						// �˾� ��Ʈ��: �ʺ�
						DGShowItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS);
						DGHideItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS);
						// �˾� ��Ʈ��: ����
						DGShowItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS);
						DGHideItem (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS);
					} else if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == FALSE) {
						// Edit ��Ʈ��: �ʺ�
						DGHideItem (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS);
						DGShowItem (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS);
						DGSetItemMinDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS, 0.050);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS, 0.900);
						// Edit ��Ʈ��: ����
						DGHideItem (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS);
						DGShowItem (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS);
						DGSetItemMinDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS, 0.050);
						DGSetItemMaxDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS, 1.500);
					}

					break;
			}

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:

					// slabBottomPlacerHandler2 ���� Ŭ���� �׸��� ��ư�� �ε��� ���� �̿��Ͽ� �� �ε��� �� �ε�
					idxCell = (clickedBtnItemIdx - itemInitIdx);
					rIdx = 0;
					while (idxCell >= (placingZone.eu_count_hor)) {
						idxCell -= ((placingZone.eu_count_hor));
						++rIdx;
					}
					cIdx = idxCell;

					//////////////////////////////////////////////////////////// �ʵ� ���� (Ŭ���� ��)
					// �Է��� ���� �ٽ� ���� ����
					if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == NONE + 1) {
						placingZone.cells [rIdx][cIdx].objType = NONE;
						placingZone.adjustOtherCellsInSameRow (&placingZone, rIdx, cIdx);
						placingZone.adjustOtherCellsInSameCol (&placingZone, rIdx, cIdx);

					} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == EUROFORM + 1) {
						placingZone.cells [rIdx][cIdx].objType = EUROFORM;

						// �԰���
						if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == TRUE)
							placingZone.cells [rIdx][cIdx].libPart.form.eu_stan_onoff = true;
						else if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == FALSE)
							placingZone.cells [rIdx][cIdx].libPart.form.eu_stan_onoff = false;

						if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == TRUE) {
							// �ʺ�
							placingZone.cells [rIdx][cIdx].libPart.form.eu_wid = atof (DGPopUpGetItemText (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS, DGPopUpGetSelected (dialogID, POPUP_EUROFORM_WIDTH_OPTIONS)).ToCStr ()) / 1000.0;
							placingZone.cells [rIdx][cIdx].horLen = placingZone.cells [rIdx][cIdx].libPart.form.eu_wid;
							// ����
							placingZone.cells [rIdx][cIdx].libPart.form.eu_hei = atof (DGPopUpGetItemText (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS, DGPopUpGetSelected (dialogID, POPUP_EUROFORM_HEIGHT_OPTIONS)).ToCStr ()) / 1000.0;
							placingZone.cells [rIdx][cIdx].verLen = placingZone.cells [rIdx][cIdx].libPart.form.eu_hei;
						} else if (DGGetItemValLong (dialogID, CHECKBOX_SET_STANDARD) == FALSE) {
							// �ʺ�
							placingZone.cells [rIdx][cIdx].libPart.form.eu_wid2 = DGGetItemValDouble (dialogID, EDITCONTROL_EUROFORM_WIDTH_OPTIONS);
							placingZone.cells [rIdx][cIdx].horLen = placingZone.cells [rIdx][cIdx].libPart.form.eu_wid2;
							// ����
							placingZone.cells [rIdx][cIdx].libPart.form.eu_hei2 = DGGetItemValDouble (dialogID, EDITCONTROL_EUROFORM_HEIGHT_OPTIONS);
							placingZone.cells [rIdx][cIdx].verLen = placingZone.cells [rIdx][cIdx].libPart.form.eu_hei2;
						}

						// ��ġ����
						if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM) == TRUE)
							placingZone.cells [rIdx][cIdx].libPart.form.u_ins_wall = true;
						else if (DGGetItemValLong (dialogID, RADIO_ORIENTATION_1_EUROFORM) == FALSE) {
							placingZone.cells [rIdx][cIdx].libPart.form.u_ins_wall = false;
							// ����, ���� ���� ��ȯ
							temp = placingZone.cells [rIdx][cIdx].horLen;
							placingZone.cells [rIdx][cIdx].horLen = placingZone.cells [rIdx][cIdx].verLen;
							placingZone.cells [rIdx][cIdx].verLen = temp;
						}

						placingZone.adjustOtherCellsInSameRow (&placingZone, rIdx, cIdx);
						placingZone.adjustOtherCellsInSameCol (&placingZone, rIdx, cIdx);
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
