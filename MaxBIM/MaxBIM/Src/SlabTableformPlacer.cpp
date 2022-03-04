#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "SlabTableformPlacer.hpp"

using namespace slabTableformPlacerDG;

static SlabTableformPlacingZone		placingZone;	// �⺻ ������ �Ϻ� ���� ����
static InfoSlab		infoSlab;						// ������ ��ü ����

API_Guid	structuralObject_forTableformSlab;		// ���� ��ü�� GUID

static short		clickedBtnItemIdx;			// �׸��� ��ư���� Ŭ���� ��ư�� �ε��� ��ȣ�� ����
static bool			clickedExcludeRestButton;	// ������ ���� ��ư�� �������ϱ�?
static bool			clickedPrevButton;			// ���� ��ư�� �������ϱ�?
static short		clickedRow, clickedCol;		// Ŭ���� ��, �� �ε���
static short		layerInd_Euroform;			// ���̾� ��ȣ: ������
static short		layerInd_SlabTableform;		// ���̾� ��ȣ: ������ ���̺���
static short		layerInd_Plywood;			// ���̾� ��ȣ: ����
static short		layerInd_Timber;			// ���̾� ��ȣ: ����
static short		layerInd_CProfile;			// ���̾� ��ȣ: KS�������� - C����
static short		layerInd_Pinbolt;			// ���̾� ��ȣ: �ɺ�Ʈ
static short		layerInd_Fittings;			// ���̾� ��ȣ: ����ö��
static short		layerInd_GT24Girder;		// ���̾� ��ȣ: GT24 �Ŵ�
static short		layerInd_PERI_Support;		// ���̾� ��ȣ: PERI���ٸ� ������
static short		layerInd_Steel_Support;		// ���̾� ��ȣ: ���� ���ٸ�
static API_Coord3D		firstClickPoint;		// 1��°�� Ŭ���� ��
static GS::Array<API_Guid>	elemList;			// �׷�ȭ�� ���� ������ ��������� GUID�� ���� ������
short	MAX_IND = 50;


// ������ �Ϻο� ���̺����� ��ġ�ϴ� ���� ��ƾ
GSErrCode	placeTableformOnSlabBottom (void)
{
	GSErrCode	err = NoError;
	long		nSel;
	short		xx, yy;
	double		dx, dy, ang;

	// Selection Manager ���� ����
	API_SelectionInfo		selectionInfo;
	API_Element				tElem;
	API_Neig				**selNeigs;
	GS::Array<API_Guid>		morphs;
	GS::Array<API_Guid>		slabs;
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
	GS::Array<API_Coord3D>	coords;

	// ���� ��ü ����
	InfoMorphForSlabTableform	infoMorph;

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


	// ������ ��� ��������
	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err == APIERR_NOPLAN) {
		WriteReport_Alert ("���� ������Ʈ â�� �����ϴ�.");
	}
	if (err == APIERR_NOSEL) {
		WriteReport_Alert ("�ƹ� �͵� �������� �ʾҽ��ϴ�.\n�ʼ� ����: ������ (1��), ������ �Ϻθ� ���� ���� (1��)");
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
		WriteReport_Alert ("�����긦 1�� �����ؾ� �մϴ�.");
		err = APIERR_GENERAL;
		return err;
	}

	// ������ 1���ΰ�?
	if (nMorphs != 1) {
		WriteReport_Alert ("������ �Ϻθ� ���� ������ 1�� �����ϼž� �մϴ�.");
		err = APIERR_GENERAL;
		return err;
	}

	// (1) ������ ������ ������
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	elem.header.guid = slabs.Pop ();
	structuralObject_forTableformSlab = elem.header.guid;
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
		WriteReport_Alert ("������ ���� ���� �ʽ��ϴ�.");
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
		WriteReport_Alert ("������ 3D ���� �������� ���߽��ϴ�.");
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
			if ( (abs (trCoord.x - 1.0) < EPS) || (abs (trCoord.y - 0.0) < EPS) || (abs (trCoord.z - 0.0) < EPS) )
				;
			else if ( (abs (trCoord.x - 0.0) < EPS) || (abs (trCoord.y - 1.0) < EPS) || (abs (trCoord.z - 0.0) < EPS) )
				;
			else if ( (abs (trCoord.x - 0.0) < EPS) || (abs (trCoord.y - 0.0) < EPS) || (abs (trCoord.z - 1.0) < EPS) )
				;
			else if ( (abs (trCoord.x - 0.0) < EPS) || (abs (trCoord.y - 0.0) < EPS) || (abs (trCoord.z - 0.0) < EPS) )
				;
			else
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
		WriteReport_Alert ("�����￡ ������ ���� ���� Ŭ���߽��ϴ�.");
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

	// �۾� �� ���� �ݿ�
	BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	workLevel_slab = 0.0;
	ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
	for (xx = 0 ; xx <= (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
		if (storyInfo.data [0][xx].index == infoSlab.floorInd) {
			workLevel_slab = storyInfo.data [0][xx].level;
			break;
		}
	}
	BMKillHandle ((GSHandle *) &storyInfo.data);

	// �ֿܰ� ����, ���� ���� ���
	double	xMin, xMax;
	double	yMin, yMax;

	xMin = xMax = nodes_sequential [0].x;
	yMin = yMax = nodes_sequential [0].y;

	for (xx = 1 ; xx < nEntered ; ++xx) {
		if (xMin > nodes_sequential [xx].x)		xMin = nodes_sequential [xx].x;
		if (xMax < nodes_sequential [xx].x)		xMax = nodes_sequential [xx].x;
		
		if (yMin > nodes_sequential [xx].y)		yMin = nodes_sequential [xx].y;
		if (yMax < nodes_sequential [xx].y)		yMax = nodes_sequential [xx].y;
	}

	placingZone.borderHorLen = xMax - xMin;
	placingZone.borderVerLen = yMax - yMin;

	// �ֿܰ� ���ϴ� ��, ���� �� ã��
	if (GetDistance (nodes_sequential [0], nodes_sequential [nEntered - 2]) < (placingZone.borderHorLen/2)) {
		// �ڳʰ� ���� ������ ���
		placingZone.bRectangleArea = false;

		// ���ϴ� ��
		placingZone.leftBottom = getUnrotatedPoint (nodes_sequential [nEntered - 2], nodes_sequential [0], RadToDegree (placingZone.ang));
		moveIn2D ('y', placingZone.ang, GetDistance (nodes_sequential [0], nodes_sequential [nEntered - 1]), &placingZone.leftBottom.x, &placingZone.leftBottom.y);
		// ���� ��
		placingZone.rightTop = placingZone.leftBottom;
		moveIn2D ('x', placingZone.ang, placingZone.borderHorLen, &placingZone.rightTop.x, &placingZone.rightTop.y);
		moveIn2D ('y', placingZone.ang, -placingZone.borderVerLen, &placingZone.rightTop.x, &placingZone.rightTop.y);
	} else {
		// �ڳʰ� ���� ������ �ƴ� ���
		placingZone.bRectangleArea = true;

		// ���ϴ� ��
		placingZone.leftBottom = nodes_sequential [0];
		// ���� ��
		placingZone.rightTop = placingZone.leftBottom;
		moveIn2D ('x', placingZone.ang, placingZone.borderHorLen, &placingZone.rightTop.x, &placingZone.rightTop.y);
		moveIn2D ('y', placingZone.ang, -placingZone.borderVerLen, &placingZone.rightTop.x, &placingZone.rightTop.y);
	}	

	// �ֿܰ� �߽� �� ã��
	placingZone.center = placingZone.leftBottom;
	moveIn2D ('x', placingZone.ang, placingZone.borderHorLen/2, &placingZone.center.x, &placingZone.center.y);
	moveIn2D ('y', placingZone.ang, -placingZone.borderVerLen/2, &placingZone.center.x, &placingZone.center.y);

FIRST:

	// [DIALOG] 1��° ���̾�α׿��� ���̺��� ���� �Է� ����
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32502, ACAPI_GetOwnResModule (), slabBottomTableformPlacerHandler1, 0);

	// ���� ������ �� ���� ����
	placingZone.level = infoSlab.level + workLevel_slab + infoSlab.offsetFromTop - infoSlab.thickness - placingZone.gap;

	if (result != DG_OK)
		return err;

	// placingZone�� Cell ���� �ʱ�ȭ
	placingZone.initCells (&placingZone);

	// [DIALOG] 2��° ���̾�α׿��� ���̺��� ��ġ�� �����ϰų� ���� ���縦 �����մϴ�.
	clickedExcludeRestButton = false;	// ������ ä��� ���� ���� Ŭ�� ����
	clickedPrevButton = false;			// ���� ��ư Ŭ�� ����
	result = DGBlankModalDialog (450, 300, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, slabBottomTableformPlacerHandler2, 0);

	// ���� ��ư�� ������ 1��° ���̾�α� �ٽ� ����
	if (clickedPrevButton == true)
		goto FIRST;

	// ���̺��� ���� ä���
	err = placingZone.fillTableformAreas ();

	// ������ ���� ä��� - ����, ����
	if (clickedExcludeRestButton == false)
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

	// ȭ�� ���ΰ�ħ
	ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	bool	regenerate = true;
	ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

	return	err;
}

// Cell �迭�� �ʱ�ȭ��
void	SlabTableformPlacingZone::initCells (SlabTableformPlacingZone* placingZone)
{
	short xx, yy;

	// �ʱ� �� ����
	for (xx = 0 ; xx < MAX_IND ; ++xx) {
		for (yy = 0 ; yy < MAX_IND ; ++yy) {
			placingZone->cells [xx][yy].objType = placingZone->iTableformType;
			placingZone->cells [xx][yy].ang = placingZone->ang;
			placingZone->cells [xx][yy].horLen = placingZone->initHorLen;
			placingZone->cells [xx][yy].verLen = placingZone->initVerLen;
			placingZone->cells [xx][yy].leftBottomX = 0.0;
			placingZone->cells [xx][yy].leftBottomY = 0.0;
			placingZone->cells [xx][yy].leftBottomZ = placingZone->level;
		}
	}

	// �ʱ� �� ���� ����
	if (placingZone->bVertical == true) {
		placingZone->nHorCells = (short)floor (placingZone->borderHorLen / placingZone->initHorLen);
		placingZone->nVerCells = (short)floor (placingZone->borderVerLen / placingZone->initVerLen);
	} else {
		placingZone->nHorCells = (short)floor (placingZone->borderHorLen / placingZone->initVerLen);
		placingZone->nVerCells = (short)floor (placingZone->borderVerLen / placingZone->initHorLen);
	}

	// �ʱ� ���̺��� �迭 ��ü �ʺ�, ���� ����
	placingZone->formArrayWidth = 0.0;
	placingZone->formArrayHeight = 0.0;
	if (placingZone->bVertical == true) {
		for (xx = 0 ; xx < placingZone->nHorCells ; ++xx)	placingZone->formArrayWidth += placingZone->cells [0][xx].horLen;
		for (xx = 0 ; xx < placingZone->nVerCells ; ++xx)	placingZone->formArrayHeight += placingZone->cells [xx][0].verLen;
	} else {
		for (xx = 0 ; xx < placingZone->nHorCells ; ++xx)	placingZone->formArrayWidth += placingZone->cells [0][xx].verLen;
		for (xx = 0 ; xx < placingZone->nVerCells ; ++xx)	placingZone->formArrayHeight += placingZone->cells [xx][0].horLen;
	}

	// �ʱ� ���� ���� ����
	placingZone->marginLeft = placingZone->marginRight = (placingZone->borderHorLen - placingZone->formArrayWidth) / 2;
	placingZone->marginBottom = placingZone->marginTop = (placingZone->borderVerLen - placingZone->formArrayHeight) / 2;
}

