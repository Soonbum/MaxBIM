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
static insulElemForSlabTableform	insulElem;		// �ܿ��� ����
API_Guid	structuralObject_forTableformSlab;		// ���� ��ü�� GUID

static short		clickedBtnItemIdx;			// �׸��� ��ư���� Ŭ���� ��ư�� �ε��� ��ȣ�� ����
static bool			clickedExcludeRestButton;	// ������ ���� ��ư�� �������ϱ�?
static bool			clickedPrevButton;			// ���� ��ư�� �������ϱ�?
static short		clickedRow, clickedCol;		// Ŭ���� ��, �� �ε���
static short		layerInd_Euroform;			// ���̾� ��ȣ: ������
static short		layerInd_ConPanel;			// ���̾� ��ȣ: ���ǳ�
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
static GS::Array<API_Guid>	elemList_Insulation;	// �׷�ȭ (�ܿ���)
short	MAX_IND = 50;
double	BEAM_OVERLAP = 0.200;
double	GIRDER_OVERLAP = 0.200;


// ������ �Ϻο� ���̺����� ��ġ�ϴ� ���� ��ƾ
GSErrCode	placeTableformOnSlabBottom (void)
{
	GSErrCode	err = NoError;
	short		xx, yy;
	double		dx, dy, ang;

	// Selection Manager ���� ����
	GS::Array<API_Guid>		morphs;
	GS::Array<API_Guid>		slabs;
	long					nMorphs = 0;
	long					nSlabs = 0;

	// ��ü ���� ��������
	API_Element				elem;
	API_ElementMemo			memo;
	API_ElemInfo3D			info3D;

	// ���� 3D ������� ��������
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
	double			workLevel_slab;


	// ������ ��� �������� (������ 1��, ���� 1�� �����ؾ� ��)
	err = getGuidsOfSelection (&slabs, API_SlabID, &nSlabs);
	err = getGuidsOfSelection (&morphs, API_MorphID, &nMorphs);
	if (err == APIERR_NOPLAN) {
		DGAlert (DG_ERROR, L"����", L"���� ������Ʈ â�� �����ϴ�.", "", L"Ȯ��", "", "");
	}
	if (err == APIERR_NOSEL) {
		DGAlert (DG_ERROR, L"����", L"�ƹ� �͵� �������� �ʾҽ��ϴ�.\n�ʼ� ����: �ٴ� ������ (1��), �ٴڰ� õ�忡 �´�� ������ü ���� (1��)", "", L"Ȯ��", "", "");
	}

	// �����갡 1���ΰ�?
	if (nSlabs != 1) {
		DGAlert (DG_ERROR, L"����", L"�ٴ� �����긦 1�� �����ؾ� �մϴ�.", "", L"Ȯ��", "", "");
		err = APIERR_GENERAL;
		return err;
	}

	// ������ 1���ΰ�?
	if (nMorphs != 1) {
		DGAlert (DG_ERROR, L"����", L"�ٴڰ� õ�忡 �´�� ������ü ������ 1�� �����ϼž� �մϴ�.", "", L"Ȯ��", "", "");
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

	// ������ ������ü�̾�� ��
	if ((abs (info3D.bounds.xMax - info3D.bounds.xMin) < EPS) || (abs (info3D.bounds.yMax - info3D.bounds.yMin) < EPS) || (abs (info3D.bounds.zMax - info3D.bounds.zMin) < EPS)) {
		DGAlert (DG_ERROR, L"����", L"������ ������ü�� �ƴմϴ�.", "", L"Ȯ��", "", "");
		err = APIERR_GENERAL;
		return err;
	}

	// ������ ���� ����
	infoMorph.guid		= elem.header.guid;
	infoMorph.floorInd	= elem.header.floorInd;
	infoMorph.level		= info3D.bounds.zMin;

	// ������ 3D ���� �������� ���ϸ� ����
	if (err != NoError) {
		DGAlert (DG_ERROR, L"����", L"������ 3D ���� �������� ���߽��ϴ�.", "", L"Ȯ��", "", "");
		return err;
	}

	// ������ ���������� ������
	nNodes = getVerticesOfMorph (infoMorph.guid, &coords);
	
	// õ����� �ϴ� �� 2���� Ŭ��
	BNZeroMemory (&pointInfo, sizeof (API_GetPointType));
	CHCopyC ("������ õ�忡 �´�� �ϴ� ������ ���� ���� Ŭ���Ͻʽÿ�.", pointInfo.prompt);
	pointInfo.enableQuickSelection = true;
	err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
	point1 = pointInfo.pos;
	firstClickPoint = point1;

	BNZeroMemory (&pointInfo, sizeof (API_GetPointType));
	CHCopyC ("������ õ�忡 �´�� �ϴ� ������ ������ ���� Ŭ���Ͻʽÿ�.", pointInfo.prompt);
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
		DGAlert (DG_ERROR, L"����", L"�����￡ ������ ���� ���� Ŭ���߽��ϴ�.", "", L"Ȯ��", "", "");
		return err;
	}

	// õ��-�ٴڰ� �Ÿ��� ����
	placingZone.roomHeight = info3D.bounds.zMax - info3D.bounds.zMin;

	// ���� ���� ����
	GS::Array<API_Element>	elems;
	elems.Push (elem);
	deleteElements (elems);

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

	// �۾� �� ���� �ݿ�
	workLevel_slab = getWorkLevel (infoSlab.floorInd);

	// ���� ������ �� ���� ����
	placingZone.level = infoSlab.level + infoSlab.offsetFromTop - placingZone.gap + placingZone.roomHeight;
	placingZone.leftBottom.z = placingZone.level;

	if (result != DG_OK)
		return err;

	// [DIALOG] 2��° ���̾�α׿��� ���̺��� ��ġ�� �����ϰų� ���� ���縦 �����մϴ�.
	clickedExcludeRestButton = false;	// ������ ä��� ���� ���� Ŭ�� ����
	clickedPrevButton = false;			// ���� ��ư Ŭ�� ����
	result = DGBlankModalDialog (450, 300, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, slabBottomTableformPlacerHandler2, 0);

	// ���� ��ư�� ������ 1��° ���̾�α� �ٽ� ����
	if (clickedPrevButton == true)
		goto FIRST;

	// �� �迭 ���� ä���
	err = placingZone.fillCellAreas ();

	// ���� ���� ä��� (����)
	if (clickedExcludeRestButton == false)
		err = placingZone.fillMarginAreas ();

	// �ܿ��� ��ġ
	if (placingZone.gap > EPS) {
		result = DGModalDialog (ACAPI_GetOwnResModule (), 32512, ACAPI_GetOwnResModule (), slabBottomTableformPlacerHandler4_Insulation, 0);
		
		if (result == DG_OK)
			err = placingZone.placeInsulations ();
	}
	
	// �弱 ��ġ
	placingZone.placeBeams ();

	// �ۿ���, ���ٸ� ��ġ
	placingZone.placeGirdersAndPosts ();

	// ����� ��ü �׷�ȭ !!!
	groupElements (elemList);
	groupElements (elemList_Insulation);

	return	err;
}

// Cell �迭�� �ʱ�ȭ��
void	SlabTableformPlacingZone::initCells (SlabTableformPlacingZone* placingZone)
{
	short xx, yy;

	// �ʱ� �� ����
	for (xx = 0 ; xx < MAX_IND ; ++xx) {
		for (yy = 0 ; yy < MAX_IND ; ++yy) {
			placingZone->cells [xx][yy].objType = placingZone->iPanelType;
			placingZone->cells [xx][yy].leftBottomX = 0.0;
			placingZone->cells [xx][yy].leftBottomY = 0.0;
			placingZone->cells [xx][yy].leftBottomZ = placingZone->level;
			placingZone->cells [xx][yy].ang = placingZone->ang;
			// �� ���⿡ ���� �޶���
			if (placingZone->iPanelDirection == HORIZONTAL) {
				if (placingZone->iPanelType == PANEL_TYPE_CONPANEL) {
					// ���ǳ� ũ��: 2x6 [606x1820]
					placingZone->cells [xx][yy].horLen = 1.820;
					placingZone->cells [xx][yy].verLen = 0.606;
				} else if (placingZone->iPanelType == PANEL_TYPE_PLYWOOD) {
					// ���� ũ��: 4x8 [1220x2440]
					placingZone->cells [xx][yy].horLen = 2.440;
					placingZone->cells [xx][yy].verLen = 1.220;
				} else if (placingZone->iPanelType == PANEL_TYPE_EUROFORM) {
					// ������ ũ��: 600x1200
					placingZone->cells [xx][yy].horLen = 1.200;
					placingZone->cells [xx][yy].verLen = 0.600;
				}
			} else {
				if (placingZone->iPanelType == PANEL_TYPE_CONPANEL) {
					// ���ǳ� ũ��: 2x6 [606x1820]
					placingZone->cells [xx][yy].horLen = 0.606;
					placingZone->cells [xx][yy].verLen = 1.820;
				} else if (placingZone->iPanelType == PANEL_TYPE_PLYWOOD) {
					// ���� ũ��: 4x8 [1220x2440]
					placingZone->cells [xx][yy].horLen = 1.220;
					placingZone->cells [xx][yy].verLen = 2.440;
				} else if (placingZone->iPanelType == PANEL_TYPE_EUROFORM) {
					// ������ ũ��: 600x1200
					placingZone->cells [xx][yy].horLen = 0.600;
					placingZone->cells [xx][yy].verLen = 1.200;
				}
			}
		}
	}

	// �� �ʱ� �ʺ�, ���� ����
	if (placingZone->iPanelDirection == HORIZONTAL) {
		if (placingZone->iPanelType == PANEL_TYPE_CONPANEL) {
			// ���ǳ� ũ��: 2x6 [606x1820]
			placingZone->initCellHorLen = 1.820;
			placingZone->initCellVerLen = 0.606;
		} else if (placingZone->iPanelType == PANEL_TYPE_PLYWOOD) {
			// ���� ũ��: 4x8 [1220x2440]
			placingZone->initCellHorLen = 2.440;
			placingZone->initCellVerLen = 1.220;
		} else if (placingZone->iPanelType == PANEL_TYPE_EUROFORM) {
			// ������ ũ��: 600x1200
			placingZone->initCellHorLen = 1.200;
			placingZone->initCellVerLen = 0.600;
		}
	} else {
		if (placingZone->iPanelType == PANEL_TYPE_CONPANEL) {
			// ���ǳ� ũ��: 2x6 [606x1820]
			placingZone->initCellHorLen = 0.606;
			placingZone->initCellVerLen = 1.820;
		} else if (placingZone->iPanelType == PANEL_TYPE_PLYWOOD) {
			// ���� ũ��: 4x8 [1220x2440]
			placingZone->initCellHorLen = 1.220;
			placingZone->initCellVerLen = 2.440;
		} else if (placingZone->iPanelType == PANEL_TYPE_EUROFORM) {
			// ������ ũ��: 600x1200
			placingZone->initCellHorLen = 0.600;
			placingZone->initCellVerLen = 1.200;
		}
	}

	// �ʱ� �� ���� ����
	placingZone->nHorCells = (short)floor (placingZone->borderHorLen / placingZone->initCellHorLen);
	placingZone->nVerCells = (short)floor (placingZone->borderVerLen / placingZone->initCellVerLen);

	// �ʱ� �� �迭 ��ü �ʺ�, ���� ����
	placingZone->cellArrayWidth = 0.0;
	placingZone->cellArrayHeight = 0.0;
	for (xx = 0 ; xx < placingZone->nHorCells ; ++xx)	placingZone->cellArrayWidth += placingZone->cells [0][xx].horLen;
	for (xx = 0 ; xx < placingZone->nVerCells ; ++xx)	placingZone->cellArrayHeight += placingZone->cells [xx][0].verLen;

	// �ʱ� ���� ���� ����
	placingZone->marginLeft = 0.0;
	placingZone->marginRight = (placingZone->borderHorLen - placingZone->cellArrayWidth);
	placingZone->marginBottom = 0.0;
	placingZone->marginTop = (placingZone->borderVerLen - placingZone->cellArrayHeight);
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
}

// �� �迭 ���� ä���
GSErrCode	SlabTableformPlacingZone::fillCellAreas (void)
{
	GSErrCode	err = NoError;
	short	xx, yy;
	EasyObjectPlacement	conpanel, plywood, euroform;
	bool	bStandardForm;
	const char *conpanel_size;
	const char *panelThkStr = placingZone.panelThkStr;
	
	for (xx = 0 ; xx < placingZone.nVerCells ; ++xx) {
		for (yy = 0 ; yy < placingZone.nHorCells ; ++yy) {
			if (placingZone.cells [xx][yy].objType == PANEL_TYPE_CONPANEL) {
				conpanel.init (L("����v1.0.gsm"), layerInd_ConPanel, infoSlab.floorInd, placingZone.cells [xx][yy].leftBottomX, placingZone.cells [xx][yy].leftBottomY, placingZone.cells [xx][yy].leftBottomZ, placingZone.cells [xx][yy].ang);

				if (placingZone.iPanelDirection == VERTICAL) {
					if ((abs (placingZone.cells [xx][yy].horLen - 0.910) < EPS) && (abs (placingZone.cells [xx][yy].verLen - 1.820) < EPS))
						conpanel_size = "3x6 [910x1820]";
					else if ((abs (placingZone.cells [xx][yy].horLen - 0.910) < EPS) && (abs (placingZone.cells [xx][yy].verLen - 1.520) < EPS))
						conpanel_size = "3x5 [910x1520]";
					else if ((abs (placingZone.cells [xx][yy].horLen - 0.606) < EPS) && (abs (placingZone.cells [xx][yy].verLen - 1.820) < EPS))
						conpanel_size = "2x6 [606x1820]";
					else if ((abs (placingZone.cells [xx][yy].horLen - 0.606) < EPS) && (abs (placingZone.cells [xx][yy].verLen - 1.520) < EPS))
						conpanel_size = "2x5 [606x1520]";

					conpanel.radAng -= DegreeToRad (90.0);
					elemList.Push (conpanel.placeObject (9,
						"g_comp", APIParT_CString, "���ǳ�",
						"p_stan", APIParT_CString, conpanel_size,
						"w_dir", APIParT_CString, "�ٴڱ��",
						"p_thk", APIParT_CString, panelThkStr,
						"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"p_ang_alter", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"p_rot", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"A", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].horLen),
						"B", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].verLen)));
				} else {
					if ((abs (placingZone.cells [xx][yy].verLen - 0.910) < EPS) && (abs (placingZone.cells [xx][yy].horLen - 1.820) < EPS))
						conpanel_size = "3x6 [910x1820]";
					else if ((abs (placingZone.cells [xx][yy].verLen - 0.910) < EPS) && (abs (placingZone.cells [xx][yy].horLen - 1.520) < EPS))
						conpanel_size = "3x5 [910x1520]";
					else if ((abs (placingZone.cells [xx][yy].verLen - 0.606) < EPS) && (abs (placingZone.cells [xx][yy].horLen - 1.820) < EPS))
						conpanel_size = "2x6 [606x1820]";
					else if ((abs (placingZone.cells [xx][yy].verLen - 0.606) < EPS) && (abs (placingZone.cells [xx][yy].horLen - 1.520) < EPS))
						conpanel_size = "2x5 [606x1520]";

					moveIn3D ('y', conpanel.radAng, -placingZone.cells [xx][yy].verLen, &conpanel.posX, &conpanel.posY, &conpanel.posZ);
					elemList.Push (conpanel.placeObject (9,
						"g_comp", APIParT_CString, "���ǳ�",
						"p_stan", APIParT_CString, conpanel_size,
						"w_dir", APIParT_CString, "�ٴڱ��",
						"p_thk", APIParT_CString, panelThkStr,
						"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"p_ang_alter", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"p_rot", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"A", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].verLen),
						"B", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].horLen)));
				}
			} else if (placingZone.cells [xx][yy].objType == PANEL_TYPE_PLYWOOD) {
				plywood.init (L("����v1.0.gsm"), layerInd_ConPanel, infoSlab.floorInd, placingZone.cells [xx][yy].leftBottomX, placingZone.cells [xx][yy].leftBottomY, placingZone.cells [xx][yy].leftBottomZ, placingZone.cells [xx][yy].ang);

				if (placingZone.iPanelDirection == VERTICAL) {
					plywood.radAng -= DegreeToRad (90.0);
					elemList.Push (plywood.placeObject (12,
						"g_comp", APIParT_CString, "����",
						"p_stan", APIParT_CString, "��԰�",
						"w_dir", APIParT_CString, "�ٴڱ��",
						"p_thk", APIParT_CString, panelThkStr,
						"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"p_ang_alter", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"p_rot", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"sogak", APIParT_Boolean, "0.0",
						"p_wid", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].horLen),
						"p_leng", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].verLen),
						"A", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].horLen),
						"B", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].verLen)));
				} else {
					moveIn3D ('y', plywood.radAng, -placingZone.cells [xx][yy].verLen, &plywood.posX, &plywood.posY, &plywood.posZ);
					elemList.Push (plywood.placeObject (12,
						"g_comp", APIParT_CString, "����",
						"p_stan", APIParT_CString, "��԰�",
						"w_dir", APIParT_CString, "�ٴڱ��",
						"p_thk", APIParT_CString, panelThkStr,
						"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"p_ang_alter", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"p_rot", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"sogak", APIParT_Boolean, "0.0",
						"p_wid", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].verLen),
						"p_leng", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].horLen),
						"A", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].verLen),
						"B", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].horLen)));
				}
			} else if (placingZone.cells [xx][yy].objType == PANEL_TYPE_EUROFORM) {
				euroform.init (L("������v2.0.gsm"), layerInd_Euroform, infoSlab.floorInd, placingZone.cells [xx][yy].leftBottomX, placingZone.cells [xx][yy].leftBottomY, placingZone.cells [xx][yy].leftBottomZ, placingZone.cells [xx][yy].ang);

				if (placingZone.iPanelDirection == VERTICAL) {
					bStandardForm = false;
					if ( ((abs (placingZone.cells [xx][yy].horLen - 0.600) < EPS) || (abs (placingZone.cells [xx][yy].horLen - 0.500) < EPS) ||
						  (abs (placingZone.cells [xx][yy].horLen - 0.450) < EPS) || (abs (placingZone.cells [xx][yy].horLen - 0.400) < EPS) ||
						  (abs (placingZone.cells [xx][yy].horLen - 0.300) < EPS) || (abs (placingZone.cells [xx][yy].horLen - 0.200) < EPS)) &&
						 ((abs (placingZone.cells [xx][yy].verLen - 1.200) < EPS) || (abs (placingZone.cells [xx][yy].verLen - 0.900) < EPS) ||
						  (abs (placingZone.cells [xx][yy].verLen - 0.600) < EPS)) )
						  bStandardForm = true;

					if (bStandardForm == true) {
						elemList.Push (euroform.placeObject (8,
							"eu_stan_onoff", APIParT_Boolean, "1.0",
							"u_ins", APIParT_CString, "�������",
							"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
							"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
							"eu_wid", APIParT_CString, format_string ("%.0f", (double)placingZone.cells [xx][yy].horLen * 1000),
							"eu_hei", APIParT_CString, format_string ("%.0f", (double)placingZone.cells [xx][yy].verLen * 1000),
							"A", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].horLen),
							"B", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].verLen)));
					} else {
						elemList.Push (euroform.placeObject (8,
							"eu_stan_onoff", APIParT_Boolean, "0.0",
							"u_ins", APIParT_CString, "�������",
							"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
							"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
							"eu_wid2", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].horLen),
							"eu_hei2", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].verLen),
							"A", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].horLen),
							"B", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].verLen)));
					}
				} else {
					bStandardForm = false;
					if ( ((abs (placingZone.cells [xx][yy].verLen - 0.600) < EPS) || (abs (placingZone.cells [xx][yy].verLen - 0.500) < EPS) ||
						  (abs (placingZone.cells [xx][yy].verLen - 0.450) < EPS) || (abs (placingZone.cells [xx][yy].verLen - 0.400) < EPS) ||
						  (abs (placingZone.cells [xx][yy].verLen - 0.300) < EPS) || (abs (placingZone.cells [xx][yy].verLen - 0.200) < EPS)) &&
						 ((abs (placingZone.cells [xx][yy].horLen - 1.200) < EPS) || (abs (placingZone.cells [xx][yy].horLen - 0.900) < EPS) ||
						  (abs (placingZone.cells [xx][yy].horLen - 0.600) < EPS)) )
						  bStandardForm = true;

					moveIn3D ('y', euroform.radAng, -placingZone.cells [xx][yy].verLen, &euroform.posX, &euroform.posY, &euroform.posZ);
					euroform.radAng += DegreeToRad (90.0);
					if (bStandardForm == true) {
						elemList.Push (euroform.placeObject (8,
							"eu_stan_onoff", APIParT_Boolean, "1.0",
							"u_ins", APIParT_CString, "�������",
							"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
							"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
							"eu_wid", APIParT_CString, format_string ("%.0f", (double)placingZone.cells [xx][yy].verLen * 1000),
							"eu_hei", APIParT_CString, format_string ("%.0f", (double)placingZone.cells [xx][yy].horLen * 1000),
							"A", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].verLen),
							"B", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].horLen)));
					} else {
						elemList.Push (euroform.placeObject (8,
							"eu_stan_onoff", APIParT_Boolean, "0.0",
							"u_ins", APIParT_CString, "�������",
							"ang_x", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
							"ang_y", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
							"eu_wid2", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].verLen),
							"eu_hei2", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].horLen),
							"A", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].verLen),
							"B", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].horLen)));
					}
				}
			}
		}
	}

	return err;
}

