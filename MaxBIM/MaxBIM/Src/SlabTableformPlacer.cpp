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
		DGAlert (DG_ERROR, L"����", L"�ƹ� �͵� �������� �ʾҽ��ϴ�.\n�ʼ� ����: ������ (1��), ������ �Ϻθ� ���� ���� (1��)", "", L"Ȯ��", "", "");
	}

	// �����갡 1���ΰ�?
	if (nSlabs != 1) {
		DGAlert (DG_ERROR, L"����", L"�����긦 1�� �����ؾ� �մϴ�.", "", L"Ȯ��", "", "");
		err = APIERR_GENERAL;
		return err;
	}

	// ������ 1���ΰ�?
	if (nMorphs != 1) {
		DGAlert (DG_ERROR, L"����", L"������ �Ϻθ� ���� ������ 1�� �����ϼž� �մϴ�.", "", L"Ȯ��", "", "");
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
		DGAlert (DG_ERROR, L"����", L"������ ���� ���� �ʽ��ϴ�.", "", L"Ȯ��", "", "");
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
		DGAlert (DG_ERROR, L"����", L"�����￡ ������ ���� ���� Ŭ���߽��ϴ�.", "", L"Ȯ��", "", "");
		return err;
	}

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
	placingZone.level = infoSlab.level + infoSlab.offsetFromTop - infoSlab.thickness - placingZone.gap;
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
	
	// �弱, �ۿ���, ���ٸ� ��ġ !!!
	placingZone.place_Joist_Yoke_SupportingPost ();

	// ����� ��ü �׷�ȭ
	groupElements (elemList);
	groupElements (elemList_Insulation);
	// !!! �弱, �ۿ���, ���ٸ� �׷�ȭ

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
			placingZone->cells [xx][yy].leftBottomX = 0.0;
			placingZone->cells [xx][yy].leftBottomY = 0.0;
			placingZone->cells [xx][yy].leftBottomZ = placingZone->level;
			placingZone->cells [xx][yy].ang = placingZone->ang;
			// �� ���⿡ ���� �޶���
			if (placingZone->iCellDirection == HORIZONTAL) {
				if (placingZone->iTableformType == CONPANEL) {
					// ���ǳ� ũ��: 2x6 [606x1820]
					placingZone->cells [xx][yy].horLen = 1.820;
					placingZone->cells [xx][yy].verLen = 0.606;
				} else if (placingZone->iTableformType == PLYWOOD) {
					// ���� ũ��: 4x8 [1220x2440]
					placingZone->cells [xx][yy].horLen = 2.440;
					placingZone->cells [xx][yy].verLen = 1.220;
				} else if (placingZone->iTableformType == EUROFORM) {
					// ������ ũ��: 600x1200
					placingZone->cells [xx][yy].horLen = 1.200;
					placingZone->cells [xx][yy].verLen = 0.600;
				}
			} else {
				if (placingZone->iTableformType == CONPANEL) {
					// ���ǳ� ũ��: 2x6 [606x1820]
					placingZone->cells [xx][yy].horLen = 0.606;
					placingZone->cells [xx][yy].verLen = 1.820;
				} else if (placingZone->iTableformType == PLYWOOD) {
					// ���� ũ��: 4x8 [1220x2440]
					placingZone->cells [xx][yy].horLen = 1.220;
					placingZone->cells [xx][yy].verLen = 2.440;
				} else if (placingZone->iTableformType == EUROFORM) {
					// ������ ũ��: 600x1200
					placingZone->cells [xx][yy].horLen = 0.600;
					placingZone->cells [xx][yy].verLen = 1.200;
				}
			}
		}
	}

	// �� �ʱ� �ʺ�, ���� ����
	if (placingZone->iCellDirection == HORIZONTAL) {
		if (placingZone->iTableformType == CONPANEL) {
			// ���ǳ� ũ��: 2x6 [606x1820]
			placingZone->initCellHorLen = 1.820;
			placingZone->initCellVerLen = 0.606;
		} else if (placingZone->iTableformType == PLYWOOD) {
			// ���� ũ��: 4x8 [1220x2440]
			placingZone->initCellHorLen = 2.440;
			placingZone->initCellVerLen = 1.220;
		} else if (placingZone->iTableformType == EUROFORM) {
			// ������ ũ��: 600x1200
			placingZone->initCellHorLen = 1.200;
			placingZone->initCellVerLen = 0.600;
		}
	} else {
		if (placingZone->iTableformType == CONPANEL) {
			// ���ǳ� ũ��: 2x6 [606x1820]
			placingZone->initCellHorLen = 0.606;
			placingZone->initCellVerLen = 1.820;
		} else if (placingZone->iTableformType == PLYWOOD) {
			// ���� ũ��: 4x8 [1220x2440]
			placingZone->initCellHorLen = 1.220;
			placingZone->initCellVerLen = 2.440;
		} else if (placingZone->iTableformType == EUROFORM) {
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
	placingZone->marginLeft = placingZone->marginRight = (placingZone->borderHorLen - placingZone->cellArrayWidth) / 2;
	placingZone->marginBottom = placingZone->marginTop = (placingZone->borderVerLen - placingZone->cellArrayHeight) / 2;
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

	for (xx = 0 ; xx < placingZone.nVerCells ; ++xx) {
		for (yy = 0 ; yy < placingZone.nHorCells ; ++yy) {
			if (placingZone.cells [xx][yy].objType == CONPANEL) {
				conpanel.init (L("����v1.0.gsm"), layerInd_ConPanel, infoSlab.floorInd, placingZone.cells [xx][yy].leftBottomX, placingZone.cells [xx][yy].leftBottomY, placingZone.cells [xx][yy].leftBottomZ, placingZone.cells [xx][yy].ang);

				if (placingZone.iCellDirection == VERTICAL) {
					conpanel.radAng -= DegreeToRad (90.0);
					elemList.Push (conpanel.placeObject (9,
						"g_comp", APIParT_CString, "���ǳ�",
						"p_stan", APIParT_CString, "2x6 [606x1820]",
						"w_dir", APIParT_CString, "�ٴڱ��",
						"p_thk", APIParT_CString, "11.5T",
						"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"p_ang_alter", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"p_rot", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"A", APIParT_Length, "0.606",
						"B", APIParT_Length, "1.820"));
				} else {
					moveIn3D ('y', conpanel.radAng, -placingZone.cells [xx][yy].verLen, &conpanel.posX, &conpanel.posY, &conpanel.posZ);
					elemList.Push (conpanel.placeObject (9,
						"g_comp", APIParT_CString, "���ǳ�",
						"p_stan", APIParT_CString, "2x6 [606x1820]",
						"w_dir", APIParT_CString, "�ٴڱ��",
						"p_thk", APIParT_CString, "11.5T",
						"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"p_ang_alter", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"p_rot", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"A", APIParT_Length, "0.606",
						"B", APIParT_Length, "1.820"));
				}
			} else if (placingZone.cells [xx][yy].objType == PLYWOOD) {
				plywood.init (L("����v1.0.gsm"), layerInd_ConPanel, infoSlab.floorInd, placingZone.cells [xx][yy].leftBottomX, placingZone.cells [xx][yy].leftBottomY, placingZone.cells [xx][yy].leftBottomZ, placingZone.cells [xx][yy].ang);

				if (placingZone.iCellDirection == VERTICAL) {
					plywood.radAng -= DegreeToRad (90.0);
					elemList.Push (plywood.placeObject (12,
						"g_comp", APIParT_CString, "����",
						"p_stan", APIParT_CString, "��԰�",
						"w_dir", APIParT_CString, "�ٴڱ��",
						"p_thk", APIParT_CString, "11.5T",
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
						"p_thk", APIParT_CString, "11.5T",
						"p_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"p_ang_alter", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"p_rot", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
						"sogak", APIParT_Boolean, "0.0",
						"p_wid", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].verLen),
						"p_leng", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].horLen),
						"A", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].verLen),
						"B", APIParT_Length, format_string ("%f", placingZone.cells [xx][yy].horLen)));
				}
			} else if (placingZone.cells [xx][yy].objType == EUROFORM) {
				euroform.init (L("������v2.0.gsm"), layerInd_Euroform, infoSlab.floorInd, placingZone.cells [xx][yy].leftBottomX, placingZone.cells [xx][yy].leftBottomY, placingZone.cells [xx][yy].leftBottomZ, placingZone.cells [xx][yy].ang);

				if (placingZone.iCellDirection == VERTICAL) {
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

	// ���� ��ġ (BOTTOM)
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

	// ���� ��ġ (LEFT)
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

	// ���� ��ġ (RIGHT)
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
			moveIn3D ('y', insul.radAng, -placingZone.borderHorLen, &insul.posX, &insul.posY, &insul.posZ);
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

// �弱, �ۿ���, ���ٸ� ��ġ
GSErrCode	SlabTableformPlacingZone::place_Joist_Yoke_SupportingPost (void)
{
	GSErrCode	err = NoError;
	EasyObjectPlacement	timber, yoke, supp;
	double	remainLength, length;
	bool	bShifted;

	// �弱 ��ġ
	if (placingZone.iJoistDirection == HORIZONTAL) {
		// ���� ����
		timber.init (L("����v1.0.gsm"), layerInd_Timber, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
		moveIn3D ('z', timber.radAng, -0.0115, &timber.posX, &timber.posY, &timber.posZ);
		moveIn3D ('x', timber.radAng, placingZone.marginLeft / 2, &timber.posX, &timber.posY, &timber.posZ);
		moveIn3D ('y', timber.radAng, -placingZone.marginBottom - 0.160, &timber.posX, &timber.posY, &timber.posZ);
		
		bShifted = false;
		remainLength = placingZone.borderHorLen - placingZone.marginLeft / 2 - placingZone.marginRight / 2;
		while (remainLength > EPS) {
			if (remainLength > 3.600)
				length = 3.600;
			else
				length = remainLength;

			timber.placeObject (6,
				"w_ins", APIParT_CString, "�ٴڴ�����",
				"w_w", APIParT_Length, "0.080",
				"w_h", APIParT_Length, "0.080",
				"w_leng", APIParT_Length, format_string ("%f", length),
				"w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"torsion_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)));
			
			if (length > 1.000)
				moveIn3D ('x', timber.radAng, length - 1.000, &timber.posX, &timber.posY, &timber.posZ);
			else
				moveIn3D ('x', timber.radAng, length, &timber.posX, &timber.posY, &timber.posZ);

			if (bShifted == false) {
				moveIn3D ('y', timber.radAng, 0.080, &timber.posX, &timber.posY, &timber.posZ);
				bShifted = true;
			} else {
				moveIn3D ('y', timber.radAng, -0.080, &timber.posX, &timber.posY, &timber.posZ);
				bShifted = false;
			}

			remainLength -= 2.600;
		}
	} else {
		// ���� ����
		timber.init (L("����v1.0.gsm"), layerInd_Timber, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
		moveIn3D ('z', timber.radAng, -0.0115, &timber.posX, &timber.posY, &timber.posZ);
		moveIn3D ('y', timber.radAng, -placingZone.marginBottom / 2, &timber.posX, &timber.posY, &timber.posZ);
		moveIn3D ('x', timber.radAng, placingZone.marginLeft + 0.160, &timber.posX, &timber.posY, &timber.posZ);
		
		bShifted = false;
		remainLength = placingZone.borderVerLen - placingZone.marginBottom / 2 - placingZone.marginTop / 2;
		while (remainLength > EPS) {
			if (remainLength > 3.600)
				length = 3.600;
			else
				length = remainLength;

			timber.radAng -= DegreeToRad (90.0);
			timber.placeObject (6,
				"w_ins", APIParT_CString, "�ٴڴ�����",
				"w_w", APIParT_Length, "0.080",
				"w_h", APIParT_Length, "0.080",
				"w_leng", APIParT_Length, format_string ("%f", length),
				"w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"torsion_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)));
			timber.radAng += DegreeToRad (90.0);
			
			if (length > 1.000)
				moveIn3D ('y', timber.radAng, -(length - 1.000), &timber.posX, &timber.posY, &timber.posZ);
			else
				moveIn3D ('y', timber.radAng, -length, &timber.posX, &timber.posY, &timber.posZ);

			if (bShifted == false) {
				moveIn3D ('x', timber.radAng, 0.080, &timber.posX, &timber.posY, &timber.posZ);
				bShifted = true;
			} else {
				moveIn3D ('x', timber.radAng, -0.080, &timber.posX, &timber.posY, &timber.posZ);
				bShifted = false;
			}

			remainLength -= 2.600;
		}
	}

	// �ۿ��� ��ġ
	if (placingZone.iYokeType == GT24_GIRDER) {
		// GT24 �Ŵ�
		yoke.init (L("GT24 �Ŵ� v1.0.gsm"), layerInd_GT24Girder, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
		moveIn3D ('z', yoke.radAng, -0.0115 - 0.240 - 0.080, &yoke.posX, &yoke.posY, &yoke.posZ);

		if (placingZone.iJoistDirection == HORIZONTAL) {
			// �弱�� ���� ����, �ۿ����� ���� ����
			moveIn3D ('x', yoke.radAng, placingZone.marginLeft + 0.160, &yoke.posX, &yoke.posY, &yoke.posZ);
			moveIn3D ('y', yoke.radAng, -placingZone.marginBottom, &yoke.posX, &yoke.posY, &yoke.posZ);
			yoke.radAng -= DegreeToRad (90.0);
			yoke.placeObject (6,
				"type", APIParT_CString, "3000",
				"length", APIParT_Length, format_string ("%f", 2.990),
				"change_rot_method", APIParT_Boolean, "0.0",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"bWood", APIParT_Boolean, "0.0");
			yoke.radAng += DegreeToRad (90.0);
		} else {
			// �弱�� ���� ����, �ۿ����� ���� ����
			moveIn3D ('y', yoke.radAng, -placingZone.marginBottom - 0.160, &yoke.posX, &yoke.posY, &yoke.posZ);
			moveIn3D ('x', yoke.radAng, placingZone.marginLeft, &yoke.posX, &yoke.posY, &yoke.posZ);
			yoke.placeObject (6,
				"type", APIParT_CString, "3000",
				"length", APIParT_Length, format_string ("%f", 2.990),
				"change_rot_method", APIParT_Boolean, "0.0",
				"angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"bWood", APIParT_Boolean, "0.0");
		}
	} else if (placingZone.iYokeType == SANSUNGAK) {
		// ��°�
		timber.init (L("����v1.0.gsm"), layerInd_Timber, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
		moveIn3D ('z', timber.radAng, -0.0115 - 0.080, &timber.posX, &timber.posY, &timber.posZ);

		if (placingZone.iJoistDirection == HORIZONTAL) {
			// �弱�� ���� ����, �ۿ����� ���� ����
			moveIn3D ('x', timber.radAng, placingZone.marginLeft + 0.160 + 0.040, &timber.posX, &timber.posY, &timber.posZ);
			moveIn3D ('y', timber.radAng, -placingZone.marginBottom, &timber.posX, &timber.posY, &timber.posZ);
			timber.radAng -= DegreeToRad (90.0);
			timber.placeObject (6,
				"w_ins", APIParT_CString, "�ٴڴ�����",
				"w_w", APIParT_Length, "0.080",
				"w_h", APIParT_Length, "0.080",
				"w_leng", APIParT_Length, format_string ("%f", 3.000),
				"w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"torsion_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)));
			timber.radAng += DegreeToRad (90.0);
		} else {
			// �弱�� ���� ����, �ۿ����� ���� ����
			moveIn3D ('y', timber.radAng, -placingZone.marginBottom - 0.160 - 0.040, &timber.posX, &timber.posY, &timber.posZ);
			moveIn3D ('x', timber.radAng, placingZone.marginLeft, &timber.posX, &timber.posY, &timber.posZ);
			timber.placeObject (6,
				"w_ins", APIParT_CString, "�ٴڴ�����",
				"w_w", APIParT_Length, "0.080",
				"w_h", APIParT_Length, "0.080",
				"w_leng", APIParT_Length, format_string ("%f", 3.000),
				"w_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"torsion_ang", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)));
		}
	}

	// ���ٸ� ��ġ
	if (placingZone.iSuppPostType == SUPPORT) {
		// ���� ���ٸ�
		supp.init (L("����Ʈv1.0.gsm"), layerInd_Steel_Support, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
		moveIn3D ('z', supp.radAng, -2 - 0.0115 - 0.080, &supp.posX, &supp.posY, &supp.posZ);

		if (placingZone.iYokeType == GT24_GIRDER) {
			moveIn3D ('z', supp.radAng, -0.240 - 0.003, &supp.posX, &supp.posY, &supp.posZ);
		} else if (placingZone.iYokeType == SANSUNGAK) {
			moveIn3D ('z', supp.radAng, -0.080 - 0.003, &supp.posX, &supp.posY, &supp.posZ);
		}

		if (placingZone.iJoistDirection == HORIZONTAL) {
			moveIn3D ('x', supp.radAng, placingZone.marginLeft + 0.160 + 0.040, &supp.posX, &supp.posY, &supp.posZ);
			moveIn3D ('y', supp.radAng, -placingZone.marginBottom - 0.200, &supp.posX, &supp.posY, &supp.posZ);
			supp.placeObject (7,
				"s_bimj", APIParT_Boolean, "0.0",
				"s_stan", APIParT_CString, "V0 (2.0m)",
				"s_leng", APIParT_Length, "2.000",
				"s_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
				"bCrosshead", APIParT_Boolean, "1.0",
				"crossheadType", APIParT_CString, "PERI",
				"angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)));
		} else {
			moveIn3D ('y', supp.radAng, -placingZone.marginBottom - 0.160 - 0.040, &supp.posX, &supp.posY, &supp.posZ);
			moveIn3D ('x', supp.radAng, placingZone.marginLeft + 0.200, &supp.posX, &supp.posY, &supp.posZ);
			supp.placeObject (7,
				"s_bimj", APIParT_Boolean, "0.0",
				"s_stan", APIParT_CString, "V0 (2.0m)",
				"s_leng", APIParT_Length, "2.000",
				"s_ang", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
				"bCrosshead", APIParT_Boolean, "1.0",
				"crossheadType", APIParT_CString, "PERI",
				"angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)));
		}
	} else if (placingZone.iSuppPostType == PERI) {
		// PERI ���ٸ�
		supp.init (L("PERI���ٸ� ������ v0.1.gsm"), layerInd_PERI_Support, infoSlab.floorInd, placingZone.leftBottom.x, placingZone.leftBottom.y, placingZone.leftBottom.z, placingZone.ang);
		moveIn3D ('z', supp.radAng, -0.0115 - 0.080, &supp.posX, &supp.posY, &supp.posZ);

		if (placingZone.iYokeType == GT24_GIRDER) {
			moveIn3D ('z', supp.radAng, -0.240 - 0.003, &supp.posX, &supp.posY, &supp.posZ);
		} else if (placingZone.iYokeType == SANSUNGAK) {
			moveIn3D ('z', supp.radAng, -0.080 - 0.003, &supp.posX, &supp.posY, &supp.posZ);
		}

		if (placingZone.iJoistDirection == HORIZONTAL) {
			moveIn3D ('x', supp.radAng, placingZone.marginLeft + 0.160 + 0.040, &supp.posX, &supp.posY, &supp.posZ);
			moveIn3D ('y', supp.radAng, -placingZone.marginBottom - 0.200, &supp.posX, &supp.posY, &supp.posZ);
			supp.placeObject (9,
				"stType", APIParT_CString, "MP 350",
				"bCrosshead", APIParT_Boolean, "1.0",
				"posCrosshead", APIParT_CString, "�ϴ�",
				"crossheadType", APIParT_CString, "PERI",
				"angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
				"len_current", APIParT_Length, "2.000",
				"Z", APIParT_Length, "2.000",
				"pos_lever", APIParT_Length, "1.750",
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)));
		} else {
			moveIn3D ('y', supp.radAng, -placingZone.marginBottom - 0.160 - 0.040, &supp.posX, &supp.posY, &supp.posZ);
			moveIn3D ('x', supp.radAng, placingZone.marginLeft + 0.200, &supp.posX, &supp.posY, &supp.posZ);
			supp.placeObject (9,
				"stType", APIParT_CString, "MP 350",
				"bCrosshead", APIParT_Boolean, "1.0",
				"posCrosshead", APIParT_CString, "�ϴ�",
				"crossheadType", APIParT_CString, "PERI",
				"angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
				"len_current", APIParT_Length, "2.000",
				"Z", APIParT_Length, "2.000",
				"pos_lever", APIParT_Length, "1.750",
				"angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)));
		}
	}

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
			DGSetDialogTitle (dialogID, L"������ �Ϻο� ��ġ");

			//////////////////////////////////////////////////////////// ������ ��ġ
			DGSetItemText (dialogID, DG_OK, L"Ȯ ��");
			DGSetItemText (dialogID, DG_CANCEL, L"�� ��");

			DGSetItemText (dialogID, LABEL_SELECT_TYPE, L"Ÿ�� ����");

			DGSetItemText (dialogID, PUSHRADIO_CONPANEL, L"���ǳ�");
			DGSetItemText (dialogID, PUSHRADIO_PLYWOOD, L"����");
			DGSetItemText (dialogID, PUSHRADIO_EUROFORM, L"������");

			DGSetItemText (dialogID, LABEL_OTHER_SETTINGS, L"��Ÿ ����");
			DGSetItemText (dialogID, LABEL_CELL_DIRECTION, L"�ǳ�(��)����");
			DGSetItemText (dialogID, LABEL_JOIST_DIRECTION, L"�弱����");
			DGSetItemText (dialogID, LABEL_YOKE_TYPE, L"�ۿ��� ����");
			DGSetItemText (dialogID, LABEL_SUPPORTING_POST_TYPE, L"���ٸ� ����");
			DGSetItemText (dialogID, LABEL_GAP_LENGTH, L"��������� ����");

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

			// 1��° Ÿ�� ����
			DGSetItemValLong (dialogID, PUSHRADIO_CONPANEL, TRUE);

			// ������ �߰�
			DGPopUpInsertItem (dialogID, POPUP_CELL_DIRECTION, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_CELL_DIRECTION, DG_POPUP_BOTTOM, L"����");
			DGPopUpInsertItem (dialogID, POPUP_CELL_DIRECTION, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_CELL_DIRECTION, DG_POPUP_BOTTOM, L"����");

			// �弱���� �߰�
			DGPopUpInsertItem (dialogID, POPUP_JOIST_DIRECTION, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_JOIST_DIRECTION, DG_POPUP_BOTTOM, L"����");
			DGPopUpInsertItem (dialogID, POPUP_JOIST_DIRECTION, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_JOIST_DIRECTION, DG_POPUP_BOTTOM, L"����");

			// �ۿ��� ���� �߰�
			DGPopUpInsertItem (dialogID, POPUP_YOKE_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_YOKE_TYPE, DG_POPUP_BOTTOM, L"GT24 �Ŵ�");
			DGPopUpInsertItem (dialogID, POPUP_YOKE_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_YOKE_TYPE, DG_POPUP_BOTTOM, L"��°�");

			// ���ٸ� ���� �߰�
			DGPopUpInsertItem (dialogID, POPUP_SUPPORTING_POST_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_SUPPORTING_POST_TYPE, DG_POPUP_BOTTOM, L"���� ���ٸ�");
			DGPopUpInsertItem (dialogID, POPUP_SUPPORTING_POST_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_SUPPORTING_POST_TYPE, DG_POPUP_BOTTOM, L"PERI ���ٸ�");

			// ���̾� Ȱ��ȭ/��Ȱ��ȭ
			DGDisableItem (dialogID, LABEL_LAYER_EUROFORM);
			DGDisableItem (dialogID, LABEL_LAYER_CPROFILE);
			DGDisableItem (dialogID, LABEL_LAYER_PINBOLT);
			DGDisableItem (dialogID, LABEL_LAYER_FITTINGS);
			DGDisableItem (dialogID, LABEL_LAYER_PERI_SUPPORT);

			DGDisableItem (dialogID, USERCONTROL_LAYER_EUROFORM);
			DGDisableItem (dialogID, USERCONTROL_LAYER_CPROFILE);
			DGDisableItem (dialogID, USERCONTROL_LAYER_PINBOLT);
			DGDisableItem (dialogID, USERCONTROL_LAYER_FITTINGS);
			DGDisableItem (dialogID, USERCONTROL_LAYER_PERI_SUPPORT);

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

			// ���̾� Ȱ��ȭ/��Ȱ��ȭ
			for (xx = LABEL_LAYER_EUROFORM ; xx <= LABEL_LAYER_STEEL_SUPPORT ; ++xx)
				DGDisableItem (dialogID, xx);
			for (xx = USERCONTROL_LAYER_EUROFORM ; xx <= USERCONTROL_LAYER_STEEL_SUPPORT ; ++xx)
				DGDisableItem (dialogID, xx);

			if (DGGetItemValLong (dialogID, PUSHRADIO_CONPANEL) == TRUE) {
				DGEnableItem (dialogID, LABEL_LAYER_CONPANEL);
				DGEnableItem (dialogID, LABEL_LAYER_PLYWOOD);
				DGEnableItem (dialogID, LABEL_LAYER_TIMBER);

				DGEnableItem (dialogID, USERCONTROL_LAYER_CONPANEL);
				DGEnableItem (dialogID, USERCONTROL_LAYER_PLYWOOD);
				DGEnableItem (dialogID, USERCONTROL_LAYER_TIMBER);
			}
			if (DGGetItemValLong (dialogID, PUSHRADIO_PLYWOOD) == TRUE) {
				DGEnableItem (dialogID, LABEL_LAYER_PLYWOOD);
				DGEnableItem (dialogID, LABEL_LAYER_TIMBER);

				DGEnableItem (dialogID, USERCONTROL_LAYER_PLYWOOD);
				DGEnableItem (dialogID, USERCONTROL_LAYER_TIMBER);
			}
			if (DGGetItemValLong (dialogID, PUSHRADIO_EUROFORM) == TRUE) {
				DGEnableItem (dialogID, LABEL_LAYER_EUROFORM);
				DGEnableItem (dialogID, LABEL_LAYER_PLYWOOD);
				DGEnableItem (dialogID, LABEL_LAYER_TIMBER);

				DGEnableItem (dialogID, USERCONTROL_LAYER_EUROFORM);
				DGEnableItem (dialogID, USERCONTROL_LAYER_PLYWOOD);
				DGEnableItem (dialogID, USERCONTROL_LAYER_TIMBER);
			}

			if (DGPopUpGetSelected (dialogID, POPUP_YOKE_TYPE) == GT24_GIRDER) {
				DGEnableItem (dialogID, LABEL_LAYER_GT24_GIRDER);
				DGEnableItem (dialogID, USERCONTROL_LAYER_GT24_GIRDER);
			}

			if (DGPopUpGetSelected (dialogID, POPUP_SUPPORTING_POST_TYPE) == SUPPORT) {
				DGEnableItem (dialogID, LABEL_LAYER_STEEL_SUPPORT);
				DGEnableItem (dialogID, USERCONTROL_LAYER_STEEL_SUPPORT);
			} else if (DGPopUpGetSelected (dialogID, POPUP_SUPPORTING_POST_TYPE) == PERI) {
				DGEnableItem (dialogID, LABEL_LAYER_PERI_SUPPORT);
				DGEnableItem (dialogID, USERCONTROL_LAYER_PERI_SUPPORT);
			}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					//////////////////////////////////////// ���̾�α� â ������ �Է� ����
					// Ÿ�� ����
					if (DGGetItemValLong (dialogID, PUSHRADIO_CONPANEL) == TRUE)
						placingZone.iTableformType = CONPANEL;
					else if (DGGetItemValLong (dialogID, PUSHRADIO_PLYWOOD) == TRUE)
						placingZone.iTableformType = PLYWOOD;
					else if (DGGetItemValLong (dialogID, PUSHRADIO_EUROFORM) == TRUE)
						placingZone.iTableformType = EUROFORM;
					else
						placingZone.iTableformType = NONE;

					// �� ����
					placingZone.iCellDirection = DGPopUpGetSelected (dialogID, POPUP_CELL_DIRECTION);

					// �弱 ����
					placingZone.iJoistDirection = DGPopUpGetSelected (dialogID, POPUP_JOIST_DIRECTION);

					// �ۿ��� ����
					placingZone.iYokeType = DGPopUpGetSelected (dialogID, POPUP_YOKE_TYPE);

					// ���ٸ� ����
					placingZone.iSuppPostType = DGPopUpGetSelected (dialogID, POPUP_SUPPORTING_POST_TYPE);

					// ��������� ���ݰ� ����
					placingZone.gap = DGGetItemValDouble (dialogID, EDITCONTROL_GAP_LENGTH);

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
					if (placingZone.iTableformType == CONPANEL) {
						txtButton = format_string ("���ǳ�\n��%.0f\n��%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
					} else if (placingZone.iTableformType == PLYWOOD) {
						txtButton = format_string ("����\n��%.0f\n��%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
					} else if (placingZone.iTableformType == EUROFORM) {
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
						if (placingZone.iTableformType == CONPANEL) {
							txtButton = format_string ("���ǳ�\n��%.0f\n��%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
						} else if (placingZone.iTableformType == PLYWOOD) {
							txtButton = format_string ("����\n��%.0f\n��%.0f", placingZone.cells [xx][yy].horLen * 1000, placingZone.cells [xx][yy].verLen * 1000);
						} else if (placingZone.iTableformType == EUROFORM) {
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
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, L"���ǳ�");
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, L"����");
			DGPopUpInsertItem (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_OBJ_TYPE, DG_POPUP_BOTTOM, L"������");
			DGShowItem (dialogID, POPUP_OBJ_TYPE);
			if (placingZone.cells [clickedRow][clickedCol].objType == CONPANEL)
				DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, CONPANEL);
			else if (placingZone.cells [clickedRow][clickedCol].objType == PLYWOOD)
				DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, PLYWOOD);
			else if (placingZone.cells [clickedRow][clickedCol].objType == EUROFORM)
				DGPopUpSelectItem (dialogID, POPUP_OBJ_TYPE, EUROFORM);
			DGDisableItem (dialogID, POPUP_OBJ_TYPE);

			// ��: �ʺ�
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 60, 70, 23);
			DGSetItemFont (dialogID, LABEL_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_WIDTH, L"�ʺ�");
			DGShowItem (dialogID, LABEL_WIDTH);

			// Edit��Ʈ��: �ʺ�
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100, 60-6, 100, 25);
			DGSetItemFont (dialogID, EDITCONTROL_WIDTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, EDITCONTROL_WIDTH);
			DGSetItemValDouble (dialogID, EDITCONTROL_WIDTH, placingZone.cells [clickedRow][clickedCol].horLen);
			if (placingZone.cells [clickedRow][clickedCol].objType == CONPANEL) {
				// ���ǳ� ũ��: 2x6 [606x1820] - ũ�� ����
				DGDisableItem (dialogID, EDITCONTROL_WIDTH);
			} else if (placingZone.cells [clickedRow][clickedCol].objType == PLYWOOD) {
				DGEnableItem (dialogID, EDITCONTROL_WIDTH);
				// ���� ũ��: 4x8 [1220x2440]
				if (placingZone.iCellDirection == VERTICAL)
					DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH, 1.220);
				else
					DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH, 2.440);
			} else if (placingZone.cells [clickedRow][clickedCol].objType == EUROFORM) {
				DGEnableItem (dialogID, EDITCONTROL_WIDTH);
				// ������ ũ��: 600x1200 (�ִ� 900x1500)
				if (placingZone.iCellDirection == VERTICAL)
					DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH, 0.900);
				else
					DGSetItemMaxDouble (dialogID, EDITCONTROL_WIDTH, 1.500);
			}

			// ��: ����
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, 20, 100, 70, 23);
			DGSetItemFont (dialogID, LABEL_HEIGHT, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_HEIGHT, L"����");
			DGShowItem (dialogID, LABEL_HEIGHT);

			// Edit��Ʈ��: ����
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 100, 100-6, 100, 25);
			DGSetItemFont (dialogID, EDITCONTROL_HEIGHT, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, EDITCONTROL_HEIGHT);
			DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT, placingZone.cells [clickedRow][clickedCol].verLen);
			if (placingZone.cells [clickedRow][clickedCol].objType == CONPANEL) {
				// ���ǳ� ũ��: 2x6 [606x1820] - ũ�� ����
				DGDisableItem (dialogID, EDITCONTROL_HEIGHT);
			} else if (placingZone.cells [clickedRow][clickedCol].objType == PLYWOOD) {
				DGEnableItem (dialogID, EDITCONTROL_HEIGHT);
				// ���� ũ��: 4x8 [1220x2440]
				if (placingZone.iCellDirection == VERTICAL)
					DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT, 2.440);
				else
					DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT, 1.220);
			} else if (placingZone.cells [clickedRow][clickedCol].objType == EUROFORM) {
				DGEnableItem (dialogID, EDITCONTROL_HEIGHT);
				// ������ ũ��: 600x1200 (�ִ� 900x1500)
				if (placingZone.iCellDirection == VERTICAL)
					DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT, 1.500);
				else
					DGSetItemMaxDouble (dialogID, EDITCONTROL_HEIGHT, 0.900);
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

			// ��: ��� ����
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 10, 170, 220, 40);
			DGSetItemFont (dialogID, LABEL_CAUTION, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_CAUTION, L"��� ����");

			if (placingZone.iCellDirection == HORIZONTAL)
				DGPopUpSelectItem (dialogID, POPUP_ORIENTATION, HORIZONTAL);
			else
				DGPopUpSelectItem (dialogID, POPUP_ORIENTATION, VERTICAL);

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					for (xx = 0 ; xx < MAX_IND ; ++xx) {
						placingZone.cells [xx][clickedCol].horLen = DGGetItemValDouble (dialogID, EDITCONTROL_WIDTH);
						placingZone.cells [clickedRow][xx].verLen = DGGetItemValDouble (dialogID, EDITCONTROL_HEIGHT);
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