// Cell �迭�� ��ġ�� ������
void	SlabTableformPlacingZone::alignPlacingZone (SlabTableformPlacingZone* placingZone)
{
	short	xx, yy;
	double	accumLength;

	API_Coord3D	point = placingZone->leftBottom;

	moveIn2D ('x', placingZone->ang, placingZone->marginLeft, &point.x, &point.y);
	moveIn2D ('y', placingZone->ang, -placingZone->marginBottom, &point.x, &point.y);

	// ���̺��� ���� ��ǥ ����
	placingZone->leftBottomX = point.x;
	placingZone->leftBottomY = point.y;
	placingZone->leftBottomZ = placingZone->level;

	if (placingZone->bVertical) {
		for (xx = 0 ; xx < placingZone->nVerCells ; ++xx) {
			accumLength = 0.0;
			for (yy = 0 ; yy < placingZone->nHorCells ; ++yy) {
				placingZone->cells [xx][yy].ang = placingZone->ang;
				placingZone->cells [xx][yy].leftBottomX = point.x;
				placingZone->cells [xx][yy].leftBottomY = point.y;
				placingZone->cells [xx][yy].leftBottomZ = placingZone->level;

				moveIn2D ('x', placingZone->ang, placingZone->cells [xx][yy].horLen, &point.x, &point.y);
				accumLength += placingZone->cells [xx][yy].horLen;
			}
			moveIn2D ('x', placingZone->ang, -accumLength, &point.x, &point.y);
			moveIn2D ('y', placingZone->ang, -placingZone->cells [xx][0].verLen, &point.x, &point.y);
		}
	} else {
		for (xx = 0 ; xx < placingZone->nVerCells ; ++xx) {
			accumLength = 0.0;
			for (yy = 0 ; yy < placingZone->nHorCells ; ++yy) {
				placingZone->cells [xx][yy].ang = placingZone->ang;
				placingZone->cells [xx][yy].leftBottomX = point.x;
				placingZone->cells [xx][yy].leftBottomY = point.y;
				placingZone->cells [xx][yy].leftBottomZ = placingZone->level;

				moveIn2D ('x', placingZone->ang, placingZone->cells [xx][yy].verLen, &point.x, &point.y);
				accumLength += placingZone->cells [xx][yy].verLen;
			}
			moveIn2D ('x', placingZone->ang, -accumLength, &point.x, &point.y);
			moveIn2D ('y', placingZone->ang, -placingZone->cells [xx][0].horLen, &point.x, &point.y);
		}
	}
}

//// �ش� �� ������ ������� ���̺귯�� ��ġ
//API_Guid	SlabTableformPlacingZone::placeLibPart (CellForSlabTableform objInfo)
//{
//	GSErrCode	err = NoError;
//
//	API_Element			element;
//	API_ElementMemo		memo;
//	API_LibPart			libPart;
//
//	const	GS::uchar_t* gsmName = NULL;
//	double	aParam;
//	double	bParam;
//	Int32	addParNum;
//
//	char	tempString [20];
//
//	// GUID ���� �ʱ�ȭ
//	element.header.guid.clock_seq_hi_and_reserved = 0;
//	element.header.guid.clock_seq_low = 0;
//	element.header.guid.node[0] = 0;
//	element.header.guid.node[1] = 0;
//	element.header.guid.node[2] = 0;
//	element.header.guid.node[3] = 0;
//	element.header.guid.node[4] = 0;
//	element.header.guid.node[5] = 0;
//	element.header.guid.time_hi_and_version = 0;
//	element.header.guid.time_low = 0;
//	element.header.guid.time_mid = 0;
//
//	// ���̺귯�� �̸� ����
//	if (objInfo.objType == NONE)			return element.header.guid;
//	if (objInfo.objType == SLAB_TABLEFORM)	gsmName = L("������ ���̺��� (���ǳ�) v1.0.gsm");
//	if (objInfo.objType == PLYWOOD)			gsmName = L("����v1.0.gsm");
//	if (objInfo.objType == WOOD)			gsmName = L("����v1.0.gsm");
//
//	// ��ü �ε�
//	BNZeroMemory (&libPart, sizeof (libPart));
//	GS::ucscpy (libPart.file_UName, gsmName);
//	err = ACAPI_LibPart_Search (&libPart, false);
//	if (err != NoError)
//		return element.header.guid;
//	if (libPart.location != NULL)
//		delete libPart.location;
//
//	ACAPI_LibPart_Get (&libPart);
//
//	BNZeroMemory (&element, sizeof (API_Element));
//	BNZeroMemory (&memo, sizeof (API_ElementMemo));
//
//	element.header.typeID = API_ObjectID;
//	element.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));
//
//	ACAPI_Element_GetDefaults (&element, &memo);
//	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);
//
//	// ���̺귯���� �Ķ���� �� �Է�
//	element.object.libInd = libPart.index;
//	element.object.reflected = false;
//	element.object.pos.x = objInfo.leftBottomX;
//	element.object.pos.y = objInfo.leftBottomY;
//	element.object.level = objInfo.leftBottomZ;
//	element.object.xRatio = aParam;
//	element.object.yRatio = bParam;
//	element.object.angle = objInfo.ang;
//	element.header.floorInd = infoSlab.floorInd;
//
//	// ������ ���̺����� C������ ������ �� ����ϴ� �糡 ����
//	double	marginEnds;
//
//	if (objInfo.objType == SLAB_TABLEFORM) {
//		element.header.layer = layerInd_SlabTableform;
//
//		// ���ι��� (Horizontal)
//		if (objInfo.libPart.tableform.direction == true) {
//			// Ÿ��: ���� ���� x ���� ����
//			sprintf (tempString, "%.0f x %.0f", round (objInfo.horLen, 3) * 1000, round (objInfo.verLen, 3) * 1000);
//			element.object.xRatio = objInfo.horLen;
//			element.object.yRatio = objInfo.verLen;
//
//			// �̵��Ͽ� ��ġ �ٷ����
//			element.object.pos.x += ( objInfo.verLen * sin(objInfo.ang) );
//			element.object.pos.y -= ( objInfo.verLen * cos(objInfo.ang) );
//
//			setParameterByName (&memo, "bLprofileOnNorth", objInfo.libPart.tableform.bLprofileOnNorth);
//			setParameterByName (&memo, "bLprofileOnSouth", objInfo.libPart.tableform.bLprofileOnSouth);
//			setParameterByName (&memo, "bLprofileOnWest", objInfo.libPart.tableform.bLprofileOnWest);
//			setParameterByName (&memo, "bLprofileOnEast", objInfo.libPart.tableform.bLprofileOnEast);
//
//			// C���� ��ġ
//			CellForSlabTableform	cprofile;
//
//			cprofile.objType = CPROFILE;
//			cprofile.leftBottomX = objInfo.leftBottomX;
//			cprofile.leftBottomY = objInfo.leftBottomY;
//			cprofile.leftBottomZ = objInfo.leftBottomZ;
//			cprofile.ang = objInfo.ang - DegreeToRad (90.0);
//			cprofile.libPart.cprofile.angX = DegreeToRad (270.0);
//			cprofile.libPart.cprofile.angY = DegreeToRad (0.0);
//			cprofile.libPart.cprofile.iAnchor = 8;
//			cprofile.libPart.cprofile.len = floor (objInfo.horLen * 10) / 10;
//			marginEnds = objInfo.horLen - cprofile.libPart.cprofile.len;
//
//			moveIn3D ('x', cprofile.ang, 0.300 + 0.006 + 0.020, &cprofile.leftBottomX, &cprofile.leftBottomY, &cprofile.leftBottomZ);
//			moveIn3D ('y', cprofile.ang, marginEnds / 2, &cprofile.leftBottomX, &cprofile.leftBottomY, &cprofile.leftBottomZ);
//			moveIn3D ('z', cprofile.ang, -0.0615, &cprofile.leftBottomX, &cprofile.leftBottomY, &cprofile.leftBottomZ);
//			elemList.Push (placeLibPartOnSlabTableform (cprofile));
//			moveIn3D ('x', cprofile.ang, -(0.300 + 0.006 + 0.020) + objInfo.verLen - (0.300 - 0.006 - 0.020), &cprofile.leftBottomX, &cprofile.leftBottomY, &cprofile.leftBottomZ);
//			elemList.Push (placeLibPartOnSlabTableform (cprofile));
//
//			cprofile.ang = objInfo.ang + DegreeToRad (90.0);
//			cprofile.leftBottomX = objInfo.leftBottomX;
//			cprofile.leftBottomY = objInfo.leftBottomY;
//			moveIn3D ('x', cprofile.ang, -objInfo.verLen, &cprofile.leftBottomX, &cprofile.leftBottomY, &cprofile.leftBottomZ);
//			moveIn3D ('y', cprofile.ang, -objInfo.horLen, &cprofile.leftBottomX, &cprofile.leftBottomY, &cprofile.leftBottomZ);
//			moveIn3D ('x', cprofile.ang, 0.300 + 0.006 + 0.020, &cprofile.leftBottomX, &cprofile.leftBottomY, &cprofile.leftBottomZ);
//			moveIn3D ('y', cprofile.ang, marginEnds / 2, &cprofile.leftBottomX, &cprofile.leftBottomY, &cprofile.leftBottomZ);
//			elemList.Push (placeLibPartOnSlabTableform (cprofile));
//			moveIn3D ('x', cprofile.ang, -(0.300 + 0.006 + 0.020) + objInfo.verLen - (0.300 - 0.006 - 0.020), &cprofile.leftBottomX, &cprofile.leftBottomY, &cprofile.leftBottomZ);
//			elemList.Push (placeLibPartOnSlabTableform (cprofile));
//
//			// ����ö�� (�簢�ͼ�Ȱ��) ��ġ
//			CellForSlabTableform	fittings;
//
//			fittings.objType = FITTINGS;
//			fittings.leftBottomX = objInfo.leftBottomX;
//			fittings.leftBottomY = objInfo.leftBottomY;
//			fittings.leftBottomZ = objInfo.leftBottomZ;
//			fittings.ang = objInfo.ang;
//			fittings.libPart.fittings.angX = DegreeToRad (270.0);
//			fittings.libPart.fittings.angY = DegreeToRad (0.0);
//			fittings.libPart.fittings.bolt_len = 0.150;
//			fittings.libPart.fittings.bolt_dia = 0.012;
//			fittings.libPart.fittings.bWasher1 = false;
//			fittings.libPart.fittings.washer_pos1 = 0.0;
//			fittings.libPart.fittings.bWasher2 = true;
//			fittings.libPart.fittings.washer_pos2 = 0.0766;
//			fittings.libPart.fittings.washer_size = 0.100;
//			strncpy (fittings.libPart.fittings.nutType, "������Ʈ", strlen ("������Ʈ"));
//
//			moveIn3D ('x', fittings.ang, 0.328, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
//			moveIn3D ('y', fittings.ang, -0.300, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
//			moveIn3D ('z', fittings.ang, -0.0499, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
//			elemList.Push (placeLibPartOnSlabTableform (fittings));
//			moveIn3D ('y', fittings.ang, 0.300 - objInfo.verLen + 0.300, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
//			elemList.Push (placeLibPartOnSlabTableform (fittings));
//			moveIn3D ('x', fittings.ang, -0.328 + objInfo.horLen - 0.328, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
//			elemList.Push (placeLibPartOnSlabTableform (fittings));
//			moveIn3D ('y', fittings.ang, -0.300 + objInfo.verLen - 0.300, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
//			elemList.Push (placeLibPartOnSlabTableform (fittings));
//			
//		// ���ι��� (Vertical)
//		} else {
//			// Ÿ��: ���� ���� x ���� ����
//			sprintf (tempString, "%.0f x %.0f", round (objInfo.verLen, 3) * 1000, round (objInfo.horLen, 3) * 1000);
//			element.object.xRatio = objInfo.verLen;
//			element.object.yRatio = objInfo.horLen;
//			
//			// 90�� ȸ���� ���·� ��ġ
//			element.object.angle += DegreeToRad (90.0);
//
//			// �̵��Ͽ� ��ġ �ٷ����
//			element.object.pos.x += ( objInfo.verLen * sin(objInfo.ang) );
//			element.object.pos.y -= ( objInfo.verLen * cos(objInfo.ang) );
//			element.object.pos.x += ( objInfo.horLen * cos(objInfo.ang) );
//			element.object.pos.y += ( objInfo.horLen * sin(objInfo.ang) );
//
//			setParameterByName (&memo, "bLprofileOnNorth", objInfo.libPart.tableform.bLprofileOnNorth);
//			setParameterByName (&memo, "bLprofileOnSouth", objInfo.libPart.tableform.bLprofileOnSouth);
//			setParameterByName (&memo, "bLprofileOnWest", objInfo.libPart.tableform.bLprofileOnWest);
//			setParameterByName (&memo, "bLprofileOnEast", objInfo.libPart.tableform.bLprofileOnEast);
//
//			// C���� ��ġ
//			CellForSlabTableform	cprofile;
//
//			cprofile.objType = CPROFILE;
//			cprofile.leftBottomX = objInfo.leftBottomX;
//			cprofile.leftBottomY = objInfo.leftBottomY;
//			cprofile.leftBottomZ = objInfo.leftBottomZ;
//			cprofile.ang = objInfo.ang;
//			cprofile.libPart.cprofile.angX = DegreeToRad (270.0);
//			cprofile.libPart.cprofile.angY = DegreeToRad (0.0);
//			cprofile.libPart.cprofile.iAnchor = 8;
//			cprofile.libPart.cprofile.len = floor (objInfo.verLen * 10) / 10;
//			marginEnds = objInfo.verLen - cprofile.libPart.cprofile.len;
//
//			moveIn3D ('x', cprofile.ang, 0.300 + 0.006 + 0.020, &cprofile.leftBottomX, &cprofile.leftBottomY, &cprofile.leftBottomZ);
//			moveIn3D ('y', cprofile.ang, marginEnds / 2 - objInfo.verLen, &cprofile.leftBottomX, &cprofile.leftBottomY, &cprofile.leftBottomZ);
//			moveIn3D ('z', cprofile.ang, -0.0615, &cprofile.leftBottomX, &cprofile.leftBottomY, &cprofile.leftBottomZ);
//			elemList.Push (placeLibPartOnSlabTableform (cprofile));
//			moveIn3D ('x', cprofile.ang, -(0.300 + 0.006 + 0.020) + objInfo.horLen - (0.300 - 0.006 - 0.020), &cprofile.leftBottomX, &cprofile.leftBottomY, &cprofile.leftBottomZ);
//			elemList.Push (placeLibPartOnSlabTableform (cprofile));
//
//			cprofile.ang = objInfo.ang + DegreeToRad (180.0);
//			cprofile.leftBottomX = objInfo.leftBottomX;
//			cprofile.leftBottomY = objInfo.leftBottomY;
//			moveIn3D ('x', cprofile.ang, -objInfo.horLen, &cprofile.leftBottomX, &cprofile.leftBottomY, &cprofile.leftBottomZ);
//			moveIn3D ('y', cprofile.ang, -objInfo.verLen, &cprofile.leftBottomX, &cprofile.leftBottomY, &cprofile.leftBottomZ);
//			moveIn3D ('x', cprofile.ang, 0.300 + 0.006 + 0.020, &cprofile.leftBottomX, &cprofile.leftBottomY, &cprofile.leftBottomZ);
//			moveIn3D ('y', cprofile.ang, marginEnds / 2 + objInfo.verLen, &cprofile.leftBottomX, &cprofile.leftBottomY, &cprofile.leftBottomZ);
//			elemList.Push (placeLibPartOnSlabTableform (cprofile));
//			moveIn3D ('x', cprofile.ang, -(0.300 + 0.006 + 0.020) + objInfo.horLen - (0.300 - 0.006 - 0.020), &cprofile.leftBottomX, &cprofile.leftBottomY, &cprofile.leftBottomZ);
//			elemList.Push (placeLibPartOnSlabTableform (cprofile));
//
//			// ����ö�� (�簢�ͼ�Ȱ��) ��ġ
//			CellForSlabTableform	fittings;
//
//			fittings.objType = FITTINGS;
//			fittings.leftBottomX = objInfo.leftBottomX;
//			fittings.leftBottomY = objInfo.leftBottomY;
//			fittings.leftBottomZ = objInfo.leftBottomZ;
//			fittings.ang = objInfo.ang;
//			fittings.libPart.fittings.angX = DegreeToRad (270.0);
//			fittings.libPart.fittings.angY = DegreeToRad (0.0);
//			fittings.libPart.fittings.bolt_len = 0.150;
//			fittings.libPart.fittings.bolt_dia = 0.012;
//			fittings.libPart.fittings.bWasher1 = false;
//			fittings.libPart.fittings.washer_pos1 = 0.0;
//			fittings.libPart.fittings.bWasher2 = true;
//			fittings.libPart.fittings.washer_pos2 = 0.0766;
//			fittings.libPart.fittings.washer_size = 0.100;
//			strncpy (fittings.libPart.fittings.nutType, "������Ʈ", strlen ("������Ʈ"));
//
//			moveIn3D ('x', fittings.ang, 0.300, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
//			moveIn3D ('y', fittings.ang, -0.328, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
//			moveIn3D ('z', fittings.ang, -0.0499, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
//			elemList.Push (placeLibPartOnSlabTableform (fittings));
//			moveIn3D ('x', fittings.ang, -0.300 + objInfo.horLen - 0.300, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
//			elemList.Push (placeLibPartOnSlabTableform (fittings));
//			moveIn3D ('y', fittings.ang, 0.328 - objInfo.verLen + 0.328, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
//			elemList.Push (placeLibPartOnSlabTableform (fittings));
//			moveIn3D ('x', fittings.ang, 0.300 - objInfo.horLen + 0.300, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
//			elemList.Push (placeLibPartOnSlabTableform (fittings));
//		}
//		setParameterByName (&memo, "type", tempString);
//
//	} else if (objInfo.objType == PLYWOOD) {
//		element.header.layer = layerInd_Plywood;
//		setParameterByName (&memo, "p_stan", "��԰�");							// �԰�
//		setParameterByName (&memo, "w_dir", "�ٴڱ��");						// ��ġ����
//		setParameterByName (&memo, "p_thk", "11.5T");							// �β�
//		setParameterByName (&memo, "p_wid", objInfo.libPart.plywood.p_wid);		// ����
//		setParameterByName (&memo, "p_leng", objInfo.libPart.plywood.p_leng);	// ����
//		setParameterByName (&memo, "sogak", 0.0);								// ����Ʋ OFF
//		
//	} else if (objInfo.objType == WOOD) {
//		element.header.layer = layerInd_Wood;
//		setParameterByName (&memo, "w_ins", "�ٴڴ�����");					// ��ġ����
//		setParameterByName (&memo, "w_w", objInfo.libPart.wood.w_w);		// �β�
//		setParameterByName (&memo, "w_h", objInfo.libPart.wood.w_h);		// �ʺ�
//		setParameterByName (&memo, "w_leng", objInfo.libPart.wood.w_leng);	// ����
//		setParameterByName (&memo, "w_ang", objInfo.libPart.wood.w_ang);	// ����
//	}
//
//	// ��ü ��ġ
//	ACAPI_Element_Create (&element, &memo);
//	ACAPI_DisposeElemMemoHdls (&memo);
//
//	return element.header.guid;
//}
//
//// ������ ���̺����� �μ� ö���鿡 �ش��ϴ� ���̺귯�� ��ġ
//API_Guid	SlabTableformPlacingZone::placeLibPartOnSlabTableform (CellForSlabTableform objInfo)
//{
//	GSErrCode	err = NoError;
//
//	API_Element			element;
//	API_ElementMemo		memo;
//	API_LibPart			libPart;
//
//	const	GS::uchar_t* gsmName = NULL;
//	double	aParam;
//	double	bParam;
//	Int32	addParNum;
//
//	// GUID ���� �ʱ�ȭ
//	element.header.guid.clock_seq_hi_and_reserved = 0;
//	element.header.guid.clock_seq_low = 0;
//	element.header.guid.node[0] = 0;
//	element.header.guid.node[1] = 0;
//	element.header.guid.node[2] = 0;
//	element.header.guid.node[3] = 0;
//	element.header.guid.node[4] = 0;
//	element.header.guid.node[5] = 0;
//	element.header.guid.time_hi_and_version = 0;
//	element.header.guid.time_low = 0;
//	element.header.guid.time_mid = 0;
//
//	// ���̺귯�� �̸� ����
//	if (objInfo.objType == NONE)			return element.header.guid;
//	if (objInfo.objType == CPROFILE)		gsmName = L("KS��������v1.0.gsm");
//	if (objInfo.objType == FITTINGS)		gsmName = L("����ö�� (�簢�ͼ�Ȱ��) v1.0.gsm");
//
//	// ��ü �ε�
//	BNZeroMemory (&libPart, sizeof (libPart));
//	GS::ucscpy (libPart.file_UName, gsmName);
//	err = ACAPI_LibPart_Search (&libPart, false);
//	if (err != NoError)
//		return element.header.guid;
//	if (libPart.location != NULL)
//		delete libPart.location;
//
//	ACAPI_LibPart_Get (&libPart);
//
//	BNZeroMemory (&element, sizeof (API_Element));
//	BNZeroMemory (&memo, sizeof (API_ElementMemo));
//
//	element.header.typeID = API_ObjectID;
//	element.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));
//
//	ACAPI_Element_GetDefaults (&element, &memo);
//	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);
//
//	// ���̺귯���� �Ķ���� �� �Է�
//	element.object.libInd = libPart.index;
//	element.object.reflected = false;
//	element.object.pos.x = objInfo.leftBottomX;
//	element.object.pos.y = objInfo.leftBottomY;
//	element.object.level = objInfo.leftBottomZ;
//	element.object.xRatio = aParam;
//	element.object.yRatio = bParam;
//	element.object.angle = objInfo.ang;
//	element.header.floorInd = infoSlab.floorInd;
//
//	if (objInfo.objType == CPROFILE) {
//		element.header.layer = layerInd_CProfile;
//
//		setParameterByName (&memo, "angX", objInfo.libPart.cprofile.angX);	// ȸ��X
//		setParameterByName (&memo, "angY", objInfo.libPart.cprofile.angY);	// ȸ��Y
//
//		setParameterByName (&memo, "type", "��");									// �з� (���, ��)
//		setParameterByName (&memo, "shape", "C����");								// ���� (C����, H����...)
//		setParameterByName (&memo, "iAnchor", objInfo.libPart.cprofile.iAnchor);	// ��Ŀ ����Ʈ
//		setParameterByName (&memo, "len", objInfo.libPart.cprofile.len);			// ����
//		setParameterByName (&memo, "ZZYZX", objInfo.libPart.cprofile.len);			// ����
//		setParameterByName (&memo, "nom", "75 x 40 x 5 x 7");						// �԰�
//		setParameterByName (&memo, "HH", 0.075);
//		setParameterByName (&memo, "BB", 0.040);
//		setParameterByName (&memo, "t1", 0.005);
//		setParameterByName (&memo, "t2", 0.007);
//		setParameterByName (&memo, "mat", 19.0);									// ����
//
//	} else if (objInfo.objType == FITTINGS) {
//		element.header.layer = layerInd_Fittings;
//
//		setParameterByName (&memo, "angX", objInfo.libPart.fittings.angX);	// ȸ��X
//		setParameterByName (&memo, "angY", objInfo.libPart.fittings.angY);	// ȸ��Y
//
//		setParameterByName (&memo, "bolt_len", objInfo.libPart.fittings.bolt_len);			// ��Ʈ ����
//		setParameterByName (&memo, "bolt_dia", objInfo.libPart.fittings.bolt_dia);			// ��Ʈ ����
//		setParameterByName (&memo, "bWasher1", objInfo.libPart.fittings.bWasher1);			// �ͼ�1 On/Off
//		setParameterByName (&memo, "washer_pos1", objInfo.libPart.fittings.washer_pos1);	// �ͼ�1 ��ġ
//		setParameterByName (&memo, "bWasher2", objInfo.libPart.fittings.bWasher2);			// �ͼ�2 On/Off
//		setParameterByName (&memo, "washer_pos2", objInfo.libPart.fittings.washer_pos2);	// �ͼ�2 ��ġ
//		setParameterByName (&memo, "washer_size", objInfo.libPart.fittings.washer_size);	// �ͼ� ũ��
//		setParameterByName (&memo, "nutType", objInfo.libPart.fittings.nutType);			// ��Ʈ Ÿ��
//	
//	}
//
//	// ��ü ��ġ
//	ACAPI_Element_Create (&element, &memo);
//	ACAPI_DisposeElemMemoHdls (&memo);
//
//	return element.header.guid;
//}