// ���� ���� ä��� (����)
GSErrCode	SlabTableformPlacingZone::fillMarginAreas (void)
{
	GSErrCode	err = NoError;
	double		remainLength;
	double		currentLength;
	EasyObjectPlacement	plywood;

	// ���� ��ġ (TOP)
	if (placingZone.marginTop > EPS) {
		plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
		moveIn3D ('y', placingZone.ang, -placingZone.borderVerLen, &plywood.posX, &plywood.posY, &plywood.posZ);
		remainLength = placingZone.borderHorLen;

		while (remainLength > EPS) {
			if (remainLength > 2.440 + EPS) {
				currentLength = 2.440;
				remainLength -= 2.440;
			} else {
				currentLength = remainLength;
				remainLength = 0;
			}
		
			elemList.Push (plywood.placeObject (12,
				"g_comp", APIParT_CString, "����",
				"p_stan", APIParT_CString, "��԰�",
				"w_dir", APIParT_CString, "�ٴڱ��",
				"p_thk", APIParT_CString, "11.5T",
				"sogak", APIParT_Boolean, "0.0",
				"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"p_ang_alter", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"p_rot", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"p_wid", APIParT_Length, format_string ("%f", placingZone.marginTop),
				"p_leng", APIParT_Length, format_string ("%f", currentLength),
				"A", APIParT_Length, format_string ("%f", placingZone.marginTop),
				"B", APIParT_Length, format_string ("%f", currentLength)));
			moveIn3D ('x', placingZone.ang, currentLength, &plywood.posX, &plywood.posY, &plywood.posZ);
		}
	}

	// ���� ��ġ (BOTTOM)
	if (placingZone.marginBottom > EPS) {
		plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
		moveIn3D ('y', placingZone.ang, -placingZone.marginBottom, &plywood.posX, &plywood.posY, &plywood.posZ);
		remainLength = placingZone.borderHorLen;

		while (remainLength > EPS) {
			if (remainLength > 2.440 + EPS) {
				currentLength = 2.440;
				remainLength -= 2.440;
			} else {
				currentLength = remainLength;
				remainLength = 0;
			}

			elemList.Push (plywood.placeObject (12,
				"g_comp", APIParT_CString, "����",
				"p_stan", APIParT_CString, "��԰�",
				"w_dir", APIParT_CString, "�ٴڱ��",
				"p_thk", APIParT_CString, "11.5T",
				"sogak", APIParT_Boolean, "0.0",
				"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"p_ang_alter", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"p_rot", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"p_wid", APIParT_Length, format_string ("%f", placingZone.marginBottom),
				"p_leng", APIParT_Length, format_string ("%f", currentLength),
				"A", APIParT_Length, format_string ("%f", placingZone.marginBottom),
				"B", APIParT_Length, format_string ("%f", currentLength)));
			moveIn3D ('x', placingZone.ang, currentLength, &plywood.posX, &plywood.posY, &plywood.posZ);
		}
	}

	// ���� ��ġ (LEFT)
	if (placingZone.marginLeft > EPS) {
		plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
		moveIn3D ('y', placingZone.ang, -placingZone.marginBottom, &plywood.posX, &plywood.posY, &plywood.posZ);
		remainLength = placingZone.borderVerLen - placingZone.marginTop - placingZone.marginBottom;

		while (remainLength > EPS) {
			if (remainLength > 2.440 + EPS) {
				currentLength = 2.440;
				remainLength -= 2.440;
			} else {
				currentLength = remainLength;
				remainLength = 0;
			}

			plywood.radAng += DegreeToRad (-90.0);
			elemList.Push (plywood.placeObject (12,
				"g_comp", APIParT_CString, "����",
				"p_stan", APIParT_CString, "��԰�",
				"w_dir", APIParT_CString, "�ٴڱ��",
				"p_thk", APIParT_CString, "11.5T",
				"sogak", APIParT_Boolean, "0.0",
				"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"p_ang_alter", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"p_rot", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"p_wid", APIParT_Length, format_string ("%f", placingZone.marginLeft),
				"p_leng", APIParT_Length, format_string ("%f", currentLength),
				"A", APIParT_Length, format_string ("%f", placingZone.marginLeft),
				"B", APIParT_Length, format_string ("%f", currentLength)));
			plywood.radAng -= DegreeToRad (-90.0);
			moveIn3D ('y', placingZone.ang, -currentLength, &plywood.posX, &plywood.posY, &plywood.posZ);
		}
	}

	// ���� ��ġ (RIGHT)
	if (placingZone.marginRight > EPS) {
		plywood.init (L("����v1.0.gsm"), layerInd_Plywood, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
		moveIn3D ('y', placingZone.ang, -placingZone.marginBottom, &plywood.posX, &plywood.posY, &plywood.posZ);
		moveIn3D ('x', placingZone.ang, placingZone.borderHorLen - placingZone.marginRight, &plywood.posX, &plywood.posY, &plywood.posZ);
		remainLength = placingZone.borderVerLen - placingZone.marginTop - placingZone.marginBottom;

		while (remainLength > EPS) {
			if (remainLength > 2.440 + EPS) {
				currentLength = 2.440;
				remainLength -= 2.440;
			} else {
				currentLength = remainLength;
				remainLength = 0;
			}

			plywood.radAng += DegreeToRad (-90.0);
			elemList.Push (plywood.placeObject (12,
				"g_comp", APIParT_CString, "����",
				"p_stan", APIParT_CString, "��԰�",
				"w_dir", APIParT_CString, "�ٴڱ��",
				"p_thk", APIParT_CString, "11.5T",
				"sogak", APIParT_Boolean, "0.0",
				"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"p_ang_alter", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"p_rot", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"p_wid", APIParT_Length, format_string ("%f", placingZone.marginRight),
				"p_leng", APIParT_Length, format_string ("%f", currentLength),
				"A", APIParT_Length, format_string ("%f", placingZone.marginRight),
				"B", APIParT_Length, format_string ("%f", currentLength)));
			plywood.radAng -= DegreeToRad (-90.0);
			moveIn3D ('y', placingZone.ang, -currentLength, &plywood.posX, &plywood.posY, &plywood.posZ);
		}
	}

	return err;
}

// �ܿ��� ��ġ
GSErrCode	SlabTableformPlacingZone::placeInsulations (void)
{
	GSErrCode	err = NoError;

	short	xx, yy;
	short	totalXX, totalYY;
	double	horLen, verLen;
	double	remainHorLen, remainVerLen;

	EasyObjectPlacement insul;

	if (insulElem.bLimitSize == true) {
		// ����/���� ũ�� ������ true�� ��
		remainHorLen = placingZone.borderHorLen;
		remainVerLen = placingZone.borderVerLen;
		totalXX = (short)floor (remainHorLen / insulElem.maxHorLen);
		totalYY = (short)floor (remainVerLen / insulElem.maxVerLen);

		insul.init (L("�ܿ���v1.0.gsm"), insulElem.layerInd, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
		moveIn3D ('y', insul.radAng, -placingZone.borderVerLen, &insul.posX, &insul.posY, &insul.posZ);

		for (xx = 0 ; xx < totalXX+1 ; ++xx) {
			for (yy = 0 ; yy < totalYY+1 ; ++yy) {
				(remainHorLen > insulElem.maxHorLen) ? horLen = insulElem.maxHorLen : horLen = remainHorLen;
				(remainVerLen > insulElem.maxVerLen) ? verLen = insulElem.maxVerLen : verLen = remainVerLen;

				elemList_Insulation.Push (insul.placeObject (10,
					"A", APIParT_Length, format_string ("%f", horLen),
					"B", APIParT_Length, format_string ("%f", verLen),
					"ZZYZX", APIParT_Length, format_string ("%f", insulElem.thk),
					"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
					"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
					"bRestrictSize", APIParT_Boolean, (insulElem.bLimitSize ? "1.0" : "0.0"),
					"maxHorLen", APIParT_Length, format_string ("%f", insulElem.maxHorLen),
					"maxVerLen", APIParT_Length, format_string ("%f", insulElem.maxVerLen),
					"bLShape", APIParT_Boolean, "0.0",
					"bVerticalCut", APIParT_Boolean, "0.0"));

				remainVerLen -= insulElem.maxVerLen;
				moveIn3D ('y', insul.radAng, verLen, &insul.posX, &insul.posY, &insul.posZ);
			}
			remainHorLen -= insulElem.maxHorLen;
			remainVerLen = placingZone.borderVerLen;
			moveIn3D ('y', insul.radAng, -placingZone.borderVerLen, &insul.posX, &insul.posY, &insul.posZ);
			moveIn3D ('x', insul.radAng, horLen, &insul.posX, &insul.posY, &insul.posZ);
		}
	} else {
		// ����/���� ũ�� ������ false�� ��
		horLen = placingZone.borderHorLen;
		verLen = placingZone.borderVerLen;

		insul.init (L("�ܿ���v1.0.gsm"), insulElem.layerInd, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
		moveIn3D ('y', insul.radAng, -placingZone.borderVerLen, &insul.posX, &insul.posY, &insul.posZ);

		elemList_Insulation.Push (insul.placeObject (10,
			"A", APIParT_Length, format_string ("%f", horLen),
			"B", APIParT_Length, format_string ("%f", verLen),
			"ZZYZX", APIParT_Length, format_string ("%f", insulElem.thk),
			"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
			"bRestrictSize", APIParT_Boolean, (insulElem.bLimitSize ? "1.0" : "0.0"),
			"maxHorLen", APIParT_Length, format_string ("%f", insulElem.maxHorLen),
			"maxVerLen", APIParT_Length, format_string ("%f", insulElem.maxVerLen),
			"bLShape", APIParT_Boolean, "0.0",
			"bVerticalCut", APIParT_Boolean, "0.0"));
	}

	return err;
}

// �弱 ��ġ
GSErrCode	SlabTableformPlacingZone::placeBeams (void)
{
	GSErrCode	err = NoError;
	
	EasyObjectPlacement	beam;
	short	xx;
	double	remainLength, length;
	bool	bShifted;
	int		lineCount;
	
	const char	*gt24_nomsize;
	double		gt24_realsize;

	if (placingZone.iBeamDirection == HORIZONTAL) {
		// �弱 ����: ����
		lineCount = (int)((placingZone.borderVerLen - (placingZone.beamOffsetVertical * 2)) / placingZone.beamGap);

		if (placingZone.iBeamType == BEAM_TYPE_SANSEUNGGAK) {
			// ��°�
			for (xx = 0 ; xx < lineCount ; ++xx) {
				beam.init (L("����v1.0.gsm"), layerInd_Timber, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
				moveIn3D ('x', beam.radAng, placingZone.beamOffsetHorizontal, &beam.posX, &beam.posY, &beam.posZ);
				moveIn3D ('y', beam.radAng, -(placingZone.beamOffsetVertical + placingZone.beamGap * xx), &beam.posX, &beam.posY, &beam.posZ);
				moveIn3D ('z', beam.radAng, -placingZone.panelThk, &beam.posX, &beam.posY, &beam.posZ);

				remainLength = placingZone.borderHorLen - (placingZone.beamOffsetHorizontal * 2) - BEAM_OVERLAP;
				bShifted = false;
				while (remainLength > EPS) {
					if (remainLength > placingZone.girderGap)
						length = placingZone.girderGap + BEAM_OVERLAP;
					else
						length = remainLength;

					elemList.Push (beam.placeObject (6,
						"w_ins", APIParT_CString, "�ٴڴ�����",
						"w_w", APIParT_Length, "0.080",
						"w_h", APIParT_Length, "0.080",
						"w_leng", APIParT_Length, format_string ("%f", length),
						"w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"torsion_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

					moveIn3D ('x', beam.radAng, length - BEAM_OVERLAP, &beam.posX, &beam.posY, &beam.posZ);
					if (bShifted == false) {
						moveIn3D ('y', beam.radAng, -0.080, &beam.posX, &beam.posY, &beam.posZ);
						bShifted = true;
					} else {
						moveIn3D ('y', beam.radAng, 0.080, &beam.posX, &beam.posY, &beam.posZ);
						bShifted = false;
					}

					remainLength -= placingZone.girderGap;
				}
			}
		} else if (placingZone.iBeamType == BEAM_TYPE_TUBAI) {
			// ������
			for (xx = 0 ; xx < lineCount ; ++xx) {
				beam.init (L("����v1.0.gsm"), layerInd_Timber, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
				moveIn3D ('x', beam.radAng, placingZone.beamOffsetHorizontal, &beam.posX, &beam.posY, &beam.posZ);
				moveIn3D ('y', beam.radAng, -(placingZone.beamOffsetVertical + placingZone.beamGap * xx), &beam.posX, &beam.posY, &beam.posZ);
				moveIn3D ('z', beam.radAng, -placingZone.panelThk, &beam.posX, &beam.posY, &beam.posZ);

				remainLength = placingZone.borderHorLen - (placingZone.beamOffsetHorizontal * 2) - BEAM_OVERLAP;
				bShifted = false;
				while (remainLength > EPS) {
					if (remainLength > placingZone.girderGap)
						length = placingZone.girderGap + BEAM_OVERLAP;
					else
						length = remainLength;

					elemList.Push (beam.placeObject (6,
						"w_ins", APIParT_CString, "�ٴڴ�����",
						"w_w", APIParT_Length, "0.080",
						"w_h", APIParT_Length, "0.050",
						"w_leng", APIParT_Length, format_string ("%f", length),
						"w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"torsion_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

					moveIn3D ('x', beam.radAng, length - BEAM_OVERLAP, &beam.posX, &beam.posY, &beam.posZ);
					if (bShifted == false) {
						moveIn3D ('y', beam.radAng, -0.050, &beam.posX, &beam.posY, &beam.posZ);
						bShifted = true;
					} else {
						moveIn3D ('y', beam.radAng, 0.050, &beam.posX, &beam.posY, &beam.posZ);
						bShifted = false;
					}

					remainLength -= placingZone.girderGap;
				}
			}
		} else if (placingZone.iBeamType == BEAM_TYPE_GT24) {
			// GT24 �Ŵ�
			for (xx = 0 ; xx < lineCount ; ++xx) {
				beam.init (L("GT24 �Ŵ� v1.0.gsm"), layerInd_GT24Girder, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
				moveIn3D ('x', beam.radAng, placingZone.beamOffsetHorizontal, &beam.posX, &beam.posY, &beam.posZ);
				moveIn3D ('y', beam.radAng, -(placingZone.beamOffsetVertical + 0.040 + placingZone.beamGap * xx), &beam.posX, &beam.posY, &beam.posZ);
				moveIn3D ('z', beam.radAng, -placingZone.panelThk - 0.240, &beam.posX, &beam.posY, &beam.posZ);

				remainLength = placingZone.borderHorLen - (placingZone.beamOffsetHorizontal * 2) - BEAM_OVERLAP;
				bShifted = false;
				while (remainLength > EPS) {
					if (remainLength > placingZone.girderGap)
						length = placingZone.girderGap + BEAM_OVERLAP;
					else
						length = remainLength;

					if (length > 5.700) { gt24_nomsize = "6000"; gt24_realsize = 5.950; }
					else if (length > 5.400) { gt24_nomsize = "5700"; gt24_realsize = 5.654; }
					else if (length > 5.100) { gt24_nomsize = "5400"; gt24_realsize = 5.358; }
					else if (length > 4.800) { gt24_nomsize = "5100"; gt24_realsize = 5.062; }
					else if (length > 4.500) { gt24_nomsize = "4800"; gt24_realsize = 4.766; }
					else if (length > 4.200) { gt24_nomsize = "4500"; gt24_realsize = 4.470; }
					else if (length > 3.900) { gt24_nomsize = "4200"; gt24_realsize = 4.174; }
					else if (length > 3.600) { gt24_nomsize = "3900"; gt24_realsize = 3.878; }
					else if (length > 3.300) { gt24_nomsize = "3600"; gt24_realsize = 3.582; }
					else if (length > 3.000) { gt24_nomsize = "3300"; gt24_realsize = 3.286; }
					else if (length > 2.700) { gt24_nomsize = "3000"; gt24_realsize = 2.990; }
					else if (length > 2.400) { gt24_nomsize = "2700"; gt24_realsize = 2.694; }
					else if (length > 2.100) { gt24_nomsize = "2400"; gt24_realsize = 2.398; }
					else if (length > 1.800) { gt24_nomsize = "2100"; gt24_realsize = 2.102; }
					else if (length > 1.500) { gt24_nomsize = "1800"; gt24_realsize = 1.806; }
					else if (length > 1.200) { gt24_nomsize = "1500"; gt24_realsize = 1.510; }
					else if (length > 0.900) { gt24_nomsize = "1200"; gt24_realsize = 1.214; }
					else { gt24_nomsize = "900"; gt24_realsize = 0.918; }

					elemList.Push (beam.placeObject (6,
						"type", APIParT_CString, gt24_nomsize,
						"length", APIParT_Length, format_string ("%f", gt24_realsize),
						"change_rot_method", APIParT_Boolean, "0.0",
						"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"bWood", APIParT_Boolean, "0.0"));

					moveIn3D ('x', beam.radAng, length - BEAM_OVERLAP, &beam.posX, &beam.posY, &beam.posZ);
					if (bShifted == false) {
						moveIn3D ('y', beam.radAng, -0.080, &beam.posX, &beam.posY, &beam.posZ);
						bShifted = true;
					} else {
						moveIn3D ('y', beam.radAng, 0.080, &beam.posX, &beam.posY, &beam.posZ);
						bShifted = false;
					}

					remainLength -= placingZone.girderGap;
				}
			}
		}
	} else {
		// �弱 ����: ����
		lineCount = (int)((placingZone.borderHorLen - (placingZone.beamOffsetHorizontal * 2)) / placingZone.beamGap);

		if (placingZone.iBeamType == BEAM_TYPE_SANSEUNGGAK) {
			// ��°�
			for (xx = 0 ; xx < lineCount ; ++xx) {
				beam.init (L("����v1.0.gsm"), layerInd_Timber, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
				moveIn3D ('x', beam.radAng, placingZone.beamOffsetHorizontal + placingZone.beamGap * xx, &beam.posX, &beam.posY, &beam.posZ);
				moveIn3D ('y', beam.radAng, -placingZone.beamOffsetVertical, &beam.posX, &beam.posY, &beam.posZ);
				moveIn3D ('z', beam.radAng, -placingZone.panelThk, &beam.posX, &beam.posY, &beam.posZ);

				remainLength = placingZone.borderVerLen - (placingZone.beamOffsetVertical * 2) - BEAM_OVERLAP;
				bShifted = false;
				while (remainLength > EPS) {
					if (remainLength > placingZone.girderGap)
						length = placingZone.girderGap + BEAM_OVERLAP;
					else
						length = remainLength;

					beam.radAng -= DegreeToRad (90.0);
					elemList.Push (beam.placeObject (6,
						"w_ins", APIParT_CString, "�ٴڴ�����",
						"w_w", APIParT_Length, "0.080",
						"w_h", APIParT_Length, "0.080",
						"w_leng", APIParT_Length, format_string ("%f", length),
						"w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"torsion_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
					beam.radAng += DegreeToRad (90.0);

					moveIn3D ('y', beam.radAng, -(length - BEAM_OVERLAP), &beam.posX, &beam.posY, &beam.posZ);
					if (bShifted == false) {
						moveIn3D ('x', beam.radAng, 0.080, &beam.posX, &beam.posY, &beam.posZ);
						bShifted = true;
					} else {
						moveIn3D ('x', beam.radAng, -0.080, &beam.posX, &beam.posY, &beam.posZ);
						bShifted = false;
					}

					remainLength -= placingZone.girderGap;
				}
			}
		} else if (placingZone.iBeamType == BEAM_TYPE_TUBAI) {
			// ������
			for (xx = 0 ; xx < lineCount ; ++xx) {
				beam.init (L("����v1.0.gsm"), layerInd_Timber, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
				moveIn3D ('x', beam.radAng, placingZone.beamOffsetHorizontal + placingZone.beamGap * xx, &beam.posX, &beam.posY, &beam.posZ);
				moveIn3D ('y', beam.radAng, -placingZone.beamOffsetVertical, &beam.posX, &beam.posY, &beam.posZ);
				moveIn3D ('z', beam.radAng, -placingZone.panelThk, &beam.posX, &beam.posY, &beam.posZ);

				remainLength = placingZone.borderVerLen - (placingZone.beamOffsetVertical * 2) - BEAM_OVERLAP;
				bShifted = false;
				while (remainLength > EPS) {
					if (remainLength > placingZone.girderGap)
						length = placingZone.girderGap + BEAM_OVERLAP;
					else
						length = remainLength;

					beam.radAng -= DegreeToRad (90.0);
					elemList.Push (beam.placeObject (6,
						"w_ins", APIParT_CString, "�ٴڴ�����",
						"w_w", APIParT_Length, "0.080",
						"w_h", APIParT_Length, "0.050",
						"w_leng", APIParT_Length, format_string ("%f", length),
						"w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"torsion_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
					beam.radAng += DegreeToRad (90.0);

					moveIn3D ('y', beam.radAng, -(length - BEAM_OVERLAP), &beam.posX, &beam.posY, &beam.posZ);
					if (bShifted == false) {
						moveIn3D ('x', beam.radAng, 0.050, &beam.posX, &beam.posY, &beam.posZ);
						bShifted = true;
					} else {
						moveIn3D ('x', beam.radAng, -0.050, &beam.posX, &beam.posY, &beam.posZ);
						bShifted = false;
					}

					remainLength -= placingZone.girderGap;
				}
			}
		} else if (placingZone.iBeamType == BEAM_TYPE_GT24) {
			// GT24 �Ŵ�
			for (xx = 0 ; xx < lineCount ; ++xx) {
				beam.init (L("GT24 �Ŵ� v1.0.gsm"), layerInd_GT24Girder, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
				moveIn3D ('x', beam.radAng, placingZone.beamOffsetHorizontal + 0.040 + placingZone.beamGap * xx, &beam.posX, &beam.posY, &beam.posZ);
				moveIn3D ('y', beam.radAng, -placingZone.beamOffsetVertical, &beam.posX, &beam.posY, &beam.posZ);
				moveIn3D ('z', beam.radAng, -placingZone.panelThk - 0.240, &beam.posX, &beam.posY, &beam.posZ);

				remainLength = placingZone.borderVerLen - (placingZone.beamOffsetVertical * 2) - BEAM_OVERLAP;
				bShifted = false;
				while (remainLength > EPS) {
					if (remainLength > placingZone.girderGap)
						length = placingZone.girderGap + BEAM_OVERLAP;
					else
						length = remainLength;

					if (length > 5.700) { gt24_nomsize = "6000"; gt24_realsize = 5.950; }
					else if (length > 5.400) { gt24_nomsize = "5700"; gt24_realsize = 5.654; }
					else if (length > 5.100) { gt24_nomsize = "5400"; gt24_realsize = 5.358; }
					else if (length > 4.800) { gt24_nomsize = "5100"; gt24_realsize = 5.062; }
					else if (length > 4.500) { gt24_nomsize = "4800"; gt24_realsize = 4.766; }
					else if (length > 4.200) { gt24_nomsize = "4500"; gt24_realsize = 4.470; }
					else if (length > 3.900) { gt24_nomsize = "4200"; gt24_realsize = 4.174; }
					else if (length > 3.600) { gt24_nomsize = "3900"; gt24_realsize = 3.878; }
					else if (length > 3.300) { gt24_nomsize = "3600"; gt24_realsize = 3.582; }
					else if (length > 3.000) { gt24_nomsize = "3300"; gt24_realsize = 3.286; }
					else if (length > 2.700) { gt24_nomsize = "3000"; gt24_realsize = 2.990; }
					else if (length > 2.400) { gt24_nomsize = "2700"; gt24_realsize = 2.694; }
					else if (length > 2.100) { gt24_nomsize = "2400"; gt24_realsize = 2.398; }
					else if (length > 1.800) { gt24_nomsize = "2100"; gt24_realsize = 2.102; }
					else if (length > 1.500) { gt24_nomsize = "1800"; gt24_realsize = 1.806; }
					else if (length > 1.200) { gt24_nomsize = "1500"; gt24_realsize = 1.510; }
					else if (length > 0.900) { gt24_nomsize = "1200"; gt24_realsize = 1.214; }
					else { gt24_nomsize = "900"; gt24_realsize = 0.918; }

					beam.radAng -= DegreeToRad (90.0);
					elemList.Push (beam.placeObject (6,
						"type", APIParT_CString, gt24_nomsize,
						"length", APIParT_Length, format_string ("%f", gt24_realsize),
						"change_rot_method", APIParT_Boolean, "0.0",
						"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"bWood", APIParT_Boolean, "0.0"));
					beam.radAng += DegreeToRad (90.0);

					moveIn3D ('y', beam.radAng, -(length - BEAM_OVERLAP), &beam.posX, &beam.posY, &beam.posZ);
					if (bShifted == false) {
						moveIn3D ('x', beam.radAng, 0.080, &beam.posX, &beam.posY, &beam.posZ);
						bShifted = true;
					} else {
						moveIn3D ('x', beam.radAng, -0.080, &beam.posX, &beam.posY, &beam.posZ);
						bShifted = false;
					}

					remainLength -= placingZone.girderGap;
				}
			}
		}
	}

	return	err;
}

// �ۿ��� ��ġ
GSErrCode	SlabTableformPlacingZone::placeGirdersAndPosts (void)
{
	GSErrCode	err = NoError;
	
	EasyObjectPlacement	girder, post, mrk;
	short	xx, yy;
	double	remainLength, length;
	bool	bShifted;
	int		lineCount;
	int		girderCount;

	const char	*gt24_nomsize;
	double		gt24_realsize;
	
	double		post_length;
	const char	*post_type;

	const char	*mrk_type;
	double		gt24_length_first;
	double		gt24_length_last;

	if (placingZone.iBeamDirection == HORIZONTAL) {
		// �弱 ����: ����
		lineCount = (int)((placingZone.borderHorLen - (placingZone.beamOffsetHorizontal * 2)) / placingZone.suppPostGap) + 1;

		if (placingZone.iGirderType == GIRDER_TYPE_GT24) {
			// GT24 �Ŵ�
			for (xx = 0 ; xx < lineCount ; ++xx) {
				if (placingZone.nGirders == 1) {
					girder.init (L("GT24 �Ŵ� v1.0.gsm"), layerInd_GT24Girder, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
					moveIn3D ('x', girder.radAng, placingZone.beamOffsetHorizontal + 0.080 + placingZone.suppPostGap * xx, &girder.posX, &girder.posY, &girder.posZ);
					moveIn3D ('y', girder.radAng, -(placingZone.beamOffsetVertical - 0.080), &girder.posX, &girder.posY, &girder.posZ);
					moveIn3D ('z', girder.radAng, -placingZone.panelThk - placingZone.beamThk - 0.240, &girder.posX, &girder.posY, &girder.posZ);

					remainLength = placingZone.borderVerLen - (placingZone.beamOffsetVertical * 2) - GIRDER_OVERLAP;
					bShifted = false;
					girderCount = 0;
					while (remainLength > placingZone.suppPostGap) {
						if (remainLength > placingZone.suppPostGap)
							length = placingZone.suppPostGap + GIRDER_OVERLAP;
						else
							length = remainLength;

						if (length > 5.700) { gt24_nomsize = "6000"; gt24_realsize = 5.950; }
						else if (length > 5.400) { gt24_nomsize = "5700"; gt24_realsize = 5.654; }
						else if (length > 5.100) { gt24_nomsize = "5400"; gt24_realsize = 5.358; }
						else if (length > 4.800) { gt24_nomsize = "5100"; gt24_realsize = 5.062; }
						else if (length > 4.500) { gt24_nomsize = "4800"; gt24_realsize = 4.766; }
						else if (length > 4.200) { gt24_nomsize = "4500"; gt24_realsize = 4.470; }
						else if (length > 3.900) { gt24_nomsize = "4200"; gt24_realsize = 4.174; }
						else if (length > 3.600) { gt24_nomsize = "3900"; gt24_realsize = 3.878; }
						else if (length > 3.300) { gt24_nomsize = "3600"; gt24_realsize = 3.582; }
						else if (length > 3.000) { gt24_nomsize = "3300"; gt24_realsize = 3.286; }
						else if (length > 2.700) { gt24_nomsize = "3000"; gt24_realsize = 2.990; }
						else if (length > 2.400) { gt24_nomsize = "2700"; gt24_realsize = 2.694; }
						else if (length > 2.100) { gt24_nomsize = "2400"; gt24_realsize = 2.398; }
						else if (length > 1.800) { gt24_nomsize = "2100"; gt24_realsize = 2.102; }
						else if (length > 1.500) { gt24_nomsize = "1800"; gt24_realsize = 1.806; }
						else if (length > 1.200) { gt24_nomsize = "1500"; gt24_realsize = 1.510; }
						else if (length > 0.900) { gt24_nomsize = "1200"; gt24_realsize = 1.214; }
						else { gt24_nomsize = "900"; gt24_realsize = 0.918; }

						girder.radAng -= DegreeToRad (90.0);
						elemList.Push (girder.placeObject (6,
							"type", APIParT_CString, gt24_nomsize,
							"length", APIParT_Length, format_string ("%f", gt24_realsize),
							"change_rot_method", APIParT_Boolean, "0.0",
							"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
							"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
							"bWood", APIParT_Boolean, "0.0"));
						girderCount ++;
						girder.radAng += DegreeToRad (90.0);

						moveIn3D ('y', girder.radAng, -(length - GIRDER_OVERLAP), &girder.posX, &girder.posY, &girder.posZ);
						if (bShifted == false) {
							moveIn3D ('x', girder.radAng, 0.080, &girder.posX, &girder.posY, &girder.posZ);
							bShifted = true;
						} else {
							moveIn3D ('x', girder.radAng, -0.080, &girder.posX, &girder.posY, &girder.posZ);
							bShifted = false;
						}

						remainLength -= placingZone.suppPostGap;
					}

					// ���ٸ� ��ġ
					for (yy = 0 ; yy <= girderCount ; ++yy) {
						if (placingZone.iSuppPostType == POST_TYPE_PERI_SUPPORT) {
							// PERI ���ٸ�
							post_length = placingZone.roomHeight - placingZone.panelThk - placingZone.beamThk - placingZone.girderThk;

							post.init (L("PERI���ٸ� ������ v0.1.gsm"), layerInd_PERI_Support, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
							moveIn3D ('x', post.radAng, placingZone.beamOffsetHorizontal + 0.080 + 0.040 + placingZone.suppPostGap * xx, &post.posX, &post.posY, &post.posZ);
							moveIn3D ('y', post.radAng, -placingZone.beamOffsetVertical - placingZone.girderGap * yy, &post.posX, &post.posY, &post.posZ);
							moveIn3D ('z', post.radAng, -placingZone.panelThk - placingZone.beamThk - 0.240 - placingZone.crossHeadThk, &post.posX, &post.posY, &post.posZ);

							if ((post_length > 0.800 - EPS) && (post_length < 1.200 + EPS))
								post_type = "MP 120";
							else if ((post_length > 1.450 - EPS) && (post_length < 2.500 + EPS))
								post_type = "MP 250";
							else if ((post_length > 1.950 - EPS) && (post_length < 3.500 + EPS))
								post_type = "MP 350";
							else if ((post_length > 2.600 - EPS) && (post_length < 4.800 + EPS))
								post_type = "MP 480";
							else if ((post_length > 4.300 - EPS) && (post_length < 6.250 + EPS))
								post_type = "MP 625";
							else
								post_type = "Custom";

							post.placeObject (8,
								"stType", APIParT_CString, post_type,
								"bCrosshead", APIParT_Boolean, "1.0",
								"posCrosshead", APIParT_CString, "�ϴ�",
								"crossheadType", APIParT_CString, "PERI",
								"angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
								"len_current", APIParT_Length, format_string ("%f", post_length),
								"ZZYZX", APIParT_Length, format_string ("%f", post_length),
								"angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)));
						} else if (placingZone.iSuppPostType == POST_TYPE_STEEL_SUPPORT) {
							// ���� ���ٸ�
							post_length = placingZone.roomHeight - placingZone.panelThk - placingZone.beamThk - placingZone.girderThk;

							post.init (L("����Ʈv1.0.gsm"), layerInd_Steel_Support, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
							moveIn3D ('x', post.radAng, placingZone.beamOffsetHorizontal + 0.080 + 0.040 + placingZone.suppPostGap * xx, &post.posX, &post.posY, &post.posZ);
							moveIn3D ('y', post.radAng, -placingZone.beamOffsetVertical - placingZone.girderGap * yy, &post.posX, &post.posY, &post.posZ);
							moveIn3D ('z', post.radAng, -placingZone.panelThk - placingZone.beamThk - 0.240 - post_length, &post.posX, &post.posY, &post.posZ);

							if ((post_length > 1.200 - EPS) && (post_length < 2.000 + EPS))
								post_type = "V0 (2.0m)";
							else if ((post_length > 1.850 - EPS) && (post_length < 3.100 + EPS))
								post_type = "V1 (3.2m)";
							else if ((post_length > 2.150 - EPS) && (post_length < 3.300 + EPS))
								post_type = "V2 (3.4m)";
							else if ((post_length > 2.450 - EPS) && (post_length < 3.700 + EPS))
								post_type = "V3 (3.8m)";
							else if ((post_length > 2.650 - EPS) && (post_length < 4.000 + EPS))
								post_type = "V4 (4.0m)";
							else if ((post_length > 3.600 - EPS) && (post_length < 5.000 + EPS))
								post_type = "V5 (5.0m)";
							else if ((post_length > 3.200 - EPS) && (post_length < 5.900 + EPS))
								post_type = "V6 (5.9m)";

							post.placeObject (7,
								"s_bimj", APIParT_Boolean, "0.0",
								"s_stan", APIParT_CString, post_type,
								"s_leng", APIParT_Length, format_string ("%f", post_length - placingZone.crossHeadThk),
								"s_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
								"bCrosshead", APIParT_Boolean, "1.0",
								"crossheadType", APIParT_CString, "PERI",
								"angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)));
						}
					}

					// MRK ��ġ (PERI ���ٸ����� ������) !!!
					for (yy = 0 ; yy < girderCount ; ++yy) {
						if (placingZone.iSuppPostType == POST_TYPE_PERI_SUPPORT) {
							//mrk.init (L("PERI���ٸ� ������ v0.2.gsm"), layerind_??? ���̾ ������

							// mrk_type
						}
					}

					// 625 -> 62.5 cm
					// 750 -> 75 cm
					// 900 -> 90 cm
					// 1200 -> 120 cm
					// 1375 -> 137.5 cm
					// 1500 -> 150 cm
					// 2015 -> 201.5 cm
					// 2250 -> 225 cm
					// 2300 -> 230 cm
					// 2370 -> 237 cm
					// 2660 -> 266 cm
					// 2960 -> 296 cm
				} else if (placingZone.nGirders == 2) {
					girder.init (L("GT24 �Ŵ� v1.0.gsm"), layerInd_GT24Girder, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
					moveIn3D ('x', girder.radAng, placingZone.beamOffsetHorizontal + 0.080 + placingZone.suppPostGap * xx, &girder.posX, &girder.posY, &girder.posZ);
					moveIn3D ('y', girder.radAng, -(placingZone.beamOffsetVertical - 0.080), &girder.posX, &girder.posY, &girder.posZ);
					moveIn3D ('z', girder.radAng, -placingZone.panelThk - placingZone.beamThk - 0.240, &girder.posX, &girder.posY, &girder.posZ);

					remainLength = placingZone.borderVerLen - (placingZone.beamOffsetVertical * 2) - GIRDER_OVERLAP;
					girderCount = 0;
					gt24_length_first = 0.0;
					gt24_length_last = 0.0;
					while (remainLength > placingZone.suppPostGap) {
						if (remainLength > placingZone.suppPostGap)
							length = placingZone.suppPostGap + GIRDER_OVERLAP;
						else
							length = remainLength;

						if (length > 5.700) { gt24_nomsize = "6000"; gt24_realsize = 5.950; }
						else if (length > 5.400) { gt24_nomsize = "5700"; gt24_realsize = 5.654; }
						else if (length > 5.100) { gt24_nomsize = "5400"; gt24_realsize = 5.358; }
						else if (length > 4.800) { gt24_nomsize = "5100"; gt24_realsize = 5.062; }
						else if (length > 4.500) { gt24_nomsize = "4800"; gt24_realsize = 4.766; }
						else if (length > 4.200) { gt24_nomsize = "4500"; gt24_realsize = 4.470; }
						else if (length > 3.900) { gt24_nomsize = "4200"; gt24_realsize = 4.174; }
						else if (length > 3.600) { gt24_nomsize = "3900"; gt24_realsize = 3.878; }
						else if (length > 3.300) { gt24_nomsize = "3600"; gt24_realsize = 3.582; }
						else if (length > 3.000) { gt24_nomsize = "3300"; gt24_realsize = 3.286; }
						else if (length > 2.700) { gt24_nomsize = "3000"; gt24_realsize = 2.990; }
						else if (length > 2.400) { gt24_nomsize = "2700"; gt24_realsize = 2.694; }
						else if (length > 2.100) { gt24_nomsize = "2400"; gt24_realsize = 2.398; }
						else if (length > 1.800) { gt24_nomsize = "2100"; gt24_realsize = 2.102; }
						else if (length > 1.500) { gt24_nomsize = "1800"; gt24_realsize = 1.806; }
						else if (length > 1.200) { gt24_nomsize = "1500"; gt24_realsize = 1.510; }
						else if (length > 0.900) { gt24_nomsize = "1200"; gt24_realsize = 1.214; }
						else { gt24_nomsize = "900"; gt24_realsize = 0.918; }

						if (abs (gt24_length_first) < EPS)
							gt24_length_first = gt24_realsize;
						else
							gt24_length_last = gt24_realsize;

						girder.radAng -= DegreeToRad (90.0);
						elemList.Push (girder.placeObject (6,
							"type", APIParT_CString, gt24_nomsize,
							"length", APIParT_Length, format_string ("%f", gt24_realsize),
							"change_rot_method", APIParT_Boolean, "0.0",
							"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
							"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
							"bWood", APIParT_Boolean, "0.0"));
						moveIn3D ('y', girder.radAng, 0.080, &girder.posX, &girder.posY, &girder.posZ);
						elemList.Push (girder.placeObject (6,
							"type", APIParT_CString, gt24_nomsize,
							"length", APIParT_Length, format_string ("%f", gt24_realsize),
							"change_rot_method", APIParT_Boolean, "0.0",
							"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
							"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
							"bWood", APIParT_Boolean, "0.0"));
						moveIn3D ('y', girder.radAng, -0.080, &girder.posX, &girder.posY, &girder.posZ);
						girderCount ++;
						girder.radAng += DegreeToRad (90.0);

						moveIn3D ('y', girder.radAng, -gt24_realsize, &girder.posX, &girder.posY, &girder.posZ);

						remainLength -= placingZone.suppPostGap;
					}

					// ���ٸ� ��ġ
					for (yy = 0 ; yy <= girderCount ; ++yy) {
						if (placingZone.iSuppPostType == POST_TYPE_PERI_SUPPORT) {
							// PERI ���ٸ�
							post_length = placingZone.roomHeight - placingZone.panelThk - placingZone.beamThk - placingZone.girderThk;

							post.init (L("PERI���ٸ� ������ v0.1.gsm"), layerInd_PERI_Support, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
							moveIn3D ('x', post.radAng, placingZone.beamOffsetHorizontal + 0.080 + 0.040 + placingZone.suppPostGap * xx, &post.posX, &post.posY, &post.posZ);
							if (yy < girderCount)
								moveIn3D ('y', post.radAng, -placingZone.beamOffsetVertical + 0.150/2 - gt24_length_first * yy, &post.posX, &post.posY, &post.posZ);
							else
								moveIn3D ('y', post.radAng, -placingZone.beamOffsetVertical + 0.150/2 - gt24_length_last * yy, &post.posX, &post.posY, &post.posZ);
							moveIn3D ('z', post.radAng, -placingZone.panelThk - placingZone.beamThk - 0.240 - placingZone.crossHeadThk, &post.posX, &post.posY, &post.posZ);

							if ((post_length > 0.800 - EPS) && (post_length < 1.200 + EPS))
								post_type = "MP 120";
							else if ((post_length > 1.450 - EPS) && (post_length < 2.500 + EPS))
								post_type = "MP 250";
							else if ((post_length > 1.950 - EPS) && (post_length < 3.500 + EPS))
								post_type = "MP 350";
							else if ((post_length > 2.600 - EPS) && (post_length < 4.800 + EPS))
								post_type = "MP 480";
							else if ((post_length > 4.300 - EPS) && (post_length < 6.250 + EPS))
								post_type = "MP 625";
							else
								post_type = "Custom";

							post.placeObject (8,
								"stType", APIParT_CString, post_type,
								"bCrosshead", APIParT_Boolean, "1.0",
								"posCrosshead", APIParT_CString, "�ϴ�",
								"crossheadType", APIParT_CString, "PERI",
								"angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
								"len_current", APIParT_Length, format_string ("%f", post_length),
								"ZZYZX", APIParT_Length, format_string ("%f", post_length),
								"angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)));
						} else if (placingZone.iSuppPostType == POST_TYPE_STEEL_SUPPORT) {
							// ���� ���ٸ�
							post_length = placingZone.roomHeight - placingZone.panelThk - placingZone.beamThk - placingZone.girderThk;

							post.init (L("����Ʈv1.0.gsm"), layerInd_Steel_Support, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
							moveIn3D ('x', post.radAng, placingZone.beamOffsetHorizontal + 0.080 + 0.040 + placingZone.suppPostGap * xx, &post.posX, &post.posY, &post.posZ);
							if (yy < girderCount)
								moveIn3D ('y', post.radAng, -placingZone.beamOffsetVertical + 0.150/2 - gt24_length_first * yy, &post.posX, &post.posY, &post.posZ);
							else
								moveIn3D ('y', post.radAng, -placingZone.beamOffsetVertical + 0.150/2 - gt24_length_last * yy, &post.posX, &post.posY, &post.posZ);
							moveIn3D ('z', post.radAng, -placingZone.panelThk - placingZone.beamThk - 0.240 - post_length, &post.posX, &post.posY, &post.posZ);

							if ((post_length > 1.200 - EPS) && (post_length < 2.000 + EPS))
								post_type = "V0 (2.0m)";
							else if ((post_length > 1.850 - EPS) && (post_length < 3.100 + EPS))
								post_type = "V1 (3.2m)";
							else if ((post_length > 2.150 - EPS) && (post_length < 3.300 + EPS))
								post_type = "V2 (3.4m)";
							else if ((post_length > 2.450 - EPS) && (post_length < 3.700 + EPS))
								post_type = "V3 (3.8m)";
							else if ((post_length > 2.650 - EPS) && (post_length < 4.000 + EPS))
								post_type = "V4 (4.0m)";
							else if ((post_length > 3.600 - EPS) && (post_length < 5.000 + EPS))
								post_type = "V5 (5.0m)";
							else if ((post_length > 3.200 - EPS) && (post_length < 5.900 + EPS))
								post_type = "V6 (5.9m)";

							post.placeObject (7,
								"s_bimj", APIParT_Boolean, "0.0",
								"s_stan", APIParT_CString, post_type,
								"s_leng", APIParT_Length, format_string ("%f", post_length - placingZone.crossHeadThk),
								"s_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
								"bCrosshead", APIParT_Boolean, "1.0",
								"crossheadType", APIParT_CString, "PERI",
								"angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)));
						}
					}
				}
			}
		} else if (placingZone.iGirderType == GIRDER_TYPE_SANSEUNGGAK) {
			// ��°�
			for (xx = 0 ; xx < lineCount ; ++xx) {
				if (placingZone.nGirders == 1) {
					girder.init (L("����v1.0.gsm"), layerInd_Timber, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
					moveIn3D ('x', girder.radAng, placingZone.beamOffsetHorizontal + 0.080 + placingZone.suppPostGap * xx, &girder.posX, &girder.posY, &girder.posZ);
					moveIn3D ('y', girder.radAng, -(placingZone.beamOffsetVertical - 0.080), &girder.posX, &girder.posY, &girder.posZ);
					moveIn3D ('z', girder.radAng, -placingZone.panelThk - placingZone.beamThk, &girder.posX, &girder.posY, &girder.posZ);

					remainLength = placingZone.borderVerLen - (placingZone.beamOffsetVertical * 2) - GIRDER_OVERLAP;
					bShifted = false;
					while (remainLength > placingZone.suppPostGap) {
						if (remainLength > placingZone.suppPostGap)
							length = placingZone.suppPostGap + GIRDER_OVERLAP;
						else
							length = remainLength;

						girder.radAng -= DegreeToRad (90.0);
						elemList.Push (girder.placeObject (6,
							"w_ins", APIParT_CString, "�ٴڴ�����",
							"w_w", APIParT_Length, "0.080",
							"w_h", APIParT_Length, "0.080",
							"w_leng", APIParT_Length, format_string ("%f", length),
							"w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
							"torsion_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
						girder.radAng += DegreeToRad (90.0);

						moveIn3D ('y', girder.radAng, -(length - GIRDER_OVERLAP), &girder.posX, &girder.posY, &girder.posZ);
						if (bShifted == false) {
							moveIn3D ('x', girder.radAng, 0.080, &girder.posX, &girder.posY, &girder.posZ);
							bShifted = true;
						} else {
							moveIn3D ('x', girder.radAng, -0.080, &girder.posX, &girder.posY, &girder.posZ);
							bShifted = false;
						}

						remainLength -= placingZone.suppPostGap;
					}

					// ���ٸ� ��ġ !!!
				} else if (placingZone.nGirders == 2) {
					girder.init (L("����v1.0.gsm"), layerInd_Timber, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
					moveIn3D ('x', girder.radAng, placingZone.beamOffsetHorizontal + 0.080 + placingZone.suppPostGap * xx, &girder.posX, &girder.posY, &girder.posZ);
					moveIn3D ('y', girder.radAng, -(placingZone.beamOffsetVertical - 0.080), &girder.posX, &girder.posY, &girder.posZ);
					moveIn3D ('z', girder.radAng, -placingZone.panelThk - placingZone.beamThk, &girder.posX, &girder.posY, &girder.posZ);
					remainLength = placingZone.borderVerLen - (placingZone.beamOffsetVertical * 2) - GIRDER_OVERLAP;

					while (remainLength > placingZone.suppPostGap) {
						if (remainLength > placingZone.suppPostGap)
							length = placingZone.suppPostGap + GIRDER_OVERLAP;
						else
							length = remainLength;

						girder.radAng -= DegreeToRad (90.0);
						elemList.Push (girder.placeObject (6,
							"w_ins", APIParT_CString, "�ٴڴ�����",
							"w_w", APIParT_Length, "0.080",
							"w_h", APIParT_Length, "0.080",
							"w_leng", APIParT_Length, format_string ("%f", length),
							"w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
							"torsion_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
						moveIn3D ('y', girder.radAng, 0.080, &girder.posX, &girder.posY, &girder.posZ);
						elemList.Push (girder.placeObject (6,
							"w_ins", APIParT_CString, "�ٴڴ�����",
							"w_w", APIParT_Length, "0.080",
							"w_h", APIParT_Length, "0.080",
							"w_leng", APIParT_Length, format_string ("%f", length),
							"w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
							"torsion_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
						moveIn3D ('y', girder.radAng, -0.080, &girder.posX, &girder.posY, &girder.posZ);
						girder.radAng += DegreeToRad (90.0);

						moveIn3D ('y', girder.radAng, -length, &girder.posX, &girder.posY, &girder.posZ);

						remainLength -= placingZone.suppPostGap;
					}

					// ���ٸ� ��ġ !!!
				}
			}
		}
	} else {
		// �弱 ����: ����
		lineCount = (int)((placingZone.borderVerLen - (placingZone.beamOffsetVertical * 2)) / placingZone.suppPostGap) + 1;

		if (placingZone.iGirderType == GIRDER_TYPE_GT24) {
			// GT24 �Ŵ�
			for (xx = 0 ; xx < lineCount ; ++xx) {
				if (placingZone.nGirders == 1) {
					girder.init (L("GT24 �Ŵ� v1.0.gsm"), layerInd_GT24Girder, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
					moveIn3D ('x', girder.radAng, placingZone.beamOffsetHorizontal - 0.080, &girder.posX, &girder.posY, &girder.posZ);
					moveIn3D ('y', girder.radAng, -(placingZone.beamOffsetVertical + 0.080) - (placingZone.suppPostGap * xx), &girder.posX, &girder.posY, &girder.posZ);
					moveIn3D ('z', girder.radAng, -placingZone.panelThk - placingZone.beamThk - 0.240, &girder.posX, &girder.posY, &girder.posZ);

					remainLength = placingZone.borderHorLen - (placingZone.beamOffsetHorizontal * 2) - GIRDER_OVERLAP;
					bShifted = false;
					while (remainLength > placingZone.suppPostGap) {
						if (remainLength > placingZone.suppPostGap)
							length = placingZone.suppPostGap + GIRDER_OVERLAP;
						else
							length = remainLength;

						if (length > 5.700) { gt24_nomsize = "6000"; gt24_realsize = 5.950; }
						else if (length > 5.400) { gt24_nomsize = "5700"; gt24_realsize = 5.654; }
						else if (length > 5.100) { gt24_nomsize = "5400"; gt24_realsize = 5.358; }
						else if (length > 4.800) { gt24_nomsize = "5100"; gt24_realsize = 5.062; }
						else if (length > 4.500) { gt24_nomsize = "4800"; gt24_realsize = 4.766; }
						else if (length > 4.200) { gt24_nomsize = "4500"; gt24_realsize = 4.470; }
						else if (length > 3.900) { gt24_nomsize = "4200"; gt24_realsize = 4.174; }
						else if (length > 3.600) { gt24_nomsize = "3900"; gt24_realsize = 3.878; }
						else if (length > 3.300) { gt24_nomsize = "3600"; gt24_realsize = 3.582; }
						else if (length > 3.000) { gt24_nomsize = "3300"; gt24_realsize = 3.286; }
						else if (length > 2.700) { gt24_nomsize = "3000"; gt24_realsize = 2.990; }
						else if (length > 2.400) { gt24_nomsize = "2700"; gt24_realsize = 2.694; }
						else if (length > 2.100) { gt24_nomsize = "2400"; gt24_realsize = 2.398; }
						else if (length > 1.800) { gt24_nomsize = "2100"; gt24_realsize = 2.102; }
						else if (length > 1.500) { gt24_nomsize = "1800"; gt24_realsize = 1.806; }
						else if (length > 1.200) { gt24_nomsize = "1500"; gt24_realsize = 1.510; }
						else if (length > 0.900) { gt24_nomsize = "1200"; gt24_realsize = 1.214; }
						else { gt24_nomsize = "900"; gt24_realsize = 0.918; }

						elemList.Push (girder.placeObject (6,
							"type", APIParT_CString, gt24_nomsize,
							"length", APIParT_Length, format_string ("%f", gt24_realsize),
							"change_rot_method", APIParT_Boolean, "0.0",
							"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
							"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
							"bWood", APIParT_Boolean, "0.0"));

						moveIn3D ('x', girder.radAng, (length - GIRDER_OVERLAP), &girder.posX, &girder.posY, &girder.posZ);
						if (bShifted == false) {
							moveIn3D ('y', girder.radAng, -0.080, &girder.posX, &girder.posY, &girder.posZ);
							bShifted = true;
						} else {
							moveIn3D ('y', girder.radAng, 0.080, &girder.posX, &girder.posY, &girder.posZ);
							bShifted = false;
						}

						remainLength -= placingZone.suppPostGap;
					}

					// ���ٸ� ��ġ !!!
				} else if (placingZone.nGirders == 2) {
					girder.init (L("GT24 �Ŵ� v1.0.gsm"), layerInd_GT24Girder, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
					moveIn3D ('x', girder.radAng, placingZone.beamOffsetHorizontal - 0.080, &girder.posX, &girder.posY, &girder.posZ);
					moveIn3D ('y', girder.radAng, -(placingZone.beamOffsetVertical + 0.080) - (placingZone.suppPostGap * xx), &girder.posX, &girder.posY, &girder.posZ);
					moveIn3D ('z', girder.radAng, -placingZone.panelThk - placingZone.beamThk - 0.240, &girder.posX, &girder.posY, &girder.posZ);

					remainLength = placingZone.borderHorLen - (placingZone.beamOffsetHorizontal * 2) - GIRDER_OVERLAP;

					while (remainLength > placingZone.suppPostGap) {
						if (remainLength > placingZone.suppPostGap)
							length = placingZone.suppPostGap + GIRDER_OVERLAP;
						else
							length = remainLength;

						if (length > 5.700) { gt24_nomsize = "6000"; gt24_realsize = 5.950; }
						else if (length > 5.400) { gt24_nomsize = "5700"; gt24_realsize = 5.654; }
						else if (length > 5.100) { gt24_nomsize = "5400"; gt24_realsize = 5.358; }
						else if (length > 4.800) { gt24_nomsize = "5100"; gt24_realsize = 5.062; }
						else if (length > 4.500) { gt24_nomsize = "4800"; gt24_realsize = 4.766; }
						else if (length > 4.200) { gt24_nomsize = "4500"; gt24_realsize = 4.470; }
						else if (length > 3.900) { gt24_nomsize = "4200"; gt24_realsize = 4.174; }
						else if (length > 3.600) { gt24_nomsize = "3900"; gt24_realsize = 3.878; }
						else if (length > 3.300) { gt24_nomsize = "3600"; gt24_realsize = 3.582; }
						else if (length > 3.000) { gt24_nomsize = "3300"; gt24_realsize = 3.286; }
						else if (length > 2.700) { gt24_nomsize = "3000"; gt24_realsize = 2.990; }
						else if (length > 2.400) { gt24_nomsize = "2700"; gt24_realsize = 2.694; }
						else if (length > 2.100) { gt24_nomsize = "2400"; gt24_realsize = 2.398; }
						else if (length > 1.800) { gt24_nomsize = "2100"; gt24_realsize = 2.102; }
						else if (length > 1.500) { gt24_nomsize = "1800"; gt24_realsize = 1.806; }
						else if (length > 1.200) { gt24_nomsize = "1500"; gt24_realsize = 1.510; }
						else if (length > 0.900) { gt24_nomsize = "1200"; gt24_realsize = 1.214; }
						else { gt24_nomsize = "900"; gt24_realsize = 0.918; }

						elemList.Push (girder.placeObject (6,
							"type", APIParT_CString, gt24_nomsize,
							"length", APIParT_Length, format_string ("%f", gt24_realsize),
							"change_rot_method", APIParT_Boolean, "0.0",
							"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
							"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
							"bWood", APIParT_Boolean, "0.0"));
						moveIn3D ('y', girder.radAng, -0.080, &girder.posX, &girder.posY, &girder.posZ);
						elemList.Push (girder.placeObject (6,
							"type", APIParT_CString, gt24_nomsize,
							"length", APIParT_Length, format_string ("%f", gt24_realsize),
							"change_rot_method", APIParT_Boolean, "0.0",
							"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
							"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
							"bWood", APIParT_Boolean, "0.0"));
						moveIn3D ('y', girder.radAng, 0.080, &girder.posX, &girder.posY, &girder.posZ);

						moveIn3D ('x', girder.radAng, gt24_realsize, &girder.posX, &girder.posY, &girder.posZ);

						remainLength -= placingZone.suppPostGap;
					}

					// ���ٸ� ��ġ !!!
				}
			}
		} else if (placingZone.iGirderType == GIRDER_TYPE_SANSEUNGGAK) {
			// ��°�
			for (xx = 0 ; xx < lineCount ; ++xx) {
				if (placingZone.nGirders == 1) {
					girder.init (L("����v1.0.gsm"), layerInd_Timber, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
					moveIn3D ('x', girder.radAng, placingZone.beamOffsetHorizontal - 0.080, &girder.posX, &girder.posY, &girder.posZ);
					moveIn3D ('y', girder.radAng, -(placingZone.beamOffsetVertical + 0.080) - (placingZone.suppPostGap * xx), &girder.posX, &girder.posY, &girder.posZ);
					moveIn3D ('z', girder.radAng, -placingZone.panelThk - placingZone.beamThk, &girder.posX, &girder.posY, &girder.posZ);

					remainLength = placingZone.borderHorLen - (placingZone.beamOffsetHorizontal * 2) - GIRDER_OVERLAP;
					bShifted = false;
					while (remainLength > placingZone.suppPostGap) {
						if (remainLength > placingZone.suppPostGap)
							length = placingZone.suppPostGap + GIRDER_OVERLAP;
						else
							length = remainLength;

						elemList.Push (girder.placeObject (6,
							"w_ins", APIParT_CString, "�ٴڴ�����",
							"w_w", APIParT_Length, "0.080",
							"w_h", APIParT_Length, "0.080",
							"w_leng", APIParT_Length, format_string ("%f", length),
							"w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
							"torsion_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));

						moveIn3D ('x', girder.radAng, (length - GIRDER_OVERLAP), &girder.posX, &girder.posY, &girder.posZ);
						if (bShifted == false) {
							moveIn3D ('y', girder.radAng, -0.080, &girder.posX, &girder.posY, &girder.posZ);
							bShifted = true;
						} else {
							moveIn3D ('y', girder.radAng, 0.080, &girder.posX, &girder.posY, &girder.posZ);
							bShifted = false;
						}

						remainLength -= placingZone.suppPostGap;
					}

					// ���ٸ� ��ġ !!!
				} else if (placingZone.nGirders == 2) {
					girder.init (L("����v1.0.gsm"), layerInd_Timber, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
					moveIn3D ('x', girder.radAng, placingZone.beamOffsetHorizontal - 0.080, &girder.posX, &girder.posY, &girder.posZ);
					moveIn3D ('y', girder.radAng, -(placingZone.beamOffsetVertical + 0.080) - (placingZone.suppPostGap * xx), &girder.posX, &girder.posY, &girder.posZ);
					moveIn3D ('z', girder.radAng, -placingZone.panelThk - placingZone.beamThk, &girder.posX, &girder.posY, &girder.posZ);

					remainLength = placingZone.borderHorLen - (placingZone.beamOffsetHorizontal * 2) - GIRDER_OVERLAP;

					while (remainLength > placingZone.suppPostGap) {
						if (remainLength > placingZone.suppPostGap)
							length = placingZone.suppPostGap + GIRDER_OVERLAP;
						else
							length = remainLength;

						elemList.Push (girder.placeObject (6,
							"w_ins", APIParT_CString, "�ٴڴ�����",
							"w_w", APIParT_Length, "0.080",
							"w_h", APIParT_Length, "0.080",
							"w_leng", APIParT_Length, format_string ("%f", length),
							"w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
							"torsion_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
						moveIn3D ('y', girder.radAng, -0.080, &girder.posX, &girder.posY, &girder.posZ);
						elemList.Push (girder.placeObject (6,
							"w_ins", APIParT_CString, "�ٴڴ�����",
							"w_w", APIParT_Length, "0.080",
							"w_h", APIParT_Length, "0.080",
							"w_leng", APIParT_Length, format_string ("%f", length),
							"w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
							"torsion_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
						moveIn3D ('y', girder.radAng, 0.080, &girder.posX, &girder.posY, &girder.posZ);

						moveIn3D ('x', girder.radAng, length, &girder.posX, &girder.posY, &girder.posZ);

						remainLength -= placingZone.suppPostGap;
					}

					// ���ٸ� ��ġ !!!
				}
			}
		}
	}

	return	err;
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
			DGSetDialogTitle (dialogID, L"������ �Ϻο� ��ġ");

			//////////////////////////////////////////////////////////// ������ ��ġ
			DGSetItemText (dialogID, DG_OK, L"Ȯ ��");
			DGSetItemText (dialogID, DG_CANCEL, L"�� ��");

			DGSetItemText (dialogID, GROUPBOX_PANEL, L"�ǳ�");
			DGSetItemText (dialogID, LABEL_PANEL_TYPE, L"Ÿ��");
			DGSetItemText (dialogID, LABEL_PANEL_DIRECTION, L"����");
			DGSetItemText (dialogID, LABEL_PANEL_THICKNESS, L"�β�");

			DGSetItemText (dialogID, GROUPBOX_BEAM, L"�弱");
			DGSetItemText (dialogID, LABEL_BEAM_TYPE, L"Ÿ��");
			DGSetItemText (dialogID, LABEL_BEAM_DIRECTION, L"����");
			DGSetItemText (dialogID, LABEL_BEAM_OFFSET_HORIZONTAL, L"������(����)");
			DGSetItemText (dialogID, LABEL_BEAM_OFFSET_VERTICAL, L"������(����)");
			DGSetItemText (dialogID, LABEL_BEAM_GAP, L"����");

			DGSetItemText (dialogID, GROUPBOX_GIRDER, L"�ۿ���");
			DGSetItemText (dialogID, LABEL_GIRDER_TYPE, L"Ÿ��");
			DGSetItemText (dialogID, LABEL_GIRDER_QUANTITY, L"����");
			DGSetItemText (dialogID, LABEL_GIRDER_OFFSET_HORIZONTAL, L"������(����)");
			DGSetItemText (dialogID, LABEL_GIRDER_OFFSET_VERTICAL, L"������(����)");
			DGSetItemText (dialogID, LABEL_GIRDER_GAP, L"����");

			DGSetItemText (dialogID, GROUPBOX_SUPPORTING_POST, L"���ٸ�");
			DGSetItemText (dialogID, LABEL_POST_TYPE, L"Ÿ��");
			DGSetItemText (dialogID, LABEL_POST_GAP, L"����");

			DGSetItemText (dialogID, LABEL_GAP_FROM_SLAB, L"õ����� ����");
			DGSetItemText (dialogID, LABEL_ROOM_HEIGHT, L"õ��-�ٴڰ� �Ÿ�");

			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, L"���纰 ���̾� ����");
			DGSetItemText (dialogID, CHECKBOX_LAYER_COUPLING, L"���̾� ����");
			DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, TRUE);

			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM, L"������");
			DGSetItemText (dialogID, LABEL_LAYER_CONPANEL, L"���ǳ�");
			DGSetItemText (dialogID, LABEL_LAYER_PLYWOOD, L"����");
			DGSetItemText (dialogID, LABEL_LAYER_TIMBER, L"����");
			DGSetItemText (dialogID, LABEL_LAYER_CPROFILE, L"C����");
			DGSetItemText (dialogID, LABEL_LAYER_PINBOLT, L"�ɺ�Ʈ");
			DGSetItemText (dialogID, LABEL_LAYER_FITTINGS, L"����ö��");
			DGSetItemText (dialogID, LABEL_LAYER_GT24_GIRDER, L"GT24 �Ŵ�");
			DGSetItemText (dialogID, LABEL_LAYER_PERI_SUPPORT, L"PERI���ٸ�");
			DGSetItemText (dialogID, LABEL_LAYER_STEEL_SUPPORT, L"���� ���ٸ�");

			DGSetItemText (dialogID, BUTTON_AUTOSET, L"���̾� �ڵ� ����");

			// ���� ��Ʈ�� �ʱ�ȭ
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_LAYER_EUROFORM;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, 1);

			ucb.itemID	 = USERCONTROL_LAYER_CONPANEL;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_CONPANEL, 1);

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

			// �ǳ� ����
			DGPopUpInsertItem (dialogID, POPUP_PANEL_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PANEL_TYPE, DG_POPUP_BOTTOM, L"���ǳ�");
			DGPopUpInsertItem (dialogID, POPUP_PANEL_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PANEL_TYPE, DG_POPUP_BOTTOM, L"����");
			DGPopUpInsertItem (dialogID, POPUP_PANEL_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PANEL_TYPE, DG_POPUP_BOTTOM, L"������");
			DGPopUpSelectItem (dialogID, POPUP_PANEL_TYPE, DG_POPUP_TOP);				// �ǳ� Ÿ�� �⺻��: ���ǳ�

			DGPopUpInsertItem (dialogID, POPUP_PANEL_DIRECTION, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PANEL_DIRECTION, DG_POPUP_BOTTOM, L"����");
			DGPopUpInsertItem (dialogID, POPUP_PANEL_DIRECTION, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PANEL_DIRECTION, DG_POPUP_BOTTOM, L"����");
			DGPopUpSelectItem (dialogID, POPUP_PANEL_DIRECTION, DG_POPUP_TOP);

			DGPopUpInsertItem (dialogID, POPUP_PANEL_THICKNESS, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PANEL_THICKNESS, DG_POPUP_BOTTOM, L"11.5T");
			DGPopUpInsertItem (dialogID, POPUP_PANEL_THICKNESS, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_PANEL_THICKNESS, DG_POPUP_BOTTOM, L"14.5T");

			// �弱 ����
			DGPopUpInsertItem (dialogID, POPUP_BEAM_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_BEAM_TYPE, DG_POPUP_BOTTOM, L"��°�");
			DGPopUpInsertItem (dialogID, POPUP_BEAM_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_BEAM_TYPE, DG_POPUP_BOTTOM, L"������");
			DGPopUpInsertItem (dialogID, POPUP_BEAM_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_BEAM_TYPE, DG_POPUP_BOTTOM, L"GT24 �Ŵ�");
			DGPopUpSelectItem (dialogID, POPUP_BEAM_TYPE, DG_POPUP_TOP);				// �弱 Ÿ�� �⺻��: ��°�

			DGPopUpInsertItem (dialogID, POPUP_BEAM_DIRECTION, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_BEAM_DIRECTION, DG_POPUP_BOTTOM, L"����");
			DGPopUpInsertItem (dialogID, POPUP_BEAM_DIRECTION, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_BEAM_DIRECTION, DG_POPUP_BOTTOM, L"����");
			DGPopUpSelectItem (dialogID, POPUP_BEAM_DIRECTION, DG_POPUP_TOP);

			DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_OFFSET_HORIZONTAL, 0.200);	// �弱 ������(����) �⺻��
			DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_OFFSET_VERTICAL, 0.300);		// �弱 ������(����) �⺻��
			DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_GAP, 0.300);					// �弱 ���� �⺻��

			// �ۿ��� ����
			DGPopUpInsertItem (dialogID, POPUP_GIRDER_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_GIRDER_TYPE, DG_POPUP_BOTTOM, L"GT24 �Ŵ�");
			DGPopUpInsertItem (dialogID, POPUP_GIRDER_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_GIRDER_TYPE, DG_POPUP_BOTTOM, L"��°�");
			DGPopUpSelectItem (dialogID, POPUP_GIRDER_TYPE, DG_POPUP_TOP);				// �ۿ��� Ÿ�� �⺻��: GT24 �Ŵ�

			DGPopUpInsertItem (dialogID, POPUP_GIRDER_QUANTITY, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_GIRDER_QUANTITY, DG_POPUP_BOTTOM, L"1");
			DGPopUpInsertItem (dialogID, POPUP_GIRDER_QUANTITY, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_GIRDER_QUANTITY, DG_POPUP_BOTTOM, L"2");

			DGSetItemValDouble (dialogID, EDITCONTROL_GIRDER_OFFSET_HORIZONTAL, 0.250);	// �ۿ��� ������(����) �⺻��
			DGSetItemValDouble (dialogID, EDITCONTROL_GIRDER_OFFSET_VERTICAL, 0.600);	// �ۿ��� ������(����) �⺻��
			DGSetItemValDouble (dialogID, EDITCONTROL_GIRDER_GAP, 1.500);				// �ۿ��� ���� �⺻��
			DGDisableItem (dialogID, EDITCONTROL_GIRDER_OFFSET_HORIZONTAL);				// �ۿ��� �������� �ʿ� �����Ƿ� ��Ȱ��ȭ
			DGDisableItem (dialogID, EDITCONTROL_GIRDER_OFFSET_VERTICAL);				// �ۿ��� �������� �ʿ� �����Ƿ� ��Ȱ��ȭ

			// ���ٸ� ����
			DGPopUpInsertItem (dialogID, POPUP_POST_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_POST_TYPE, DG_POPUP_BOTTOM, L"PERI ���ٸ�");
			DGPopUpInsertItem (dialogID, POPUP_POST_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_POST_TYPE, DG_POPUP_BOTTOM, L"���� ���ٸ�");
			DGPopUpSelectItem (dialogID, POPUP_POST_TYPE, DG_POPUP_TOP);				// ���ٸ� Ÿ�� �⺻��: PERI ���ٸ�

			DGPopUpInsertItem (dialogID, POPUP_POST_GAP, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_POST_GAP, DG_POPUP_BOTTOM, L"625");
			DGPopUpInsertItem (dialogID, POPUP_POST_GAP, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_POST_GAP, DG_POPUP_BOTTOM, L"750");
			DGPopUpInsertItem (dialogID, POPUP_POST_GAP, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_POST_GAP, DG_POPUP_BOTTOM, L"900");
			DGPopUpInsertItem (dialogID, POPUP_POST_GAP, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_POST_GAP, DG_POPUP_BOTTOM, L"1200");
			DGPopUpInsertItem (dialogID, POPUP_POST_GAP, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_POST_GAP, DG_POPUP_BOTTOM, L"1375");
			DGPopUpInsertItem (dialogID, POPUP_POST_GAP, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_POST_GAP, DG_POPUP_BOTTOM, L"1500");
			DGPopUpInsertItem (dialogID, POPUP_POST_GAP, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_POST_GAP, DG_POPUP_BOTTOM, L"2015");
			DGPopUpInsertItem (dialogID, POPUP_POST_GAP, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_POST_GAP, DG_POPUP_BOTTOM, L"2250");
			DGPopUpInsertItem (dialogID, POPUP_POST_GAP, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_POST_GAP, DG_POPUP_BOTTOM, L"2300");
			DGPopUpInsertItem (dialogID, POPUP_POST_GAP, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_POST_GAP, DG_POPUP_BOTTOM, L"2370");
			DGPopUpInsertItem (dialogID, POPUP_POST_GAP, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_POST_GAP, DG_POPUP_BOTTOM, L"2660");
			DGPopUpInsertItem (dialogID, POPUP_POST_GAP, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_POST_GAP, DG_POPUP_BOTTOM, L"2960");
			DGPopUpSelectItem (dialogID, POPUP_POST_GAP, 6);							// PERI ���ٸ� ���� �⺻��: 1500

			DGSetItemValDouble (dialogID, EDITCONTROL_POST_GAP, 1.500);					// ���� ���ٸ� ���� �⺻��: 1500
			DGHideItem (dialogID, EDITCONTROL_POST_GAP);

			// õ��-�ٴڰ� �Ÿ� ���
			DGSetItemValDouble (dialogID, EDITCONTROL_ROOM_HEIGHT, placingZone.roomHeight);
			DGDisableItem (dialogID, EDITCONTROL_ROOM_HEIGHT);

			// ���̾� Ȱ��ȭ/��Ȱ��ȭ
			DGDisableItem (dialogID, LABEL_LAYER_EUROFORM);
			DGDisableItem (dialogID, LABEL_LAYER_CPROFILE);
			DGDisableItem (dialogID, LABEL_LAYER_PINBOLT);
			DGDisableItem (dialogID, LABEL_LAYER_FITTINGS);
			DGDisableItem (dialogID, LABEL_LAYER_STEEL_SUPPORT);

			DGDisableItem (dialogID, USERCONTROL_LAYER_EUROFORM);
			DGDisableItem (dialogID, USERCONTROL_LAYER_CPROFILE);
			DGDisableItem (dialogID, USERCONTROL_LAYER_PINBOLT);
			DGDisableItem (dialogID, USERCONTROL_LAYER_FITTINGS);
			DGDisableItem (dialogID, USERCONTROL_LAYER_STEEL_SUPPORT);

			break;

		case DG_MSG_CHANGE:
			if (item == POPUP_PANEL_TYPE) {
				// �ǳ� �β� �ɼ��� ������ ���� �޶���
				if (DGPopUpGetSelected (dialogID, POPUP_PANEL_TYPE) == PANEL_TYPE_CONPANEL) {
					DGPopUpDeleteItem (dialogID, POPUP_PANEL_THICKNESS, DG_ALL_ITEMS);
					DGPopUpInsertItem (dialogID, POPUP_PANEL_THICKNESS, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_PANEL_THICKNESS, DG_POPUP_BOTTOM, L"11.5T");
					DGPopUpInsertItem (dialogID, POPUP_PANEL_THICKNESS, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_PANEL_THICKNESS, DG_POPUP_BOTTOM, L"14.5T");
				} else if (DGPopUpGetSelected (dialogID, POPUP_PANEL_TYPE) == PANEL_TYPE_PLYWOOD) {
					DGPopUpDeleteItem (dialogID, POPUP_PANEL_THICKNESS, DG_ALL_ITEMS);
					DGPopUpInsertItem (dialogID, POPUP_PANEL_THICKNESS, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_PANEL_THICKNESS, DG_POPUP_BOTTOM, L"11.5T");
					DGPopUpInsertItem (dialogID, POPUP_PANEL_THICKNESS, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_PANEL_THICKNESS, DG_POPUP_BOTTOM, L"14.5T");
					DGPopUpInsertItem (dialogID, POPUP_PANEL_THICKNESS, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_PANEL_THICKNESS, DG_POPUP_BOTTOM, L"17.5T");
				} else if (DGPopUpGetSelected (dialogID, POPUP_PANEL_TYPE) == PANEL_TYPE_EUROFORM) {
					DGPopUpDeleteItem (dialogID, POPUP_PANEL_THICKNESS, DG_ALL_ITEMS);
					DGPopUpInsertItem (dialogID, POPUP_PANEL_THICKNESS, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_PANEL_THICKNESS, DG_POPUP_BOTTOM, L"63.5T");
				}
			}

			if (item == POPUP_POST_TYPE) {
				// ���ٸ� ������ ���� ���� �Է��ϴ� ��Ʈ���� ������ �ٲ�
				if (DGPopUpGetSelected (dialogID, POPUP_POST_TYPE) == POST_TYPE_PERI_SUPPORT) {
					DGShowItem (dialogID, POPUP_POST_GAP);
					DGHideItem (dialogID, EDITCONTROL_POST_GAP);
				} else {
					DGHideItem (dialogID, POPUP_POST_GAP);
					DGShowItem (dialogID, EDITCONTROL_POST_GAP);
				}
			}

			// ���̾� ���� �ٲ�
			if ((item >= USERCONTROL_LAYER_EUROFORM) && (item <= USERCONTROL_LAYER_STEEL_SUPPORT)) {
				if (DGGetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING) == 1) {
					long selectedLayer;

					selectedLayer = DGGetItemValLong (dialogID, item);

					for (xx = USERCONTROL_LAYER_EUROFORM ; xx <= USERCONTROL_LAYER_STEEL_SUPPORT ; ++xx)
						DGSetItemValLong (dialogID, xx, selectedLayer);
				}
			}

			// ���̾� Ȱ��ȭ/��Ȱ��ȭ
			for (xx = LABEL_LAYER_EUROFORM ; xx <= LABEL_LAYER_STEEL_SUPPORT ; ++xx)
				DGDisableItem (dialogID, xx);
			for (xx = USERCONTROL_LAYER_EUROFORM ; xx <= USERCONTROL_LAYER_STEEL_SUPPORT ; ++xx)
				DGDisableItem (dialogID, xx);

			// ���� ���̾�� �׻� ���� ����
			DGEnableItem (dialogID, LABEL_LAYER_PLYWOOD);
			DGEnableItem (dialogID, USERCONTROL_LAYER_PLYWOOD);

			// ����ڰ� ������ �ǳ��� Ÿ�Կ� ���� ���̾� �Ϻθ� �Ѱų� ��
			if (DGPopUpGetSelected (dialogID, POPUP_PANEL_TYPE) == PANEL_TYPE_CONPANEL) {
				DGEnableItem (dialogID, LABEL_LAYER_CONPANEL);
				DGEnableItem (dialogID, USERCONTROL_LAYER_CONPANEL);
			} else if (DGPopUpGetSelected (dialogID, POPUP_PANEL_TYPE) == PANEL_TYPE_EUROFORM) {
				DGEnableItem (dialogID, LABEL_LAYER_EUROFORM);
				DGEnableItem (dialogID, USERCONTROL_LAYER_EUROFORM);
			}

			// ����ڰ� ������ �弱, �ۿ����� Ÿ�Կ� ���� ���̾� �Ϻθ� �Ѱų� ��
			if ((DGPopUpGetSelected (dialogID, POPUP_BEAM_TYPE) == BEAM_TYPE_SANSEUNGGAK) || (DGPopUpGetSelected (dialogID, POPUP_BEAM_TYPE) == BEAM_TYPE_TUBAI) || (DGPopUpGetSelected (dialogID, POPUP_GIRDER_TYPE) == GIRDER_TYPE_SANSEUNGGAK)) {
				DGEnableItem (dialogID, LABEL_LAYER_TIMBER);
				DGEnableItem (dialogID, USERCONTROL_LAYER_TIMBER);
			}
			if ((DGPopUpGetSelected (dialogID, POPUP_BEAM_TYPE) == BEAM_TYPE_GT24) || (DGPopUpGetSelected (dialogID, POPUP_GIRDER_TYPE) == GIRDER_TYPE_GT24)) {
				DGEnableItem (dialogID, LABEL_LAYER_GT24_GIRDER);
				DGEnableItem (dialogID, USERCONTROL_LAYER_GT24_GIRDER);
			}

			// ����ڰ� ������ ���ٸ��� Ÿ�Կ� ���� ���̾� �Ϻθ� �Ѱų� ��
			if (DGPopUpGetSelected (dialogID, POPUP_POST_TYPE) == POST_TYPE_PERI_SUPPORT) {
				DGEnableItem (dialogID, LABEL_LAYER_PERI_SUPPORT);
				DGEnableItem (dialogID, USERCONTROL_LAYER_PERI_SUPPORT);
			} else if (DGPopUpGetSelected (dialogID, POPUP_POST_TYPE) == POST_TYPE_STEEL_SUPPORT) {
				DGEnableItem (dialogID, LABEL_LAYER_STEEL_SUPPORT);
				DGEnableItem (dialogID, USERCONTROL_LAYER_STEEL_SUPPORT);
			}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					//////////////////////////////////////// ���̾�α� â ������ �Է� ����
					// �ǳ� ����
					placingZone.iPanelType = DGPopUpGetSelected (dialogID, POPUP_PANEL_TYPE);
					placingZone.iPanelDirection = DGPopUpGetSelected (dialogID, POPUP_PANEL_DIRECTION);
					strcpy (placingZone.panelThkStr, DGPopUpGetItemText (dialogID, POPUP_PANEL_THICKNESS, DGPopUpGetSelected (dialogID, POPUP_PANEL_THICKNESS)).ToCStr ());
					placingZone.panelThk = atof (placingZone.panelThkStr) / 1000.0;

					// �弱 ����
					placingZone.iBeamType = DGPopUpGetSelected (dialogID, POPUP_BEAM_TYPE);
					placingZone.iBeamDirection = DGPopUpGetSelected (dialogID, POPUP_BEAM_DIRECTION);
					placingZone.beamOffsetHorizontal = DGGetItemValDouble (dialogID, EDITCONTROL_BEAM_OFFSET_HORIZONTAL);
					placingZone.beamOffsetVertical = DGGetItemValDouble (dialogID, EDITCONTROL_BEAM_OFFSET_VERTICAL);
					placingZone.beamGap = DGGetItemValDouble (dialogID, EDITCONTROL_BEAM_GAP);
					if ((placingZone.iBeamType == BEAM_TYPE_SANSEUNGGAK) || (placingZone.iBeamType == BEAM_TYPE_SANSEUNGGAK))
						placingZone.beamThk = 0.080;
					else if (placingZone.iBeamType == BEAM_TYPE_GT24)
						placingZone.beamThk = 0.240;

					// �ۿ��� ����
					placingZone.iGirderType = DGPopUpGetSelected (dialogID, POPUP_GIRDER_TYPE);
					placingZone.nGirders = (short)atoi (DGPopUpGetItemText (dialogID, POPUP_GIRDER_QUANTITY, DGPopUpGetSelected (dialogID, POPUP_GIRDER_QUANTITY)).ToCStr ());
					placingZone.girderOffsetHorizontal = DGGetItemValDouble (dialogID, EDITCONTROL_GIRDER_OFFSET_HORIZONTAL);
					placingZone.girderOffsetVertical = DGGetItemValDouble (dialogID, EDITCONTROL_GIRDER_OFFSET_VERTICAL);
					placingZone.girderGap = DGGetItemValDouble (dialogID, EDITCONTROL_GIRDER_GAP);
					if (placingZone.iGirderType == GIRDER_TYPE_GT24)
						placingZone.girderThk = 0.240;
					else if (placingZone.iGirderType == GIRDER_TYPE_SANSEUNGGAK)
						placingZone.girderThk = 0.080;

					// ��Ÿ ����
					placingZone.gap = DGGetItemValDouble (dialogID, EDITCONTROL_GAP_FROM_SLAB);
					placingZone.roomHeight = DGGetItemValDouble (dialogID, EDITCONTROL_ROOM_HEIGHT);

					// ���ٸ� ����
					placingZone.iSuppPostType = DGPopUpGetSelected (dialogID, POPUP_POST_TYPE);
					if (placingZone.iSuppPostType == POST_TYPE_PERI_SUPPORT) {
						placingZone.suppPostGap = atof (DGPopUpGetItemText (dialogID, POPUP_POST_GAP, DGPopUpGetSelected (dialogID, POPUP_POST_GAP)).ToCStr ()) / 1000.0;
					} else if (placingZone.iSuppPostType == POST_TYPE_STEEL_SUPPORT) {
						placingZone.suppPostGap = DGGetItemValDouble (dialogID, EDITCONTROL_POST_GAP);
					}
					placingZone.crossHeadThk = 0.003;
					placingZone.postHeight = placingZone.roomHeight - (placingZone.panelThk + placingZone.beamThk + placingZone.girderThk + placingZone.crossHeadThk);

					// ���̾� ��ȣ ����
					layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM);
					layerInd_ConPanel		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_CONPANEL);
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
					layerInd_ConPanel		= makeTemporaryLayer (structuralObject_forTableformSlab, "CONP", NULL);
					layerInd_Plywood		= makeTemporaryLayer (structuralObject_forTableformSlab, "PLYW", NULL);
					layerInd_Timber			= makeTemporaryLayer (structuralObject_forTableformSlab, "TIMB", NULL);
					layerInd_CProfile		= makeTemporaryLayer (structuralObject_forTableformSlab, "CPRO", NULL);
					layerInd_Pinbolt		= makeTemporaryLayer (structuralObject_forTableformSlab, "PINB", NULL);
					layerInd_Fittings		= makeTemporaryLayer (structuralObject_forTableformSlab, "CLAM", NULL);
					layerInd_GT24Girder		= makeTemporaryLayer (structuralObject_forTableformSlab, "GIDR", NULL);
					layerInd_PERI_Support	= makeTemporaryLayer (structuralObject_forTableformSlab, "MULT", NULL);
					layerInd_Steel_Support	= makeTemporaryLayer (structuralObject_forTableformSlab, "SUPT", NULL);

					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, layerInd_Euroform);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_CONPANEL, layerInd_ConPanel);
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
			// placingZone�� Cell ���� �ʱ�ȭ
			placingZone.initCells (&placingZone);

			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, L"������ �Ϻο� ��ġ - ��ġ ����");

			//////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			// ��ġ ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 110, 130, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, L"��ġ - ������ ä���");
			DGShowItem (dialogID, DG_OK);

			// ���� ��ư 1
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 150, 130, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, L"��ġ - ������ ����");
			DGShowItem (dialogID, DG_CANCEL);

			// ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 190, 130, 25);
			DGSetItemFont (dialogID, DG_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_PREV, L"����");
			DGShowItem (dialogID, DG_PREV);

			// �� �߰� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 30, 65, 25);
			DGSetItemFont (dialogID, PUSHBUTTON_ADD_ROW, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, PUSHBUTTON_ADD_ROW, L"�� �߰�");
			DGShowItem (dialogID, PUSHBUTTON_ADD_ROW);

			// �� ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 85, 30, 65, 25);
			DGSetItemFont (dialogID, PUSHBUTTON_DEL_ROW, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, PUSHBUTTON_DEL_ROW, L"�� ����");
			DGShowItem (dialogID, PUSHBUTTON_DEL_ROW);

			// �� �߰� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 70, 65, 25);
			DGSetItemFont (dialogID, PUSHBUTTON_ADD_COL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, PUSHBUTTON_ADD_COL, L"�� �߰�");
			DGShowItem (dialogID, PUSHBUTTON_ADD_COL);
		
			// �� ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 85, 70, 65, 25);
			DGSetItemFont (dialogID, PUSHBUTTON_DEL_COL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, PUSHBUTTON_DEL_COL, L"�� ����");
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
					if (placingZone.cells [xx][yy].objType == PANEL_TYPE_CONPANEL) {
						txtButton = format_string ("���ǳ�\n��%.0f\n��%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
					} else if (placingZone.cells [xx][yy].objType == PANEL_TYPE_PLYWOOD) {
						txtButton = format_string ("����\n��%.0f\n��%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
					} else if (placingZone.cells [xx][yy].objType == PANEL_TYPE_EUROFORM) {
						txtButton = format_string ("������\n��%.0f\n��%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
					} else {
						txtButton = format_string ("����");
					}
					DGSetItemText (dialogID, placingZone.CELL_BUTTON [xx][yy], charToWchar (txtButton.c_str ()));		// �׸��� ��ư �ؽ�Ʈ ����
					DGShowItem (dialogID, placingZone.CELL_BUTTON [xx][yy]);
					btnPosX += btnSizeX;
				}
				btnPosX = btnInitPosX;
				btnPosY -= btnSizeY;
			}

			// ��: ����(��)
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnInitPosX - 60, btnPosY + (placingZone.nVerCells * btnSizeY)/2, 50, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, L"����(��)");
			DGShowItem (dialogID, itmIdx);

			// ��: ����(��)
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnInitPosX + (placingZone.nHorCells * btnSizeX) + 20, btnPosY + (placingZone.nVerCells * btnSizeY)/2, 50, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, L"����(��)");
			DGShowItem (dialogID, itmIdx);

			// ��: ����(��)
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnInitPosX + (placingZone.nHorCells * btnSizeX)/2 - 100, 30, 50, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, L"����(��)");
			DGShowItem (dialogID, itmIdx);

			// ��: ����(��)
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnInitPosX + (placingZone.nHorCells * btnSizeX)/2 - 100, btnPosY + ((placingZone.nVerCells + 2) * btnSizeY), 50, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, L"����(��)");
			DGShowItem (dialogID, itmIdx);

			// Edit ��Ʈ��: ����(��)
			placingZone.EDITCONTROL_MARGIN_LEFT = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnInitPosX - 60, btnPosY + (placingZone.nVerCells * btnSizeY)/2 + 30 - 7, 50, 25);
			DGSetItemFont (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT);
			DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT, placingZone.borderHorLen - placingZone.cellArrayWidth - 0.001);
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT, placingZone.marginLeft);

			// Edit ��Ʈ��: ����(��)
			placingZone.EDITCONTROL_MARGIN_RIGHT = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnInitPosX + (placingZone.nHorCells * btnSizeX) + 20, btnPosY + (placingZone.nVerCells * btnSizeY)/2 + 30 - 7, 50, 25);
			DGSetItemFont (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT);
			DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT, placingZone.borderHorLen - placingZone.cellArrayWidth - 0.001);
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT, placingZone.marginRight);

			// Edit ��Ʈ��: ����(��)
			placingZone.EDITCONTROL_MARGIN_TOP = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnInitPosX + (placingZone.nHorCells * btnSizeX)/2 - 100 + 60, 30 - 7, 50, 25);
			DGSetItemFont (dialogID, placingZone.EDITCONTROL_MARGIN_TOP, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_TOP);
			DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_TOP, placingZone.borderVerLen - placingZone.cellArrayHeight - 0.001);
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_TOP, placingZone.marginTop);

			// Edit ��Ʈ��: ����(��)
			placingZone.EDITCONTROL_MARGIN_BOTTOM = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnInitPosX + (placingZone.nHorCells * btnSizeX)/2 - 100 + 60, btnPosY + ((placingZone.nVerCells + 2) * btnSizeY) - 7, 50, 25);
			DGSetItemFont (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM);
			DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM, placingZone.borderVerLen - placingZone.cellArrayHeight - 0.001);
			DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM, placingZone.marginBottom);

			break;

		case DG_MSG_CHANGE:
			// ����(��) �����, ����(��) �����
			if (item == placingZone.EDITCONTROL_MARGIN_LEFT) {
				placingZone.marginLeft = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT);
				placingZone.marginRight = placingZone.borderHorLen - placingZone.cellArrayWidth - placingZone.marginLeft;
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT, placingZone.marginRight);
			}

			// ����(��) �����, ����(��) �����
			if (item == placingZone.EDITCONTROL_MARGIN_RIGHT) {
				placingZone.marginRight = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT);
				placingZone.marginLeft = placingZone.borderHorLen - placingZone.cellArrayWidth - placingZone.marginRight;
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT, placingZone.marginLeft);
			}

			// ����(��) �����, ����(��) �����
			if (item == placingZone.EDITCONTROL_MARGIN_TOP) {
				placingZone.marginTop = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_TOP);
				placingZone.marginBottom = placingZone.borderVerLen - placingZone.cellArrayHeight - placingZone.marginTop;
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM, placingZone.marginBottom);
			}

			// ����(��) �����, ����(��) �����
			if (item == placingZone.EDITCONTROL_MARGIN_BOTTOM) {
				placingZone.marginBottom = DGGetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM);
				placingZone.marginTop = placingZone.borderVerLen - placingZone.cellArrayHeight - placingZone.marginBottom;
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_TOP, placingZone.marginTop);
			}

			break;

		case DG_MSG_CLICK:
			if (item == DG_OK) {
				// ��ġ - ������ ä��� ��ư
				clickedExcludeRestButton = false;

				// �� �迭 ��ü �ʺ�, ���� ����
				placingZone.cellArrayWidth = 0.0;
				placingZone.cellArrayHeight = 0.0;
				for (xx = 0 ; xx < placingZone.nHorCells ; ++xx)	placingZone.cellArrayWidth += placingZone.cells [0][xx].horLen;
				for (xx = 0 ; xx < placingZone.nVerCells ; ++xx)	placingZone.cellArrayHeight += placingZone.cells [xx][0].verLen;

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

				// �� �迭 ��ü �ʺ�, ���� ����
				placingZone.cellArrayWidth = 0.0;
				placingZone.cellArrayHeight = 0.0;
				for (xx = 0 ; xx < placingZone.nHorCells ; ++xx)	placingZone.cellArrayWidth += placingZone.cells [0][xx].horLen;
				for (xx = 0 ; xx < placingZone.nVerCells ; ++xx)	placingZone.cellArrayHeight += placingZone.cells [xx][0].verLen;

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

				// �� �迭 ��ü �ʺ�, ���� ����
				placingZone.cellArrayWidth = 0.0;
				placingZone.cellArrayHeight = 0.0;
				for (xx = 0 ; xx < placingZone.nHorCells ; ++xx)	placingZone.cellArrayWidth += placingZone.cells [0][xx].horLen;
				for (xx = 0 ; xx < placingZone.nVerCells ; ++xx)	placingZone.cellArrayHeight += placingZone.cells [xx][0].verLen;

				// ���� ���� ����
				placingZone.marginLeft = placingZone.marginRight = (placingZone.borderHorLen - placingZone.cellArrayWidth) / 2;
				placingZone.marginBottom = placingZone.marginTop = (placingZone.borderVerLen - placingZone.cellArrayHeight) / 2;

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
						if (placingZone.cells [xx][yy].objType == PANEL_TYPE_CONPANEL) {
							txtButton = format_string ("���ǳ�\n��%.0f\n��%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
						} else if (placingZone.cells [xx][yy].objType == PANEL_TYPE_PLYWOOD) {
							txtButton = format_string ("����\n��%.0f\n��%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
						} else if (placingZone.cells [xx][yy].objType == PANEL_TYPE_EUROFORM) {
							txtButton = format_string ("������\n��%.0f\n��%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
						} else {
							txtButton = format_string ("����");
						}
						DGSetItemText (dialogID, placingZone.CELL_BUTTON [xx][yy], charToWchar (txtButton.c_str ()));		// �׸��� ��ư �ؽ�Ʈ ����
						DGShowItem (dialogID, placingZone.CELL_BUTTON [xx][yy]);
						btnPosX += btnSizeX;
					}
					btnPosX = btnInitPosX;
					btnPosY -= btnSizeY;
				}

				// ��: ����(��)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnInitPosX - 60, btnPosY + (placingZone.nVerCells * btnSizeY)/2, 50, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"����(��)");
				DGShowItem (dialogID, itmIdx);

				// ��: ����(��)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnInitPosX + (placingZone.nHorCells * btnSizeX) + 20, btnPosY + (placingZone.nVerCells * btnSizeY)/2, 50, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"����(��)");
				DGShowItem (dialogID, itmIdx);

				// ��: ����(��)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnInitPosX + (placingZone.nHorCells * btnSizeX)/2 - 100, 30, 50, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"����(��)");
				DGShowItem (dialogID, itmIdx);

				// ��: ����(��)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnInitPosX + (placingZone.nHorCells * btnSizeX)/2 - 100, btnPosY + ((placingZone.nVerCells + 2) * btnSizeY), 50, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, L"����(��)");
				DGShowItem (dialogID, itmIdx);

				// Edit ��Ʈ��: ����(��)
				placingZone.EDITCONTROL_MARGIN_LEFT = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnInitPosX - 60, btnPosY + (placingZone.nVerCells * btnSizeY)/2 + 30 - 7, 50, 25);
				DGSetItemFont (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT);
				DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT, placingZone.borderHorLen - placingZone.cellArrayWidth - 0.001);
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT, placingZone.marginLeft);

				// Edit ��Ʈ��: ����(��)
				placingZone.EDITCONTROL_MARGIN_RIGHT = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnInitPosX + (placingZone.nHorCells * btnSizeX) + 20, btnPosY + (placingZone.nVerCells * btnSizeY)/2 + 30 - 7, 50, 25);
				DGSetItemFont (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT);
				DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT, placingZone.borderHorLen - placingZone.cellArrayWidth - 0.001);
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_RIGHT, placingZone.marginRight);

				// Edit ��Ʈ��: ����(��)
				placingZone.EDITCONTROL_MARGIN_TOP = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnInitPosX + (placingZone.nHorCells * btnSizeX)/2 - 100 + 60, 30 - 7, 50, 25);
				DGSetItemFont (dialogID, placingZone.EDITCONTROL_MARGIN_TOP, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_TOP);
				DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_TOP, placingZone.borderVerLen - placingZone.cellArrayHeight - 0.001);
				DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_MARGIN_TOP, placingZone.marginTop);

				// Edit ��Ʈ��: ����(��)
				placingZone.EDITCONTROL_MARGIN_BOTTOM = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnInitPosX + (placingZone.nHorCells * btnSizeX)/2 - 100 + 60, btnPosY + ((placingZone.nVerCells + 2) * btnSizeY) - 7, 50, 25);
				DGSetItemFont (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM);
				DGSetItemMaxDouble (dialogID, placingZone.EDITCONTROL_MARGIN_BOTTOM, placingZone.borderVerLen - placingZone.cellArrayHeight - 0.001);
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

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, L"�� �� ����");

			////////////////////////////////////////////////////////////  ������ ��ġ (�⺻ ��ư)
			// ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 215, 70, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, L"����");
			DGShowItem (dialogID, DG_OK);

			// ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 130, 215, 70, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, L"���");
			DGShowItem (dialogID, DG_CANCEL);

			//////////////////////////////////////////////////////////// �ʵ� ���� (Ŭ���� ��)
			// ��: ��ü Ÿ��
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 20, 70, 23);
			DGSetItemFont (dialogID, LABEL_OBJ_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_OBJ_TYPE, L"��ü Ÿ��");
			DGShowItem (dialogID, LABEL_OBJ_TYPE);

			// �˾���Ʈ��: ��ü Ÿ�� (���ǳ�, ����, ������)
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100, 20-7, 120, 25);
			DGSetItemFont (dialogID, POPUP_OBJ_TYPE, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, L"����");
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, L"���ǳ�");
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, L"����");
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, L"������");
			DGShowItem (dialogID, POPUP_OBJ_TYPE);
			if (placingZone.cells [clickedRow][clickedCol].objType == PANEL_TYPE_CONPANEL)
				DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, PANEL_TYPE_CONPANEL+1);
			else if (placingZone.cells [clickedRow][clickedCol].objType == PANEL_TYPE_PLYWOOD)
				DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, PANEL_TYPE_PLYWOOD+1);
			else if (placingZone.cells [clickedRow][clickedCol].objType == PANEL_TYPE_EUROFORM)
				DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, PANEL_TYPE_EUROFORM+1);
			else
				DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, PANEL_TYPE_NONE+1);

			// ��: �ʺ�
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 60, 70, 23);
			DGSetItemFont (dialogID, LABEL_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_WIDTH, L"�ʺ�");
			DGShowItem (dialogID, LABEL_WIDTH);

			// Edit��Ʈ��: �ʺ�
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100, 60-6, 100, 25);
			DGSetItemFont (dialogID, EDITCONTROL_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);

			// �˾���Ʈ��: �ʺ�
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100, 60-6, 100, 25);
			DGSetItemFont (dialogID, POPUP_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);

			if (placingZone.iPanelDirection == HORIZONTAL) {
				if (placingZone.cells [clickedRow][clickedCol].objType == PANEL_TYPE_CONPANEL) {
					DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, L"1820");
					DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, L"1520");
					
					if (placingZone.cells [clickedRow][clickedCol].horLen - 1.820 < EPS)
						DGPopUpSelectItem (dialogID, POPUP_WIDTH, 1);
					else if (placingZone.cells [clickedRow][clickedCol].horLen - 1.520 < EPS)
						DGPopUpSelectItem (dialogID, POPUP_WIDTH, 2);

					DGShowItem (dialogID, POPUP_WIDTH);

				} else if (placingZone.cells [clickedRow][clickedCol].objType == PANEL_TYPE_PLYWOOD) {
					DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH, 2.440);
					DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH, 0.0);
					DGSetItemValDouble (dialogID, EDITCONTROL_WIDTH, placingZone.cells [clickedRow][clickedCol].horLen);
					DGShowItem (dialogID, EDITCONTROL_WIDTH);

				} else if (placingZone.cells [clickedRow][clickedCol].objType == PANEL_TYPE_EUROFORM) {
					DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, L"1200");
					DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, L"900");
					DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, L"600");

					if (placingZone.cells [clickedRow][clickedCol].horLen - 1.200 < EPS)
						DGPopUpSelectItem (dialogID, POPUP_WIDTH, 1);
					else if (placingZone.cells [clickedRow][clickedCol].horLen - 0.900 < EPS)
						DGPopUpSelectItem (dialogID, POPUP_WIDTH, 2);
					else if (placingZone.cells [clickedRow][clickedCol].horLen - 0.600 < EPS)
						DGPopUpSelectItem (dialogID, POPUP_WIDTH, 3);

					DGShowItem (dialogID, POPUP_WIDTH);

				} else {
					DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH, 0.0);
					DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH, 0.0);
					DGSetItemValDouble (dialogID, EDITCONTROL_WIDTH, 0.0);
					DGShowItem (dialogID, EDITCONTROL_WIDTH);
				}
			} else {
				if (placingZone.cells [clickedRow][clickedCol].objType == PANEL_TYPE_CONPANEL) {
					DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, L"910");
					DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, L"606");
					
					if (placingZone.cells [clickedRow][clickedCol].horLen - 0.910 < EPS)
						DGPopUpSelectItem (dialogID, POPUP_WIDTH, 1);
					else if (placingZone.cells [clickedRow][clickedCol].horLen - 0.606 < EPS)
						DGPopUpSelectItem (dialogID, POPUP_WIDTH, 2);

					DGShowItem (dialogID, POPUP_WIDTH);

				} else if (placingZone.cells [clickedRow][clickedCol].objType == PANEL_TYPE_PLYWOOD) {
					DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH, 1.220);
					DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH, 0.0);
					DGSetItemValDouble (dialogID, EDITCONTROL_WIDTH, placingZone.cells [clickedRow][clickedCol].horLen);
					DGShowItem (dialogID, EDITCONTROL_WIDTH);

				} else if (placingZone.cells [clickedRow][clickedCol].objType == PANEL_TYPE_EUROFORM) {
					DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, L"600");
					DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, L"500");
					DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, L"450");
					DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, L"400");
					DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, L"300");
					DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, L"200");

					if (placingZone.cells [clickedRow][clickedCol].horLen - 0.600 < EPS)
						DGPopUpSelectItem (dialogID, POPUP_WIDTH, 1);
					else if (placingZone.cells [clickedRow][clickedCol].horLen - 0.500 < EPS)
						DGPopUpSelectItem (dialogID, POPUP_WIDTH, 2);
					else if (placingZone.cells [clickedRow][clickedCol].horLen - 0.450 < EPS)
						DGPopUpSelectItem (dialogID, POPUP_WIDTH, 3);
					else if (placingZone.cells [clickedRow][clickedCol].horLen - 0.400 < EPS)
						DGPopUpSelectItem (dialogID, POPUP_WIDTH, 4);
					else if (placingZone.cells [clickedRow][clickedCol].horLen - 0.300 < EPS)
						DGPopUpSelectItem (dialogID, POPUP_WIDTH, 5);
					else if (placingZone.cells [clickedRow][clickedCol].horLen - 0.200 < EPS)
						DGPopUpSelectItem (dialogID, POPUP_WIDTH, 6);

					DGShowItem (dialogID, POPUP_WIDTH);

				} else {
					DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH, 0.0);
					DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH, 0.0);
					DGSetItemValDouble (dialogID, EDITCONTROL_WIDTH, 0.0);
					DGShowItem (dialogID, EDITCONTROL_WIDTH);
				}
			}

			// ��: ����
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 100, 70, 23);
			DGSetItemFont (dialogID, LABEL_HEIGHT, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_HEIGHT, L"����");
			DGShowItem (dialogID, LABEL_HEIGHT);

			// Edit��Ʈ��: ����
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100, 100-6, 100, 25);
			DGSetItemFont (dialogID, EDITCONTROL_HEIGHT, DG_IS_LARGE | DG_IS_PLAIN);

			// �˾���Ʈ��: ����
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100, 100-6, 100, 25);
			DGSetItemFont (dialogID, POPUP_HEIGHT, DG_IS_LARGE | DG_IS_PLAIN);

			if (placingZone.iPanelDirection == HORIZONTAL) {
				if (placingZone.cells [clickedRow][clickedCol].objType == PANEL_TYPE_CONPANEL) {
					DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, L"910");
					DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, L"606");
					
					if (placingZone.cells [clickedRow][clickedCol].verLen - 0.910 < EPS)
						DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 1);
					else if (placingZone.cells [clickedRow][clickedCol].verLen - 0.606 < EPS)
						DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 2);

					DGShowItem (dialogID, POPUP_HEIGHT);

				} else if (placingZone.cells [clickedRow][clickedCol].objType == PANEL_TYPE_PLYWOOD) {
					DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT, 1.220);
					DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT, 0.0);
					DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT, placingZone.cells [clickedRow][clickedCol].verLen);
					DGShowItem (dialogID, EDITCONTROL_HEIGHT);

				} else if (placingZone.cells [clickedRow][clickedCol].objType == PANEL_TYPE_EUROFORM) {
					DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, L"600");
					DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, L"500");
					DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, L"450");
					DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, L"400");
					DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, L"300");
					DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, L"200");

					if (placingZone.cells [clickedRow][clickedCol].verLen - 0.600 < EPS)
						DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 1);
					else if (placingZone.cells [clickedRow][clickedCol].verLen - 0.500 < EPS)
						DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 2);
					else if (placingZone.cells [clickedRow][clickedCol].verLen - 0.450 < EPS)
						DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 3);
					else if (placingZone.cells [clickedRow][clickedCol].verLen - 0.400 < EPS)
						DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 4);
					else if (placingZone.cells [clickedRow][clickedCol].verLen - 0.300 < EPS)
						DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 5);
					else if (placingZone.cells [clickedRow][clickedCol].verLen - 0.200 < EPS)
						DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 6);

					DGShowItem (dialogID, POPUP_HEIGHT);

				} else {
					DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT, 0.0);
					DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT, 0.0);
					DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT, 0.0);
					DGShowItem (dialogID, EDITCONTROL_HEIGHT);
				}
			} else {
				if (placingZone.cells [clickedRow][clickedCol].objType == PANEL_TYPE_CONPANEL) {
					DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, L"1820");
					DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, L"1520");
					
					if (placingZone.cells [clickedRow][clickedCol].verLen - 1.820 < EPS)
						DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 1);
					else if (placingZone.cells [clickedRow][clickedCol].verLen - 1.520 < EPS)
						DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 2);

					DGShowItem (dialogID, POPUP_HEIGHT);

				} else if (placingZone.cells [clickedRow][clickedCol].objType == PANEL_TYPE_PLYWOOD) {
					DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT, 2.440);
					DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT, 0.0);
					DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT, placingZone.cells [clickedRow][clickedCol].verLen);
					DGShowItem (dialogID, EDITCONTROL_HEIGHT);

				} else if (placingZone.cells [clickedRow][clickedCol].objType == PANEL_TYPE_EUROFORM) {
					DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, L"1200");
					DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, L"900");
					DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, L"600");

					if (placingZone.cells [clickedRow][clickedCol].verLen - 1.200 < EPS)
						DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 1);
					else if (placingZone.cells [clickedRow][clickedCol].verLen - 0.900 < EPS)
						DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 2);
					else if (placingZone.cells [clickedRow][clickedCol].verLen - 0.600 < EPS)
						DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 3);

					DGShowItem (dialogID, POPUP_HEIGHT);

				} else {
					DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT, 0.0);
					DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT, 0.0);
					DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT, 0.0);
					DGShowItem (dialogID, EDITCONTROL_HEIGHT);
				}
			}

			// ��: ��ġ����
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 140, 70, 23);
			DGSetItemFont (dialogID, LABEL_ORIENTATION, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_ORIENTATION, L"��ġ����");
			DGShowItem (dialogID, LABEL_ORIENTATION);

			// �˾���Ʈ��: ��ġ����
			DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, 100, 140-7, 100, 25);
			DGSetItemFont (dialogID, POPUP_ORIENTATION, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, POPUP_ORIENTATION, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_ORIENTATION, DG_POPUP_BOTTOM, L"���ι���");
			DGPopUpInsertItem (dialogID, POPUP_ORIENTATION, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_ORIENTATION, DG_POPUP_BOTTOM, L"���ι���");
			DGShowItem (dialogID, POPUP_ORIENTATION);
			DGDisableItem (dialogID, POPUP_ORIENTATION);

			if (placingZone.iPanelDirection == HORIZONTAL)
				DGPopUpSelectItem (dialogID, POPUP_ORIENTATION, HORIZONTAL);
			else
				DGPopUpSelectItem (dialogID, POPUP_ORIENTATION, VERTICAL);

			break;

		case DG_MSG_CHANGE:
			switch (item) {
				case POPUP_OBJ_TYPE:
					if (placingZone.iPanelDirection == HORIZONTAL) {
						if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == PANEL_TYPE_CONPANEL+1) {
							DGPopUpDeleteItem (dialogID, POPUP_WIDTH, DG_ALL_ITEMS);
							DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, L"1820");
							DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, L"1520");
					
							if (placingZone.cells [clickedRow][clickedCol].horLen - 1.820 < EPS)
								DGPopUpSelectItem (dialogID, POPUP_WIDTH, 1);
							else if (placingZone.cells [clickedRow][clickedCol].horLen - 1.520 < EPS)
								DGPopUpSelectItem (dialogID, POPUP_WIDTH, 2);

							DGShowItem (dialogID, POPUP_WIDTH);
							DGHideItem (dialogID, EDITCONTROL_WIDTH);

						} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == PANEL_TYPE_PLYWOOD+1) {
							DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH, 2.440);
							DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH, 0.0);
							DGSetItemValDouble (dialogID, EDITCONTROL_WIDTH, placingZone.cells [clickedRow][clickedCol].horLen);
							DGHideItem (dialogID, POPUP_WIDTH);
							DGShowItem (dialogID, EDITCONTROL_WIDTH);

						} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == PANEL_TYPE_EUROFORM+1) {
							DGPopUpDeleteItem (dialogID, POPUP_WIDTH, DG_ALL_ITEMS);
							DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, L"1200");
							DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, L"900");
							DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, L"600");

							if (placingZone.cells [clickedRow][clickedCol].horLen - 1.200 < EPS)
								DGPopUpSelectItem (dialogID, POPUP_WIDTH, 1);
							else if (placingZone.cells [clickedRow][clickedCol].horLen - 0.900 < EPS)
								DGPopUpSelectItem (dialogID, POPUP_WIDTH, 2);
							else if (placingZone.cells [clickedRow][clickedCol].horLen - 0.600 < EPS)
								DGPopUpSelectItem (dialogID, POPUP_WIDTH, 3);

							DGShowItem (dialogID, POPUP_WIDTH);
							DGHideItem (dialogID, EDITCONTROL_WIDTH);

						} else {
							DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH, 0.0);
							DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH, 0.0);
							DGSetItemValDouble (dialogID, EDITCONTROL_WIDTH, 0.0);
							DGHideItem (dialogID, POPUP_WIDTH);
							DGShowItem (dialogID, EDITCONTROL_WIDTH);
						}
					} else {
						if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == PANEL_TYPE_CONPANEL+1) {
							DGPopUpDeleteItem (dialogID, POPUP_WIDTH, DG_ALL_ITEMS);
							DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, L"910");
							DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, L"606");
					
							if (placingZone.cells [clickedRow][clickedCol].horLen - 0.910 < EPS)
								DGPopUpSelectItem (dialogID, POPUP_WIDTH, 1);
							else if (placingZone.cells [clickedRow][clickedCol].horLen - 0.606 < EPS)
								DGPopUpSelectItem (dialogID, POPUP_WIDTH, 2);

							DGShowItem (dialogID, POPUP_WIDTH);
							DGHideItem (dialogID, EDITCONTROL_WIDTH);

						} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == PANEL_TYPE_PLYWOOD+1) {
							DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH, 1.220);
							DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH, 0.0);
							DGSetItemValDouble (dialogID, EDITCONTROL_WIDTH, placingZone.cells [clickedRow][clickedCol].horLen);
							DGHideItem (dialogID, POPUP_WIDTH);
							DGShowItem (dialogID, EDITCONTROL_WIDTH);

						} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == PANEL_TYPE_EUROFORM+1) {
							DGPopUpDeleteItem (dialogID, POPUP_WIDTH, DG_ALL_ITEMS);
							DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, L"600");
							DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, L"500");
							DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, L"450");
							DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, L"400");
							DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, L"300");
							DGPopUpInsertItem (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, POPUP_WIDTH, DG_POPUP_BOTTOM, L"200");

							if (placingZone.cells [clickedRow][clickedCol].horLen - 0.600 < EPS)
								DGPopUpSelectItem (dialogID, POPUP_WIDTH, 1);
							else if (placingZone.cells [clickedRow][clickedCol].horLen - 0.500 < EPS)
								DGPopUpSelectItem (dialogID, POPUP_WIDTH, 2);
							else if (placingZone.cells [clickedRow][clickedCol].horLen - 0.450 < EPS)
								DGPopUpSelectItem (dialogID, POPUP_WIDTH, 3);
							else if (placingZone.cells [clickedRow][clickedCol].horLen - 0.400 < EPS)
								DGPopUpSelectItem (dialogID, POPUP_WIDTH, 4);
							else if (placingZone.cells [clickedRow][clickedCol].horLen - 0.300 < EPS)
								DGPopUpSelectItem (dialogID, POPUP_WIDTH, 5);
							else if (placingZone.cells [clickedRow][clickedCol].horLen - 0.200 < EPS)
								DGPopUpSelectItem (dialogID, POPUP_WIDTH, 6);

							DGShowItem (dialogID, POPUP_WIDTH);
							DGHideItem (dialogID, EDITCONTROL_WIDTH);

						} else {
							DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH, 0.0);
							DGSetItemMinDouble (dialogID, EDITCONTROL_WIDTH, 0.0);
							DGSetItemValDouble (dialogID, EDITCONTROL_WIDTH, 0.0);
							DGHideItem (dialogID, POPUP_WIDTH);
							DGShowItem (dialogID, EDITCONTROL_WIDTH);
						}
					}

					if (placingZone.iPanelDirection == HORIZONTAL) {
						if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == PANEL_TYPE_CONPANEL+1) {
							DGPopUpDeleteItem (dialogID, POPUP_HEIGHT, DG_ALL_ITEMS);
							DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, L"910");
							DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, L"606");
					
							if (placingZone.cells [clickedRow][clickedCol].verLen - 0.910 < EPS)
								DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 1);
							else if (placingZone.cells [clickedRow][clickedCol].verLen - 0.606 < EPS)
								DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 2);

							DGShowItem (dialogID, POPUP_HEIGHT);
							DGHideItem (dialogID, EDITCONTROL_HEIGHT);

						} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == PANEL_TYPE_PLYWOOD+1) {
							DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT, 1.220);
							DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT, 0.0);
							DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT, placingZone.cells [clickedRow][clickedCol].verLen);
							DGHideItem (dialogID, POPUP_HEIGHT);
							DGShowItem (dialogID, EDITCONTROL_HEIGHT);

						} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == PANEL_TYPE_EUROFORM+1) {
							DGPopUpDeleteItem (dialogID, POPUP_HEIGHT, DG_ALL_ITEMS);
							DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, L"600");
							DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, L"500");
							DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, L"450");
							DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, L"400");
							DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, L"300");
							DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, L"200");

							if (placingZone.cells [clickedRow][clickedCol].verLen - 0.600 < EPS)
								DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 1);
							else if (placingZone.cells [clickedRow][clickedCol].verLen - 0.500 < EPS)
								DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 2);
							else if (placingZone.cells [clickedRow][clickedCol].verLen - 0.450 < EPS)
								DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 3);
							else if (placingZone.cells [clickedRow][clickedCol].verLen - 0.400 < EPS)
								DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 4);
							else if (placingZone.cells [clickedRow][clickedCol].verLen - 0.300 < EPS)
								DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 5);
							else if (placingZone.cells [clickedRow][clickedCol].verLen - 0.200 < EPS)
								DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 6);

							DGShowItem (dialogID, POPUP_HEIGHT);
							DGHideItem (dialogID, EDITCONTROL_HEIGHT);

						} else {
							DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT, 0.0);
							DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT, 0.0);
							DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT, 0.0);
							DGHideItem (dialogID, POPUP_HEIGHT);
							DGShowItem (dialogID, EDITCONTROL_HEIGHT);
						}
					} else {
						if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == PANEL_TYPE_CONPANEL+1) {
							DGPopUpDeleteItem (dialogID, POPUP_HEIGHT, DG_ALL_ITEMS);
							DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, L"1820");
							DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, L"1520");
					
							if (placingZone.cells [clickedRow][clickedCol].verLen - 1.820 < EPS)
								DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 1);
							else if (placingZone.cells [clickedRow][clickedCol].verLen - 1.520 < EPS)
								DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 2);

							DGShowItem (dialogID, POPUP_HEIGHT);
							DGHideItem (dialogID, EDITCONTROL_HEIGHT);

						} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == PANEL_TYPE_PLYWOOD+1) {
							DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT, 2.440);
							DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT, 0.0);
							DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT, placingZone.cells [clickedRow][clickedCol].verLen);
							DGHideItem (dialogID, POPUP_HEIGHT);
							DGShowItem (dialogID, EDITCONTROL_HEIGHT);

						} else if (DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) == PANEL_TYPE_EUROFORM+1) {
							DGPopUpDeleteItem (dialogID, POPUP_HEIGHT, DG_ALL_ITEMS);
							DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, L"1200");
							DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, L"900");
							DGPopUpInsertItem (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM);
							DGPopUpSetItemText (dialogID, POPUP_HEIGHT, DG_POPUP_BOTTOM, L"600");

							if (placingZone.cells [clickedRow][clickedCol].verLen - 1.200 < EPS)
								DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 1);
							else if (placingZone.cells [clickedRow][clickedCol].verLen - 0.900 < EPS)
								DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 2);
							else if (placingZone.cells [clickedRow][clickedCol].verLen - 0.600 < EPS)
								DGPopUpSelectItem (dialogID, POPUP_HEIGHT, 3);

							DGShowItem (dialogID, POPUP_HEIGHT);
							DGHideItem (dialogID, EDITCONTROL_HEIGHT);

						} else {
							DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT, 0.0);
							DGSetItemMinDouble (dialogID, EDITCONTROL_HEIGHT, 0.0);
							DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT, 0.0);
							DGHideItem (dialogID, POPUP_HEIGHT);
							DGShowItem (dialogID, EDITCONTROL_HEIGHT);
						}
					}

					break;
			}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					for (xx = 0 ; xx < MAX_IND ; ++xx) {
						// ������ �� ��ȣ�� ���� ��
						placingZone.cells [xx][clickedCol].objType = DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) - 1;		// ��ü Ÿ�� ����
						// ���� ũ�� ������ ����
						if ((DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) - 1) == PANEL_TYPE_CONPANEL) {
							placingZone.cells [xx][clickedCol].horLen = atof (DGPopUpGetItemText (dialogID, POPUP_WIDTH, DGPopUpGetSelected (dialogID, POPUP_WIDTH)).ToCStr ()) / 1000.0;
						} else if ((DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) - 1) == PANEL_TYPE_PLYWOOD) {
							placingZone.cells [xx][clickedCol].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH);
						} else if ((DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) - 1) == PANEL_TYPE_EUROFORM) {
							placingZone.cells [xx][clickedCol].horLen = atof (DGPopUpGetItemText (dialogID, POPUP_WIDTH, DGPopUpGetSelected (dialogID, POPUP_WIDTH)).ToCStr ()) / 1000.0;
						}

						// ������ �� ��ȣ�� ���� ��
						placingZone.cells [clickedRow][xx].objType = DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) - 1;		// ��ü Ÿ�� ����
						// ���� ũ�� ������ ����
						if ((DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) - 1) == PANEL_TYPE_CONPANEL) {
							placingZone.cells [clickedRow][xx].verLen = atof (DGPopUpGetItemText (dialogID, POPUP_HEIGHT, DGPopUpGetSelected (dialogID, POPUP_HEIGHT)).ToCStr ()) / 1000.0;
						} else if ((DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) - 1) == PANEL_TYPE_PLYWOOD) {
							placingZone.cells [clickedRow][xx].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT);
						} else if ((DGPopUpGetSelected (dialogID, POPUP_OBJ_TYPE) - 1) == PANEL_TYPE_EUROFORM) {
							placingZone.cells [clickedRow][xx].verLen = atof (DGPopUpGetItemText (dialogID, POPUP_HEIGHT, DGPopUpGetSelected (dialogID, POPUP_HEIGHT)).ToCStr ()) / 1000.0;
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

// ������ �Ϻ��� ���ݿ� �ܿ��縦 ��ġ��
short DGCALLBACK slabBottomTableformPlacerHandler4_Insulation (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	API_UCCallbackType	ucb;

	switch (message) {
		case DG_MSG_INIT:
			// Ÿ��Ʋ
			DGSetDialogTitle (dialogID, L"�ܿ��� ��ġ");

			// ��
			DGSetItemText (dialogID, LABEL_EXPLANATION_INS, L"�ܿ��� �԰��� �Է��Ͻʽÿ�.");
			DGSetItemText (dialogID, LABEL_INSULATION_THK, L"�β�");
			DGSetItemText (dialogID, LABEL_INS_HORLEN, L"����");
			DGSetItemText (dialogID, LABEL_INS_VERLEN, L"����");

			// üũ�ڽ�
			DGSetItemText (dialogID, CHECKBOX_INS_LIMIT_SIZE, L"����/���� ũ�� ����");
			DGSetItemValLong (dialogID, CHECKBOX_INS_LIMIT_SIZE, TRUE);

			// Edit ��Ʈ��
			DGSetItemValDouble (dialogID, EDITCONTROL_INS_HORLEN, 0.900);
			DGSetItemValDouble (dialogID, EDITCONTROL_INS_VERLEN, 1.800);

			// ���̾�
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_INSULATION_LAYER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_INSULATION_LAYER, 1);

			// ��ư
			DGSetItemText (dialogID, DG_OK, L"Ȯ��");
			DGSetItemText (dialogID, DG_CANCEL, L"���");

			// �β��� �ڵ�
			DGSetItemValDouble (dialogID, EDITCONTROL_INSULATION_THK, placingZone.gap);
			DGDisableItem (dialogID, EDITCONTROL_INSULATION_THK);
 
			break;

		case DG_MSG_CHANGE:
			switch (item) {
				case CHECKBOX_INS_LIMIT_SIZE:
					if (DGGetItemValLong (dialogID, CHECKBOX_INS_LIMIT_SIZE) == TRUE) {
						DGEnableItem (dialogID, EDITCONTROL_INS_HORLEN);
						DGEnableItem (dialogID, EDITCONTROL_INS_VERLEN);
					} else {
						DGDisableItem (dialogID, EDITCONTROL_INS_HORLEN);
						DGDisableItem (dialogID, EDITCONTROL_INS_VERLEN);
					}
					break;
			}
 
			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// ���̾� ���� ����
					insulElem.layerInd = (short)DGGetItemValLong (dialogID, USERCONTROL_INSULATION_LAYER);

					// �β�, ����, ���� ����
					insulElem.thk = DGGetItemValDouble (dialogID, EDITCONTROL_INSULATION_THK);
					insulElem.maxHorLen = DGGetItemValDouble (dialogID, EDITCONTROL_INS_HORLEN);
					insulElem.maxVerLen = DGGetItemValDouble (dialogID, EDITCONTROL_INS_VERLEN);
					if (DGGetItemValLong (dialogID, CHECKBOX_INS_LIMIT_SIZE) == TRUE)
						insulElem.bLimitSize = true;
					else
						insulElem.bLimitSize = false;

					break;
				case DG_CANCEL:
					break;
			}
			break;

		case DG_MSG_CLOSE:
			break;
	}

	result = item;

	return	result;
}