// ���̺��� ���� ä���
GSErrCode	SlabTableformPlacingZone::fillTableformAreas (void)
{
	GSErrCode	err = NoError;
	short	xx, yy;

	// Ÿ��, ��ġ���⿡ ���� �ٸ��� ��ġ��
	if (placingZone.iTableformType == EUROFORM) {
		// ������ ��ġ
		EasyObjectPlacement euroform;

		for (xx = 0 ; xx < placingZone.nVerCells ; ++xx) {
			for (yy = 0 ; yy < placingZone.nHorCells ; ++yy) {
				euroform.init (L("������v2.0.gsm"), layerInd_Euroform, infoSlab.floorInd, placingZone.cells [xx][yy].leftBottomX, placingZone.cells [xx][yy].leftBottomY, placingZone.cells [xx][yy].leftBottomZ, placingZone.cells [xx][yy].ang);
				if (placingZone.bVertical == false) {
					moveIn3D ('y', euroform.radAng, -placingZone.cells [xx][yy].horLen, &euroform.posX, &euroform.posY, &euroform.posZ);
					euroform.radAng += DegreeToRad (90.0);
				}
				elemList.Push (euroform.placeObject (6,
					"eu_stan_onoff", APIParT_Boolean, "1.0",
					"eu_wid", APIParT_CString, format_string ("%.0f", placingZone.cells [xx][yy].horLen * 1000),
					"eu_hei", APIParT_CString, format_string ("%.0f", placingZone.cells [xx][yy].verLen * 1000),
					"u_ins", APIParT_CString, "�������",
					"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
					"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0))));
			}
		}

		// C���� ��ġ
		// ...

		// �ɺ�Ʈ ��ġ
		// ...
	} else if (placingZone.iTableformType == TABLEFORM) {
		// ���ǳ� ��ġ
		EasyObjectPlacement tableform;

		char verLenIntStr [12];
		char horLenIntStr [12];
		char typeStr [24];
		char verLenDoubleStr [12];
		char horLenDoubleStr [12];

		for (xx = 0 ; xx < placingZone.nVerCells ; ++xx) {
			for (yy = 0 ; yy < placingZone.nHorCells ; ++yy) {
				tableform.init (L("������ ���̺��� (���ǳ�) v1.0.gsm"), layerInd_SlabTableform, infoSlab.floorInd, placingZone.cells [xx][yy].leftBottomX, placingZone.cells [xx][yy].leftBottomY, placingZone.cells [xx][yy].leftBottomZ, placingZone.cells [xx][yy].ang);
				if (placingZone.bVertical == true) {
					tableform.radAng -= DegreeToRad (90.0);
				} else {
					moveIn3D ('y', tableform.radAng, -placingZone.cells [xx][yy].horLen, &tableform.posX, &tableform.posY, &tableform.posZ);
				}
				sprintf (verLenIntStr, "%.0f", placingZone.cells [xx][yy].verLen * 1000);
				sprintf (horLenIntStr, "%.0f", placingZone.cells [xx][yy].horLen * 1000);
				sprintf (typeStr, "%s x %s", verLenIntStr, horLenIntStr);
				sprintf (verLenDoubleStr, "%f", placingZone.cells [xx][yy].verLen);
				sprintf (horLenDoubleStr, "%f", placingZone.cells [xx][yy].horLen);
				elemList.Push (tableform.placeObject (8,
					"type", APIParT_CString, typeStr,
					"A", APIParT_Length, verLenDoubleStr,
					"B", APIParT_Length, horLenDoubleStr,
					"ZZYZX", APIParT_Length, "0.0615",
					"bLprofileOnNorth", APIParT_Boolean, "0.0",
					"bLprofileOnSouth", APIParT_Boolean, "0.0",
					"bLprofileOnWest", APIParT_Boolean, "0.0",
					"bLprofileOnEast", APIParT_Boolean, "0.0"));
			}
		}

		// C���� ��ġ
		// ...

		// ����ö�� ��ġ
		// ...

		// PERI���ٸ� ��ġ
		// ...
	} else if (placingZone.iTableformType == PLYWOOD) {
		// ���� ��ġ
		EasyObjectPlacement plywood;

		for (xx = 0 ; xx < placingZone.nVerCells ; ++xx) {
			for (yy = 0 ; yy < placingZone.nHorCells ; ++yy) {
				plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoSlab.floorInd, placingZone.cells [xx][yy].leftBottomX, placingZone.cells [xx][yy].leftBottomY, placingZone.cells [xx][yy].leftBottomZ, placingZone.cells [xx][yy].ang);
				if (placingZone.bVertical == true) {
					plywood.radAng -= DegreeToRad (90.0);
				} else {
					moveIn3D ('y', plywood.radAng, -placingZone.cells [xx][yy].horLen, &plywood.posX, &plywood.posY, &plywood.posZ);
				}
				elemList.Push (plywood.placeObject (7,
					"p_stan", APIParT_CString, "��԰�",
					"w_dir", APIParT_CString, "�ٴڱ��",
					"p_thk", APIParT_CString, "11.5T",
					"p_wid", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].horLen),
					"p_leng", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].verLen),
					"p_ang", APIParT_Angle, format_string ("%f", 0.0),
					"sogak", APIParT_Boolean, "0.0"));
			}
		}

		// GT24 �Ŵ� ��ġ
		// ...

		// PERI���ٸ� ��ġ
		// ...

		// �������ٸ� ��ġ
		// ...
	}

	return err;
}

// ������ ���� ä��� (����, ����)
GSErrCode	SlabTableformPlacingZone::fillRestAreas (void)
{
	GSErrCode	err = NoError;
	short		xx;
//	EasyObjectPlacement	plywood, timber;
//	double		startXPos, startYPos;
//	API_Coord3D	axisPoint, rotatedPoint, unrotatedPoint;
//
//	// ȸ������ �Ǵ� ��
//	axisPoint.x = firstClickPoint.x;
//	axisPoint.y = firstClickPoint.y;
//	axisPoint.z = firstClickPoint.y;
//
//	// ���� ��ġ (TOP)
//	plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoSlab.floorInd, axisPoint.x, axisPoint.y - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) - placingZone.upMove, placingZone.level, placingZone.ang);
//
//	// ��ġ ���� ��ȸ�������� ��ȯ
//	rotatedPoint.x = plywood.posX;
//	rotatedPoint.y = plywood.posY;
//	rotatedPoint.z = plywood.posZ;
//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (plywood.radAng));
//	plywood.posX = unrotatedPoint.x;
//	plywood.posY = unrotatedPoint.y;
//	plywood.posZ = unrotatedPoint.z;
//
//	remainLength = placingZone.outerRight - placingZone.outerLeft;
//	while (remainLength > EPS) {
//		if (remainLength > (2.440 + EPS)) {
//			currentLength = 2.440;
//			remainLength -= 2.440;
//		} else {
//			currentLength = remainLength;
//			remainLength = 0;
//		}
//
//		elemList.Push (plywood.placeObject (7,
//			"p_stan", APIParT_CString, "��԰�",
//			"w_dir", APIParT_CString, "�ٴڱ��",
//			"p_thk", APIParT_CString, "11.5T",
//			"p_wid", APIParT_Length, format_string ("%f", (placingZone.outerTop - placingZone.outerBottom) / 2 - (placingZone.formArrayHeight / 2) + placingZone.upMove),
//			"p_leng", APIParT_Length, format_string ("%f", currentLength),
//			"p_ang", APIParT_Angle, format_string ("%f", 0.0),
//			"sogak", APIParT_Boolean, "0.0"));
//		moveIn3D ('x', plywood.radAng, currentLength, &plywood.posX, &plywood.posY, &plywood.posZ);
//	}
//
//	// ���� ��ġ (BOTTOM)
//	plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoSlab.floorInd, axisPoint.x, axisPoint.y - (placingZone.outerTop - placingZone.outerBottom), placingZone.level, placingZone.ang);
//
//	// ��ġ ���� ��ȸ�������� ��ȯ
//	rotatedPoint.x = plywood.posX;
//	rotatedPoint.y = plywood.posY;
//	rotatedPoint.z = plywood.posZ;
//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (plywood.radAng));
//	plywood.posX = unrotatedPoint.x;
//	plywood.posY = unrotatedPoint.y;
//	plywood.posZ = unrotatedPoint.z;
//
//	remainLength = placingZone.outerRight - placingZone.outerLeft;
//	while (remainLength > EPS) {
//		if (remainLength > (2.440 + EPS)) {
//			currentLength = 2.440;
//			remainLength -= 2.440;
//		} else {
//			currentLength = remainLength;
//			remainLength = 0;
//		}
//
//		elemList.Push (plywood.placeObject (7,
//			"p_stan", APIParT_CString, "��԰�",
//			"w_dir", APIParT_CString, "�ٴڱ��",
//			"p_thk", APIParT_CString, "11.5T",
//			"p_wid", APIParT_Length, format_string ("%f", (placingZone.outerTop - placingZone.outerBottom) / 2 - (placingZone.formArrayHeight / 2) - placingZone.upMove),
//			"p_leng", APIParT_Length, format_string ("%f", currentLength),
//			"p_ang", APIParT_Angle, format_string ("%f", 0.0),
//			"sogak", APIParT_Boolean, "0.0"));
//		moveIn3D ('x', plywood.radAng, currentLength, &plywood.posX, &plywood.posY, &plywood.posZ);
//	}
//
//	// ���� ��ġ (LEFT)
//	plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoSlab.floorInd, axisPoint.x + ((placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) - placingZone.leftMove), axisPoint.y - ((placingZone.outerTop - placingZone.outerBottom) / 2 - (placingZone.formArrayHeight / 2) + placingZone.upMove), placingZone.level, placingZone.ang);
//
//	// ��ġ ���� ��ȸ�������� ��ȯ
//	rotatedPoint.x = plywood.posX;
//	rotatedPoint.y = plywood.posY;
//	rotatedPoint.z = plywood.posZ;
//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (plywood.radAng));
//	plywood.posX = unrotatedPoint.x;
//	plywood.posY = unrotatedPoint.y;
//	plywood.posZ = unrotatedPoint.z;
//
//	remainLength = placingZone.formArrayHeight;
//	while (remainLength > EPS) {
//		if (remainLength > (2.440 + EPS)) {
//			currentLength = 2.440;
//			remainLength -= 2.440;
//		} else {
//			currentLength = remainLength;
//			remainLength = 0;
//		}
//
//		moveIn3D ('y', plywood.radAng, -currentLength, &plywood.posX, &plywood.posY, &plywood.posZ);
//		plywood.radAng += DegreeToRad (90.0);
//		elemList.Push (plywood.placeObject (7,
//			"p_stan", APIParT_CString, "��԰�",
//			"w_dir", APIParT_CString, "�ٴڱ��",
//			"p_thk", APIParT_CString, "11.5T",
//			"p_wid", APIParT_Length, format_string ("%f", (placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) - placingZone.leftMove),
//			"p_leng", APIParT_Length, format_string ("%f", currentLength),
//			"p_ang", APIParT_Angle, format_string ("%f", 0.0),
//			"sogak", APIParT_Boolean, "0.0"));
//		plywood.radAng -= DegreeToRad (90.0);
//	}
//
//	// ���� ��ġ (RIGHT)
//	plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoSlab.floorInd, axisPoint.x + (placingZone.outerRight - placingZone.outerLeft), axisPoint.y - ((placingZone.outerTop - placingZone.outerBottom) / 2 - (placingZone.formArrayHeight / 2) + placingZone.upMove), placingZone.level, placingZone.ang);
//
//	// ��ġ ���� ��ȸ�������� ��ȯ
//	rotatedPoint.x = plywood.posX;
//	rotatedPoint.y = plywood.posY;
//	rotatedPoint.z = plywood.posZ;
//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (plywood.radAng));
//	plywood.posX = unrotatedPoint.x;
//	plywood.posY = unrotatedPoint.y;
//	plywood.posZ = unrotatedPoint.z;
//
//	remainLength = placingZone.formArrayHeight;
//	while (remainLength > EPS) {
//		if (remainLength > (2.440 + EPS)) {
//			currentLength = 2.440;
//			remainLength -= 2.440;
//		} else {
//			currentLength = remainLength;
//			remainLength = 0;
//		}
//
//		moveIn3D ('y', plywood.radAng, -currentLength, &plywood.posX, &plywood.posY, &plywood.posZ);
//		plywood.radAng += DegreeToRad (90.0);
//		elemList.Push (plywood.placeObject (7,
//			"p_stan", APIParT_CString, "��԰�",
//			"w_dir", APIParT_CString, "�ٴڱ��",
//			"p_thk", APIParT_CString, "11.5T",
//			"p_wid", APIParT_Length, format_string ("%f", (placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) + placingZone.leftMove),
//			"p_leng", APIParT_Length, format_string ("%f", currentLength),
//			"p_ang", APIParT_Angle, format_string ("%f", 0.0),
//			"sogak", APIParT_Boolean, "0.0"));
//		plywood.radAng-= DegreeToRad (90.0);
//	}
//
//	// ���̺��� �ѷ� ���� ��ġ (TOP)
//	insCell.objType = WOOD;
//	insCell.ang = placingZone.ang;
//	insCell.leftBottomX = axisPoint.x + ((placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) - placingZone.leftMove) - 0.0545;
//	insCell.leftBottomY = axisPoint.y - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) + 0.0545 - placingZone.upMove;
//	insCell.leftBottomZ = placingZone.level - 0.0115;	// ���� �β���ŭ ������
//	insCell.libPart.wood.w_ang = 0.0;
//	insCell.libPart.wood.w_h = 0.050;	// �ʺ�
//	//insCell.libPart.wood.w_leng;
//	insCell.libPart.wood.w_w = 0.040;	// �β�
//	
//	// ��ġ ���� ��ȸ�������� ��ȯ
//	rotatedPoint.x = insCell.leftBottomX;
//	rotatedPoint.y = insCell.leftBottomY;
//	rotatedPoint.z = insCell.leftBottomZ;
//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
//	insCell.leftBottomX = unrotatedPoint.x;
//	insCell.leftBottomY = unrotatedPoint.y;
//	insCell.leftBottomZ = unrotatedPoint.z;
//
//	remainLength = placingZone.formArrayWidth + 0.0545 * 2;
//	while (remainLength > EPS) {
//		if (remainLength > (3.600 + EPS)) {
//			insCell.libPart.wood.w_leng = 3.600;
//			remainLength -= 3.600;
//		} else {
//			insCell.libPart.wood.w_leng = remainLength;
//			remainLength = 0;
//		}
//
//		elemList.Push (placingZone.placeLibPart (insCell));
//		moveIn3D ('x', insCell.ang, insCell.libPart.wood.w_leng, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
//	}
//
//	// ���̺��� �ѷ� ���� ��ġ (BOTTOM)
//	insCell.objType = WOOD;
//	insCell.ang = placingZone.ang;
//	insCell.leftBottomX = axisPoint.x + ((placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) - placingZone.leftMove) - 0.0545;
//	insCell.leftBottomY = axisPoint.y - (placingZone.outerTop - placingZone.outerBottom) + ((placingZone.outerTop - placingZone.outerBottom) / 2 - (placingZone.formArrayHeight / 2) - placingZone.upMove) - 0.005;
//	insCell.leftBottomZ = placingZone.level - 0.0115;	// ���� �β���ŭ ������
//	insCell.libPart.wood.w_ang = 0.0;
//	insCell.libPart.wood.w_h = 0.050;	// �ʺ�
//	//insCell.libPart.wood.w_leng;
//	insCell.libPart.wood.w_w = 0.040;	// �β�
//	
//	// ��ġ ���� ��ȸ�������� ��ȯ
//	rotatedPoint.x = insCell.leftBottomX;
//	rotatedPoint.y = insCell.leftBottomY;
//	rotatedPoint.z = insCell.leftBottomZ;
//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
//	insCell.leftBottomX = unrotatedPoint.x;
//	insCell.leftBottomY = unrotatedPoint.y;
//	insCell.leftBottomZ = unrotatedPoint.z;
//
//	remainLength = placingZone.formArrayWidth + 0.0545 * 2;
//	while (remainLength > EPS) {
//		if (remainLength > (3.600 + EPS)) {
//			insCell.libPart.wood.w_leng = 3.600;
//			remainLength -= 3.600;
//		} else {
//			insCell.libPart.wood.w_leng = remainLength;
//			remainLength = 0;
//		}
//
//		elemList.Push (placingZone.placeLibPart (insCell));
//		moveIn3D ('x', insCell.ang, insCell.libPart.wood.w_leng, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
//	}
//
//	// ���̺��� �ѷ� ���� ��ġ (LEFT)
//	insCell.objType = WOOD;
//	insCell.ang = placingZone.ang;
//	insCell.leftBottomX = axisPoint.x + ((placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) - placingZone.leftMove) - 0.0545;
//	insCell.leftBottomY = axisPoint.y - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) - placingZone.upMove + 0.005;
//	insCell.leftBottomZ = placingZone.level - 0.0115;	// ���� �β���ŭ ������
//	insCell.libPart.wood.w_ang = 0.0;
//	insCell.libPart.wood.w_h = 0.050;	// �ʺ�
//	//insCell.libPart.wood.w_leng;
//	insCell.libPart.wood.w_w = 0.040;	// �β�
//	
//	// ��ġ ���� ��ȸ�������� ��ȯ
//	rotatedPoint.x = insCell.leftBottomX;
//	rotatedPoint.y = insCell.leftBottomY;
//	rotatedPoint.z = insCell.leftBottomZ;
//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
//	insCell.leftBottomX = unrotatedPoint.x;
//	insCell.leftBottomY = unrotatedPoint.y;
//	insCell.leftBottomZ = unrotatedPoint.z;
//
//	remainLength = placingZone.formArrayHeight + 0.005 * 2;
//	while (remainLength > EPS) {
//		if (remainLength > (3.600 + EPS)) {
//			insCell.libPart.wood.w_leng = 3.600;
//			remainLength -= 3.600;
//		} else {
//			insCell.libPart.wood.w_leng = remainLength;
//			remainLength = 0;
//		}
//
//		moveIn3D ('y', insCell.ang, -insCell.libPart.wood.w_leng, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
//		insCell.ang += DegreeToRad (90.0);
//		elemList.Push (placingZone.placeLibPart (insCell));
//		insCell.ang -= DegreeToRad (90.0);
//	}
//
//	// ���̺��� �ѷ� ���� ��ġ (RIGHT)
//	insCell.objType = WOOD;
//	insCell.ang = placingZone.ang;
//	insCell.leftBottomX = axisPoint.x + (placingZone.outerRight - placingZone.outerLeft) - ((placingZone.outerRight - placingZone.outerLeft) / 2 - (placingZone.formArrayWidth / 2) + placingZone.leftMove) + 0.005;
//	insCell.leftBottomY = axisPoint.y - (placingZone.outerTop - placingZone.outerBottom) / 2 + (placingZone.formArrayHeight / 2) - placingZone.upMove + 0.005;
//	insCell.leftBottomZ = placingZone.level - 0.0115;	// ���� �β���ŭ ������
//	insCell.libPart.wood.w_ang = 0.0;
//	insCell.libPart.wood.w_h = 0.050;	// �ʺ�
//	//insCell.libPart.wood.w_leng;
//	insCell.libPart.wood.w_w = 0.040;	// �β�
//	
//	// ��ġ ���� ��ȸ�������� ��ȯ
//	rotatedPoint.x = insCell.leftBottomX;
//	rotatedPoint.y = insCell.leftBottomY;
//	rotatedPoint.z = insCell.leftBottomZ;
//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (insCell.ang));
//	insCell.leftBottomX = unrotatedPoint.x;
//	insCell.leftBottomY = unrotatedPoint.y;
//	insCell.leftBottomZ = unrotatedPoint.z;
//
//	remainLength = placingZone.formArrayHeight + 0.005 * 2;
//	while (remainLength > EPS) {
//		if (remainLength > (3.600 + EPS)) {
//			insCell.libPart.wood.w_leng = 3.600;
//			remainLength -= 3.600;
//		} else {
//			insCell.libPart.wood.w_leng = remainLength;
//			remainLength = 0;
//		}
//
//		moveIn3D ('y', insCell.ang, -insCell.libPart.wood.w_leng, &insCell.leftBottomX, &insCell.leftBottomY, &insCell.leftBottomZ);
//		insCell.ang += DegreeToRad (90.0);
//		elemList.Push (placingZone.placeLibPart (insCell));
//		insCell.ang -= DegreeToRad (90.0);
//	}

	return err;
}

// 1�� ��ġ�� ���� ���Ǹ� ��û�ϴ� 1�� ���̾�α�
short DGCALLBACK slabBottomTableformPlacerHandler1 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short		xx;
	short		result;
	API_UCCallbackType	ucb;

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, "������ �Ϻο� ��ġ");

			//////////////////////////////////////////////////////////// ������ ��ġ
			DGSetItemText (dialogID, DG_OK, "Ȯ ��");
			DGSetItemText (dialogID, DG_CANCEL, "�� ��");

			DGSetItemText (dialogID, LABEL_SELECT_TYPE, "Ÿ�� ����");

			DGSetItemText (dialogID, PUSHRADIO_EUROFORM, "������");
			DGSetItemText (dialogID, PUSHRADIO_TABLEFORM, "���ǳ�");
			DGSetItemText (dialogID, PUSHRADIO_PLYWOOD, "����");

			DGSetItemText (dialogID, LABEL_CELL_SETTINGS, "�� ����");
			DGSetItemText (dialogID, LABEL_TABLEFORM_WIDTH, "�ʺ�");
			DGSetItemText (dialogID, LABEL_TABLEFORM_HEIGHT, "����");
			DGSetItemText (dialogID, LABEL_TABLEFORM_ORIENTATION, "��ġ����");
			DGSetItemText (dialogID, LABEL_GAP_LENGTH, "��������� ����");

			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, "���纰 ���̾� ����");
			DGSetItemText (dialogID, CHECKBOX_LAYER_COUPLING, "���̾� ����");
			DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, TRUE);

			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM, "������");
			DGSetItemText (dialogID, LABEL_LAYER_SLABTABLEFORM, "���ǳ�");
			DGSetItemText (dialogID, LABEL_LAYER_PLYWOOD, "����");
			DGSetItemText (dialogID, LABEL_LAYER_TIMBER, "����");
			DGSetItemText (dialogID, LABEL_LAYER_CPROFILE, "C����");
			DGSetItemText (dialogID, LABEL_LAYER_PINBOLT, "�ɺ�Ʈ");
			DGSetItemText (dialogID, LABEL_LAYER_FITTINGS, "����ö��");
			DGSetItemText (dialogID, LABEL_LAYER_GT24_GIRDER, "GT24 �Ŵ�");
			DGSetItemText (dialogID, LABEL_LAYER_PERI_SUPPORT, "PERI���ٸ�");
			DGSetItemText (dialogID, LABEL_LAYER_STEEL_SUPPORT, "���� ���ٸ�");

			DGSetItemText (dialogID, BUTTON_AUTOSET, "���̾� �ڵ� ����");

			// ���� ��Ʈ�� �ʱ�ȭ
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_LAYER_EUROFORM;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, 1);

			ucb.itemID	 = USERCONTROL_LAYER_SLABTABLEFORM;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, 1);

			ucb.itemID	 = USERCONTROL_LAYER_PLYWOOD;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, 1);

			ucb.itemID	 = USERCONTROL_LAYER_TIMBER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, 1);

			ucb.itemID	 = USERCONTROL_LAYER_CPROFILE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_CPROFILE, 1);

			ucb.itemID	 = USERCONTROL_LAYER_PINBOLT;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, 1);

			ucb.itemID	 = USERCONTROL_LAYER_FITTINGS;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_FITTINGS, 1);

			ucb.itemID	 = USERCONTROL_LAYER_GT24_GIRDER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_GT24_GIRDER, 1);

			ucb.itemID	 = USERCONTROL_LAYER_PERI_SUPPORT;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PERI_SUPPORT, 1);

			ucb.itemID	 = USERCONTROL_LAYER_STEEL_SUPPORT;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEEL_SUPPORT, 1);

			// ���� �߰�
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_ORIENTATION, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_ORIENTATION, DG_POPUP_BOTTOM, "���ι���");	// �ʱⰪ
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_ORIENTATION, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_ORIENTATION, DG_POPUP_BOTTOM, "���ι���");

			// 1��° Ÿ�� ����
			DGSetItemValLong (dialogID, PUSHRADIO_EUROFORM, TRUE);		// �ʱⰪ: ������

			// �ʺ� �߰�
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM, "600");
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM, "500");
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM, "450");
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM, "400");
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM, "300");
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM, "200");

			// ���� �߰�
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "1200");
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "900");
			DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "600");

			// ���̾� Ȱ��ȭ/��Ȱ��ȭ
			DGDisableItem (dialogID, LABEL_LAYER_SLABTABLEFORM);
			DGDisableItem (dialogID, LABEL_LAYER_FITTINGS);
			DGDisableItem (dialogID, LABEL_LAYER_GT24_GIRDER);
			DGDisableItem (dialogID, LABEL_LAYER_PERI_SUPPORT);
			DGDisableItem (dialogID, LABEL_LAYER_STEEL_SUPPORT);

			DGDisableItem (dialogID, USERCONTROL_LAYER_SLABTABLEFORM);
			DGDisableItem (dialogID, USERCONTROL_LAYER_FITTINGS);
			DGDisableItem (dialogID, USERCONTROL_LAYER_GT24_GIRDER);
			DGDisableItem (dialogID, USERCONTROL_LAYER_PERI_SUPPORT);
			DGDisableItem (dialogID, USERCONTROL_LAYER_STEEL_SUPPORT);

			break;

		case DG_MSG_CHANGE:
			// ���̾� ���� �ٲ�
			if ((item >= USERCONTROL_LAYER_EUROFORM) && (item <= USERCONTROL_LAYER_STEEL_SUPPORT)) {
				if (DGGetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING) == 1) {
					long selectedLayer;

					selectedLayer = DGGetItemValLong (dialogID, item);

					for (xx = USERCONTROL_LAYER_EUROFORM ; xx <= USERCONTROL_LAYER_STEEL_SUPPORT ; ++xx)
						DGSetItemValLong (dialogID, xx, selectedLayer);
				}
			}

			// �ʺ�, ���� �˾���Ʈ�� �׸�
			if ((item == PUSHRADIO_EUROFORM) || (item == PUSHRADIO_TABLEFORM) || (item == PUSHRADIO_PLYWOOD)) {
				// �˾� ������ ��� �����,
				while (DGPopUpGetItemCount (dialogID, POPUP_TABLEFORM_WIDTH) > 0)
					DGPopUpDeleteItem (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM);
				while (DGPopUpGetItemCount (dialogID, POPUP_TABLEFORM_HEIGHT) > 0)
					DGPopUpDeleteItem (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);

				// ������(1) ���ý�
				if (DGGetItemValLong (dialogID, PUSHRADIO_EUROFORM) == TRUE) {
					DGSetItemText (dialogID, LABEL_CELL_SETTINGS, "�� ����");

					// �ʺ� �߰�
					DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM, "600");
					DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM, "500");
					DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM, "450");
					DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM, "400");
					DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM, "300");
					DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM, "200");

					// ���� �߰�
					DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "1200");
					DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "900");
					DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "600");
				}
				// ���ǳ�(2) ���ý�
				if (DGGetItemValLong (dialogID, PUSHRADIO_TABLEFORM) == TRUE) {
					DGSetItemText (dialogID, LABEL_CELL_SETTINGS, "�� ���� (1818 x 3032, 1818 x 2426,\n1212 x 1820 �� ��ȿ��)");

					// �ʺ� �߰�
					DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM, "1818");
					DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM, "1212");

					// ���� �߰�
					DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "3032");
					DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "2426");
					DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "1820");
				}
				// ����(3) ���ý�
				if (DGGetItemValLong (dialogID, PUSHRADIO_PLYWOOD) == TRUE) {
					DGSetItemText (dialogID, LABEL_CELL_SETTINGS, "�� ����");

					// �ʺ� �߰�
					DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_WIDTH, DG_POPUP_BOTTOM, "1220");

					// ���� �߰�
					DGPopUpInsertItem (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_TABLEFORM_HEIGHT, DG_POPUP_BOTTOM, "2440");
				}
			}

			// ���̾� Ȱ��ȭ/��Ȱ��ȭ
			for (xx = LABEL_LAYER_EUROFORM ; xx <= LABEL_LAYER_STEEL_SUPPORT ; ++xx)
				DGEnableItem (dialogID, xx);
			for (xx = USERCONTROL_LAYER_EUROFORM ; xx <= USERCONTROL_LAYER_STEEL_SUPPORT ; ++xx)
				DGEnableItem (dialogID, xx);

			if (DGGetItemValLong (dialogID, PUSHRADIO_EUROFORM) == TRUE) {
				DGDisableItem (dialogID, LABEL_LAYER_SLABTABLEFORM);
				DGDisableItem (dialogID, LABEL_LAYER_FITTINGS);
				DGDisableItem (dialogID, LABEL_LAYER_GT24_GIRDER);
				DGDisableItem (dialogID, LABEL_LAYER_PERI_SUPPORT);
				DGDisableItem (dialogID, LABEL_LAYER_STEEL_SUPPORT);

				DGDisableItem (dialogID, USERCONTROL_LAYER_SLABTABLEFORM);
				DGDisableItem (dialogID, USERCONTROL_LAYER_FITTINGS);
				DGDisableItem (dialogID, USERCONTROL_LAYER_GT24_GIRDER);
				DGDisableItem (dialogID, USERCONTROL_LAYER_PERI_SUPPORT);
				DGDisableItem (dialogID, USERCONTROL_LAYER_STEEL_SUPPORT);
			}
			if (DGGetItemValLong (dialogID, PUSHRADIO_TABLEFORM) == TRUE) {
				DGDisableItem (dialogID, LABEL_LAYER_EUROFORM);
				DGDisableItem (dialogID, LABEL_LAYER_PINBOLT);
				DGDisableItem (dialogID, LABEL_LAYER_GT24_GIRDER);
				DGDisableItem (dialogID, LABEL_LAYER_STEEL_SUPPORT);

				DGDisableItem (dialogID, USERCONTROL_LAYER_EUROFORM);
				DGDisableItem (dialogID, USERCONTROL_LAYER_PINBOLT);
				DGDisableItem (dialogID, USERCONTROL_LAYER_GT24_GIRDER);
				DGDisableItem (dialogID, USERCONTROL_LAYER_STEEL_SUPPORT);
			}
			// ����(3) ���ý�
			if (DGGetItemValLong (dialogID, PUSHRADIO_PLYWOOD) == TRUE) {
				DGDisableItem (dialogID, LABEL_LAYER_EUROFORM);
				DGDisableItem (dialogID, LABEL_LAYER_SLABTABLEFORM);
				DGDisableItem (dialogID, LABEL_LAYER_CPROFILE);
				DGDisableItem (dialogID, LABEL_LAYER_PINBOLT);
				DGDisableItem (dialogID, LABEL_LAYER_FITTINGS);

				DGDisableItem (dialogID, USERCONTROL_LAYER_EUROFORM);
				DGDisableItem (dialogID, USERCONTROL_LAYER_SLABTABLEFORM);
				DGDisableItem (dialogID, USERCONTROL_LAYER_CPROFILE);
				DGDisableItem (dialogID, USERCONTROL_LAYER_PINBOLT);
				DGDisableItem (dialogID, USERCONTROL_LAYER_FITTINGS);
			}

			// ���ǳ��� �� ��ȿ�� �ʺ�, ���̰� �ƴϸ� ���� �ܰ�� ������ �� ����
			if (DGGetItemValLong (dialogID, PUSHRADIO_TABLEFORM) == TRUE) {
				// ��ȿ�� �ʺ�, ���� ������ ������ ����: 1818 x 3032, 1818 x 2426, 1212 x 1820
				int width = atoi (DGPopUpGetItemText (dialogID, POPUP_TABLEFORM_WIDTH, static_cast<short>(DGGetItemValLong (dialogID, POPUP_TABLEFORM_WIDTH))).ToCStr ().Get ());
				int height = atoi (DGPopUpGetItemText (dialogID, POPUP_TABLEFORM_HEIGHT, static_cast<short>(DGGetItemValLong (dialogID, POPUP_TABLEFORM_HEIGHT))).ToCStr ().Get ());

				if ( ((width == 1818) && (height == 3032)) || ((width == 1818) && (height == 2426)) || ((width == 1212) && (height == 1820)) )
					DGEnableItem (dialogID, DG_OK);
				else
					DGDisableItem (dialogID, DG_OK);
			} else {
				DGEnableItem (dialogID, DG_OK);
			}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					//////////////////////////////////////// ���̾�α� â ������ �Է� ����
					// �� �ʺ�, ����, ���Ⱚ ����
					placingZone.initHorLen = atof (DGPopUpGetItemText (dialogID, POPUP_TABLEFORM_WIDTH, static_cast<short>(DGGetItemValLong (dialogID, POPUP_TABLEFORM_WIDTH))).ToCStr ().Get ()) / 1000.0;
					placingZone.initVerLen = atof (DGPopUpGetItemText (dialogID, POPUP_TABLEFORM_HEIGHT, static_cast<short>(DGGetItemValLong (dialogID, POPUP_TABLEFORM_HEIGHT))).ToCStr ().Get ()) / 1000.0;
					if (my_strcmp (DGPopUpGetItemText (dialogID, POPUP_TABLEFORM_ORIENTATION, static_cast<short>(DGGetItemValLong (dialogID, POPUP_TABLEFORM_ORIENTATION))).ToCStr ().Get (), "���ι���") == 0)
						placingZone.bVertical = true;
					else
						placingZone.bVertical = false;

					// Ÿ�� ����
					if (DGGetItemValLong (dialogID, PUSHRADIO_EUROFORM) == TRUE)
						placingZone.iTableformType = EUROFORM;
					else if (DGGetItemValLong (dialogID, PUSHRADIO_TABLEFORM) == TRUE)
						placingZone.iTableformType = TABLEFORM;
					else if (DGGetItemValLong (dialogID, PUSHRADIO_PLYWOOD) == TRUE)
						placingZone.iTableformType = PLYWOOD;
					else
						placingZone.iTableformType = NONE;

					// ��������� ���ݰ� ����
					placingZone.gap = DGGetItemValDouble (dialogID, EDITCONTROL_GAP_LENGTH);

					// ���̾� ��ȣ ����
					layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM);
					layerInd_SlabTableform	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM);
					layerInd_Plywood		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD);
					layerInd_Timber			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER);
					layerInd_CProfile		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_CPROFILE);
					layerInd_Pinbolt		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT);
					layerInd_Fittings		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_FITTINGS);
					layerInd_GT24Girder		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_GT24_GIRDER);
					layerInd_PERI_Support	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PERI_SUPPORT);
					layerInd_Steel_Support	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEEL_SUPPORT);

					break;

				case BUTTON_AUTOSET:
					item = 0;

					DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, FALSE);

					layerInd_Euroform		= makeTemporaryLayer (structuralObject_forTableformSlab, "UFOM", NULL);
					layerInd_SlabTableform	= makeTemporaryLayer (structuralObject_forTableformSlab, "CONP", NULL);
					layerInd_Plywood		= makeTemporaryLayer (structuralObject_forTableformSlab, "PLYW", NULL);
					layerInd_Timber			= makeTemporaryLayer (structuralObject_forTableformSlab, "TIMB", NULL);
					layerInd_CProfile		= makeTemporaryLayer (structuralObject_forTableformSlab, "CPRO", NULL);
					layerInd_Pinbolt		= makeTemporaryLayer (structuralObject_forTableformSlab, "PINB", NULL);
					layerInd_Fittings		= makeTemporaryLayer (structuralObject_forTableformSlab, "CLAM", NULL);
					layerInd_GT24Girder		= makeTemporaryLayer (structuralObject_forTableformSlab, "GIDR", NULL);
					layerInd_PERI_Support	= makeTemporaryLayer (structuralObject_forTableformSlab, "MULT", NULL);
					layerInd_Steel_Support	= makeTemporaryLayer (structuralObject_forTableformSlab, "SUPT", NULL);

					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, layerInd_Euroform);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, layerInd_SlabTableform);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, layerInd_Plywood);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, layerInd_Timber);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_CPROFILE, layerInd_CProfile);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, layerInd_Pinbolt);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_FITTINGS, layerInd_Fittings);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_GT24_GIRDER, layerInd_GT24Girder);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PERI_SUPPORT, layerInd_PERI_Support);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEEL_SUPPORT, layerInd_Steel_Support);

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
short DGCALLBACK slabBottomTableformPlacerHandler2 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	xx, yy;
	short	btnSizeX = 50, btnSizeY = 50;
	short	dialogSizeX, dialogSizeY;
	short	btnInitPosX;
	short	btnPosX, btnPosY;
	short	itmIdx;
	std::string		txtButton = "";

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, "������ �Ϻο� ��ġ - ��ġ ����");

			//////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			// ��ġ ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 110, 130, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "��ġ - ������ ä���");
			DGShowItem (dialogID, DG_OK);

			// ���� ��ư 1
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 150, 130, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "��ġ - ������ ����");
			DGShowItem (dialogID, DG_CANCEL);

			// ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 190, 130, 25);
			DGSetItemFont (dialogID, DG_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_PREV, "����");
			DGShowItem (dialogID, DG_PREV);

			// �� �߰� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 30, 65, 25);
			DGSetItemFont (dialogID, PUSHBUTTON_ADD_ROW, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, PUSHBUTTON_ADD_ROW, "�� �߰�");
			DGShowItem (dialogID, PUSHBUTTON_ADD_ROW);

			// �� ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 85, 30, 65, 25);
			DGSetItemFont (dialogID, PUSHBUTTON_DEL_ROW, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, PUSHBUTTON_DEL_ROW, "�� ����");
			DGShowItem (dialogID, PUSHBUTTON_DEL_ROW);

			// �� �߰� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 70, 65, 25);
			DGSetItemFont (dialogID, PUSHBUTTON_ADD_COL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, PUSHBUTTON_ADD_COL, "�� �߰�");
			DGShowItem (dialogID, PUSHBUTTON_ADD_COL);
		
			// �� ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 85, 70, 65, 25);
			DGSetItemFont (dialogID, PUSHBUTTON_DEL_COL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, PUSHBUTTON_DEL_COL, "�� ����");
			DGShowItem (dialogID, PUSHBUTTON_DEL_COL);

			// ���� â ũ�⸦ ����
			dialogSizeX = 450 + (btnSizeX * (placingZone.nHorCells-1));
			dialogSizeY = 300 + (btnSizeY * (placingZone.nVerCells-1));
			DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);

			// �׸��� ����ü�� ���� ��ư�� �������� ��ġ��
			btnInitPosX = btnPosX = 220 + 25;
			btnPosY = (btnSizeY * placingZone.nVerCells) + 25;
			for (xx = 0 ; xx < placingZone.nVerCells ; ++xx) {
				for (yy = 0 ; yy < placingZone.nHorCells ; ++yy) {
					placingZone.CELL_BUTTON [xx][yy] = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
					DGSetItemFont (dialogID, placingZone.CELL_BUTTON [xx][yy], DG_IS_SMALL);

					// Ÿ�Կ� ���� �� ������ �޶���
					if (placingZone.iTableformType == EUROFORM) {
						if (placingZone.bVertical == true) {
							txtButton = format_string ("������\n(����)\n��%.0f\n��%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
						} else {
							txtButton = format_string ("������\n(����)\n��%.0f\n��%.0f", placingZone.cells [xx][yy].verLen * 1000, placingZone.cells [xx][yy].horLen * 1000);
						}
					} else if (placingZone.iTableformType == TABLEFORM) {
						if (placingZone.bVertical == true) {
							txtButton = format_string ("���ǳ�\n(����)\n��%.0f\n��%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
						} else {
							txtButton = format_string ("���ǳ�\n(����)\n��%.0f\n��%.0f", placingZone.cells [xx][yy].verLen * 1000, placingZone.cells [xx][yy].horLen * 1000);
						}
					} else if (placingZone.iTableformType == PLYWOOD) {
						if (placingZone.bVertical == true) {
							txtButton = format_string ("����\n(����)\n��%.0f\n��%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
						} else {
							txtButton = format_string ("����\n(����)\n��%.0f\n��%.0f", placingZone.cells [xx][yy].verLen * 1000, placingZone.cells [xx][yy].horLen * 1000);
						}
					} else {
						txtButton = format_string ("����");
					}
					DGSetItemText (dialogID, placingZone.CELL_BUTTON [xx][yy], txtButton.c_str ());		// �׸��� ��ư �ؽ�Ʈ ����
					DGShowItem (dialogID, placingZone.CELL_BUTTON [xx][yy]);
					btnPosX += btnSizeX;
				}
				btnPosX = btnInitPosX;
				btnPosY -= btnSizeY;
			}

			// ��: ����(��)
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnInitPosX - 60, btnPosY + (placingZone.nVerCells * btnSizeY)/2, 50, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "����(��)");
			DGShowItem (dialogID, itmIdx);

			// ��: ����(��)
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnInitPosX + (placingZone.nHorCells * btnSizeX) + 20, btnPosY + (placingZone.nVerCells * btnSizeY)/2, 50, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "����(��)");
			DGShowItem (dialogID, itmIdx);

			// ��: ����(��)
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnInitPosX + (placingZone.nHorCells * btnSizeX)/2 - 100, 30, 50, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "����(��)");
			DGShowItem (dialogID, itmIdx);

			// ��: ����(��)
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnInitPosX + (placingZone.nHorCells * btnSizeX)/2 - 100, btnPosY + ((placingZone.nVerCells + 2) * btnSizeY), 50, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "����(��)");
			DGShowItem (dialogID, itmIdx);

			// Edit ��Ʈ��: ����(��)
			placingZone.EDITCONTROL_MARGIN_LEFT = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnInitPosX - 60, btnPosY + (placingZone.nVerCells * btnSizeY)/2 + 30 - 7, 50, 25);
			DGSetItemFont (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT);
			DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT, placingZone.borderHorLen - placingZone.formArrayWidth - 0.001);
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT, placingZone.marginLeft);

			// Edit ��Ʈ��: ����(��)
			placingZone.EDITCONTROL_MARGIN_RIGHT = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnInitPosX + (placingZone.nHorCells * btnSizeX) + 20, btnPosY + (placingZone.nVerCells * btnSizeY)/2 + 30 - 7, 50, 25);
			DGSetItemFont (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT);
			DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT, placingZone.borderHorLen - placingZone.formArrayWidth - 0.001);
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT, placingZone.marginRight);

			// Edit ��Ʈ��: ����(��)
			placingZone.EDITCONTROL_MARGIN_TOP = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnInitPosX + (placingZone.nHorCells * btnSizeX)/2 - 100 + 60, 30 - 7, 50, 25);
			DGSetItemFont (dialogID, placingZone.EDITCONTROL_MARGIN_TOP, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_TOP);
			DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_TOP, placingZone.borderVerLen - placingZone.formArrayHeight - 0.001);
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_TOP, placingZone.marginTop);

			// Edit ��Ʈ��: ����(��)
			placingZone.EDITCONTROL_MARGIN_BOTTOM = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnInitPosX + (placingZone.nHorCells * btnSizeX)/2 - 100 + 60, btnPosY + ((placingZone.nVerCells + 2) * btnSizeY) - 7, 50, 25);
			DGSetItemFont (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM);
			DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM, placingZone.borderVerLen - placingZone.formArrayHeight - 0.001);
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM, placingZone.marginBottom);

			break;

		case DG_MSG_CHANGE:
			// ����(��) �����, ����(��) �����
			if (item == placingZone.EDITCONTROL_MARGIN_LEFT) {
				placingZone.marginLeft = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT);
				placingZone.marginRight = placingZone.borderHorLen - placingZone.formArrayWidth - placingZone.marginLeft;
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT, placingZone.marginRight);
			}

			// ����(��) �����, ����(��) �����
			if (item == placingZone.EDITCONTROL_MARGIN_RIGHT) {
				placingZone.marginRight = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT);
				placingZone.marginLeft = placingZone.borderHorLen - placingZone.formArrayWidth - placingZone.marginRight;
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT, placingZone.marginLeft);
			}

			// ����(��) �����, ����(��) �����
			if (item == placingZone.EDITCONTROL_MARGIN_TOP) {
				placingZone.marginTop = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_TOP);
				placingZone.marginBottom = placingZone.borderVerLen - placingZone.formArrayHeight - placingZone.marginTop;
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM, placingZone.marginBottom);
			}

			// ����(��) �����, ����(��) �����
			if (item == placingZone.EDITCONTROL_MARGIN_BOTTOM) {
				placingZone.marginBottom = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM);
				placingZone.marginTop = placingZone.borderVerLen - placingZone.formArrayHeight - placingZone.marginBottom;
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_TOP, placingZone.marginTop);
			}

			break;

		case DG_MSG_CLICK:
			if (item == DG_OK) {
				// ��ġ - ������ ä��� ��ư
				clickedExcludeRestButton = false;

				// ���̺��� �迭 ��ü �ʺ�, ���� ����
				placingZone.formArrayWidth = 0.0;
				placingZone.formArrayHeight = 0.0;
				if (placingZone.bVertical == true) {
					for (xx = 0 ; xx < placingZone.nHorCells ; ++xx)	placingZone.formArrayWidth += placingZone.cells [0][xx].horLen;
					for (xx = 0 ; xx < placingZone.nVerCells ; ++xx)	placingZone.formArrayHeight += placingZone.cells [xx][0].verLen;
				} else {
					for (xx = 0 ; xx < placingZone.nHorCells ; ++xx)	placingZone.formArrayWidth += placingZone.cells [0][xx].verLen;
					for (xx = 0 ; xx < placingZone.nVerCells ; ++xx)	placingZone.formArrayHeight += placingZone.cells [xx][0].horLen;
				}

				// ���� ���� ����
				placingZone.marginLeft = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT);
				placingZone.marginRight = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT);
				placingZone.marginTop = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_TOP);
				placingZone.marginBottom = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM);

				// �� ��ġ ������
				placingZone.alignPlacingZone (&placingZone);

			} else if (item == DG_CANCEL) {
				// ��ġ - ������ ���� ��ư
				clickedExcludeRestButton = true;

				// ���̺��� �迭 ��ü �ʺ�, ���� ����
				placingZone.formArrayWidth = 0.0;
				placingZone.formArrayHeight = 0.0;
				if (placingZone.bVertical == true) {
					for (xx = 0 ; xx < placingZone.nHorCells ; ++xx)	placingZone.formArrayWidth += placingZone.cells [0][xx].horLen;
					for (xx = 0 ; xx < placingZone.nVerCells ; ++xx)	placingZone.formArrayHeight += placingZone.cells [xx][0].verLen;
				} else {
					for (xx = 0 ; xx < placingZone.nHorCells ; ++xx)	placingZone.formArrayWidth += placingZone.cells [0][xx].verLen;
					for (xx = 0 ; xx < placingZone.nVerCells ; ++xx)	placingZone.formArrayHeight += placingZone.cells [xx][0].horLen;
				}

				// ���� ���� ����
				placingZone.marginLeft = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT);
				placingZone.marginRight = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT);
				placingZone.marginTop = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_TOP);
				placingZone.marginBottom = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM);

				// �� ��ġ ������
				placingZone.alignPlacingZone (&placingZone);

			} else if (item == DG_PREV) {
				// ���� ��ư
				clickedPrevButton = true;

			} else {
				if (item == PUSHBUTTON_ADD_ROW)
					++ placingZone.nVerCells;	// �� �߰� ��ư

				if (item == PUSHBUTTON_DEL_ROW)
					-- placingZone.nVerCells;	// �� ���� ��ư

				if (item == PUSHBUTTON_ADD_COL)
					++ placingZone.nHorCells;	// �� �߰� ��ư

				if (item == PUSHBUTTON_DEL_COL)
					-- placingZone.nHorCells;	// �� ���� ��ư

				// �׸��� ��ư Ŭ����
				for (xx = 0 ; xx < placingZone.nVerCells ; ++xx) {
					for (yy = 0 ; yy < placingZone.nHorCells ; ++yy) {
						if (item == placingZone.CELL_BUTTON [xx][yy]) {
							// Ŭ���� �ε��� ���� ������
							clickedRow = xx;
							clickedCol = yy;

							// 3��° ���̾�α� ���� (�� ���� ����)
							result = DGBlankModalDialog (250, 270, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, slabBottomTableformPlacerHandler3, 0);
						}
					}
				}

				// ���� ���ɼ��� �ִ� DG �׸� ��� ����
				DGRemoveDialogItems (dialogID, AFTER_ALL);

				// ���̺��� �迭 ��ü �ʺ�, ���� ����
				placingZone.formArrayWidth = 0.0;
				placingZone.formArrayHeight = 0.0;
				if (placingZone.bVertical == true) {
					for (xx = 0 ; xx < placingZone.nHorCells ; ++xx)	placingZone.formArrayWidth += placingZone.cells [0][xx].horLen;
					for (xx = 0 ; xx < placingZone.nVerCells ; ++xx)	placingZone.formArrayHeight += placingZone.cells [xx][0].verLen;
				} else {
					for (xx = 0 ; xx < placingZone.nHorCells ; ++xx)	placingZone.formArrayWidth += placingZone.cells [0][xx].verLen;
					for (xx = 0 ; xx < placingZone.nVerCells ; ++xx)	placingZone.formArrayHeight += placingZone.cells [xx][0].horLen;
				}

				// ���� ���� ����
				placingZone.marginLeft = placingZone.marginRight = (placingZone.borderHorLen - placingZone.formArrayWidth) / 2;
				placingZone.marginBottom = placingZone.marginTop = (placingZone.borderVerLen - placingZone.formArrayHeight) / 2;

				// ���� â ũ�⸦ ����
				dialogSizeX = 450 + (btnSizeX * (placingZone.nHorCells-1));
				dialogSizeY = 300 + (btnSizeY * (placingZone.nVerCells-1));
				DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);

				// �׸��� ����ü�� ���� ��ư�� �������� ��ġ��
				btnInitPosX = btnPosX = 220 + 25;
				btnPosY = (btnSizeY * placingZone.nVerCells) + 25;
				for (xx = 0 ; xx < placingZone.nVerCells ; ++xx) {
					for (yy = 0 ; yy < placingZone.nHorCells ; ++yy) {
						placingZone.CELL_BUTTON [xx][yy] = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
						DGSetItemFont (dialogID, placingZone.CELL_BUTTON [xx][yy], DG_IS_SMALL);

						// Ÿ�Կ� ���� �� ������ �޶���
						if (placingZone.iTableformType == EUROFORM) {
							if (placingZone.bVertical == true) {
								txtButton = format_string ("������\n(����)\n��%.0f\n��%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
							} else {
								txtButton = format_string ("������\n(����)\n��%.0f\n��%.0f", placingZone.cells [xx][yy].verLen * 1000, placingZone.cells [xx][yy].horLen * 1000);
							}
						} else if (placingZone.iTableformType == TABLEFORM) {
							if (placingZone.bVertical == true) {
								txtButton = format_string ("���ǳ�\n(����)\n��%.0f\n��%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
							} else {
								txtButton = format_string ("���ǳ�\n(����)\n��%.0f\n��%.0f", placingZone.cells [xx][yy].verLen * 1000, placingZone.cells [xx][yy].horLen * 1000);
							}
						} else if (placingZone.iTableformType == PLYWOOD) {
							if (placingZone.bVertical == true) {
								txtButton = format_string ("����\n(����)\n��%.0f\n��%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
							} else {
								txtButton = format_string ("����\n(����)\n��%.0f\n��%.0f", placingZone.cells [xx][yy].verLen * 1000, placingZone.cells [xx][yy].horLen * 1000);
							}
						} else {
							txtButton = format_string ("����");
						}
						DGSetItemText (dialogID, placingZone.CELL_BUTTON [xx][yy], txtButton.c_str ());		// �׸��� ��ư �ؽ�Ʈ ����
						DGShowItem (dialogID, placingZone.CELL_BUTTON [xx][yy]);
						btnPosX += btnSizeX;
					}
					btnPosX = btnInitPosX;
					btnPosY -= btnSizeY;
				}

				// ��: ����(��)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnInitPosX - 60, btnPosY + (placingZone.nVerCells * btnSizeY)/2, 50, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����(��)");
				DGShowItem (dialogID, itmIdx);

				// ��: ����(��)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnInitPosX + (placingZone.nHorCells * btnSizeX) + 20, btnPosY + (placingZone.nVerCells * btnSizeY)/2, 50, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����(��)");
				DGShowItem (dialogID, itmIdx);

				// ��: ����(��)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnInitPosX + (placingZone.nHorCells * btnSizeX)/2 - 100, 30, 50, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����(��)");
				DGShowItem (dialogID, itmIdx);

				// ��: ����(��)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnInitPosX + (placingZone.nHorCells * btnSizeX)/2 - 100, btnPosY + ((placingZone.nVerCells + 2) * btnSizeY), 50, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����(��)");
				DGShowItem (dialogID, itmIdx);

				// Edit ��Ʈ��: ����(��)
				placingZone.EDITCONTROL_MARGIN_LEFT = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnInitPosX - 60, btnPosY + (placingZone.nVerCells * btnSizeY)/2 + 30 - 7, 50, 25);
				DGSetItemFont (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT);
				DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT, placingZone.borderHorLen - placingZone.formArrayWidth - 0.001);
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT, placingZone.marginLeft);

				// Edit ��Ʈ��: ����(��)
				placingZone.EDITCONTROL_MARGIN_RIGHT = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnInitPosX + (placingZone.nHorCells * btnSizeX) + 20, btnPosY + (placingZone.nVerCells * btnSizeY)/2 + 30 - 7, 50, 25);
				DGSetItemFont (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT);
				DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT, placingZone.borderHorLen - placingZone.formArrayWidth - 0.001);
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT, placingZone.marginRight);

				// Edit ��Ʈ��: ����(��)
				placingZone.EDITCONTROL_MARGIN_TOP = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnInitPosX + (placingZone.nHorCells * btnSizeX)/2 - 100 + 60, 30 - 7, 50, 25);
				DGSetItemFont (dialogID, placingZone.EDITCONTROL_MARGIN_TOP, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_TOP);
				DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_TOP, placingZone.borderVerLen - placingZone.formArrayHeight - 0.001);
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_TOP, placingZone.marginTop);

				// Edit ��Ʈ��: ����(��)
				placingZone.EDITCONTROL_MARGIN_BOTTOM = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnInitPosX + (placingZone.nHorCells * btnSizeX)/2 - 100 + 60, btnPosY + ((placingZone.nVerCells + 2) * btnSizeY) - 7, 50, 25);
				DGSetItemFont (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM);
				DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM, placingZone.borderVerLen - placingZone.formArrayHeight - 0.001);
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM, placingZone.marginBottom);

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

// 2�� ���̾�α׿��� �� ���� ��ü Ÿ���� �����ϱ� ���� 3�� ���̾�α�
short DGCALLBACK slabBottomTableformPlacerHandler3 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	xx;
	short	result;
	short	itmIdx;
	int		widthInt, heightInt;
	double	widthDouble, heightDouble;

	switch (message) {
		case DG_MSG_INIT:
			
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, "Cell �� ����");

			////////////////////////////////////////////////////////////  ������ ��ġ (�⺻ ��ư)
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
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 20, 70, 23);
			DGSetItemFont (dialogID, LABEL_OBJ_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_OBJ_TYPE, "��ü Ÿ��");
			DGShowItem (dialogID, LABEL_OBJ_TYPE);

			// �˾���Ʈ��: ��ü Ÿ�� (������, ������ ���̺��� (���ǳ�), ����)
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100, 20-7, 120, 25);
			DGSetItemFont (dialogID, POPUP_OBJ_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, "������");
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, "���ǳ�");
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, "����");
			DGShowItem (dialogID, POPUP_OBJ_TYPE);
			if (placingZone.cells [clickedRow][clickedCol].objType == EUROFORM)
				DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, EUROFORM);
			else if (placingZone.cells [clickedRow][clickedCol].objType == TABLEFORM)
				DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, TABLEFORM);
			else if (placingZone.cells [clickedRow][clickedCol].objType == PLYWOOD)
				DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, PLYWOOD);
			DGDisableItem (dialogID, POPUP_OBJ_TYPE);

			// ��: �ʺ�
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 60, 70, 23);
			DGSetItemFont (dialogID, LABEL_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_WIDTH, "�ʺ�");
			DGShowItem (dialogID, LABEL_WIDTH);

			// �˾���Ʈ��: �ʺ�
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100, 60-7, 120, 25);
			DGSetItemFont (dialogID, POPUP_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, POPUP_WIDTH);

			// ��: ����
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 100, 70, 23);
			DGSetItemFont (dialogID, LABEL_HEIGHT, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_HEIGHT, "����");
			DGShowItem (dialogID, LABEL_HEIGHT);

			// �˾���Ʈ��: ����
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100, 100-7, 120, 25);
			DGSetItemFont (dialogID, POPUP_HEIGHT, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, POPUP_HEIGHT);

			// ��: ��ġ����
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 140, 70, 23);
			DGSetItemFont (dialogID, LABEL_ORIENTATION, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_ORIENTATION, "��ġ����");
			DGShowItem (dialogID, LABEL_ORIENTATION);

			// �˾���Ʈ��: ��ġ����
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100, 140-7, 120, 25);
			DGSetItemFont (dialogID, POPUP_ORIENTATION, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_ORIENTATION, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_ORIENTATION, DG_POPUP_BOTTOM, "���ι���");
			DGPopUpInsertItem (dialogID, POPUP_ORIENTATION, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_ORIENTATION, DG_POPUP_BOTTOM, "���ι���");
			DGShowItem (dialogID, POPUP_ORIENTATION);

			// �˾���Ʈ��(�ʺ�, ����)�� ������ ��ü Ÿ�Կ� ���� �޶���
			if (placingZone.cells [clickedRow][clickedCol].objType == EUROFORM) {
				DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, "600");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, "500");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, "450");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, "400");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, "300");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, "200");

				DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, "1200");
				DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, "900");
				DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, "600");
			} else if (placingZone.cells [clickedRow][clickedCol].objType == TABLEFORM) {
				DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, "1818");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, "1212");

				DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, "3032");
				DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, "2426");
				DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, "1820");
			} else if (placingZone.cells [clickedRow][clickedCol].objType == PLYWOOD) {
				DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, "1220");

				DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, "2440");
			}

			// ��: ��� ����
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 10, 170, 220, 40);
			DGSetItemFont (dialogID, LABEL_CAUTION, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_CAUTION, "1818 x 3032, 1818 x 2426,\n1212 x 1820 �� ��ȿ��");

			// �ʱ� �Է� �ʵ� ǥ��
			if (placingZone.cells [clickedRow][clickedCol].objType == EUROFORM) {
				if (abs (placingZone.cells [clickedRow][clickedCol].horLen - 0.600) < EPS)
					DGPopUpSelectItem (dialogID, POPUP_WIDTH, 1);
				else if (abs (placingZone.cells [clickedRow][clickedCol].horLen - 0.500) < EPS)
					DGPopUpSelectItem (dialogID, POPUP_WIDTH, 2);
				else if (abs (placingZone.cells [clickedRow][clickedCol].horLen - 0.450) < EPS)
					DGPopUpSelectItem (dialogID, POPUP_WIDTH, 3);
				else if (abs (placingZone.cells [clickedRow][clickedCol].horLen - 0.400) < EPS)
					DGPopUpSelectItem (dialogID, POPUP_WIDTH, 4);
				else if (abs (placingZone.cells [clickedRow][clickedCol].horLen - 0.300) < EPS)
					DGPopUpSelectItem (dialogID, POPUP_WIDTH, 5);
				else if (abs (placingZone.cells [clickedRow][clickedCol].horLen - 0.200) < EPS)
					DGPopUpSelectItem (dialogID, POPUP_WIDTH, 6);

				if (abs (placingZone.cells [clickedRow][clickedCol].verLen - 1.200) < EPS)
					DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 1);
				else if (abs (placingZone.cells [clickedRow][clickedCol].verLen - 0.900) < EPS)
					DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 2);
				else if (abs (placingZone.cells [clickedRow][clickedCol].verLen - 0.600) < EPS)
					DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 3);
			} else if (placingZone.cells [clickedRow][clickedCol].objType == TABLEFORM) {
				if (abs (placingZone.cells [clickedRow][clickedCol].horLen - 1.818) < EPS)
					DGPopUpSelectItem (dialogID, POPUP_WIDTH, 1);
				else if (abs (placingZone.cells [clickedRow][clickedCol].horLen - 1.212) < EPS)
					DGPopUpSelectItem (dialogID, POPUP_WIDTH, 2);

				if (abs (placingZone.cells [clickedRow][clickedCol].verLen - 3.032) < EPS)
					DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 1);
				else if (abs (placingZone.cells [clickedRow][clickedCol].verLen - 2.426) < EPS)
					DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 2);
				else if (abs (placingZone.cells [clickedRow][clickedCol].verLen - 1.820) < EPS)
					DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 3);
			} else if (placingZone.cells [clickedRow][clickedCol].objType == PLYWOOD) {
				if (abs (placingZone.cells [clickedRow][clickedCol].horLen - 1.220) < EPS)
					DGPopUpSelectItem (dialogID, POPUP_WIDTH, 1);

				if (abs (placingZone.cells [clickedRow][clickedCol].verLen - 2.440) < EPS)
					DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 1);
			}

			if (placingZone.bVertical == true)
				DGPopUpSelectItem (dialogID, POPUP_ORIENTATION, 1);
			else
				DGPopUpSelectItem (dialogID, POPUP_ORIENTATION, 2);

			break;

		case DG_MSG_CHANGE:
			// ���ǳ��� �� ��ȿ�� �ʺ�, ���̰� �ƴϸ� ���� �ܰ�� ������ �� ����
			if (placingZone.cells [clickedRow][clickedCol].objType == TABLEFORM) {
				// ��ȿ�� �ʺ�, ���� ������ ������ ����: 1818 x 3032, 1818 x 2426, 1212 x 1820
				widthInt = atoi (DGPopUpGetItemText (dialogID, POPUP_WIDTH, static_cast<short>(DGGetItemValLong (dialogID, POPUP_WIDTH))).ToCStr ().Get ());
				heightInt = atoi (DGPopUpGetItemText (dialogID, POPUP_HEIGHT, static_cast<short>(DGGetItemValLong (dialogID, POPUP_HEIGHT))).ToCStr ().Get ());

				if ( ((widthInt == 1818) && (heightInt == 3032)) || ((widthInt == 1818) && (heightInt == 2426)) || ((widthInt == 1212) && (heightInt == 1820)) ) {
					DGEnableItem (dialogID, DG_OK);
					DGHideItem (dialogID, LABEL_CAUTION);
				} else {
					DGDisableItem (dialogID, DG_OK);
					DGShowItem (dialogID, LABEL_CAUTION);
				}
			} else {
				DGEnableItem (dialogID, DG_OK);
				DGHideItem (dialogID, LABEL_CAUTION);
			}

			 break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// ���� ����
					if (DGPopUpGetSelected (dialogID, POPUP_ORIENTATION) == 1)
						placingZone.bVertical = true;
					else
						placingZone.bVertical = false;

					widthDouble = atof (DGPopUpGetItemText (dialogID, POPUP_WIDTH, static_cast<short>(DGGetItemValLong (dialogID, POPUP_WIDTH))).ToCStr ().Get ()) / 1000.0;
					heightDouble = atof (DGPopUpGetItemText (dialogID, POPUP_HEIGHT, static_cast<short>(DGGetItemValLong (dialogID, POPUP_HEIGHT))).ToCStr ().Get ()) / 1000.0;

					if (placingZone.bVertical == true) {
						for (xx = 0 ; xx < MAX_IND ; ++xx) {
							placingZone.cells [xx][clickedCol].horLen = widthDouble;	// �ʺ� ���� - ������ ���� ��� �ʺ� �����ؾ� ��
							placingZone.cells [clickedRow][xx].verLen = heightDouble;	// ���� ���� - ������ ���� ��� ���̵� �����ؾ� ��
						}
					} else {
						for (xx = 0 ; xx < MAX_IND ; ++xx) {
							placingZone.cells [clickedRow][xx].horLen = widthDouble;	// �ʺ� ���� - ������ ���� ��� �ʺ� �����ؾ� ��
							placingZone.cells [xx][clickedCol].verLen = heightDouble;	// ���� ���� - ������ ���� ��� ���̵� �����ؾ� ��
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
			break;
	}

	result = item;

	return	result;
}
