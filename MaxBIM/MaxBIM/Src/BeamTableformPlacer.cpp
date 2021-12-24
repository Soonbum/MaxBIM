#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "BeamTableformPlacer.hpp"

using namespace beamTableformPlacerDG;

static BeamTableformPlacingZone		placingZone;			// �⺻ �� ���� ����
static InfoBeam			infoBeam;							// �� ��ü ����
API_Guid				structuralObject_forTableformBeam;	// ���� ��ü�� GUID

static short	layerInd_Euroform;			// ���̾� ��ȣ: ������
static short	layerInd_Plywood;			// ���̾� ��ȣ: ����
static short	layerInd_Timber;			// ���̾� ��ȣ: ����
static short	layerInd_OutcornerAngle;	// ���̾� ��ȣ: �ƿ��ڳʾޱ�
static short	layerInd_Fillerspacer;		// ���̾� ��ȣ: �ٷ������̼�
static short	layerInd_Rectpipe;			// ���̾� ��ȣ: ���������
static short	layerInd_RectpipeHanger;	// ���̾� ��ȣ: �����������
static short	layerInd_Pinbolt;			// ���̾� ��ȣ: �ɺ�Ʈ
static short	layerInd_EuroformHook;		// ���̾� ��ȣ: ������ ��ũ
static short	layerInd_BlueClamp;			// ���̾� ��ȣ: ���Ŭ����
static short	layerInd_BlueTimberRail;	// ���̾� ��ȣ: �����
static short	clickedBtnItemIdx;			// �׸��� ��ư���� Ŭ���� ��ư�� �ε��� ��ȣ�� ����
static bool		clickedOKButton;			// OK ��ư�� �������ϱ�?
static bool		clickedPrevButton;			// ���� ��ư�� �������ϱ�?
static GS::Array<API_Guid>	elemList;		// �׷�ȭ�� ���� ������ ��������� GUID�� ���� ������


// ���� ���̺����� ��ġ�ϴ� ���� ��ƾ
GSErrCode	placeTableformOnBeam (void)
{
	GSErrCode		err = NoError;
	long			nSel;
	short			xx, yy;
	double			dx, dy;
	short			result;

	// Selection Manager ���� ����
	API_SelectionInfo		selectionInfo;
	API_Element				tElem;
	API_Neig				**selNeigs;
	GS::Array<API_Guid>		morphs;
	GS::Array<API_Guid>		beams;
	long					nMorphs = 0;
	long					nBeams = 0;

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
	long					nNodes;

	// ���� ��ü ����
	InfoMorphForBeamTableform	infoMorph [2];
	API_Coord3D					morph1_point [2];
	API_Coord3D					morph2_point [2];
	double						morph1_height = 0.0;
	double						morph2_height = 0.0;

	// �۾� �� ����
	API_StoryInfo			storyInfo;
	double					workLevel_beam;


	// ������ ��� ��������
	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err == APIERR_NOPLAN) {
		ACAPI_WriteReport ("���� ������Ʈ â�� �����ϴ�.", true);
	}
	if (err == APIERR_NOSEL) {
		ACAPI_WriteReport ("�ƹ� �͵� �������� �ʾҽ��ϴ�.\n�ʼ� ����: �� (1��), �� ����(��ü/�Ϻ�)�� ���� ���� (1��)\n�ɼ� ���� (1): �� �ݴ��� ������ ���� ���� (1��)\n�ɼ� ���� (2): �� �Ʒ��� ���� ���� ���� (1��)", true);
	}
	if (err != NoError) {
		BMKillHandle ((GSHandle *) &selNeigs);
		return err;
	}

	// �� 1��, ���� 1~2�� �����ؾ� ��
	if (selectionInfo.typeID != API_SelEmpty) {
		nSel = BMGetHandleSize ((GSHandle) selNeigs) / sizeof (API_Neig);
		for (xx = 0 ; xx < nSel && err == NoError ; ++xx) {
			tElem.header.typeID = Neig_To_ElemID ((*selNeigs)[xx].neigID);

			tElem.header.guid = (*selNeigs)[xx].guid;
			if (ACAPI_Element_Get (&tElem) != NoError)	// ������ �� �ִ� ����ΰ�?
				continue;

			if (tElem.header.typeID == API_MorphID)		// �����ΰ�?
				morphs.Push (tElem.header.guid);

			if (tElem.header.typeID == API_BeamID)		// ���ΰ�?
				beams.Push (tElem.header.guid);
		}
	}
	BMKillHandle ((GSHandle *) &selNeigs);
	nMorphs = morphs.GetSize ();
	nBeams = beams.GetSize ();

	// ���� 1���ΰ�?
	if (nBeams != 1) {
		ACAPI_WriteReport ("���� 1�� �����ؾ� �մϴ�.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// ������ 1~2���ΰ�?
	if ( !((nMorphs >= 1) && (nMorphs <= 2)) ) {
		ACAPI_WriteReport ("�� ����(��ü/�Ϻ�)�� ���� ������ 1�� �����ϼž� �մϴ�.\n���� ���̰� ���Ī�̸� �� �ݴ��� ������ ���� ������ �־�� �մϴ�.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// �� ���� ����
	infoBeam.guid = beams.Pop ();

	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	elem.header.guid = infoBeam.guid;
	structuralObject_forTableformBeam = elem.header.guid;
	err = ACAPI_Element_Get (&elem);
	err = ACAPI_Element_GetMemo (elem.header.guid, &memo);
	
	infoBeam.floorInd	= elem.header.floorInd;
	infoBeam.height		= elem.beam.height;
	infoBeam.width		= elem.beam.width;
	infoBeam.offset		= elem.beam.offset;
	infoBeam.level		= elem.beam.level;
	infoBeam.begC		= elem.beam.begC;
	infoBeam.endC		= elem.beam.endC;

	ACAPI_DisposeElemMemoHdls (&memo);

	for (xx = 0 ; xx < nMorphs ; ++xx) {
		// ���� ������ ������
		BNZeroMemory (&elem, sizeof (API_Element));
		elem.header.guid = morphs.Pop ();
		err = ACAPI_Element_Get (&elem);
		err = ACAPI_Element_Get3DInfo (elem.header, &info3D);

		// ������ ���� ����
		infoMorph [xx].guid		= elem.header.guid;
		infoMorph [xx].floorInd	= elem.header.floorInd;
		infoMorph [xx].level	= info3D.bounds.zMin;

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
		
		// ���� ��ǥ�� ���� ������� ������
		for (yy = 1 ; yy <= nVert ; ++yy) {
			component.header.typeID	= API_VertID;
			component.header.index	= yy;
			err = ACAPI_3D_GetComponent (&component);
			if (err == NoError) {
				trCoord.x = tm.tmx[0]*component.vert.x + tm.tmx[1]*component.vert.y + tm.tmx[2]*component.vert.z + tm.tmx[3];
				trCoord.y = tm.tmx[4]*component.vert.x + tm.tmx[5]*component.vert.y + tm.tmx[6]*component.vert.z + tm.tmx[7];
				trCoord.z = tm.tmx[8]*component.vert.x + tm.tmx[9]*component.vert.y + tm.tmx[10]*component.vert.z + tm.tmx[11];
				coords.Push (trCoord);
			}
		}
		nNodes = coords.GetSize ();

		// 1��° ������ �� ���� ����
		if (xx == 0) {
			morph1_point [0] = coords [0];	// ������ 1��° �� ����
			for (yy = 1 ; yy < nNodes ; ++yy) {
				// x, y ��ǥ ���� ������ ����ϰ�, �ٸ��� 2��° ������ ����
				if ( (abs (morph1_point [0].x - coords [yy].x) < EPS) && (abs (morph1_point [0].y - coords [yy].y) < EPS) ) {
					continue;
				} else {
					morph1_point [1] = coords [yy];
					break;
				}
			}

			morph1_height = info3D.bounds.zMax - info3D.bounds.zMin;
		}

		// 2��° ������ �� ���� ����
		if (xx == 1) {
			morph2_point [0] = coords [0];	// ������ 1��° �� ����
			for (yy = 1 ; yy < nNodes ; ++yy) {
				// x, y ��ǥ ���� ������ ����ϰ�, �ٸ��� 2��° ������ ����
				if ( (abs (morph2_point [0].x - coords [yy].x) < EPS) && (abs (morph2_point [0].y - coords [yy].y) < EPS) ) {
					continue;
				} else {
					morph2_point [1] = coords [yy];
					break;
				}
			}

			morph2_height = info3D.bounds.zMax - info3D.bounds.zMin;
		}

		// 1��° ������ ������� ���ϴ�, ���� �� ��������
		if (xx == 0) {
			// ������ ���ϴ�, ���� �� ����
			if (abs (elem.morph.tranmat.tmx [11] - info3D.bounds.zMin) < EPS) {
				// ���ϴ� ��ǥ ����
				infoMorph [xx].leftBottomX = elem.morph.tranmat.tmx [3];
				infoMorph [xx].leftBottomY = elem.morph.tranmat.tmx [7];
				infoMorph [xx].leftBottomZ = elem.morph.tranmat.tmx [11];

				// ���� ��ǥ��?
				if (abs (infoMorph [xx].leftBottomX - info3D.bounds.xMin) < EPS)
					infoMorph [xx].rightTopX = info3D.bounds.xMax;
				else
					infoMorph [xx].rightTopX = info3D.bounds.xMin;
				if (abs (infoMorph [xx].leftBottomY - info3D.bounds.yMin) < EPS)
					infoMorph [xx].rightTopY = info3D.bounds.yMax;
				else
					infoMorph [xx].rightTopY = info3D.bounds.yMin;
				if (abs (infoMorph [xx].leftBottomZ - info3D.bounds.zMin) < EPS)
					infoMorph [xx].rightTopZ = info3D.bounds.zMax;
				else
					infoMorph [xx].rightTopZ = info3D.bounds.zMin;
			} else {
				// ���� ��ǥ ����
				infoMorph [xx].rightTopX = elem.morph.tranmat.tmx [3];
				infoMorph [xx].rightTopY = elem.morph.tranmat.tmx [7];
				infoMorph [xx].rightTopZ = elem.morph.tranmat.tmx [11];

				// ���ϴ� ��ǥ��?
				if (abs (infoMorph [xx].rightTopX - info3D.bounds.xMin) < EPS)
					infoMorph [xx].leftBottomX = info3D.bounds.xMax;
				else
					infoMorph [xx].leftBottomX = info3D.bounds.xMin;
				if (abs (infoMorph [xx].rightTopY - info3D.bounds.yMin) < EPS)
					infoMorph [xx].leftBottomY = info3D.bounds.yMax;
				else
					infoMorph [xx].leftBottomY = info3D.bounds.yMin;
				if (abs (infoMorph [xx].rightTopZ - info3D.bounds.zMin) < EPS)
					infoMorph [xx].leftBottomZ = info3D.bounds.zMax;
				else
					infoMorph [xx].leftBottomZ = info3D.bounds.zMin;
			}

			// ������ Z�� ȸ�� ����
			dx = infoMorph [xx].rightTopX - infoMorph [xx].leftBottomX;
			dy = infoMorph [xx].rightTopY - infoMorph [xx].leftBottomY;
			infoMorph [xx].ang = atan2 (dy, dx);
		}

		// ����� ��ǥ�� ������
		coords.Clear ();

		// ���� ���� ����
		API_Elem_Head* headList = new API_Elem_Head [1];
		headList [0] = elem.header;
		err = ACAPI_Element_Delete (&headList, 1);
		delete headList;
	}

	// ���� ���� ����
	if (nMorphs == 2) {
		if (morph1_height > morph2_height) {
			placingZone.areaHeight_Left = morph1_height;
			placingZone.areaHeight_Right = morph2_height;
		} else {
			placingZone.areaHeight_Left = morph2_height;
			placingZone.areaHeight_Right = morph1_height;
		}
	} else {
		placingZone.areaHeight_Left = morph1_height;
		placingZone.areaHeight_Right = morph1_height;
	}

	// �� ����
	placingZone.beamLength = GetDistance (morph1_point [0], morph1_point [1]);

	// �� �ʺ�
	placingZone.areaWidth_Bottom = infoBeam.width;

	// �� ������
	placingZone.offset = infoBeam.offset;

	// �� ���� ��
	placingZone.level = infoBeam.level;

	// ��ġ ���� ������, ���� (���� ������ ���̰� �������� ������ ��)
	if ((infoMorph [0].rightTopZ - infoMorph [0].leftBottomZ) - (infoMorph [1].rightTopZ - infoMorph [1].leftBottomZ) > EPS) {
		placingZone.begC.x = infoMorph [0].leftBottomX;
		placingZone.begC.y = infoMorph [0].leftBottomY;
		placingZone.begC.z = infoMorph [0].leftBottomZ;

		placingZone.endC.x = infoMorph [0].rightTopX;
		placingZone.endC.y = infoMorph [0].rightTopY;
		placingZone.endC.z = infoMorph [0].leftBottomZ;

		placingZone.ang = infoMorph [0].ang;
	} else {
		placingZone.begC.x = infoMorph [1].leftBottomX;
		placingZone.begC.y = infoMorph [1].leftBottomY;
		placingZone.begC.z = infoMorph [1].leftBottomZ;

		placingZone.endC.x = infoMorph [1].rightTopX;
		placingZone.endC.y = infoMorph [1].rightTopY;
		placingZone.endC.z = infoMorph [1].leftBottomZ;

		placingZone.ang = infoMorph [1].ang;
	}

	// �۾� �� ���� �ݿ�
	BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	workLevel_beam = 0.0;
	ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
	for (xx = 0 ; xx <= (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
		if (storyInfo.data [0][xx].index == infoBeam.floorInd) {
			workLevel_beam = storyInfo.data [0][xx].level;
			break;
		}
	}
	BMKillHandle ((GSHandle *) &storyInfo.data);

FIRST:

	// placingZone�� Cell ���� �ʱ�ȭ
	placingZone.initCells (&placingZone);

	// [DIALOG] 1��° ���̾�α׿��� ������ ���� �Է� ����
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32522, ACAPI_GetOwnResModule (), beamTableformPlacerHandler1, 0);

	if (result == DG_CANCEL)
		return err;

	// !!!
	// �� ���̿��� ������ ���� ������ ����� �뷫���� �� ����(nCells) �ʱ� ����

	// [DIALOG] 2��° ���̾�α׿��� ������ ��ġ�� �����մϴ�.
	clickedOKButton = false;
	clickedPrevButton = false;
	result = DGBlankModalDialog (500, 360, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, beamTableformPlacerHandler2, 0);
	
	// ���� ��ư�� ������ 1��° ���̾�α� �ٽ� ����
	if (clickedPrevButton == true)
		goto FIRST;

	// 2��° ���̾�α׿��� OK ��ư�� �����߸� ���� �ܰ�� �Ѿ
	if (clickedOKButton != true)
		return err;

	// ... ��ġ�ϱ�

	// ... ������ ���� ä��� - ����, ����
	err = placingZone.fillRestAreas (&placingZone);

	// ����� ��ü �׷�ȭ
	//if (!elemList.IsEmpty ()) {
	//	GSSize nElems = elemList.GetSize ();
	//	API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
	//	if (elemHead != NULL) {
	//		for (GSIndex i = 0; i < nElems; i++)
	//			(*elemHead)[i].guid = elemList[i];

	//		ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

	//		BMKillHandle ((GSHandle *) &elemHead);
	//	}
	//}

	// ȭ�� ���ΰ�ħ
	ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	bool	regenerate = true;
	ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

	return	err;
}

// Cell �迭�� �ʱ�ȭ��
void	BeamTableformPlacingZone::initCells (BeamTableformPlacingZone* placingZone)
{
	short xx, yy;

	// ���� ������ ���� ä�� ���� �ʱ�ȭ
	placingZone->bFillMarginBegin = false;
	placingZone->bFillMarginEnd = false;

	// ���� ������ ���� ���� �ʱ�ȭ
	placingZone->marginBegin = 0.0;
	placingZone->marginEnd = 0.0;

	// �� ���� �ʱ�ȭ
	placingZone->nCells = 0;

	// �� ���� �ʱ�ȭ
	for (xx = 0 ; xx < 4 ; ++xx) {
		for (yy = 0 ; yy < 50 ; ++yy) {
			placingZone->cellsAtLSide [xx][yy].objType = NONE;
			placingZone->cellsAtLSide [xx][yy].leftBottomX = 0.0;
			placingZone->cellsAtLSide [xx][yy].leftBottomY = 0.0;
			placingZone->cellsAtLSide [xx][yy].leftBottomZ = 0.0;
			placingZone->cellsAtLSide [xx][yy].ang = 0.0;
			placingZone->cellsAtLSide [xx][yy].dirLen = 0.0;
			placingZone->cellsAtLSide [xx][yy].perLen = 0.0;
			placingZone->cellsAtLSide [xx][yy].attached_side = LEFT_SIDE;

			placingZone->cellsAtRSide [xx][yy].objType = NONE;
			placingZone->cellsAtRSide [xx][yy].leftBottomX = 0.0;
			placingZone->cellsAtRSide [xx][yy].leftBottomY = 0.0;
			placingZone->cellsAtRSide [xx][yy].leftBottomZ = 0.0;
			placingZone->cellsAtRSide [xx][yy].ang = 0.0;
			placingZone->cellsAtRSide [xx][yy].dirLen = 0.0;
			placingZone->cellsAtRSide [xx][yy].perLen = 0.0;
			placingZone->cellsAtRSide [xx][yy].attached_side = RIGHT_SIDE;

		}
	}

	for (xx = 0 ; xx < 3 ; ++xx) {
		for (yy = 0 ; yy < 50 ; ++yy) {
			placingZone->cellsAtBottom [xx][yy].objType = NONE;
			placingZone->cellsAtBottom [xx][yy].leftBottomX = 0.0;
			placingZone->cellsAtBottom [xx][yy].leftBottomY = 0.0;
			placingZone->cellsAtBottom [xx][yy].leftBottomZ = 0.0;
			placingZone->cellsAtBottom [xx][yy].ang = 0.0;
			placingZone->cellsAtBottom [xx][yy].dirLen = 0.0;
			placingZone->cellsAtBottom [xx][yy].perLen = 0.0;
			placingZone->cellsAtBottom [xx][yy].attached_side = BOTTOM_SIDE;
		}
	}
}

// Cell ������ ����ʿ� ���� ����ȭ�� ��ġ�� ��������
void	BeamTableformPlacingZone::alignPlacingZone (BeamTableformPlacingZone* placingZone)
{
	//short			xx, yy;
	//API_Coord3D		axisPoint, rotatedPoint, unrotatedPoint;

	//double			centerPos;		// �߽� ��ġ
	//double			width_side;		// ���� �߽� ������ �ʺ�
	//double			width_bottom;	// �Ϻ� �߽� ������ �ʺ�
	//double			remainLength;	// ���� ���̸� ����ϱ� ���� �ӽ� ����
	//double			xPos;			// ��ġ Ŀ��
	//double			accumDist;		// �̵� �Ÿ�
	//
	//double			height [4];
	//double			left [3];

	//
	//// ���鿡���� �߽� ��ġ ã��
	//if (placingZone->bInterfereBeam == true)
	//	centerPos = placingZone->posInterfereBeamFromLeft;	// ���� ���� �߽� ��ġ
	//else
	//	centerPos = placingZone->beamLength / 2;			// ���� ���� ������ �߽��� �������� ��

	//// �߽� ������ �ʺ�
	//if (placingZone->cellCenterAtRSide [0].objType != NONE)
	//	width_side = placingZone->cellCenterAtRSide [0].dirLen;
	//else
	//	width_side = placingZone->centerLengthAtSide;

	//if (placingZone->cellCenterAtBottom [0].objType != NONE)
	//	width_bottom = placingZone->cellCenterAtBottom [0].dirLen;
	//else
	//	width_bottom = 0.0;


	//// (1-1) ���� ���� �κ�
	//// �߽ɺ��� ������ �̵��ؾ� ��
	//accumDist = 0.0;
	//for (xx = 0 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)
	//	if (placingZone->cellsFromBeginAtLSide [0][xx].objType != NONE)
	//		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;

	//// ��ġ ����
	//height [0] = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//height [1] = height [0] + placingZone->cellsFromBeginAtRSide [0][0].perLen;
	//height [2] = height [1] + placingZone->cellsFromBeginAtRSide [1][0].perLen;
	//height [3] = height [2] + placingZone->cellsFromBeginAtRSide [2][0].perLen;
	//for (xx = 0 ; xx < 4 ; ++xx) {
	//	xPos = centerPos - width_side/2 - accumDist;
	//	for (yy = 0 ; yy < placingZone->nCellsFromBeginAtSide ; ++yy) {
	//		if (placingZone->cellsFromBeginAtLSide [xx][yy].objType != NONE) {
	//			// ����
	//			placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomX = placingZone->begC.x + xPos;
	//			placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
	//			placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomZ = height [xx];
	//	
	//			axisPoint.x = placingZone->begC.x;
	//			axisPoint.y = placingZone->begC.y;
	//			axisPoint.z = placingZone->begC.z;

	//			rotatedPoint.x = placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomX;
	//			rotatedPoint.y = placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomY;
	//			rotatedPoint.z = placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomZ;
	//			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//			placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomX = unrotatedPoint.x;
	//			placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomY = unrotatedPoint.y;
	//			placingZone->cellsFromBeginAtLSide [xx][yy].leftBottomZ = unrotatedPoint.z;

	//			// ����
	//			placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomX = placingZone->begC.x + xPos;
	//			placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
	//			placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomZ = height [xx];

	//			axisPoint.x = placingZone->begC.x;
	//			axisPoint.y = placingZone->begC.y;
	//			axisPoint.z = placingZone->begC.z;

	//			rotatedPoint.x = placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomX;
	//			rotatedPoint.y = placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomY;
	//			rotatedPoint.z = placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomZ;
	//			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//			placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomX = unrotatedPoint.x;
	//			placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomY = unrotatedPoint.y;
	//			placingZone->cellsFromBeginAtRSide [xx][yy].leftBottomZ = unrotatedPoint.z;

	//			// �Ÿ� �̵�
	//			xPos += placingZone->cellsFromBeginAtRSide [xx][yy].dirLen;
	//		}
	//	}
	//}

	//// (1-2) ���� �� �κ�
	//// �߽ɺ��� ������ �̵��ؾ� ��
	//accumDist = 0.0;
	//for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)
	//	if (placingZone->cellsFromEndAtLSide [0][xx].objType != NONE)
	//		accumDist += placingZone->cellsFromEndAtLSide [0][xx].dirLen;

	//// ��ġ ����
	//height [0] = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//height [1] = height [0] + placingZone->cellsFromEndAtRSide [0][0].perLen;
	//height [2] = height [1] + placingZone->cellsFromEndAtRSide [1][0].perLen;
	//height [3] = height [2] + placingZone->cellsFromEndAtRSide [2][0].perLen;
	//for (xx = 0 ; xx < 4 ; ++xx) {
	//	xPos = centerPos + width_side/2 + accumDist - placingZone->cellsFromEndAtLSide [0][0].dirLen;
	//	for (yy = 0 ; yy < placingZone->nCellsFromEndAtSide ; ++yy) {
	//		if (placingZone->cellsFromEndAtLSide [xx][yy].objType != NONE) {
	//			// ����
	//			placingZone->cellsFromEndAtLSide [xx][yy].leftBottomX = placingZone->begC.x + xPos;
	//			placingZone->cellsFromEndAtLSide [xx][yy].leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
	//			placingZone->cellsFromEndAtLSide [xx][yy].leftBottomZ = height [xx];
	//		
	//			axisPoint.x = placingZone->begC.x;
	//			axisPoint.y = placingZone->begC.y;
	//			axisPoint.z = placingZone->begC.z;

	//			rotatedPoint.x = placingZone->cellsFromEndAtLSide [xx][yy].leftBottomX;
	//			rotatedPoint.y = placingZone->cellsFromEndAtLSide [xx][yy].leftBottomY;
	//			rotatedPoint.z = placingZone->cellsFromEndAtLSide [xx][yy].leftBottomZ;
	//			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//			placingZone->cellsFromEndAtLSide [xx][yy].leftBottomX = unrotatedPoint.x;
	//			placingZone->cellsFromEndAtLSide [xx][yy].leftBottomY = unrotatedPoint.y;
	//			placingZone->cellsFromEndAtLSide [xx][yy].leftBottomZ = unrotatedPoint.z;

	//			// ����
	//			placingZone->cellsFromEndAtRSide [xx][yy].leftBottomX = placingZone->begC.x + xPos;
	//			placingZone->cellsFromEndAtRSide [xx][yy].leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
	//			placingZone->cellsFromEndAtRSide [xx][yy].leftBottomZ = height [xx];

	//			axisPoint.x = placingZone->begC.x;
	//			axisPoint.y = placingZone->begC.y;
	//			axisPoint.z = placingZone->begC.z;

	//			rotatedPoint.x = placingZone->cellsFromEndAtRSide [xx][yy].leftBottomX;
	//			rotatedPoint.y = placingZone->cellsFromEndAtRSide [xx][yy].leftBottomY;
	//			rotatedPoint.z = placingZone->cellsFromEndAtRSide [xx][yy].leftBottomZ;
	//			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//			placingZone->cellsFromEndAtRSide [xx][yy].leftBottomX = unrotatedPoint.x;
	//			placingZone->cellsFromEndAtRSide [xx][yy].leftBottomY = unrotatedPoint.y;
	//			placingZone->cellsFromEndAtRSide [xx][yy].leftBottomZ = unrotatedPoint.z;

	//			// �Ÿ� �̵�
	//			if (yy < placingZone->nCellsFromEndAtSide-1)
	//				xPos -= placingZone->cellsFromEndAtRSide [xx][yy+1].dirLen;
	//		}
	//	}
	//}

	//// (1-3) ���� �߾�
	//// ��ġ ����
	//xPos = centerPos - width_side/2;
	//height [0] = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//height [1] = height [0] + placingZone->cellCenterAtRSide [0].perLen;
	//height [2] = height [1] + placingZone->cellCenterAtRSide [1].perLen;
	//height [3] = height [2] + placingZone->cellCenterAtRSide [2].perLen;
	//for (xx = 0 ; xx < 4 ; ++xx) {
	//	// ����
	//	placingZone->cellCenterAtLSide [xx].leftBottomX = placingZone->begC.x + xPos;
	//	placingZone->cellCenterAtLSide [xx].leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
	//	placingZone->cellCenterAtLSide [xx].leftBottomZ = height [xx];
	//	
	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = placingZone->cellCenterAtLSide [xx].leftBottomX;
	//	rotatedPoint.y = placingZone->cellCenterAtLSide [xx].leftBottomY;
	//	rotatedPoint.z = placingZone->cellCenterAtLSide [xx].leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	placingZone->cellCenterAtLSide [xx].leftBottomX = unrotatedPoint.x;
	//	placingZone->cellCenterAtLSide [xx].leftBottomY = unrotatedPoint.y;
	//	placingZone->cellCenterAtLSide [xx].leftBottomZ = unrotatedPoint.z;

	//	// ����
	//	placingZone->cellCenterAtRSide [xx].leftBottomX = placingZone->begC.x + xPos;
	//	placingZone->cellCenterAtRSide [xx].leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
	//	placingZone->cellCenterAtRSide [xx].leftBottomZ = height [xx];

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = placingZone->cellCenterAtRSide [xx].leftBottomX;
	//	rotatedPoint.y = placingZone->cellCenterAtRSide [xx].leftBottomY;
	//	rotatedPoint.z = placingZone->cellCenterAtRSide [xx].leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	placingZone->cellCenterAtRSide [xx].leftBottomX = unrotatedPoint.x;
	//	placingZone->cellCenterAtRSide [xx].leftBottomY = unrotatedPoint.y;
	//	placingZone->cellCenterAtRSide [xx].leftBottomZ = unrotatedPoint.z;
	//}

	//// (2-1) �Ϻ� ���� �κ�
	//// �߽ɺ��� ������ �̵��ؾ� ��
	//accumDist = 0.0;
	//for (xx = 0 ; xx < placingZone->nCellsFromBeginAtBottom ; ++xx)
	//	if (placingZone->cellsFromBeginAtBottom [0][xx].objType != NONE)
	//		accumDist += placingZone->cellsFromBeginAtBottom [0][xx].dirLen;

	//// ��ġ ����
	//left [0] = placingZone->begC.y + infoBeam.width/2 + placingZone->gapSide;
	//left [1] = left [0] - placingZone->cellsFromBeginAtBottom [0][0].perLen;
	//left [2] = left [1] - placingZone->cellsFromBeginAtBottom [1][0].perLen;
	//for (xx = 0 ; xx < 3 ; ++xx) {
	//	xPos = centerPos - width_bottom/2 - accumDist;
	//	for (yy = 0 ; yy < placingZone->nCellsFromBeginAtBottom ; ++yy) {
	//		if (placingZone->cellsFromBeginAtBottom [xx][yy].objType != NONE) {
	//			placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomX = placingZone->begC.x + xPos;
	//			placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomY = left [xx] + infoBeam.offset;
	//			placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//		
	//			axisPoint.x = placingZone->begC.x;
	//			axisPoint.y = placingZone->begC.y;
	//			axisPoint.z = placingZone->begC.z;

	//			rotatedPoint.x = placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomX;
	//			rotatedPoint.y = placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomY;
	//			rotatedPoint.z = placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomZ;
	//			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//			placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomX = unrotatedPoint.x;
	//			placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomY = unrotatedPoint.y;
	//			placingZone->cellsFromBeginAtBottom [xx][yy].leftBottomZ = unrotatedPoint.z;

	//			// �Ÿ� �̵�
	//			xPos += placingZone->cellsFromBeginAtBottom [xx][yy].dirLen;
	//		}
	//	}
	//}

	//// (2-2) �Ϻ� �� �κ�
	//// �߽ɺ��� ������ �̵��ؾ� ��
	//accumDist = 0.0;
	//for (xx = 0 ; xx < placingZone->nCellsFromEndAtBottom ; ++xx)
	//	if (placingZone->cellsFromEndAtBottom [0][xx].objType != NONE)
	//		accumDist += placingZone->cellsFromEndAtBottom [0][xx].dirLen;

	//// ��ġ ����
	//left [0] = placingZone->begC.y + infoBeam.width/2 + placingZone->gapSide;
	//left [1] = left [0] - placingZone->cellsFromEndAtBottom [0][0].perLen;
	//left [2] = left [1] - placingZone->cellsFromEndAtBottom [1][0].perLen;
	//for (xx = 0 ; xx < 3 ; ++xx) {
	//	xPos = centerPos + width_bottom/2 + accumDist - placingZone->cellsFromEndAtBottom [0][0].dirLen;
	//	for (yy = 0 ; yy < placingZone->nCellsFromEndAtBottom ; ++yy) {
	//		if (placingZone->cellsFromEndAtBottom [xx][yy].objType != NONE) {
	//			placingZone->cellsFromEndAtBottom [xx][yy].leftBottomX = placingZone->begC.x + xPos;
	//			placingZone->cellsFromEndAtBottom [xx][yy].leftBottomY = left [xx] + infoBeam.offset;
	//			placingZone->cellsFromEndAtBottom [xx][yy].leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//		
	//			axisPoint.x = placingZone->begC.x;
	//			axisPoint.y = placingZone->begC.y;
	//			axisPoint.z = placingZone->begC.z;

	//			rotatedPoint.x = placingZone->cellsFromEndAtBottom [xx][yy].leftBottomX;
	//			rotatedPoint.y = placingZone->cellsFromEndAtBottom [xx][yy].leftBottomY;
	//			rotatedPoint.z = placingZone->cellsFromEndAtBottom [xx][yy].leftBottomZ;
	//			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//			placingZone->cellsFromEndAtBottom [xx][yy].leftBottomX = unrotatedPoint.x;
	//			placingZone->cellsFromEndAtBottom [xx][yy].leftBottomY = unrotatedPoint.y;
	//			placingZone->cellsFromEndAtBottom [xx][yy].leftBottomZ = unrotatedPoint.z;

	//			// �Ÿ� �̵�
	//			if (yy < placingZone->nCellsFromEndAtBottom-1)
	//				xPos -= placingZone->cellsFromEndAtBottom [xx][yy+1].dirLen;
	//		}
	//	}
	//}

	//// (2-3) �Ϻ� �߾�
	//// ��ġ ����
	//xPos = centerPos - width_bottom/2;
	//left [0] = placingZone->begC.y + infoBeam.width/2 + placingZone->gapSide;
	//left [1] = left [0] - placingZone->cellCenterAtBottom [0].perLen;
	//left [2] = left [1] - placingZone->cellCenterAtBottom [1].perLen;
	//for (xx = 0 ; xx < 3 ; ++xx) {
	//	placingZone->cellCenterAtBottom [xx].leftBottomX = placingZone->begC.x + xPos;
	//	placingZone->cellCenterAtBottom [xx].leftBottomY = left [xx] + infoBeam.offset;
	//	placingZone->cellCenterAtBottom [xx].leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//	
	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = placingZone->cellCenterAtBottom [xx].leftBottomX;
	//	rotatedPoint.y = placingZone->cellCenterAtBottom [xx].leftBottomY;
	//	rotatedPoint.z = placingZone->cellCenterAtBottom [xx].leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	placingZone->cellCenterAtBottom [xx].leftBottomX = unrotatedPoint.x;
	//	placingZone->cellCenterAtBottom [xx].leftBottomY = unrotatedPoint.y;
	//	placingZone->cellCenterAtBottom [xx].leftBottomZ = unrotatedPoint.z;
	//}

	//// ���� �� �ʱ�ȭ (���� ���� �κ� ����)
	//remainLength = centerPos - width_side/2;
	//for (xx = 0 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx) {
	//	if (placingZone->cellsFromBeginAtRSide [0][xx].objType != NONE)
	//		remainLength -= placingZone->cellsFromBeginAtRSide [0][xx].dirLen;
	//}
	//placingZone->marginBeginAtSide = remainLength;

	//// ���� �� �ʱ�ȭ (���� �� �κ� ����)
	//remainLength = placingZone->beamLength - centerPos - width_side/2;
	//for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx) {
	//	if (placingZone->cellsFromEndAtRSide [0][xx].objType != NONE)
	//		remainLength -= placingZone->cellsFromEndAtRSide [0][xx].dirLen;
	//}
	//placingZone->marginEndAtSide = remainLength;

	//// ���� �� �ʱ�ȭ (�Ϻ� ���� �κ� ����)
	//remainLength = centerPos - width_bottom/2;
	//for (xx = 0 ; xx < placingZone->nCellsFromBeginAtBottom ; ++xx) {
	//	if (placingZone->cellsFromBeginAtBottom [0][xx].objType != NONE)
	//		remainLength -= placingZone->cellsFromBeginAtBottom [0][xx].dirLen;
	//}
	//placingZone->marginBeginAtBottom = remainLength;

	//// ���� �� �ʱ�ȭ (�Ϻ� �� �κ� ����)
	//remainLength = placingZone->beamLength - centerPos - width_bottom/2;
	//for (xx = 0 ; xx < placingZone->nCellsFromEndAtBottom ; ++xx) {
	//	if (placingZone->cellsFromEndAtBottom [0][xx].objType != NONE)
	//		remainLength -= placingZone->cellsFromEndAtBottom [0][xx].dirLen;
	//}
	//placingZone->marginEndAtBottom = remainLength;
}

// ���ο� ���� �߰��� (�� �ϳ��� �ø��� �߰��� ���� ������ �� ���� ����)
void	BeamTableformPlacingZone::addNewCol (BeamTableformPlacingZone* placingZone)
{
	//
}

// ������ ���� ������
void	BeamTableformPlacingZone::delLastCol (BeamTableformPlacingZone* placingZone)
{
	//
}

//// �ش� �� ������ ������� ���̺귯�� ��ġ
//API_Guid	BeamTableformPlacingZone::placeLibPart (CellForBeamTableform objInfo)
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
//	double	validLength = 0.0;	// ��ȿ�� �����ΰ�?
//	double	validWidth = 0.0;	// ��ȿ�� �ʺ��ΰ�?
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
//	if (objInfo.objType == EUROFORM)		gsmName = L("������v2.0.gsm");
//	if (objInfo.objType == PLYWOOD)			gsmName = L("����v1.0.gsm");
//	if (objInfo.objType == WOOD)			gsmName = L("����v1.0.gsm");
//	if (objInfo.objType == OUTCORNER_ANGLE)	gsmName = L("�ƿ��ڳʾޱ�v1.0.gsm");
//	if (objInfo.objType == FILLERSPACER)	gsmName = L("�ٷ������̼�v1.0.gsm");
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
//	element.header.floorInd = infoBeam.floorInd;
//
//	if (objInfo.objType == EUROFORM) {
//		element.header.layer = layerInd_Euroform;
//
//		// �԰�ǰ�� ���,
//		if (objInfo.libPart.form.eu_stan_onoff == true) {
//			setParameterByName (&memo, "eu_stan_onoff", 1.0);	// �԰��� On/Off
//
//			// �ʺ�
//			sprintf (tempString, "%.0f", objInfo.libPart.form.eu_wid * 1000);
//			setParameterByName (&memo, "eu_wid", tempString);
//
//			// ����
//			sprintf (tempString, "%.0f", objInfo.libPart.form.eu_hei * 1000);
//			setParameterByName (&memo, "eu_hei", tempString);
//
//		// ��԰�ǰ�� ���,
//		} else {
//			setParameterByName (&memo, "eu_stan_onoff", 0.0);	// �԰��� On/Off
//			setParameterByName (&memo, "eu_wid2", objInfo.libPart.form.eu_wid2);	// �ʺ�
//			setParameterByName (&memo, "eu_hei2", objInfo.libPart.form.eu_hei2);	// ����
//		}
//
//		// ��ġ����
//		if (objInfo.libPart.form.u_ins_wall == true) {
//			strcpy (tempString, "�������");
//		} else {
//			strcpy (tempString, "��������");
//			if (objInfo.libPart.form.eu_stan_onoff == true) {
//				element.object.pos.x += ( objInfo.libPart.form.eu_hei * cos(objInfo.ang) );
//				element.object.pos.y += ( objInfo.libPart.form.eu_hei * sin(objInfo.ang) );
//				validLength = objInfo.libPart.form.eu_hei;
//				validWidth = objInfo.libPart.form.eu_wid;
//			} else {
//				element.object.pos.x += ( objInfo.libPart.form.eu_hei2 * cos(objInfo.ang) );
//				element.object.pos.y += ( objInfo.libPart.form.eu_hei2 * sin(objInfo.ang) );
//				validLength = objInfo.libPart.form.eu_hei2;
//				validWidth = objInfo.libPart.form.eu_wid2;
//			}
//		}
//		setParameterByName (&memo, "u_ins", tempString);
//
//		// ȸ��X
//		if (objInfo.attached_side == BOTTOM_SIDE) {
//			setParameterByName (&memo, "ang_x", DegreeToRad (0.0));
//		} else if (objInfo.attached_side == LEFT_SIDE) {
//			setParameterByName (&memo, "ang_x", DegreeToRad (90.0));
//			if (objInfo.libPart.form.eu_stan_onoff == true) {
//				element.object.pos.x -= ( objInfo.libPart.form.eu_hei * cos(objInfo.ang) );
//				element.object.pos.y -= ( objInfo.libPart.form.eu_hei * sin(objInfo.ang) );
//			} else {
//				element.object.pos.x -= ( objInfo.libPart.form.eu_hei2 * cos(objInfo.ang) );
//				element.object.pos.y -= ( objInfo.libPart.form.eu_hei2 * sin(objInfo.ang) );
//			}
//			element.object.angle += DegreeToRad (180.0);
//		} else {
//			setParameterByName (&memo, "ang_x", DegreeToRad (90.0));
//		}
//
//	} else if (objInfo.objType == FILLERSPACER) {
//		element.header.layer = layerInd_Fillerspacer;
//		setParameterByName (&memo, "f_thk", objInfo.libPart.fillersp.f_thk);	// �β�
//		setParameterByName (&memo, "f_leng", objInfo.libPart.fillersp.f_leng);	// ����
//		setParameterByName (&memo, "f_ang", 0.0);								// ����
//
//		// ȸ��
//		if (objInfo.attached_side == BOTTOM_SIDE) {
//			setParameterByName (&memo, "f_rota", DegreeToRad (90.0));
//		} else if (objInfo.attached_side == LEFT_SIDE) {
//			setParameterByName (&memo, "f_rota", DegreeToRad (0.0));
//			element.object.pos.x += ( objInfo.libPart.fillersp.f_leng * cos(objInfo.ang) );
//			element.object.pos.y += ( objInfo.libPart.fillersp.f_leng * sin(objInfo.ang) );
//			element.object.angle += DegreeToRad (180.0);
//		} else {
//			setParameterByName (&memo, "f_rota", DegreeToRad (0.0));
//		}
//
//		validLength = objInfo.libPart.fillersp.f_leng;
//		validWidth = objInfo.libPart.fillersp.f_thk;
//
//	} else if (objInfo.objType == PLYWOOD) {
//		element.header.layer = layerInd_Plywood;
//		setParameterByName (&memo, "p_stan", "��԰�");							// �԰�
//		setParameterByName (&memo, "w_dir", "��������");						// ��ġ����
//		setParameterByName (&memo, "p_thk", "11.5T");							// �β�
//		setParameterByName (&memo, "p_wid", objInfo.libPart.plywood.p_wid);		// ����
//		setParameterByName (&memo, "p_leng", objInfo.libPart.plywood.p_leng);	// ����
//		setParameterByName (&memo, "sogak", 1.0);								// ����Ʋ ON
//		setParameterByName (&memo, "bInverseSogak", 1.0);						// ���� �������� ����
//		setParameterByName (&memo, "gap_a", 0.0);
//		setParameterByName (&memo, "gap_b", 0.0);
//		setParameterByName (&memo, "gap_c", 0.0);
//		setParameterByName (&memo, "gap_d", 0.0);
//		
//		// ����
//		if (objInfo.attached_side == BOTTOM_SIDE) {
//			setParameterByName (&memo, "p_ang", DegreeToRad (90.0));
//		} else if (objInfo.attached_side == LEFT_SIDE) {
//			setParameterByName (&memo, "p_ang", DegreeToRad (0.0));
//			element.object.pos.x += ( objInfo.libPart.plywood.p_leng * cos(objInfo.ang) );
//			element.object.pos.y += ( objInfo.libPart.plywood.p_leng * sin(objInfo.ang) );
//			element.object.angle += DegreeToRad (180.0);
//		}
//
//		validLength = objInfo.libPart.plywood.p_leng;
//		validWidth = objInfo.libPart.plywood.p_wid;
//
//	} else if (objInfo.objType == WOOD) {
//		element.header.layer = layerInd_Timber;
//
//		setParameterByName (&memo, "w_w", objInfo.libPart.wood.w_w);		// �β�
//		setParameterByName (&memo, "w_h", objInfo.libPart.wood.w_h);		// �ʺ�
//		setParameterByName (&memo, "w_leng", objInfo.libPart.wood.w_leng);	// ����
//		setParameterByName (&memo, "w_ang", objInfo.libPart.wood.w_ang);	// ����
//	
//		if (objInfo.attached_side == BOTTOM_SIDE) {
//			setParameterByName (&memo, "w_ins", "�ٴڴ�����");				// ��ġ����
//		} else if (objInfo.attached_side == LEFT_SIDE) {
//			setParameterByName (&memo, "w_ins", "�������");				// ��ġ����
//			element.object.pos.x += ( objInfo.libPart.wood.w_leng * cos(objInfo.ang) );
//			element.object.pos.y += ( objInfo.libPart.wood.w_leng * sin(objInfo.ang) );
//			element.object.angle += DegreeToRad (180.0);
//		} else {
//			setParameterByName (&memo, "w_ins", "�������");				// ��ġ����
//		}
//
//		validLength = objInfo.libPart.wood.w_leng;
//		validWidth = objInfo.libPart.wood.w_h;
//
//	} else if (objInfo.objType == OUTCORNER_ANGLE) {
//		element.header.layer = layerInd_OutcornerAngle;
//		setParameterByName (&memo, "a_leng", objInfo.libPart.outangle.a_leng);	// ����
//		setParameterByName (&memo, "a_ang", 0.0);								// ����
//
//		if (objInfo.attached_side == RIGHT_SIDE) {
//			element.object.pos.x += ( objInfo.libPart.outangle.a_leng * cos(objInfo.ang) );
//			element.object.pos.y += ( objInfo.libPart.outangle.a_leng * sin(objInfo.ang) );
//			element.object.angle += DegreeToRad (180.0);
//		}
//
//		validLength = objInfo.libPart.outangle.a_leng;
//		validWidth = 0.064;
//	}
//
//	// ��ü ��ġ
//	if ((objInfo.objType != NONE) && (validLength > EPS) && (validWidth > EPS))
//		ACAPI_Element_Create (&element, &memo);
//	ACAPI_DisposeElemMemoHdls (&memo);
//
//	return element.header.guid;
//}
//
//// ���̺귯�� ��ġ: ������ ��ũ
//API_Guid	BeamTableformPlacingZone::placeLibPart (EuroformHook params)
//{
//	GSErrCode	err = NoError;
//	API_Element			elem;
//	API_ElementMemo		memo;
//	API_LibPart			libPart;
//
//	const GS::uchar_t*	gsmName = L("������ ��ũ.gsm");
//	double				aParam;
//	double				bParam;
//	Int32				addParNum;
//
//	// ��ü �ε�
//	BNZeroMemory (&elem, sizeof (API_Element));
//	BNZeroMemory (&memo, sizeof (API_ElementMemo));
//	BNZeroMemory (&libPart, sizeof (libPart));
//	GS::ucscpy (libPart.file_UName, gsmName);
//	err = ACAPI_LibPart_Search (&libPart, false);
//	if (err != NoError)
//		return elem.header.guid;
//	if (libPart.location != NULL)
//		delete libPart.location;
//
//	ACAPI_LibPart_Get (&libPart);
//
//	elem.header.typeID = API_ObjectID;
//	elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));
//
//	ACAPI_Element_GetDefaults (&elem, &memo);
//	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);
//
//	// ���̺귯���� �Ķ���� �� �Է�
//	elem.object.libInd = libPart.index;
//	elem.object.reflected = false;
//	elem.object.pos.x = params.leftBottomX;
//	elem.object.pos.y = params.leftBottomY;
//	elem.object.level = params.leftBottomZ;
//	elem.object.xRatio = aParam;
//	elem.object.yRatio = bParam;
//	elem.object.angle = params.ang + DegreeToRad (180.0);
//	elem.header.floorInd = infoBeam.floorInd;
//
//	// ���̾�
//	elem.header.layer = layerInd_EuroformHook;
//
//	setParameterByName (&memo, "rotationX", params.angX);			// X�� ȸ��
//	setParameterByName (&memo, "rotationY", params.angY);			// Y�� ȸ��
//	setParameterByName (&memo, "iHookType", params.iHookType);		// (1)����-��, (2)����-��
//	setParameterByName (&memo, "iHookShape", params.iHookShape);	// (1)����, (2)�簢
//
//	// ��ü ��ġ
//	ACAPI_Element_Create (&elem, &memo);
//	ACAPI_DisposeElemMemoHdls (&memo);
//
//	return	elem.header.guid;
//}
//
//// ���̺귯�� ��ġ: �����������
//API_Guid	BeamTableformPlacingZone::placeLibPart (RectPipeHanger params)
//{
//	GSErrCode	err = NoError;
//	API_Element			elem;
//	API_ElementMemo		memo;
//	API_LibPart			libPart;
//
//	const GS::uchar_t*	gsmName = L("�����������.gsm");
//	double				aParam;
//	double				bParam;
//	Int32				addParNum;
//
//	// ��ü �ε�
//	BNZeroMemory (&elem, sizeof (API_Element));
//	BNZeroMemory (&memo, sizeof (API_ElementMemo));
//	BNZeroMemory (&libPart, sizeof (libPart));
//	GS::ucscpy (libPart.file_UName, gsmName);
//	err = ACAPI_LibPart_Search (&libPart, false);
//	if (err != NoError)
//		return elem.header.guid;
//	if (libPart.location != NULL)
//		delete libPart.location;
//
//	ACAPI_LibPart_Get (&libPart);
//
//	elem.header.typeID = API_ObjectID;
//	elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));
//
//	ACAPI_Element_GetDefaults (&elem, &memo);
//	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);
//
//	// ���̺귯���� �Ķ���� �� �Է�
//	elem.object.libInd = libPart.index;
//	elem.object.reflected = false;
//	elem.object.pos.x = params.leftBottomX;
//	elem.object.pos.y = params.leftBottomY;
//	elem.object.level = params.leftBottomZ;
//	elem.object.xRatio = aParam;
//	elem.object.yRatio = bParam;
//	elem.object.angle = params.ang - DegreeToRad (90);
//	elem.header.floorInd = infoBeam.floorInd;
//
//	// ���̾�
//	elem.header.layer = layerInd_RectpipeHanger;
//
//	setParameterByName (&memo, "m_type", "�����������");	// ǰ��
//	setParameterByName (&memo, "angX", params.angX);		// ȸ��X
//	setParameterByName (&memo, "angY", params.angY);		// ȸ��Y
//
//	// ��ü ��ġ
//	ACAPI_Element_Create (&elem, &memo);
//	ACAPI_DisposeElemMemoHdls (&memo);
//
//	return	elem.header.guid;
//}
//
//// ���̺귯�� ��ġ: ���������
//API_Guid	BeamTableformPlacingZone::placeLibPart (SquarePipe params)
//{
//	GSErrCode	err = NoError;
//	API_Element			elem;
//	API_ElementMemo		memo;
//	API_LibPart			libPart;
//
//	const GS::uchar_t*	gsmName = L("���������v1.0.gsm");
//	double				aParam;
//	double				bParam;
//	Int32				addParNum;
//
//	// ��ü �ε�
//	BNZeroMemory (&elem, sizeof (API_Element));
//	BNZeroMemory (&memo, sizeof (API_ElementMemo));
//	BNZeroMemory (&libPart, sizeof (libPart));
//	GS::ucscpy (libPart.file_UName, gsmName);
//	err = ACAPI_LibPart_Search (&libPart, false);
//	if (err != NoError)
//		return elem.header.guid;
//	if (libPart.location != NULL)
//		delete libPart.location;
//
//	ACAPI_LibPart_Get (&libPart);
//
//	elem.header.typeID = API_ObjectID;
//	elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));
//
//	ACAPI_Element_GetDefaults (&elem, &memo);
//	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);
//
//	// ���̺귯���� �Ķ���� �� �Է�
//	elem.object.libInd = libPart.index;
//	elem.object.reflected = false;
//	elem.object.pos.x = params.leftBottomX;
//	elem.object.pos.y = params.leftBottomY;
//	elem.object.level = params.leftBottomZ;
//	elem.object.xRatio = aParam;
//	elem.object.yRatio = bParam;
//	elem.object.angle = params.ang;
//	elem.header.floorInd = infoBeam.floorInd;
//
//	// ���̾�
//	elem.header.layer = layerInd_Rectpipe;
//
//	setParameterByName (&memo, "p_comp", "�簢������");		// �簢������
//	setParameterByName (&memo, "p_leng", params.length);	// ����
//	setParameterByName (&memo, "p_ang", params.pipeAng);	// ����
//
//	// ��ü ��ġ
//	ACAPI_Element_Create (&elem, &memo);
//	ACAPI_DisposeElemMemoHdls (&memo);
//
//	return	elem.header.guid;
//}
//
//// ���̺귯�� ��ġ: �����
//API_Guid	BeamTableformPlacingZone::placeLibPart (BlueTimberRail params)
//{
//	GSErrCode	err = NoError;
//	API_Element			elem;
//	API_ElementMemo		memo;
//	API_LibPart			libPart;
//
//	const GS::uchar_t*	gsmName = L("�����v1.0.gsm");
//	double				aParam;
//	double				bParam;
//	Int32				addParNum;
//
//	// ��ü �ε�
//	BNZeroMemory (&elem, sizeof (API_Element));
//	BNZeroMemory (&memo, sizeof (API_ElementMemo));
//	BNZeroMemory (&libPart, sizeof (libPart));
//	GS::ucscpy (libPart.file_UName, gsmName);
//	err = ACAPI_LibPart_Search (&libPart, false);
//	if (err != NoError)
//		return elem.header.guid;
//	if (libPart.location != NULL)
//		delete libPart.location;
//
//	ACAPI_LibPart_Get (&libPart);
//
//	elem.header.typeID = API_ObjectID;
//	elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));
//
//	ACAPI_Element_GetDefaults (&elem, &memo);
//	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);
//
//	// ���̺귯���� �Ķ���� �� �Է�
//	elem.object.libInd = libPart.index;
//	elem.object.reflected = false;
//	elem.object.pos.x = params.leftBottomX;
//	elem.object.pos.y = params.leftBottomY;
//	elem.object.level = params.leftBottomZ;
//	elem.object.xRatio = aParam;
//	elem.object.yRatio = bParam;
//	elem.object.angle = params.ang;
//	elem.header.floorInd = infoBeam.floorInd;
//
//	// ���̾�
//	elem.header.layer = layerInd_TimberRail;
//
//	setParameterByName (&memo, "railType", params.railType);	// �԰�
//	setParameterByName (&memo, "angX", params.angX);			// ȸ��X
//	setParameterByName (&memo, "angY", params.angY);			// ȸ��Y
//
//	// ��ü ��ġ
//	ACAPI_Element_Create (&elem, &memo);
//	ACAPI_DisposeElemMemoHdls (&memo);
//
//	return	elem.header.guid;
//}

// ������/�ٷ�/���縦 ä�� �� ������ ���� ä��� (������ ����/���� �� �ƿ��ڳʾޱ� + ���������, �����������, �ɺ�Ʈ, ������ ��ũ, ���Ŭ����, �����)
GSErrCode	BeamTableformPlacingZone::fillRestAreas (BeamTableformPlacingZone* placingZone)
{
	GSErrCode	err = NoError;
	//short	xx;
	//double	centerPos;		// �߽� ��ġ
	//double	width_side;		// ���� �߽� ������ �ʺ�
	//double	width_bottom;	// �Ϻ� �߽� ������ �ʺ�
	//double	xPos;			// ��ġ Ŀ��
	//double	accumDist;		// �̵� �Ÿ�
	//double	length_outa;	// �ƿ��ڳʾޱ� ���� ���
	//double	length_pipe;	// ��������� ���� ���

	//double	cellWidth_side;
	//double	cellHeight_side, cellHeight_bottom;
	//CellForBeamTableform	insCell;
	//API_Coord3D		axisPoint, rotatedPoint, unrotatedPoint;

	//SquarePipe		squarePipe;		// ���������
	//EuroformHook	hook;			// ������ ��ũ
	//RectPipeHanger	hanger;			// �������� ���
	//BlueTimberRail	timberRail;		// �����


	//// ���鿡���� �߽� ��ġ ã��
	//if (placingZone->bInterfereBeam == true)
	//	centerPos = placingZone->posInterfereBeamFromLeft;	// ���� ���� �߽� ��ġ
	//else
	//	centerPos = placingZone->beamLength / 2;			// ���� ���� ������ �߽��� �������� ��

	//// �߽� ������ �ʺ�
	//if (placingZone->cellCenterAtRSide [0].objType != NONE)
	//	width_side = placingZone->cellCenterAtRSide [0].dirLen;
	//else
	//	width_side = placingZone->centerLengthAtSide;

	//if (placingZone->cellCenterAtBottom [0].objType != NONE)
	//	width_bottom = placingZone->cellCenterAtBottom [0].dirLen;
	//else
	//	width_bottom = 0.0;

	//// �߽� ���� �ʺ� (�߽� �������� ���� ��쿡�� �����)
	//if (placingZone->bInterfereBeam == true)
	//	cellWidth_side = (placingZone->centerLengthAtSide - placingZone->interfereBeamWidth) / 2;
	//else
	//	cellWidth_side = placingZone->centerLengthAtSide;

	//// ���� ����/���� ����
	//cellHeight_side = placingZone->cellsFromBeginAtRSide [0][0].perLen + placingZone->cellsFromBeginAtRSide [1][0].perLen + placingZone->cellsFromBeginAtRSide [2][0].perLen + placingZone->cellsFromBeginAtRSide [3][0].perLen;
	//cellHeight_bottom = placingZone->cellsFromBeginAtBottom [0][0].perLen + placingZone->cellsFromBeginAtBottom [1][0].perLen + placingZone->cellsFromBeginAtBottom [2][0].perLen;


	//// ���� �߾� ���� NONE�� ���
	//if (placingZone->cellCenterAtRSide [0].objType == NONE) {
	//	// �ʺ� 110 �̸��̸� ����, 110 �̻��̸� ����
	//	if (placingZone->bInterfereBeam == true) {
	//		// ���� 1/2��°
	//		insCell.ang = placingZone->ang;
	//		insCell.attached_side = LEFT_SIDE;
	//		insCell.dirLen = cellWidth_side;
	//		insCell.perLen = cellHeight_side;
	//		insCell.leftBottomX = placingZone->begC.x + centerPos - placingZone->centerLengthAtSide/2;
	//		insCell.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
	//		insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;

	//		if (cellWidth_side < 0.110) {
	//			insCell.objType = WOOD;
	//			insCell.libPart.wood.w_ang = DegreeToRad (90.0);
	//			insCell.libPart.wood.w_h = cellWidth_side;
	//			insCell.libPart.wood.w_leng = cellHeight_side;
	//			insCell.libPart.wood.w_w = 0.040;
	//			insCell.leftBottomX -= insCell.libPart.wood.w_leng;
	//		} else {
	//			insCell.objType = PLYWOOD;
	//			insCell.libPart.plywood.p_leng = cellWidth_side;
	//			insCell.libPart.plywood.p_wid = cellHeight_side;
	//		}

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = insCell.leftBottomX;
	//		rotatedPoint.y = insCell.leftBottomY;
	//		rotatedPoint.z = insCell.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		insCell.leftBottomX = unrotatedPoint.x;
	//		insCell.leftBottomY = unrotatedPoint.y;
	//		insCell.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placingZone->placeLibPart (insCell));

	//		// ���� 2/2��°
	//		insCell.ang = placingZone->ang;
	//		insCell.attached_side = LEFT_SIDE;
	//		insCell.dirLen = cellWidth_side;
	//		insCell.perLen = cellHeight_side;
	//		insCell.leftBottomX = placingZone->begC.x + centerPos + placingZone->centerLengthAtSide/2;
	//		insCell.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
	//		insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;

	//		if (cellWidth_side < 0.110) {
	//			insCell.objType = WOOD;
	//			insCell.libPart.wood.w_ang = DegreeToRad (90.0);
	//			insCell.libPart.wood.w_h = cellWidth_side;
	//			insCell.libPart.wood.w_leng = cellHeight_side;
	//			insCell.libPart.wood.w_w = 0.040;
	//			insCell.leftBottomX -= (insCell.libPart.wood.w_leng + insCell.libPart.wood.w_h);
	//		} else {
	//			insCell.objType = PLYWOOD;
	//			insCell.libPart.plywood.p_leng = cellWidth_side;
	//			insCell.libPart.plywood.p_wid = cellHeight_side;
	//			insCell.leftBottomX -= insCell.libPart.plywood.p_leng;
	//		}

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = insCell.leftBottomX;
	//		rotatedPoint.y = insCell.leftBottomY;
	//		rotatedPoint.z = insCell.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		insCell.leftBottomX = unrotatedPoint.x;
	//		insCell.leftBottomY = unrotatedPoint.y;
	//		insCell.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placingZone->placeLibPart (insCell));

	//		// ���� 1/2��°
	//		insCell.ang = placingZone->ang;
	//		insCell.attached_side = RIGHT_SIDE;
	//		insCell.dirLen = cellWidth_side;
	//		insCell.perLen = cellHeight_side;
	//		insCell.leftBottomX = placingZone->begC.x + centerPos - placingZone->centerLengthAtSide/2;
	//		insCell.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
	//		insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;

	//		if (cellWidth_side < 0.110) {
	//			insCell.objType = WOOD;
	//			insCell.libPart.wood.w_ang = DegreeToRad (90.0);
	//			insCell.libPart.wood.w_h = cellWidth_side;
	//			insCell.libPart.wood.w_leng = cellHeight_side;
	//			insCell.libPart.wood.w_w = 0.040;
	//			insCell.leftBottomX += insCell.libPart.wood.w_h;
	//		} else {
	//			insCell.objType = PLYWOOD;
	//			insCell.libPart.plywood.p_leng = cellWidth_side;
	//			insCell.libPart.plywood.p_wid = cellHeight_side;
	//		}

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = insCell.leftBottomX;
	//		rotatedPoint.y = insCell.leftBottomY;
	//		rotatedPoint.z = insCell.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		insCell.leftBottomX = unrotatedPoint.x;
	//		insCell.leftBottomY = unrotatedPoint.y;
	//		insCell.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placingZone->placeLibPart (insCell));

	//		// ���� 2/2��°
	//		insCell.ang = placingZone->ang;
	//		insCell.attached_side = RIGHT_SIDE;
	//		insCell.dirLen = cellWidth_side;
	//		insCell.perLen = cellHeight_side;
	//		insCell.leftBottomX = placingZone->begC.x + centerPos + placingZone->centerLengthAtSide/2;
	//		insCell.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
	//		insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;

	//		if (cellWidth_side < 0.110) {
	//			insCell.objType = WOOD;
	//			insCell.libPart.wood.w_ang = DegreeToRad (90.0);
	//			insCell.libPart.wood.w_h = cellWidth_side;
	//			insCell.libPart.wood.w_leng = cellHeight_side;
	//			insCell.libPart.wood.w_w = 0.040;
	//		} else {
	//			insCell.objType = PLYWOOD;
	//			insCell.libPart.plywood.p_leng = cellWidth_side;
	//			insCell.libPart.plywood.p_wid = cellHeight_side;
	//			insCell.leftBottomX -= insCell.libPart.plywood.p_leng;
	//		}

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = insCell.leftBottomX;
	//		rotatedPoint.y = insCell.leftBottomY;
	//		rotatedPoint.z = insCell.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		insCell.leftBottomX = unrotatedPoint.x;
	//		insCell.leftBottomY = unrotatedPoint.y;
	//		insCell.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placingZone->placeLibPart (insCell));
	//	}
	//}

	//// ���� ���� �κ� ���� ä��
	//if (placingZone->bFillMarginBeginAtSide == true) {
	//	if (placingZone->marginBeginAtSide > EPS) {
	//		// ����
	//		insCell.ang = placingZone->ang;
	//		insCell.attached_side = LEFT_SIDE;
	//		insCell.dirLen = placingZone->marginBeginAtSide;
	//		insCell.perLen = cellHeight_side;
	//		insCell.leftBottomX = placingZone->begC.x;
	//		insCell.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
	//		insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;

	//		if (placingZone->marginBeginAtSide < 0.110) {
	//			insCell.objType = WOOD;
	//			insCell.libPart.wood.w_ang = DegreeToRad (90.0);
	//			insCell.libPart.wood.w_h = placingZone->marginBeginAtSide;
	//			insCell.libPart.wood.w_leng = cellHeight_side;
	//			insCell.libPart.wood.w_w = 0.040;
	//			insCell.leftBottomX -= cellHeight_side;
	//		} else {
	//			insCell.objType = PLYWOOD;
	//			insCell.libPart.plywood.p_leng = placingZone->marginBeginAtSide;
	//			insCell.libPart.plywood.p_wid = cellHeight_side;
	//		}

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = insCell.leftBottomX;
	//		rotatedPoint.y = insCell.leftBottomY;
	//		rotatedPoint.z = insCell.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		insCell.leftBottomX = unrotatedPoint.x;
	//		insCell.leftBottomY = unrotatedPoint.y;
	//		insCell.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placingZone->placeLibPart (insCell));

	//		// ����
	//		insCell.ang = placingZone->ang;
	//		insCell.attached_side = RIGHT_SIDE;
	//		insCell.dirLen = placingZone->marginBeginAtSide;
	//		insCell.perLen = cellHeight_side;
	//		insCell.leftBottomX = placingZone->begC.x;
	//		insCell.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
	//		insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;

	//		if (placingZone->marginBeginAtSide < 0.110) {
	//			insCell.objType = WOOD;
	//			insCell.libPart.wood.w_ang = DegreeToRad (90.0);
	//			insCell.libPart.wood.w_h = placingZone->marginBeginAtSide;
	//			insCell.libPart.wood.w_leng = cellHeight_side;
	//			insCell.libPart.wood.w_w = 0.040;
	//			insCell.leftBottomX += placingZone->marginBeginAtSide;
	//		} else {
	//			insCell.objType = PLYWOOD;
	//			insCell.libPart.plywood.p_leng = placingZone->marginBeginAtSide;
	//			insCell.libPart.plywood.p_wid = cellHeight_side;
	//		}

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = insCell.leftBottomX;
	//		rotatedPoint.y = insCell.leftBottomY;
	//		rotatedPoint.z = insCell.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		insCell.leftBottomX = unrotatedPoint.x;
	//		insCell.leftBottomY = unrotatedPoint.y;
	//		insCell.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placingZone->placeLibPart (insCell));
	//	}
	//}

	//// ���� �� �κ� ���� ä��
	//if (placingZone->bFillMarginEndAtSide == true) {
	//	if (placingZone->marginEndAtSide > EPS) {
	//		// ����
	//		insCell.ang = placingZone->ang;
	//		insCell.attached_side = LEFT_SIDE;
	//		insCell.dirLen = placingZone->marginEndAtSide;
	//		insCell.perLen = cellHeight_side;
	//		insCell.leftBottomX = placingZone->begC.x + placingZone->beamLength - placingZone->marginEndAtSide;
	//		insCell.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
	//		insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;

	//		if (placingZone->marginBeginAtSide < 0.110) {
	//			insCell.objType = WOOD;
	//			insCell.libPart.wood.w_ang = DegreeToRad (90.0);
	//			insCell.libPart.wood.w_h = placingZone->marginEndAtSide;
	//			insCell.libPart.wood.w_leng = cellHeight_side;
	//			insCell.libPart.wood.w_w = 0.040;
	//			insCell.leftBottomX -= cellHeight_side;
	//		} else {
	//			insCell.objType = PLYWOOD;
	//			insCell.libPart.plywood.p_leng = placingZone->marginEndAtSide;
	//			insCell.libPart.plywood.p_wid = cellHeight_side;
	//		}

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = insCell.leftBottomX;
	//		rotatedPoint.y = insCell.leftBottomY;
	//		rotatedPoint.z = insCell.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		insCell.leftBottomX = unrotatedPoint.x;
	//		insCell.leftBottomY = unrotatedPoint.y;
	//		insCell.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placingZone->placeLibPart (insCell));

	//		// ����
	//		insCell.ang = placingZone->ang;
	//		insCell.attached_side = RIGHT_SIDE;
	//		insCell.dirLen = placingZone->marginEndAtSide;
	//		insCell.perLen = cellHeight_side;
	//		insCell.leftBottomX = placingZone->begC.x + placingZone->beamLength - placingZone->marginEndAtSide;
	//		insCell.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
	//		insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;

	//		if (placingZone->marginEndAtSide < 0.110) {
	//			insCell.objType = WOOD;
	//			insCell.libPart.wood.w_ang = DegreeToRad (90.0);
	//			insCell.libPart.wood.w_h = placingZone->marginEndAtSide;
	//			insCell.libPart.wood.w_leng = cellHeight_side;
	//			insCell.libPart.wood.w_w = 0.040;
	//			insCell.leftBottomX += placingZone->marginEndAtSide;
	//		} else {
	//			insCell.objType = PLYWOOD;
	//			insCell.libPart.plywood.p_leng = placingZone->marginEndAtSide;
	//			insCell.libPart.plywood.p_wid = cellHeight_side;
	//		}

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = insCell.leftBottomX;
	//		rotatedPoint.y = insCell.leftBottomY;
	//		rotatedPoint.z = insCell.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		insCell.leftBottomX = unrotatedPoint.x;
	//		insCell.leftBottomY = unrotatedPoint.y;
	//		insCell.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placingZone->placeLibPart (insCell));
	//	}
	//}

	//// �Ϻ� ���� �κ� ���� ä��
	//if (placingZone->bFillMarginBeginAtBottom == true) {
	//	if (placingZone->marginBeginAtBottom > EPS) {
	//		insCell.ang = placingZone->ang;
	//		insCell.attached_side = BOTTOM_SIDE;
	//		insCell.dirLen = placingZone->marginBeginAtBottom;
	//		insCell.perLen = cellHeight_bottom;
	//		insCell.leftBottomX = placingZone->begC.x;
	//		insCell.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
	//		insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;

	//		if (placingZone->marginBeginAtBottom < 0.110) {
	//			insCell.objType = WOOD;
	//			insCell.libPart.wood.w_ang = 0.0;
	//			insCell.libPart.wood.w_h = placingZone->marginBeginAtBottom;
	//			insCell.libPart.wood.w_leng = cellHeight_bottom;
	//			insCell.libPart.wood.w_w = 0.040;
	//			insCell.ang -= DegreeToRad (90.0);
	//			insCell.leftBottomX += placingZone->marginBeginAtBottom;
	//		} else {
	//			insCell.objType = PLYWOOD;
	//			insCell.libPart.plywood.p_leng = placingZone->marginBeginAtBottom;
	//			insCell.libPart.plywood.p_wid = cellHeight_bottom;
	//		}

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = insCell.leftBottomX;
	//		rotatedPoint.y = insCell.leftBottomY;
	//		rotatedPoint.z = insCell.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		insCell.leftBottomX = unrotatedPoint.x;
	//		insCell.leftBottomY = unrotatedPoint.y;
	//		insCell.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placingZone->placeLibPart (insCell));
	//	}
	//}
	//
	//// �Ϻ� �� �κ� ���� ä��
	//if (placingZone->bFillMarginEndAtBottom == true) {
	//	if (placingZone->marginEndAtBottom > EPS) {
	//		insCell.ang = placingZone->ang;
	//		insCell.attached_side = BOTTOM_SIDE;
	//		insCell.dirLen = placingZone->marginEndAtBottom;
	//		insCell.perLen = cellHeight_bottom;
	//		insCell.leftBottomX = placingZone->begC.x + placingZone->beamLength - placingZone->marginEndAtBottom;
	//		insCell.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
	//		insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;

	//		if (placingZone->marginEndAtBottom < 0.110) {
	//			insCell.objType = WOOD;
	//			insCell.libPart.wood.w_ang = 0.0;
	//			insCell.libPart.wood.w_h = placingZone->marginEndAtBottom;
	//			insCell.libPart.wood.w_leng = cellHeight_bottom;
	//			insCell.libPart.wood.w_w = 0.040;
	//			insCell.ang -= DegreeToRad (90.0);
	//			insCell.leftBottomX += placingZone->marginEndAtBottom;
	//		} else {
	//			insCell.objType = PLYWOOD;
	//			insCell.libPart.plywood.p_leng = placingZone->marginEndAtBottom;
	//			insCell.libPart.plywood.p_wid = cellHeight_bottom;
	//		}

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = insCell.leftBottomX;
	//		rotatedPoint.y = insCell.leftBottomY;
	//		rotatedPoint.z = insCell.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		insCell.leftBottomX = unrotatedPoint.x;
	//		insCell.leftBottomY = unrotatedPoint.y;
	//		insCell.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placingZone->placeLibPart (insCell));
	//	}
	//}

	//// �߽ɺ��� ������ �̵��ؾ� ��
	//accumDist = 0.0;
	//for (xx = 0 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)
	//	if (placingZone->cellsFromBeginAtRSide [0][xx].objType != NONE)
	//		accumDist += placingZone->cellsFromBeginAtRSide [0][xx].dirLen;

	//// �ƿ��ڳʾޱ� ��ġ (���� ���� �κ�)
	//length_outa = 0.0;
	//for (xx = 0 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx) {
	//	if (placingZone->cellsFromBeginAtLSide [0][xx].objType != NONE) {
	//		length_outa += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//	}
	//}

	//xPos = centerPos - width_side/2 - accumDist;
	//while (length_outa > EPS) {
	//	// ����
	//	insCell.objType = OUTCORNER_ANGLE;
	//	insCell.ang = placingZone->ang;
	//	insCell.attached_side = LEFT_SIDE;
	//	insCell.leftBottomX = placingZone->begC.x + xPos;
	//	insCell.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
	//	insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//	if (length_outa > 2.400)
	//		insCell.libPart.outangle.a_leng = 2.400;
	//	else
	//		insCell.libPart.outangle.a_leng = length_outa;

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = insCell.leftBottomX;
	//	rotatedPoint.y = insCell.leftBottomY;
	//	rotatedPoint.z = insCell.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	insCell.leftBottomX = unrotatedPoint.x;
	//	insCell.leftBottomY = unrotatedPoint.y;
	//	insCell.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placingZone->placeLibPart (insCell));

	//	// ����
	//	insCell.objType = OUTCORNER_ANGLE;
	//	insCell.ang = placingZone->ang;
	//	insCell.attached_side = RIGHT_SIDE;
	//	insCell.leftBottomX = placingZone->begC.x + xPos;
	//	insCell.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
	//	insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//	if (length_outa > 2.400)
	//		insCell.libPart.outangle.a_leng = 2.400;
	//	else
	//		insCell.libPart.outangle.a_leng = length_outa;

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = insCell.leftBottomX;
	//	rotatedPoint.y = insCell.leftBottomY;
	//	rotatedPoint.z = insCell.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	insCell.leftBottomX = unrotatedPoint.x;
	//	insCell.leftBottomY = unrotatedPoint.y;
	//	insCell.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placingZone->placeLibPart (insCell));

	//	// �Ÿ� �̵�
	//	xPos += insCell.libPart.outangle.a_leng;

	//	// ���� �Ÿ� ����
	//	if (length_outa > 2.400)
	//		length_outa -= 2.400;
	//	else
	//		length_outa = 0.0;
	//}

	//// �߽ɺ��� ������ �̵��ؾ� ��
	//accumDist = 0.0;
	//for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)
	//	if (placingZone->cellsFromEndAtRSide [0][xx].objType != NONE)
	//		accumDist += placingZone->cellsFromEndAtRSide [0][xx].dirLen;

	//// �ƿ��ڳʾޱ� ��ġ (���� �� �κ�)
	//length_outa = 0.0;
	//for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx) {
	//	if (placingZone->cellsFromEndAtLSide [0][xx].objType != NONE) {
	//		length_outa += placingZone->cellsFromEndAtLSide [0][xx].dirLen;
	//	}
	//}

	//xPos = centerPos + width_side/2;
	//while (length_outa > EPS) {
	//	// ����
	//	insCell.objType = OUTCORNER_ANGLE;
	//	insCell.ang = placingZone->ang;
	//	insCell.attached_side = LEFT_SIDE;
	//	insCell.leftBottomX = placingZone->begC.x + xPos;
	//	insCell.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
	//	insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//	if (length_outa > 2.400)
	//		insCell.libPart.outangle.a_leng = 2.400;
	//	else
	//		insCell.libPart.outangle.a_leng = length_outa;

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = insCell.leftBottomX;
	//	rotatedPoint.y = insCell.leftBottomY;
	//	rotatedPoint.z = insCell.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	insCell.leftBottomX = unrotatedPoint.x;
	//	insCell.leftBottomY = unrotatedPoint.y;
	//	insCell.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placingZone->placeLibPart (insCell));

	//	// ����
	//	insCell.objType = OUTCORNER_ANGLE;
	//	insCell.ang = placingZone->ang;
	//	insCell.attached_side = RIGHT_SIDE;
	//	insCell.leftBottomX = placingZone->begC.x + xPos;
	//	insCell.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
	//	insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//	if (length_outa > 2.400)
	//		insCell.libPart.outangle.a_leng = 2.400;
	//	else
	//		insCell.libPart.outangle.a_leng = length_outa;

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = insCell.leftBottomX;
	//	rotatedPoint.y = insCell.leftBottomY;
	//	rotatedPoint.z = insCell.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	insCell.leftBottomX = unrotatedPoint.x;
	//	insCell.leftBottomY = unrotatedPoint.y;
	//	insCell.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placingZone->placeLibPart (insCell));

	//	// �Ÿ� �̵�
	//	xPos += insCell.libPart.outangle.a_leng;

	//	// ���� �Ÿ� ����
	//	if (length_outa > 2.400)
	//		length_outa -= 2.400;
	//	else
	//		length_outa = 0.0;
	//}

	//// �ƿ��ڳ� �ޱ� ��ġ (�߾� �κ�)
	//xPos = centerPos - width_side/2;
	//if (placingZone->bInterfereBeam == false) {
	//	if (placingZone->cellCenterAtRSide [0].objType == EUROFORM) {
	//		// ����
	//		insCell.objType = OUTCORNER_ANGLE;
	//		insCell.ang = placingZone->ang;
	//		insCell.attached_side = LEFT_SIDE;
	//		insCell.leftBottomX = placingZone->begC.x + xPos;
	//		insCell.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
	//		insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//		insCell.libPart.outangle.a_leng = placingZone->cellCenterAtLSide [0].dirLen;

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = insCell.leftBottomX;
	//		rotatedPoint.y = insCell.leftBottomY;
	//		rotatedPoint.z = insCell.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		insCell.leftBottomX = unrotatedPoint.x;
	//		insCell.leftBottomY = unrotatedPoint.y;
	//		insCell.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placingZone->placeLibPart (insCell));

	//		// ����
	//		insCell.objType = OUTCORNER_ANGLE;
	//		insCell.ang = placingZone->ang;
	//		insCell.attached_side = RIGHT_SIDE;
	//		insCell.leftBottomX = placingZone->begC.x + xPos;
	//		insCell.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
	//		insCell.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//		insCell.libPart.outangle.a_leng = placingZone->cellCenterAtRSide [0].dirLen;

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = insCell.leftBottomX;
	//		rotatedPoint.y = insCell.leftBottomY;
	//		rotatedPoint.z = insCell.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		insCell.leftBottomX = unrotatedPoint.x;
	//		insCell.leftBottomY = unrotatedPoint.y;
	//		insCell.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placingZone->placeLibPart (insCell));
	//	}
	//}

	//// ��������� 1�� (���� ���� �κ� - ����), ���� ���� ������ ������ (���� �� �κ� - ����)����
	//xPos = 0.0;
	//length_pipe = 0.100;
	//for (xx = 0 ; xx < nCellsFromBeginAtSide ; ++xx)
	//	length_pipe += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//if (abs (placingZone->cellCenterAtLSide [0].dirLen) < EPS) {
	//	for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)
	//		length_pipe += placingZone->cellsFromEndAtLSide [0][xx].dirLen;
	//}

	//if ((placingZone->cellsFromBeginAtLSide [0][0].perLen > EPS) && (placingZone->nCellsFromBeginAtSide > 0)) {
	//	while (length_pipe > 0.0) {
	//		squarePipe.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) - 0.050 + xPos;
	//		squarePipe.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0885;
	//		squarePipe.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//		if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.600) < EPS)		squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.500) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.050);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.450) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.400) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.100);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.300) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.200) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.050);
	//		squarePipe.ang = placingZone->ang;
	//		if (length_pipe > 6.000)
	//			squarePipe.length = 6.000;
	//		else
	//			squarePipe.length = length_pipe;
	//		squarePipe.pipeAng = DegreeToRad (0.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = squarePipe.leftBottomX;
	//		rotatedPoint.y = squarePipe.leftBottomY;
	//		rotatedPoint.z = squarePipe.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		squarePipe.leftBottomX = unrotatedPoint.x;
	//		squarePipe.leftBottomY = unrotatedPoint.y;
	//		squarePipe.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (squarePipe));		// 1��

	//		if (placingZone->cellsFromBeginAtLSide [0][0].perLen > 0.300) {
	//			squarePipe.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) - 0.050 + xPos;
	//			squarePipe.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0885;
	//			if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.600) < EPS)		squarePipe.leftBottomZ += 0.300;
	//			else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.500) < EPS)	squarePipe.leftBottomZ += 0.250;
	//			else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.450) < EPS)	squarePipe.leftBottomZ += 0.150;
	//			else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.400) < EPS)	squarePipe.leftBottomZ += 0.150;

	//			axisPoint.x = placingZone->begC.x;
	//			axisPoint.y = placingZone->begC.y;
	//			axisPoint.z = placingZone->begC.z;

	//			rotatedPoint.x = squarePipe.leftBottomX;
	//			rotatedPoint.y = squarePipe.leftBottomY;
	//			rotatedPoint.z = squarePipe.leftBottomZ;
	//			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//			squarePipe.leftBottomX = unrotatedPoint.x;
	//			squarePipe.leftBottomY = unrotatedPoint.y;
	//			squarePipe.leftBottomZ = unrotatedPoint.z;

	//			elemList.Push (placeLibPart (squarePipe));		// 2��
	//		}

	//		xPos += squarePipe.length;

	//		if (length_pipe > 6.000)
	//			length_pipe -= 6.000;
	//		else
	//			length_pipe = 0.0;
	//	}
	//}

	//// ��������� 2�� (���� ���� �κ� - ����), ���� ���� ������ ������ (���� �� �κ� - ����)����
	//xPos = 0.0;
	//length_pipe = 0.100;
	//for (xx = 0 ; xx < nCellsFromBeginAtSide ; ++xx)
	//	length_pipe += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//if (abs (placingZone->cellCenterAtLSide [0].dirLen) < EPS) {
	//	for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)
	//		length_pipe += placingZone->cellsFromEndAtLSide [0][xx].dirLen;
	//}

	//if ((placingZone->cellsFromBeginAtLSide [2][0].perLen > EPS) && (placingZone->nCellsFromBeginAtSide > 0)) {
	//	while (length_pipe > 0.0) {
	//		squarePipe.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) - 0.050 + xPos;
	//		squarePipe.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0885;
	//		squarePipe.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + placingZone->cellsFromBeginAtLSide [0][0].perLen + placingZone->cellsFromBeginAtLSide [1][0].perLen;
	//		if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.600) < EPS)		squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.500) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.050);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.450) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.400) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.100);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.300) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.200) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.050);
	//		squarePipe.ang = placingZone->ang;
	//		if (length_pipe > 6.000)
	//			squarePipe.length = 6.000;
	//		else
	//			squarePipe.length = length_pipe;
	//		squarePipe.pipeAng = DegreeToRad (0.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = squarePipe.leftBottomX;
	//		rotatedPoint.y = squarePipe.leftBottomY;
	//		rotatedPoint.z = squarePipe.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		squarePipe.leftBottomX = unrotatedPoint.x;
	//		squarePipe.leftBottomY = unrotatedPoint.y;
	//		squarePipe.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (squarePipe));		// 1��

	//		if (placingZone->cellsFromBeginAtLSide [2][0].perLen > 0.300) {
	//			squarePipe.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) - 0.050 + xPos;
	//			squarePipe.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0885;
	//			if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.600) < EPS)		squarePipe.leftBottomZ += 0.300;
	//			else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.500) < EPS)	squarePipe.leftBottomZ += 0.250;
	//			else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.450) < EPS)	squarePipe.leftBottomZ += 0.150;
	//			else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.400) < EPS)	squarePipe.leftBottomZ += 0.150;

	//			axisPoint.x = placingZone->begC.x;
	//			axisPoint.y = placingZone->begC.y;
	//			axisPoint.z = placingZone->begC.z;

	//			rotatedPoint.x = squarePipe.leftBottomX;
	//			rotatedPoint.y = squarePipe.leftBottomY;
	//			rotatedPoint.z = squarePipe.leftBottomZ;
	//			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//			squarePipe.leftBottomX = unrotatedPoint.x;
	//			squarePipe.leftBottomY = unrotatedPoint.y;
	//			squarePipe.leftBottomZ = unrotatedPoint.z;

	//			elemList.Push (placeLibPart (squarePipe));		// 2��
	//		}

	//		xPos += squarePipe.length;

	//		if (length_pipe > 6.000)
	//			length_pipe -= 6.000;
	//		else
	//			length_pipe = 0.0;
	//	}
	//}

	//// ��������� 1�� (���� ���� �κ� - ������), ���� ���� ������ ������ (���� �� �κ� - ������)����
	//xPos = 0.0;
	//length_pipe = 0.100;
	//for (xx = 0 ; xx < nCellsFromBeginAtSide ; ++xx)
	//	length_pipe += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//if (abs (placingZone->cellCenterAtLSide [0].dirLen) < EPS) {
	//	for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)
	//		length_pipe += placingZone->cellsFromEndAtLSide [0][xx].dirLen;
	//}

	//if ((placingZone->cellsFromBeginAtLSide [0][0].perLen > EPS) && (placingZone->nCellsFromBeginAtSide > 0)) {
	//	while (length_pipe > 0.0) {
	//		squarePipe.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) - 0.050 + xPos;
	//		squarePipe.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0885;
	//		squarePipe.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//		if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.600) < EPS)		squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.500) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.050);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.450) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.400) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.100);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.300) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.200) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.050);
	//		squarePipe.ang = placingZone->ang;
	//		if (length_pipe > 6.000)
	//			squarePipe.length = 6.000;
	//		else
	//			squarePipe.length = length_pipe;
	//		squarePipe.pipeAng = DegreeToRad (0.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = squarePipe.leftBottomX;
	//		rotatedPoint.y = squarePipe.leftBottomY;
	//		rotatedPoint.z = squarePipe.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		squarePipe.leftBottomX = unrotatedPoint.x;
	//		squarePipe.leftBottomY = unrotatedPoint.y;
	//		squarePipe.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (squarePipe));		// 1��

	//		if (placingZone->cellsFromBeginAtLSide [0][0].perLen > 0.300) {
	//			squarePipe.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) - 0.050 + xPos;
	//			squarePipe.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0885;
	//			if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.600) < EPS)		squarePipe.leftBottomZ += 0.300;
	//			else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.500) < EPS)	squarePipe.leftBottomZ += 0.250;
	//			else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.450) < EPS)	squarePipe.leftBottomZ += 0.150;
	//			else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.400) < EPS)	squarePipe.leftBottomZ += 0.150;

	//			axisPoint.x = placingZone->begC.x;
	//			axisPoint.y = placingZone->begC.y;
	//			axisPoint.z = placingZone->begC.z;

	//			rotatedPoint.x = squarePipe.leftBottomX;
	//			rotatedPoint.y = squarePipe.leftBottomY;
	//			rotatedPoint.z = squarePipe.leftBottomZ;
	//			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//			squarePipe.leftBottomX = unrotatedPoint.x;
	//			squarePipe.leftBottomY = unrotatedPoint.y;
	//			squarePipe.leftBottomZ = unrotatedPoint.z;

	//			elemList.Push (placeLibPart (squarePipe));		// 2��
	//		}

	//		xPos += squarePipe.length;

	//		if (length_pipe > 6.000)
	//			length_pipe -= 6.000;
	//		else
	//			length_pipe = 0.0;
	//	}
	//}

	//// ��������� 2�� (���� ���� �κ� - ������), ���� ���� ������ ������ (���� �� �κ� - ������)����
	//xPos = 0.0;
	//length_pipe = 0.100;
	//for (xx = 0 ; xx < nCellsFromBeginAtSide ; ++xx)
	//	length_pipe += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//if (abs (placingZone->cellCenterAtLSide [0].dirLen) < EPS) {
	//	for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)
	//		length_pipe += placingZone->cellsFromEndAtLSide [0][xx].dirLen;
	//}

	//if ((placingZone->cellsFromBeginAtLSide [2][0].perLen > EPS) && (placingZone->nCellsFromBeginAtSide > 0)) {
	//	while (length_pipe > 0.0) {
	//		squarePipe.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) - 0.050 + xPos;
	//		squarePipe.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0885;
	//		squarePipe.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + placingZone->cellsFromBeginAtLSide [0][0].perLen + placingZone->cellsFromBeginAtLSide [1][0].perLen;
	//		if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.600) < EPS)		squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.500) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.050);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.450) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.400) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.100);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.300) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.200) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.050);
	//		squarePipe.ang = placingZone->ang;
	//		if (length_pipe > 6.000)
	//			squarePipe.length = 6.000;
	//		else
	//			squarePipe.length = length_pipe;
	//		squarePipe.pipeAng = DegreeToRad (0.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = squarePipe.leftBottomX;
	//		rotatedPoint.y = squarePipe.leftBottomY;
	//		rotatedPoint.z = squarePipe.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		squarePipe.leftBottomX = unrotatedPoint.x;
	//		squarePipe.leftBottomY = unrotatedPoint.y;
	//		squarePipe.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (squarePipe));		// 1��

	//		if (placingZone->cellsFromBeginAtLSide [2][0].perLen > 0.300) {
	//			squarePipe.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) - 0.050 + xPos;
	//			squarePipe.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0885;
	//			if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.600) < EPS)		squarePipe.leftBottomZ += 0.300;
	//			else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.500) < EPS)	squarePipe.leftBottomZ += 0.250;
	//			else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.450) < EPS)	squarePipe.leftBottomZ += 0.150;
	//			else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.400) < EPS)	squarePipe.leftBottomZ += 0.150;

	//			axisPoint.x = placingZone->begC.x;
	//			axisPoint.y = placingZone->begC.y;
	//			axisPoint.z = placingZone->begC.z;

	//			rotatedPoint.x = squarePipe.leftBottomX;
	//			rotatedPoint.y = squarePipe.leftBottomY;
	//			rotatedPoint.z = squarePipe.leftBottomZ;
	//			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//			squarePipe.leftBottomX = unrotatedPoint.x;
	//			squarePipe.leftBottomY = unrotatedPoint.y;
	//			squarePipe.leftBottomZ = unrotatedPoint.z;

	//			elemList.Push (placeLibPart (squarePipe));		// 2��
	//		}

	//		xPos += squarePipe.length;

	//		if (length_pipe > 6.000)
	//			length_pipe -= 6.000;
	//		else
	//			length_pipe = 0.0;
	//	}
	//}

	//// ��������� 1�� (���� �� �κ� - ����), ���� ���� ������ ������ ����
	//xPos = centerPos;
	//length_pipe = 0.100;
	//for (xx = 0 ; xx < nCellsFromEndAtSide ; ++xx)
	//	length_pipe += placingZone->cellsFromEndAtLSide [0][xx].dirLen;

	//if ((placingZone->cellsFromEndAtLSide [0][0].perLen > EPS) && (placingZone->nCellsFromEndAtSide > 0) && (abs (placingZone->cellCenterAtLSide [0].dirLen) > EPS)) {
	//	while (length_pipe > 0.0) {
	//		squarePipe.leftBottomX = placingZone->begC.x - 0.050 + xPos + (placingZone->cellCenterAtLSide [0].dirLen / 2);
	//		squarePipe.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0885;
	//		squarePipe.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//		if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.600) < EPS)			squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.500) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.050);
	//		else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.450) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.400) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.100);
	//		else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.300) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.200) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.050);
	//		squarePipe.ang = placingZone->ang;
	//		if (length_pipe > 6.000)
	//			squarePipe.length = 6.000;
	//		else
	//			squarePipe.length = length_pipe;
	//		squarePipe.pipeAng = DegreeToRad (0.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = squarePipe.leftBottomX;
	//		rotatedPoint.y = squarePipe.leftBottomY;
	//		rotatedPoint.z = squarePipe.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		squarePipe.leftBottomX = unrotatedPoint.x;
	//		squarePipe.leftBottomY = unrotatedPoint.y;
	//		squarePipe.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (squarePipe));		// 1��

	//		if (placingZone->cellsFromEndAtLSide [0][0].perLen > 0.300) {
	//			squarePipe.leftBottomX = placingZone->begC.x - 0.050 + xPos + (placingZone->cellCenterAtLSide [0].dirLen / 2);
	//			squarePipe.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0885;
	//			if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.600) < EPS)			squarePipe.leftBottomZ += 0.300;
	//			else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.500) < EPS)	squarePipe.leftBottomZ += 0.250;
	//			else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.450) < EPS)	squarePipe.leftBottomZ += 0.150;
	//			else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.400) < EPS)	squarePipe.leftBottomZ += 0.150;

	//			axisPoint.x = placingZone->begC.x;
	//			axisPoint.y = placingZone->begC.y;
	//			axisPoint.z = placingZone->begC.z;

	//			rotatedPoint.x = squarePipe.leftBottomX;
	//			rotatedPoint.y = squarePipe.leftBottomY;
	//			rotatedPoint.z = squarePipe.leftBottomZ;
	//			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//			squarePipe.leftBottomX = unrotatedPoint.x;
	//			squarePipe.leftBottomY = unrotatedPoint.y;
	//			squarePipe.leftBottomZ = unrotatedPoint.z;

	//			elemList.Push (placeLibPart (squarePipe));		// 2��
	//		}

	//		xPos += squarePipe.length;

	//		if (length_pipe > 6.000)
	//			length_pipe -= 6.000;
	//		else
	//			length_pipe = 0.0;
	//	}
	//}

	//// ��������� 2�� (���� �� �κ� - ����), ���� ���� ������ ������ ����
	//xPos = centerPos;
	//length_pipe = 0.100;
	//for (xx = 0 ; xx < nCellsFromEndAtSide ; ++xx)
	//	length_pipe += placingZone->cellsFromEndAtLSide [0][xx].dirLen;

	//if ((placingZone->cellsFromEndAtLSide [2][0].perLen > EPS) && (placingZone->nCellsFromEndAtSide > 0) && (abs (placingZone->cellCenterAtLSide [0].dirLen) > EPS)) {
	//	while (length_pipe > 0.0) {
	//		squarePipe.leftBottomX = placingZone->begC.x - 0.050 + xPos + (placingZone->cellCenterAtLSide [0].dirLen / 2);
	//		squarePipe.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0885;
	//		squarePipe.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + placingZone->cellsFromEndAtLSide [0][0].perLen + placingZone->cellsFromEndAtLSide [1][0].perLen;
	//		if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.600) < EPS)			squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.500) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.050);
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.450) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.400) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.100);
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.300) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.200) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.050);
	//		squarePipe.ang = placingZone->ang;
	//		if (length_pipe > 6.000)
	//			squarePipe.length = 6.000;
	//		else
	//			squarePipe.length = length_pipe;
	//		squarePipe.pipeAng = DegreeToRad (0.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = squarePipe.leftBottomX;
	//		rotatedPoint.y = squarePipe.leftBottomY;
	//		rotatedPoint.z = squarePipe.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		squarePipe.leftBottomX = unrotatedPoint.x;
	//		squarePipe.leftBottomY = unrotatedPoint.y;
	//		squarePipe.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (squarePipe));		// 1��

	//		if (placingZone->cellsFromEndAtLSide [2][0].perLen > 0.300) {
	//			squarePipe.leftBottomX = placingZone->begC.x - 0.050 + xPos + (placingZone->cellCenterAtLSide [0].dirLen / 2);
	//			squarePipe.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0885;
	//			if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.600) < EPS)			squarePipe.leftBottomZ += 0.300;
	//			else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.500) < EPS)	squarePipe.leftBottomZ += 0.250;
	//			else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.450) < EPS)	squarePipe.leftBottomZ += 0.150;
	//			else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.400) < EPS)	squarePipe.leftBottomZ += 0.150;

	//			axisPoint.x = placingZone->begC.x;
	//			axisPoint.y = placingZone->begC.y;
	//			axisPoint.z = placingZone->begC.z;

	//			rotatedPoint.x = squarePipe.leftBottomX;
	//			rotatedPoint.y = squarePipe.leftBottomY;
	//			rotatedPoint.z = squarePipe.leftBottomZ;
	//			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//			squarePipe.leftBottomX = unrotatedPoint.x;
	//			squarePipe.leftBottomY = unrotatedPoint.y;
	//			squarePipe.leftBottomZ = unrotatedPoint.z;

	//			elemList.Push (placeLibPart (squarePipe));		// 2��
	//		}

	//		xPos += squarePipe.length;

	//		if (length_pipe > 6.000)
	//			length_pipe -= 6.000;
	//		else
	//			length_pipe = 0.0;
	//	}
	//}

	//// ��������� 1�� (���� �� �κ� - ������), ���� ���� ������ ������ ����
	//xPos = centerPos;
	//length_pipe = 0.100;
	//for (xx = 0 ; xx < nCellsFromEndAtSide ; ++xx)
	//	length_pipe += placingZone->cellsFromEndAtLSide [0][xx].dirLen;

	//if ((placingZone->cellsFromEndAtLSide [0][0].perLen > EPS) && (placingZone->nCellsFromEndAtSide > 0) && (abs (placingZone->cellCenterAtLSide [0].dirLen) > EPS)) {
	//	while (length_pipe > 0.0) {
	//		squarePipe.leftBottomX = placingZone->begC.x - 0.050 + xPos + (placingZone->cellCenterAtLSide [0].dirLen / 2);
	//		squarePipe.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0885;
	//		squarePipe.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//		if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.600) < EPS)			squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.500) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.050);
	//		else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.450) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.400) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.100);
	//		else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.300) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.200) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.050);
	//		squarePipe.ang = placingZone->ang;
	//		if (length_pipe > 6.000)
	//			squarePipe.length = 6.000;
	//		else
	//			squarePipe.length = length_pipe;
	//		squarePipe.pipeAng = DegreeToRad (0.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = squarePipe.leftBottomX;
	//		rotatedPoint.y = squarePipe.leftBottomY;
	//		rotatedPoint.z = squarePipe.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		squarePipe.leftBottomX = unrotatedPoint.x;
	//		squarePipe.leftBottomY = unrotatedPoint.y;
	//		squarePipe.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (squarePipe));		// 1��

	//		if (placingZone->cellsFromEndAtLSide [0][0].perLen > 0.300) {
	//			squarePipe.leftBottomX = placingZone->begC.x - 0.050 + xPos + (placingZone->cellCenterAtLSide [0].dirLen / 2);
	//			squarePipe.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0885;
	//			if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.600) < EPS)			squarePipe.leftBottomZ += 0.300;
	//			else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.500) < EPS)	squarePipe.leftBottomZ += 0.250;
	//			else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.450) < EPS)	squarePipe.leftBottomZ += 0.150;
	//			else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.400) < EPS)	squarePipe.leftBottomZ += 0.150;

	//			axisPoint.x = placingZone->begC.x;
	//			axisPoint.y = placingZone->begC.y;
	//			axisPoint.z = placingZone->begC.z;

	//			rotatedPoint.x = squarePipe.leftBottomX;
	//			rotatedPoint.y = squarePipe.leftBottomY;
	//			rotatedPoint.z = squarePipe.leftBottomZ;
	//			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//			squarePipe.leftBottomX = unrotatedPoint.x;
	//			squarePipe.leftBottomY = unrotatedPoint.y;
	//			squarePipe.leftBottomZ = unrotatedPoint.z;

	//			elemList.Push (placeLibPart (squarePipe));		// 2��
	//		}

	//		xPos += squarePipe.length;

	//		if (length_pipe > 6.000)
	//			length_pipe -= 6.000;
	//		else
	//			length_pipe = 0.0;
	//	}
	//}

	//// ��������� 2�� (���� �� �κ� - ������), ���� ���� ������ ������ ����
	//xPos = centerPos;
	//length_pipe = 0.100;
	//for (xx = 0 ; xx < nCellsFromEndAtSide ; ++xx)
	//	length_pipe += placingZone->cellsFromEndAtLSide [0][xx].dirLen;

	//if ((placingZone->cellsFromEndAtLSide [2][0].perLen > EPS) && (placingZone->nCellsFromEndAtSide > 0) && (abs (placingZone->cellCenterAtLSide [0].dirLen) > EPS)) {
	//	while (length_pipe > 0.0) {
	//		squarePipe.leftBottomX = placingZone->begC.x - 0.050 + xPos + (placingZone->cellCenterAtLSide [0].dirLen / 2);
	//		squarePipe.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0885;
	//		squarePipe.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + placingZone->cellsFromEndAtLSide [0][0].perLen + placingZone->cellsFromEndAtLSide [1][0].perLen;
	//		if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.600) < EPS)			squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.500) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.050);
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.450) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.400) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.100);
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.300) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.150);
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.200) < EPS)	squarePipe.leftBottomZ += (0.030 + 0.050);
	//		squarePipe.ang = placingZone->ang;
	//		if (length_pipe > 6.000)
	//			squarePipe.length = 6.000;
	//		else
	//			squarePipe.length = length_pipe;
	//		squarePipe.pipeAng = DegreeToRad (0.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = squarePipe.leftBottomX;
	//		rotatedPoint.y = squarePipe.leftBottomY;
	//		rotatedPoint.z = squarePipe.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		squarePipe.leftBottomX = unrotatedPoint.x;
	//		squarePipe.leftBottomY = unrotatedPoint.y;
	//		squarePipe.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (squarePipe));		// 1��

	//		if (placingZone->cellsFromEndAtLSide [2][0].perLen > 0.300) {
	//			squarePipe.leftBottomX = placingZone->begC.x - 0.050 + xPos + (placingZone->cellCenterAtLSide [0].dirLen / 2);
	//			squarePipe.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0885;
	//			if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.600) < EPS)			squarePipe.leftBottomZ += 0.300;
	//			else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.500) < EPS)	squarePipe.leftBottomZ += 0.250;
	//			else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.450) < EPS)	squarePipe.leftBottomZ += 0.150;
	//			else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.400) < EPS)	squarePipe.leftBottomZ += 0.150;

	//			axisPoint.x = placingZone->begC.x;
	//			axisPoint.y = placingZone->begC.y;
	//			axisPoint.z = placingZone->begC.z;

	//			rotatedPoint.x = squarePipe.leftBottomX;
	//			rotatedPoint.y = squarePipe.leftBottomY;
	//			rotatedPoint.z = squarePipe.leftBottomZ;
	//			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//			squarePipe.leftBottomX = unrotatedPoint.x;
	//			squarePipe.leftBottomY = unrotatedPoint.y;
	//			squarePipe.leftBottomZ = unrotatedPoint.z;

	//			elemList.Push (placeLibPart (squarePipe));		// 2��
	//		}

	//		xPos += squarePipe.length;

	//		if (length_pipe > 6.000)
	//			length_pipe -= 6.000;
	//		else
	//			length_pipe = 0.0;
	//	}
	//}

	//// ��������� ���� (�Ϻ� ���� �κ�), ���� ���� ������ ������ (���� - �Ϻ� �� �κ�)����
	//xPos = 0.0;
	//length_pipe = 0.100;
	//for (xx = 0 ; xx < nCellsFromBeginAtBottom ; ++xx)
	//	length_pipe += placingZone->cellsFromBeginAtBottom [0][xx].dirLen;
	//if (abs (placingZone->cellCenterAtBottom [0].dirLen) < EPS) {
	//	for (xx = 0 ; xx < placingZone->nCellsFromEndAtBottom ; ++xx)
	//		length_pipe += placingZone->cellsFromEndAtBottom [0][xx].dirLen;
	//}

	//if ((placingZone->cellsFromBeginAtBottom [0][0].perLen > EPS) && (placingZone->nCellsFromBeginAtBottom > 0)) {
	//	while (length_pipe > 0.0) {
	//		squarePipe.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtBottom * marginBeginAtBottom) - 0.050 + xPos;
	//		squarePipe.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
	//		squarePipe.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0885;
	//		squarePipe.ang = placingZone->ang;
	//		if (length_pipe > 6.000)
	//			squarePipe.length = 6.000;
	//		else
	//			squarePipe.length = length_pipe;
	//		squarePipe.pipeAng = DegreeToRad (0.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = squarePipe.leftBottomX;
	//		rotatedPoint.y = squarePipe.leftBottomY;
	//		rotatedPoint.z = squarePipe.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		squarePipe.leftBottomX = unrotatedPoint.x;
	//		squarePipe.leftBottomY = unrotatedPoint.y;
	//		squarePipe.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (squarePipe));

	//		xPos += squarePipe.length;

	//		if (length_pipe > 6.000)
	//			length_pipe -= 6.000;
	//		else
	//			length_pipe = 0.0;
	//	}
	//}

	//// ��������� ���� (�Ϻ� ���� �κ�), ���� ���� ������ ������ (���� - �Ϻ� �� �κ�)����
	//xPos = 0.0;
	//length_pipe = 0.100;
	//for (xx = 0 ; xx < nCellsFromBeginAtBottom ; ++xx)
	//	length_pipe += placingZone->cellsFromBeginAtBottom [0][xx].dirLen;
	//if (abs (placingZone->cellCenterAtBottom [0].dirLen) < EPS) {
	//	for (xx = 0 ; xx < placingZone->nCellsFromEndAtBottom ; ++xx)
	//		length_pipe += placingZone->cellsFromEndAtBottom [0][xx].dirLen;
	//}

	//if ((placingZone->cellsFromBeginAtBottom [0][0].perLen > EPS) && (placingZone->nCellsFromBeginAtBottom > 0)) {
	//	while (length_pipe > 0.0) {
	//		squarePipe.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtBottom * marginBeginAtBottom) - 0.050 + xPos;
	//		squarePipe.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
	//		squarePipe.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0885;
	//		squarePipe.ang = placingZone->ang;
	//		if (length_pipe > 6.000)
	//			squarePipe.length = 6.000;
	//		else
	//			squarePipe.length = length_pipe;
	//		squarePipe.pipeAng = DegreeToRad (0.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = squarePipe.leftBottomX;
	//		rotatedPoint.y = squarePipe.leftBottomY;
	//		rotatedPoint.z = squarePipe.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		squarePipe.leftBottomX = unrotatedPoint.x;
	//		squarePipe.leftBottomY = unrotatedPoint.y;
	//		squarePipe.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (squarePipe));

	//		xPos += squarePipe.length;

	//		if (length_pipe > 6.000)
	//			length_pipe -= 6.000;
	//		else
	//			length_pipe = 0.0;
	//	}
	//}

	//// ��������� �߾� (�Ϻ� ���� �κ�), ���� ���� ������ ������ (�߾� - �Ϻ� �� �κ�)����
	//if (placingZone->cellsFromBeginAtBottom [2][0].perLen > EPS) {
	//	xPos = 0.0;
	//	length_pipe = 0.100;
	//	for (xx = 0 ; xx < nCellsFromBeginAtBottom ; ++xx)
	//		length_pipe += placingZone->cellsFromBeginAtBottom [0][xx].dirLen;
	//	if (abs (placingZone->cellCenterAtBottom [0].dirLen) < EPS) {
	//		for (xx = 0 ; xx < placingZone->nCellsFromEndAtBottom ; ++xx)
	//			length_pipe += placingZone->cellsFromEndAtBottom [0][xx].dirLen;
	//	}

	//	if ((placingZone->cellsFromBeginAtBottom [2][0].perLen > EPS) && (placingZone->nCellsFromBeginAtBottom > 0)) {
	//		if (round (placingZone->cellsFromBeginAtBottom [0][0].perLen, 3) > round (placingZone->cellsFromBeginAtBottom [2][0].perLen, 3)) {
	//			if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.600) < EPS)			accumDist = -0.300 + 0.030;
	//			else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.500) < EPS)		accumDist = -0.300 + 0.030;
	//			else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.450) < EPS)		accumDist = -0.300 + 0.030;
	//			else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.400) < EPS)		accumDist = -0.250 + 0.030;
	//			else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.300) < EPS)		accumDist = -0.150 + 0.030;
	//			else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.200) < EPS)		accumDist = -0.150 + 0.030;
	//		} else {
	//			if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.600) < EPS)			accumDist = 0.300 + 0.030;
	//			else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.500) < EPS)		accumDist = 0.300 + 0.030;
	//			else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.450) < EPS)		accumDist = 0.300 + 0.030;
	//			else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.400) < EPS)		accumDist = 0.250 + 0.030;
	//			else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.300) < EPS)		accumDist = 0.150 + 0.030;
	//			else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.200) < EPS)		accumDist = 0.150 + 0.030;
	//			
	//			accumDist -= (placingZone->cellsFromBeginAtBottom [0][0].perLen + placingZone->cellsFromBeginAtBottom [1][0].perLen + placingZone->cellsFromBeginAtBottom [2][0].perLen);
	//		}

	//		while (length_pipe > 0.0) {
	//			squarePipe.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtBottom * marginBeginAtBottom) - 0.050 + xPos;
	//			squarePipe.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + accumDist;
	//			squarePipe.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0885;
	//			squarePipe.ang = placingZone->ang;
	//			if (length_pipe > 6.000)
	//				squarePipe.length = 6.000;
	//			else
	//				squarePipe.length = length_pipe;
	//			squarePipe.pipeAng = DegreeToRad (0.0);

	//			axisPoint.x = placingZone->begC.x;
	//			axisPoint.y = placingZone->begC.y;
	//			axisPoint.z = placingZone->begC.z;

	//			rotatedPoint.x = squarePipe.leftBottomX;
	//			rotatedPoint.y = squarePipe.leftBottomY;
	//			rotatedPoint.z = squarePipe.leftBottomZ;
	//			unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//			squarePipe.leftBottomX = unrotatedPoint.x;
	//			squarePipe.leftBottomY = unrotatedPoint.y;
	//			squarePipe.leftBottomZ = unrotatedPoint.z;

	//			elemList.Push (placeLibPart (squarePipe));

	//			xPos += squarePipe.length;

	//			if (length_pipe > 6.000)
	//				length_pipe -= 6.000;
	//			else
	//				length_pipe = 0.0;
	//		}
	//	}
	//}

	//// ��������� ���� (�Ϻ� �� �κ�), ���� ���� ������ ������ ����
	//xPos = centerPos;
	//length_pipe = 0.100;
	//for (xx = 0 ; xx < placingZone->nCellsFromEndAtBottom ; ++xx)
	//	length_pipe += placingZone->cellsFromEndAtBottom [0][xx].dirLen;

	//if ((placingZone->cellsFromBeginAtBottom [0][0].perLen > EPS) && (placingZone->nCellsFromBeginAtBottom > 0) && (abs (placingZone->cellCenterAtBottom [0].dirLen) > EPS)) {
	//	while (length_pipe > 0.0) {
	//		squarePipe.leftBottomX = placingZone->begC.x - 0.050 + xPos + (placingZone->cellCenterAtBottom [0].dirLen / 2);
	//		squarePipe.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
	//		squarePipe.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0885;
	//		squarePipe.ang = placingZone->ang;
	//		if (length_pipe > 6.000)
	//			squarePipe.length = 6.000;
	//		else
	//			squarePipe.length = length_pipe;
	//		squarePipe.pipeAng = DegreeToRad (0.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = squarePipe.leftBottomX;
	//		rotatedPoint.y = squarePipe.leftBottomY;
	//		rotatedPoint.z = squarePipe.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		squarePipe.leftBottomX = unrotatedPoint.x;
	//		squarePipe.leftBottomY = unrotatedPoint.y;
	//		squarePipe.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (squarePipe));

	//		xPos += squarePipe.length;

	//		if (length_pipe > 6.000)
	//			length_pipe -= 6.000;
	//		else
	//			length_pipe = 0.0;
	//	}
	//}

	//// ��������� ���� (�Ϻ� �� �κ�), ���� ���� ������ ������ ����
	//xPos = centerPos;
	//length_pipe = 0.100;
	//for (xx = 0 ; xx < placingZone->nCellsFromEndAtBottom ; ++xx)
	//	length_pipe += placingZone->cellsFromEndAtBottom [0][xx].dirLen;

	//if ((placingZone->cellsFromBeginAtBottom [0][0].perLen > EPS) && (placingZone->nCellsFromBeginAtBottom > 0) && (abs (placingZone->cellCenterAtBottom [0].dirLen) > EPS)) {
	//	while (length_pipe > 0.0) {
	//		squarePipe.leftBottomX = placingZone->begC.x - 0.050 + xPos + (placingZone->cellCenterAtBottom [0].dirLen / 2);
	//		squarePipe.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
	//		squarePipe.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0885;
	//		squarePipe.ang = placingZone->ang;
	//		if (length_pipe > 6.000)
	//			squarePipe.length = 6.000;
	//		else
	//			squarePipe.length = length_pipe;
	//		squarePipe.pipeAng = DegreeToRad (0.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = squarePipe.leftBottomX;
	//		rotatedPoint.y = squarePipe.leftBottomY;
	//		rotatedPoint.z = squarePipe.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		squarePipe.leftBottomX = unrotatedPoint.x;
	//		squarePipe.leftBottomY = unrotatedPoint.y;
	//		squarePipe.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (squarePipe));

	//		xPos += squarePipe.length;

	//		if (length_pipe > 6.000)
	//			length_pipe -= 6.000;
	//		else
	//			length_pipe = 0.0;
	//	}
	//}

	//// ��������� �߾� (�Ϻ� �� �κ�), ���� ���� ������ ������ ����
	//xPos = centerPos;
	//length_pipe = 0.100;
	//for (xx = 0 ; xx < placingZone->nCellsFromEndAtBottom ; ++xx)
	//	length_pipe += placingZone->cellsFromEndAtBottom [0][xx].dirLen;

	//if ((placingZone->cellsFromBeginAtBottom [2][0].perLen > EPS) && (placingZone->nCellsFromBeginAtBottom > 0) && (abs (placingZone->cellCenterAtBottom [0].dirLen) > EPS)) {
	//	if (round (placingZone->cellsFromBeginAtBottom [0][0].perLen, 3) > round (placingZone->cellsFromBeginAtBottom [2][0].perLen, 3)) {
	//		if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.600) < EPS)			accumDist = -0.300 + 0.030;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.500) < EPS)		accumDist = -0.300 + 0.030;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.450) < EPS)		accumDist = -0.300 + 0.030;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.400) < EPS)		accumDist = -0.250 + 0.030;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.300) < EPS)		accumDist = -0.150 + 0.030;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.200) < EPS)		accumDist = -0.150 + 0.030;
	//	} else {
	//		if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.600) < EPS)			accumDist = 0.300 + 0.030;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.500) < EPS)		accumDist = 0.300 + 0.030;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.450) < EPS)		accumDist = 0.300 + 0.030;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.400) < EPS)		accumDist = 0.250 + 0.030;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.300) < EPS)		accumDist = 0.150 + 0.030;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.200) < EPS)		accumDist = 0.150 + 0.030;
	//			
	//		accumDist -= (placingZone->cellsFromBeginAtBottom [0][0].perLen + placingZone->cellsFromBeginAtBottom [1][0].perLen + placingZone->cellsFromBeginAtBottom [2][0].perLen);
	//	}

	//	while (length_pipe > 0.0) {
	//		squarePipe.leftBottomX = placingZone->begC.x - 0.050 + xPos + (placingZone->cellCenterAtBottom [0].dirLen / 2);
	//		squarePipe.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + accumDist;
	//		squarePipe.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0885;
	//		squarePipe.ang = placingZone->ang;
	//		if (length_pipe > 6.000)
	//			squarePipe.length = 6.000;
	//		else
	//			squarePipe.length = length_pipe;
	//		squarePipe.pipeAng = DegreeToRad (0.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = squarePipe.leftBottomX;
	//		rotatedPoint.y = squarePipe.leftBottomY;
	//		rotatedPoint.z = squarePipe.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		squarePipe.leftBottomX = unrotatedPoint.x;
	//		squarePipe.leftBottomY = unrotatedPoint.y;
	//		squarePipe.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (squarePipe));

	//		xPos += squarePipe.length;

	//		if (length_pipe > 6.000)
	//			length_pipe -= 6.000;
	//		else
	//			length_pipe = 0.0;
	//	}
	//}

	//// ������ ��ũ - ��������� 1�� (���� ���� �κ� - ����), ���� ���� ������ ������ (���� �� �κ� - ����)����
	//hook.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) + placingZone->cellsFromBeginAtLSide [0][0].dirLen;
	//hook.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0885;
	//hook.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.600) < EPS)		hook.leftBottomZ += (0.030 + 0.150);
	//else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.500) < EPS)	hook.leftBottomZ += (0.030 + 0.050);
	//else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.450) < EPS)	hook.leftBottomZ += (0.030 + 0.150);
	//else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.400) < EPS)	hook.leftBottomZ += (0.030 + 0.100);
	//else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.300) < EPS)	hook.leftBottomZ += (0.030 + 0.150);
	//else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.200) < EPS)	hook.leftBottomZ += (0.030 + 0.050);
	//hook.ang = placingZone->ang;
	//hook.angX = DegreeToRad (180.0);
	//hook.angY = DegreeToRad (270.0);
	//hook.iHookType = 2;
	//hook.iHookShape = 2;

	//axisPoint.x = placingZone->begC.x;
	//axisPoint.y = placingZone->begC.y;
	//axisPoint.z = placingZone->begC.z;

	//rotatedPoint.x = hook.leftBottomX;
	//rotatedPoint.y = hook.leftBottomY;
	//rotatedPoint.z = hook.leftBottomZ;
	//unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//hook.leftBottomX = unrotatedPoint.x;
	//hook.leftBottomY = unrotatedPoint.y;
	//hook.leftBottomZ = unrotatedPoint.z;

	//elemList.Push (placeLibPart (hook));

	//accumDist = 0.0;
	//if (abs (placingZone->cellCenterAtLSide [0].dirLen) < EPS) {
	//	for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//	for (xx = 1 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)			accumDist += placingZone->cellsFromEndAtLSide [0][xx].dirLen;
	//} else {
	//	for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide-1 ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//}
	//moveIn3D ('x', hook.ang, accumDist, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	//elemList.Push (placeLibPart (hook));

	//if (placingZone->cellsFromBeginAtLSide [0][0].perLen > 0.300) {
	//	hook.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) + placingZone->cellsFromBeginAtLSide [0][0].dirLen;
	//	hook.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0885;
	//	if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.600) < EPS)		hook.leftBottomZ += 0.300;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.500) < EPS)	hook.leftBottomZ += 0.250;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.450) < EPS)	hook.leftBottomZ += 0.150;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.400) < EPS)	hook.leftBottomZ += 0.150;

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = hook.leftBottomX;
	//	rotatedPoint.y = hook.leftBottomY;
	//	rotatedPoint.z = hook.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	hook.leftBottomX = unrotatedPoint.x;
	//	hook.leftBottomY = unrotatedPoint.y;
	//	hook.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (hook));

	//	accumDist = 0.0;
	//	if (abs (placingZone->cellCenterAtLSide [0].dirLen) < EPS) {
	//		for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//		for (xx = 1 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)			accumDist += placingZone->cellsFromEndAtLSide [0][xx].dirLen;
	//	} else {
	//		for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide-1 ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//	}
	//	moveIn3D ('x', hook.ang, accumDist, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	//	elemList.Push (placeLibPart (hook));
	//}

	//// ������ ��ũ - ��������� 2�� (���� ���� �κ� - ����), ���� ���� ������ ������ (���� �� �κ� - ����)����
	//if ((placingZone->cellsFromBeginAtLSide [2][0].perLen > EPS) && (placingZone->nCellsFromBeginAtSide > 0)) {
	//	hook.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) + placingZone->cellsFromBeginAtLSide [0][0].dirLen;
	//	hook.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0885;
	//	hook.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + placingZone->cellsFromBeginAtLSide [0][0].perLen + placingZone->cellsFromBeginAtLSide [1][0].perLen;
	//	if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.600) < EPS)		hook.leftBottomZ += (0.030 + 0.150);
	//	else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.500) < EPS)	hook.leftBottomZ += (0.030 + 0.050);
	//	else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.450) < EPS)	hook.leftBottomZ += (0.030 + 0.150);
	//	else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.400) < EPS)	hook.leftBottomZ += (0.030 + 0.100);
	//	else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.300) < EPS)	hook.leftBottomZ += (0.030 + 0.150);
	//	else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.200) < EPS)	hook.leftBottomZ += (0.030 + 0.050);
	//	hook.ang = placingZone->ang;
	//	hook.angX = DegreeToRad (180.0);
	//	hook.angY = DegreeToRad (270.0);
	//	hook.iHookType = 2;
	//	hook.iHookShape = 2;

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = hook.leftBottomX;
	//	rotatedPoint.y = hook.leftBottomY;
	//	rotatedPoint.z = hook.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	hook.leftBottomX = unrotatedPoint.x;
	//	hook.leftBottomY = unrotatedPoint.y;
	//	hook.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (hook));

	//	accumDist = 0.0;
	//	if (abs (placingZone->cellCenterAtLSide [0].dirLen) < EPS) {
	//		for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//		for (xx = 1 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)			accumDist += placingZone->cellsFromEndAtLSide [0][xx].dirLen;
	//	} else {
	//		for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide-1 ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//	}
	//	moveIn3D ('x', hook.ang, accumDist, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	//	elemList.Push (placeLibPart (hook));

	//	if (placingZone->cellsFromBeginAtLSide [2][0].perLen > 0.300) {
	//		hook.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) + placingZone->cellsFromBeginAtLSide [0][0].dirLen;
	//		hook.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0885;
	//		if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.600) < EPS)		hook.leftBottomZ += 0.300;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.500) < EPS)	hook.leftBottomZ += 0.250;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.450) < EPS)	hook.leftBottomZ += 0.150;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.400) < EPS)	hook.leftBottomZ += 0.150;

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = hook.leftBottomX;
	//		rotatedPoint.y = hook.leftBottomY;
	//		rotatedPoint.z = hook.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		hook.leftBottomX = unrotatedPoint.x;
	//		hook.leftBottomY = unrotatedPoint.y;
	//		hook.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (hook));

	//		accumDist = 0.0;
	//		if (abs (placingZone->cellCenterAtLSide [0].dirLen) < EPS) {
	//			for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//			for (xx = 1 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)			accumDist += placingZone->cellsFromEndAtLSide [0][xx].dirLen;
	//		} else {
	//			for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide-1 ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//		}
	//		moveIn3D ('x', hook.ang, accumDist, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	//		elemList.Push (placeLibPart (hook));
	//	}
	//}

	//// ������ ��ũ - ��������� 1�� (���� ���� �κ� - ������), ���� ���� ������ ������ (���� �� �κ� - ������)����
	//hook.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) + placingZone->cellsFromBeginAtLSide [0][0].dirLen;
	//hook.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0885;
	//hook.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.600) < EPS)		hook.leftBottomZ += (0.030 + 0.150);
	//else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.500) < EPS)	hook.leftBottomZ += (0.030 + 0.050);
	//else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.450) < EPS)	hook.leftBottomZ += (0.030 + 0.150);
	//else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.400) < EPS)	hook.leftBottomZ += (0.030 + 0.100);
	//else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.300) < EPS)	hook.leftBottomZ += (0.030 + 0.150);
	//else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.200) < EPS)	hook.leftBottomZ += (0.030 + 0.050);
	//hook.angX = DegreeToRad (180.0);
	//hook.angY = DegreeToRad (270.0);
	//hook.iHookType = 2;
	//hook.iHookShape = 2;

	//axisPoint.x = placingZone->begC.x;
	//axisPoint.y = placingZone->begC.y;
	//axisPoint.z = placingZone->begC.z;

	//rotatedPoint.x = hook.leftBottomX;
	//rotatedPoint.y = hook.leftBottomY;
	//rotatedPoint.z = hook.leftBottomZ;
	//unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//hook.leftBottomX = unrotatedPoint.x;
	//hook.leftBottomY = unrotatedPoint.y;
	//hook.leftBottomZ = unrotatedPoint.z;

	//hook.ang = placingZone->ang + DegreeToRad (180.0);
	//elemList.Push (placeLibPart (hook));
	//hook.ang = placingZone->ang;

	//accumDist = 0.0;
	//if (abs (placingZone->cellCenterAtLSide [0].dirLen) < EPS) {
	//	for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//	for (xx = 1 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)			accumDist += placingZone->cellsFromEndAtLSide [0][xx].dirLen;
	//} else {
	//	for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide-1 ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//}
	//moveIn3D ('x', hook.ang, accumDist, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	//hook.ang = placingZone->ang + DegreeToRad (180.0);
	//elemList.Push (placeLibPart (hook));
	//hook.ang = placingZone->ang;

	//if (placingZone->cellsFromBeginAtLSide [0][0].perLen > 0.300) {
	//	hook.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) + placingZone->cellsFromBeginAtLSide [0][0].dirLen;
	//	hook.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0885;
	//	if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.600) < EPS)		hook.leftBottomZ += 0.300;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.500) < EPS)	hook.leftBottomZ += 0.250;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.450) < EPS)	hook.leftBottomZ += 0.150;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.400) < EPS)	hook.leftBottomZ += 0.150;

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = hook.leftBottomX;
	//	rotatedPoint.y = hook.leftBottomY;
	//	rotatedPoint.z = hook.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	hook.leftBottomX = unrotatedPoint.x;
	//	hook.leftBottomY = unrotatedPoint.y;
	//	hook.leftBottomZ = unrotatedPoint.z;

	//	hook.ang = placingZone->ang + DegreeToRad (180.0);
	//	elemList.Push (placeLibPart (hook));
	//	hook.ang = placingZone->ang;

	//	accumDist = 0.0;
	//	if (abs (placingZone->cellCenterAtLSide [0].dirLen) < EPS) {
	//		for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//		for (xx = 1 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)			accumDist += placingZone->cellsFromEndAtLSide [0][xx].dirLen;
	//	} else {
	//		for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide-1 ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//	}
	//	moveIn3D ('x', hook.ang, accumDist, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	//	hook.ang = placingZone->ang + DegreeToRad (180.0);
	//	elemList.Push (placeLibPart (hook));
	//	hook.ang = placingZone->ang;
	//}

	//// ������ ��ũ - ��������� 2�� (���� ���� �κ� - ������), ���� ���� ������ ������ (���� �� �κ� - ������)����
	//if ((placingZone->cellsFromBeginAtLSide [2][0].perLen > EPS) && (placingZone->nCellsFromBeginAtSide > 0)) {
	//	hook.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) + placingZone->cellsFromBeginAtLSide [0][0].dirLen;
	//	hook.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0885;
	//	hook.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + placingZone->cellsFromBeginAtLSide [0][0].perLen + placingZone->cellsFromBeginAtLSide [1][0].perLen;
	//	if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.600) < EPS)		hook.leftBottomZ += (0.030 + 0.150);
	//	else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.500) < EPS)	hook.leftBottomZ += (0.030 + 0.050);
	//	else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.450) < EPS)	hook.leftBottomZ += (0.030 + 0.150);
	//	else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.400) < EPS)	hook.leftBottomZ += (0.030 + 0.100);
	//	else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.300) < EPS)	hook.leftBottomZ += (0.030 + 0.150);
	//	else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.200) < EPS)	hook.leftBottomZ += (0.030 + 0.050);
	//	hook.angX = DegreeToRad (180.0);
	//	hook.angY = DegreeToRad (270.0);
	//	hook.iHookType = 2;
	//	hook.iHookShape = 2;

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = hook.leftBottomX;
	//	rotatedPoint.y = hook.leftBottomY;
	//	rotatedPoint.z = hook.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	hook.leftBottomX = unrotatedPoint.x;
	//	hook.leftBottomY = unrotatedPoint.y;
	//	hook.leftBottomZ = unrotatedPoint.z;

	//	hook.ang = placingZone->ang + DegreeToRad (180.0);
	//	elemList.Push (placeLibPart (hook));
	//	hook.ang = placingZone->ang;

	//	accumDist = 0.0;
	//	if (abs (placingZone->cellCenterAtLSide [0].dirLen) < EPS) {
	//		for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//		for (xx = 1 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)			accumDist += placingZone->cellsFromEndAtLSide [0][xx].dirLen;
	//	} else {
	//		for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide-1 ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//	}
	//	moveIn3D ('x', hook.ang, accumDist, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	//	hook.ang = placingZone->ang + DegreeToRad (180.0);
	//	elemList.Push (placeLibPart (hook));
	//	hook.ang = placingZone->ang;

	//	if (placingZone->cellsFromBeginAtLSide [2][0].perLen > 0.300) {
	//		hook.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) + placingZone->cellsFromBeginAtLSide [0][0].dirLen;
	//		hook.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0885;
	//		if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.600) < EPS)		hook.leftBottomZ += 0.300;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.500) < EPS)	hook.leftBottomZ += 0.250;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.450) < EPS)	hook.leftBottomZ += 0.150;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.400) < EPS)	hook.leftBottomZ += 0.150;

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = hook.leftBottomX;
	//		rotatedPoint.y = hook.leftBottomY;
	//		rotatedPoint.z = hook.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		hook.leftBottomX = unrotatedPoint.x;
	//		hook.leftBottomY = unrotatedPoint.y;
	//		hook.leftBottomZ = unrotatedPoint.z;

	//		hook.ang = placingZone->ang + DegreeToRad (180.0);
	//		elemList.Push (placeLibPart (hook));
	//		hook.ang = placingZone->ang;

	//		accumDist = 0.0;
	//		if (abs (placingZone->cellCenterAtLSide [0].dirLen) < EPS) {
	//			for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//			for (xx = 1 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)			accumDist += placingZone->cellsFromEndAtLSide [0][xx].dirLen;
	//		} else {
	//			for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide-1 ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//		}
	//		moveIn3D ('x', hook.ang, accumDist, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	//		hook.ang = placingZone->ang + DegreeToRad (180.0);
	//		elemList.Push (placeLibPart (hook));
	//		hook.ang = placingZone->ang;
	//	}
	//}

	//// ������ ��ũ - ��������� 1�� (���� �� �κ� - ����), ���� ���� ������ ������ ����
	//xPos = centerPos;
	//if ((placingZone->cellsFromEndAtLSide [0][0].perLen > EPS) && (placingZone->nCellsFromEndAtSide > 0) && (abs (placingZone->cellCenterAtLSide [0].dirLen) > EPS)) {
	//	hook.leftBottomX = placingZone->begC.x + xPos + (placingZone->cellCenterAtLSide [0].dirLen / 2) + placingZone->cellsFromEndAtLSide [0][placingZone->nCellsFromEndAtSide-1].dirLen;
	//	hook.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0885;
	//	hook.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//	if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.600) < EPS)			hook.leftBottomZ += (0.030 + 0.150);
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.500) < EPS)	hook.leftBottomZ += (0.030 + 0.050);
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.450) < EPS)	hook.leftBottomZ += (0.030 + 0.150);
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.400) < EPS)	hook.leftBottomZ += (0.030 + 0.100);
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.300) < EPS)	hook.leftBottomZ += (0.030 + 0.150);
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.200) < EPS)	hook.leftBottomZ += (0.030 + 0.050);
	//	hook.ang = placingZone->ang;
	//	hook.angX = DegreeToRad (180.0);
	//	hook.angY = DegreeToRad (270.0);
	//	hook.iHookType = 2;
	//	hook.iHookShape = 2;

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = hook.leftBottomX;
	//	rotatedPoint.y = hook.leftBottomY;
	//	rotatedPoint.z = hook.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	hook.leftBottomX = unrotatedPoint.x;
	//	hook.leftBottomY = unrotatedPoint.y;
	//	hook.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (hook));

	//	accumDist = 0.0;
	//	for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide-1 ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//	moveIn3D ('x', hook.ang, accumDist, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	//	elemList.Push (placeLibPart (hook));

	//	if (placingZone->cellsFromBeginAtLSide [0][0].perLen > 0.300) {
	//		hook.leftBottomX = placingZone->begC.x + xPos + (placingZone->cellCenterAtLSide [0].dirLen / 2) + placingZone->cellsFromEndAtLSide [0][placingZone->nCellsFromEndAtSide-1].dirLen;
	//		hook.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0885;
	//		if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.600) < EPS)			hook.leftBottomZ += 0.300;
	//		else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.500) < EPS)	hook.leftBottomZ += 0.250;
	//		else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.450) < EPS)	hook.leftBottomZ += 0.150;
	//		else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.400) < EPS)	hook.leftBottomZ += 0.150;

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = hook.leftBottomX;
	//		rotatedPoint.y = hook.leftBottomY;
	//		rotatedPoint.z = hook.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		hook.leftBottomX = unrotatedPoint.x;
	//		hook.leftBottomY = unrotatedPoint.y;
	//		hook.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (hook));

	//		accumDist = 0.0;
	//		for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide-1 ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//		moveIn3D ('x', hook.ang, accumDist, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	//		elemList.Push (placeLibPart (hook));
	//	}
	//}

	//// ������ ��ũ - ��������� 2�� (���� �� �κ� - ����), ���� ���� ������ ������ ����
	//xPos = centerPos;
	//if ((placingZone->cellsFromEndAtLSide [2][0].perLen > EPS) && (placingZone->nCellsFromEndAtSide > 0) && (abs (placingZone->cellCenterAtLSide [0].dirLen) > EPS)) {
	//	hook.leftBottomX = placingZone->begC.x + xPos + (placingZone->cellCenterAtLSide [0].dirLen / 2) + placingZone->cellsFromEndAtLSide [0][placingZone->nCellsFromEndAtSide-1].dirLen;
	//	hook.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0885;
	//	hook.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + placingZone->cellsFromEndAtLSide [0][0].perLen + placingZone->cellsFromEndAtLSide [1][0].perLen;
	//	if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.600) < EPS)			hook.leftBottomZ += (0.030 + 0.150);
	//	else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.500) < EPS)	hook.leftBottomZ += (0.030 + 0.050);
	//	else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.450) < EPS)	hook.leftBottomZ += (0.030 + 0.150);
	//	else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.400) < EPS)	hook.leftBottomZ += (0.030 + 0.100);
	//	else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.300) < EPS)	hook.leftBottomZ += (0.030 + 0.150);
	//	else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.200) < EPS)	hook.leftBottomZ += (0.030 + 0.050);
	//	hook.ang = placingZone->ang;
	//	hook.angX = DegreeToRad (180.0);
	//	hook.angY = DegreeToRad (270.0);
	//	hook.iHookType = 2;
	//	hook.iHookShape = 2;

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = hook.leftBottomX;
	//	rotatedPoint.y = hook.leftBottomY;
	//	rotatedPoint.z = hook.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	hook.leftBottomX = unrotatedPoint.x;
	//	hook.leftBottomY = unrotatedPoint.y;
	//	hook.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (hook));

	//	accumDist = 0.0;
	//	for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide-1 ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//	moveIn3D ('x', hook.ang, accumDist, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	//	elemList.Push (placeLibPart (hook));

	//	if (placingZone->cellsFromEndAtLSide [2][0].perLen > 0.300) {
	//		hook.leftBottomX = placingZone->begC.x + xPos + (placingZone->cellCenterAtLSide [0].dirLen / 2) + placingZone->cellsFromEndAtLSide [0][placingZone->nCellsFromEndAtSide-1].dirLen;
	//		hook.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0885;
	//		if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.600) < EPS)			hook.leftBottomZ += 0.300;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.500) < EPS)	hook.leftBottomZ += 0.250;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.450) < EPS)	hook.leftBottomZ += 0.150;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.400) < EPS)	hook.leftBottomZ += 0.150;

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = hook.leftBottomX;
	//		rotatedPoint.y = hook.leftBottomY;
	//		rotatedPoint.z = hook.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		hook.leftBottomX = unrotatedPoint.x;
	//		hook.leftBottomY = unrotatedPoint.y;
	//		hook.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (hook));

	//		accumDist = 0.0;
	//		for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide-1 ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//		moveIn3D ('x', hook.ang, accumDist, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	//		elemList.Push (placeLibPart (hook));
	//	}
	//}

	//// ������ ��ũ - ��������� 1�� (���� �� �κ� - ������), ���� ���� ������ ������ ����
	//xPos = centerPos;
	//if ((placingZone->cellsFromEndAtLSide [0][0].perLen > EPS) && (placingZone->nCellsFromEndAtSide > 0) && (abs (placingZone->cellCenterAtLSide [0].dirLen) > EPS)) {
	//	hook.leftBottomX = placingZone->begC.x + xPos + (placingZone->cellCenterAtLSide [0].dirLen / 2) + placingZone->cellsFromEndAtLSide [0][placingZone->nCellsFromEndAtSide-1].dirLen;
	//	hook.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0885;
	//	hook.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom;
	//	if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.600) < EPS)			hook.leftBottomZ += (0.030 + 0.150);
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.500) < EPS)	hook.leftBottomZ += (0.030 + 0.050);
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.450) < EPS)	hook.leftBottomZ += (0.030 + 0.150);
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.400) < EPS)	hook.leftBottomZ += (0.030 + 0.100);
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.300) < EPS)	hook.leftBottomZ += (0.030 + 0.150);
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.200) < EPS)	hook.leftBottomZ += (0.030 + 0.050);
	//	hook.angX = DegreeToRad (180.0);
	//	hook.angY = DegreeToRad (270.0);
	//	hook.iHookType = 2;
	//	hook.iHookShape = 2;

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = hook.leftBottomX;
	//	rotatedPoint.y = hook.leftBottomY;
	//	rotatedPoint.z = hook.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	hook.leftBottomX = unrotatedPoint.x;
	//	hook.leftBottomY = unrotatedPoint.y;
	//	hook.leftBottomZ = unrotatedPoint.z;

	//	hook.ang = placingZone->ang + DegreeToRad (180.0);
	//	elemList.Push (placeLibPart (hook));
	//	hook.ang = placingZone->ang;

	//	accumDist = 0.0;
	//	for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide-1 ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//	moveIn3D ('x', hook.ang, accumDist, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	//	hook.ang = placingZone->ang + DegreeToRad (180.0);
	//	elemList.Push (placeLibPart (hook));
	//	hook.ang = placingZone->ang;

	//	if (placingZone->cellsFromBeginAtLSide [0][0].perLen > 0.300) {
	//		hook.leftBottomX = placingZone->begC.x + xPos + (placingZone->cellCenterAtLSide [0].dirLen / 2) + placingZone->cellsFromEndAtLSide [0][placingZone->nCellsFromEndAtSide-1].dirLen;
	//		hook.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0885;
	//		if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.600) < EPS)			hook.leftBottomZ += 0.300;
	//		else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.500) < EPS)	hook.leftBottomZ += 0.250;
	//		else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.450) < EPS)	hook.leftBottomZ += 0.150;
	//		else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.400) < EPS)	hook.leftBottomZ += 0.150;

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = hook.leftBottomX;
	//		rotatedPoint.y = hook.leftBottomY;
	//		rotatedPoint.z = hook.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		hook.leftBottomX = unrotatedPoint.x;
	//		hook.leftBottomY = unrotatedPoint.y;
	//		hook.leftBottomZ = unrotatedPoint.z;

	//		hook.ang = placingZone->ang + DegreeToRad (180.0);
	//		elemList.Push (placeLibPart (hook));
	//		hook.ang = placingZone->ang;

	//		accumDist = 0.0;
	//		for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide-1 ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//		moveIn3D ('x', hook.ang, accumDist, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	//		hook.ang = placingZone->ang + DegreeToRad (180.0);
	//		elemList.Push (placeLibPart (hook));
	//		hook.ang = placingZone->ang;
	//	}
	//}

	//// ������ ��ũ - ��������� 2�� (���� �� �κ� - ������), ���� ���� ������ ������ ����
	//xPos = centerPos;
	//if ((placingZone->cellsFromEndAtLSide [2][0].perLen > EPS) && (placingZone->nCellsFromEndAtSide > 0) && (abs (placingZone->cellCenterAtLSide [0].dirLen) > EPS)) {
	//	hook.leftBottomX = placingZone->begC.x + xPos + (placingZone->cellCenterAtLSide [0].dirLen / 2) + placingZone->cellsFromEndAtLSide [0][placingZone->nCellsFromEndAtSide-1].dirLen;
	//	hook.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0885;
	//	hook.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + placingZone->cellsFromEndAtLSide [0][0].perLen + placingZone->cellsFromEndAtLSide [1][0].perLen;
	//	if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.600) < EPS)			hook.leftBottomZ += (0.030 + 0.150);
	//	else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.500) < EPS)	hook.leftBottomZ += (0.030 + 0.050);
	//	else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.450) < EPS)	hook.leftBottomZ += (0.030 + 0.150);
	//	else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.400) < EPS)	hook.leftBottomZ += (0.030 + 0.100);
	//	else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.300) < EPS)	hook.leftBottomZ += (0.030 + 0.150);
	//	else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.200) < EPS)	hook.leftBottomZ += (0.030 + 0.050);
	//	hook.angX = DegreeToRad (180.0);
	//	hook.angY = DegreeToRad (270.0);
	//	hook.iHookType = 2;
	//	hook.iHookShape = 2;

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = hook.leftBottomX;
	//	rotatedPoint.y = hook.leftBottomY;
	//	rotatedPoint.z = hook.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	hook.leftBottomX = unrotatedPoint.x;
	//	hook.leftBottomY = unrotatedPoint.y;
	//	hook.leftBottomZ = unrotatedPoint.z;

	//	hook.ang = placingZone->ang + DegreeToRad (180.0);
	//	elemList.Push (placeLibPart (hook));
	//	hook.ang = placingZone->ang;

	//	accumDist = 0.0;
	//	for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide-1 ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//	moveIn3D ('x', hook.ang, accumDist, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	//	hook.ang = placingZone->ang + DegreeToRad (180.0);
	//	elemList.Push (placeLibPart (hook));
	//	hook.ang = placingZone->ang;

	//	if (placingZone->cellsFromEndAtLSide [2][0].perLen > 0.300) {
	//		hook.leftBottomX = placingZone->begC.x + xPos + (placingZone->cellCenterAtLSide [0].dirLen / 2) + placingZone->cellsFromEndAtLSide [0][placingZone->nCellsFromEndAtSide-1].dirLen;
	//		hook.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0885;
	//		if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.600) < EPS)			hook.leftBottomZ += 0.300;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.500) < EPS)	hook.leftBottomZ += 0.250;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.450) < EPS)	hook.leftBottomZ += 0.150;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.400) < EPS)	hook.leftBottomZ += 0.150;

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = hook.leftBottomX;
	//		rotatedPoint.y = hook.leftBottomY;
	//		rotatedPoint.z = hook.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		hook.leftBottomX = unrotatedPoint.x;
	//		hook.leftBottomY = unrotatedPoint.y;
	//		hook.leftBottomZ = unrotatedPoint.z;

	//		hook.ang = placingZone->ang + DegreeToRad (180.0);
	//		elemList.Push (placeLibPart (hook));
	//		hook.ang = placingZone->ang;

	//		accumDist = 0.0;
	//		for (xx = 1 ; xx < placingZone->nCellsFromBeginAtSide-1 ; ++xx)		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//		moveIn3D ('x', hook.ang, accumDist, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	//		hook.ang = placingZone->ang + DegreeToRad (180.0);
	//		elemList.Push (placeLibPart (hook));
	//		hook.ang = placingZone->ang;
	//	}
	//}

	//// ������ ��ũ - ��������� �߾� (�Ϻ� ���� �κ�), ���� ���� ������ ������ (�߾� - �Ϻ� �� �κ�)����
	//if (round (placingZone->cellsFromBeginAtBottom [0][0].perLen, 3) > round (placingZone->cellsFromBeginAtBottom [2][0].perLen, 3)) {
	//	if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.600) < EPS)			accumDist = -0.300 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.500) < EPS)		accumDist = -0.300 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.450) < EPS)		accumDist = -0.300 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.400) < EPS)		accumDist = -0.250 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.300) < EPS)		accumDist = -0.150 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.200) < EPS)		accumDist = -0.150 + 0.030;
	//} else {
	//	if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.600) < EPS)			accumDist = 0.300 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.500) < EPS)		accumDist = 0.300 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.450) < EPS)		accumDist = 0.300 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.400) < EPS)		accumDist = 0.250 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.300) < EPS)		accumDist = 0.150 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.200) < EPS)		accumDist = 0.150 + 0.030;
	//			
	//	accumDist -= (placingZone->cellsFromBeginAtBottom [0][0].perLen + placingZone->cellsFromBeginAtBottom [1][0].perLen + placingZone->cellsFromBeginAtBottom [2][0].perLen);
	//}

	//if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen) > EPS) {
	//	hook.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtBottom * marginBeginAtBottom) + placingZone->cellsFromBeginAtBottom [0][0].dirLen;
	//	hook.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + accumDist;
	//	hook.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0885;
	//	hook.ang = placingZone->ang;
	//	hook.angX = DegreeToRad (270.0);
	//	hook.angY = DegreeToRad (270.0);
	//	hook.iHookType = 2;
	//	hook.iHookShape = 2;

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = hook.leftBottomX;
	//	rotatedPoint.y = hook.leftBottomY;
	//	rotatedPoint.z = hook.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	hook.leftBottomX = unrotatedPoint.x;
	//	hook.leftBottomY = unrotatedPoint.y;
	//	hook.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (hook));

	//	accumDist = 0.0;
	//	if (abs (placingZone->cellCenterAtBottom [0].dirLen) < EPS) {
	//		for (xx = 1 ; xx < placingZone->nCellsFromBeginAtBottom ; ++xx)		accumDist += placingZone->cellsFromBeginAtBottom [0][xx].dirLen;
	//		for (xx = 1 ; xx < placingZone->nCellsFromEndAtBottom ; ++xx)		accumDist += placingZone->cellsFromEndAtBottom [0][xx].dirLen;
	//	} else {
	//		for (xx = 1 ; xx < placingZone->nCellsFromBeginAtBottom-1 ; ++xx)	accumDist += placingZone->cellsFromBeginAtBottom [0][xx].dirLen;
	//	}
	//	moveIn3D ('x', hook.ang, accumDist, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	//	elemList.Push (placeLibPart (hook));
	//}

	//// ������ ��ũ - ��������� �߾� (�Ϻ� �� �κ�), ���� ���� ������ ������ ����
	//if (round (placingZone->cellsFromBeginAtBottom [0][0].perLen, 3) > round (placingZone->cellsFromBeginAtBottom [2][0].perLen, 3)) {
	//	if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.600) < EPS)			accumDist = -0.300 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.500) < EPS)		accumDist = -0.300 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.450) < EPS)		accumDist = -0.300 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.400) < EPS)		accumDist = -0.250 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.300) < EPS)		accumDist = -0.150 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.200) < EPS)		accumDist = -0.150 + 0.030;
	//} else {
	//	if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.600) < EPS)			accumDist = 0.300 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.500) < EPS)		accumDist = 0.300 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.450) < EPS)		accumDist = 0.300 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.400) < EPS)		accumDist = 0.250 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.300) < EPS)		accumDist = 0.150 + 0.030;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.200) < EPS)		accumDist = 0.150 + 0.030;
	//			
	//	accumDist -= (placingZone->cellsFromBeginAtBottom [0][0].perLen + placingZone->cellsFromBeginAtBottom [1][0].perLen + placingZone->cellsFromBeginAtBottom [2][0].perLen);
	//}

	//xPos = centerPos;
	//if ((abs (placingZone->cellsFromBeginAtBottom [2][0].perLen) > EPS) && (abs (placingZone->cellCenterAtBottom [0].dirLen) > EPS)) {
	//	hook.leftBottomX = placingZone->begC.x + xPos + (placingZone->cellCenterAtBottom [0].dirLen / 2) + placingZone->cellsFromEndAtBottom [0][placingZone->nCellsFromEndAtBottom-1].dirLen;
	//	hook.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + accumDist;
	//	hook.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0885;
	//	hook.ang = placingZone->ang;
	//	hook.angX = DegreeToRad (270.0);
	//	hook.angY = DegreeToRad (270.0);
	//	hook.iHookType = 2;
	//	hook.iHookShape = 2;

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = hook.leftBottomX;
	//	rotatedPoint.y = hook.leftBottomY;
	//	rotatedPoint.z = hook.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	hook.leftBottomX = unrotatedPoint.x;
	//	hook.leftBottomY = unrotatedPoint.y;
	//	hook.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (hook));

	//	accumDist = 0.0;
	//	for (xx = 1 ; xx < placingZone->nCellsFromBeginAtBottom-1 ; ++xx)	accumDist += placingZone->cellsFromEndAtBottom [0][xx].dirLen;
	//	moveIn3D ('x', hook.ang, accumDist, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	//	elemList.Push (placeLibPart (hook));
	//}

	//// �������� ��� - ��������� ���� ���� �κ�, ���� ���� ������ ������ (���� - �� �κ�)����
	//if (placingZone->nCellsFromBeginAtSide > 0) {
	//	hanger.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) + 0.150;
	//	hanger.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
	//	hanger.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0635;
	//	hanger.ang = placingZone->ang;
	//	hanger.angX = DegreeToRad (0.0);
	//	hanger.angY = DegreeToRad (0.0);

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = hanger.leftBottomX;
	//	rotatedPoint.y = hanger.leftBottomY;
	//	rotatedPoint.z = hanger.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	hanger.leftBottomX = unrotatedPoint.x;
	//	hanger.leftBottomY = unrotatedPoint.y;
	//	hanger.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (hanger));

	//	length_outa = 0.0;
	//	for (xx = 0 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)		length_outa += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//	if (placingZone->cellCenterAtLSide [0].dirLen < EPS) {
	//		for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)			length_outa += placingZone->cellsFromEndAtLSide [0][xx].dirLen;
	//	}

	//	moveIn3D ('x', hanger.ang, length_outa - 0.300, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
	//	elemList.Push (placeLibPart (hanger));

	//	accumDist = 0.150 - (round (length_outa / 0.300, 3) * 0.150);
	//	moveIn3D ('x', hanger.ang, accumDist, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
	//	elemList.Push (placeLibPart (hanger));
	//}

	//// �������� ��� - ��������� ���� ���� �κ�, ���� ���� ������ ������ (���� - �� �κ�)����
	//if (placingZone->nCellsFromBeginAtSide > 0) {
	//	hanger.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) + 0.150;
	//	hanger.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
	//	hanger.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0635;
	//	hanger.ang = placingZone->ang;
	//	hanger.angX = DegreeToRad (0.0);
	//	hanger.angY = DegreeToRad (0.0);

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = hanger.leftBottomX;
	//	rotatedPoint.y = hanger.leftBottomY;
	//	rotatedPoint.z = hanger.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	hanger.leftBottomX = unrotatedPoint.x;
	//	hanger.leftBottomY = unrotatedPoint.y;
	//	hanger.leftBottomZ = unrotatedPoint.z;

	//	hanger.ang = placingZone->ang + DegreeToRad (180.0);
	//	elemList.Push (placeLibPart (hanger));
	//	hanger.ang = placingZone->ang;

	//	length_outa = 0.0;
	//	for (xx = 0 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)		length_outa += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;
	//	if (placingZone->cellCenterAtLSide [0].dirLen < EPS) {
	//		for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)			length_outa += placingZone->cellsFromEndAtLSide [0][xx].dirLen;
	//	}

	//	moveIn3D ('x', hanger.ang, length_outa - 0.300, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
	//	hanger.ang = placingZone->ang + DegreeToRad (180.0);
	//	elemList.Push (placeLibPart (hanger));
	//	hanger.ang = placingZone->ang;

	//	accumDist = 0.150 - (round (length_outa / 0.300, 3) * 0.150);
	//	moveIn3D ('x', hanger.ang, accumDist, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
	//	hanger.ang = placingZone->ang + DegreeToRad (180.0);
	//	elemList.Push (placeLibPart (hanger));
	//	hanger.ang = placingZone->ang;
	//}

	//// �������� ��� - ��������� ���� �� �κ�, ���� ���� ������ ������ ����
	//xPos = centerPos;
	//if ((placingZone->nCellsFromEndAtSide > 0) && (placingZone->cellCenterAtLSide [0].dirLen > EPS)) {
	//	hanger.leftBottomX = placingZone->begC.x + xPos + (placingZone->cellCenterAtLSide [0].dirLen / 2) + 0.150;
	//	hanger.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide;
	//	hanger.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0635;
	//	hanger.ang = placingZone->ang;
	//	hanger.angX = DegreeToRad (0.0);
	//	hanger.angY = DegreeToRad (0.0);

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = hanger.leftBottomX;
	//	rotatedPoint.y = hanger.leftBottomY;
	//	rotatedPoint.z = hanger.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	hanger.leftBottomX = unrotatedPoint.x;
	//	hanger.leftBottomY = unrotatedPoint.y;
	//	hanger.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (hanger));

	//	length_outa = 0.0;
	//	for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)			length_outa += placingZone->cellsFromEndAtLSide [0][xx].dirLen;

	//	moveIn3D ('x', hanger.ang, length_outa - 0.300, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
	//	elemList.Push (placeLibPart (hanger));

	//	accumDist = 0.150 - (round (length_outa / 0.300, 3) * 0.150);
	//	moveIn3D ('x', hanger.ang, accumDist, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
	//	elemList.Push (placeLibPart (hanger));
	//}

	//// �������� ��� - ��������� ���� �� �κ�, ���� ���� ������ ������ ����
	//xPos = centerPos;
	//if ((placingZone->nCellsFromEndAtSide > 0) && (placingZone->cellCenterAtLSide [0].dirLen > EPS)) {
	//	hanger.leftBottomX = placingZone->begC.x + xPos + (placingZone->cellCenterAtLSide [0].dirLen / 2) + 0.150;
	//	hanger.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide;
	//	hanger.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0635;
	//	hanger.ang = placingZone->ang;
	//	hanger.angX = DegreeToRad (0.0);
	//	hanger.angY = DegreeToRad (0.0);

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = hanger.leftBottomX;
	//	rotatedPoint.y = hanger.leftBottomY;
	//	rotatedPoint.z = hanger.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	hanger.leftBottomX = unrotatedPoint.x;
	//	hanger.leftBottomY = unrotatedPoint.y;
	//	hanger.leftBottomZ = unrotatedPoint.z;

	//	hanger.ang = placingZone->ang + DegreeToRad (180.0);
	//	elemList.Push (placeLibPart (hanger));
	//	hanger.ang = placingZone->ang;

	//	length_outa = 0.0;
	//	for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)			length_outa += placingZone->cellsFromEndAtLSide [0][xx].dirLen;

	//	moveIn3D ('x', hanger.ang, length_outa - 0.300, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
	//	hanger.ang = placingZone->ang + DegreeToRad (180.0);
	//	elemList.Push (placeLibPart (hanger));
	//	hanger.ang = placingZone->ang;

	//	accumDist = 0.150 - (round (length_outa / 0.300, 3) * 0.150);
	//	moveIn3D ('x', hanger.ang, accumDist, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
	//	hanger.ang = placingZone->ang + DegreeToRad (180.0);
	//	elemList.Push (placeLibPart (hanger));
	//	hanger.ang = placingZone->ang;
	//}

	//strcpy (timberRail.railType, "����� 2");

	//// ����� - ������ 1�� (���� ���� �κ� - ����)
	//if (placingZone->bFillMarginBeginAtSide == true) {
	//	timberRail.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) + 0.003;
	//	timberRail.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0525;
	//	timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + 0.023;
	//	if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.600) < EPS)			timberRail.leftBottomZ += 0.450;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.500) < EPS)		timberRail.leftBottomZ += 0.350;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.450) < EPS)		timberRail.leftBottomZ += 0.300;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.400) < EPS)		timberRail.leftBottomZ += 0.250;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.300) < EPS)		timberRail.leftBottomZ += 0.150;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.200) < EPS)		timberRail.leftBottomZ += 0.150;
	//	timberRail.angX = DegreeToRad (0.0);
	//	timberRail.angY = DegreeToRad (90.0);
	//	timberRail.ang = placingZone->ang + DegreeToRad (180.0);

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = timberRail.leftBottomX;
	//	rotatedPoint.y = timberRail.leftBottomY;
	//	rotatedPoint.z = timberRail.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	timberRail.leftBottomX = unrotatedPoint.x;
	//	timberRail.leftBottomY = unrotatedPoint.y;
	//	timberRail.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (timberRail));
	//}
	//if ((placingZone->cellCenterAtLSide [0].objType == PLYWOOD) && (placingZone->cellCenterAtLSide [0].dirLen > EPS)) {
	//	accumDist = 0.0;
	//	for (xx = 0 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)
	//		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;

	//	timberRail.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) + accumDist - 0.003;
	//	timberRail.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0525;
	//	timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + 0.023 - 0.194;
	//	if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.600) < EPS)			timberRail.leftBottomZ += 0.450;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.500) < EPS)		timberRail.leftBottomZ += 0.350;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.450) < EPS)		timberRail.leftBottomZ += 0.300;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.400) < EPS)		timberRail.leftBottomZ += 0.250;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.300) < EPS)		timberRail.leftBottomZ += 0.150;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.200) < EPS)		timberRail.leftBottomZ += 0.150;
	//	timberRail.angX = DegreeToRad (0.0);
	//	timberRail.angY = DegreeToRad (270.0);
	//	timberRail.ang = placingZone->ang + DegreeToRad (180.0);

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = timberRail.leftBottomX;
	//	rotatedPoint.y = timberRail.leftBottomY;
	//	rotatedPoint.z = timberRail.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	timberRail.leftBottomX = unrotatedPoint.x;
	//	timberRail.leftBottomY = unrotatedPoint.y;
	//	timberRail.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (timberRail));
	//}

	//// ����� - ������ 2�� (���� ���� �κ� - ����)
	//if (placingZone->cellsFromBeginAtLSide [2][0].perLen > EPS) {
	//	if (placingZone->bFillMarginBeginAtSide == true) {
	//		timberRail.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) + 0.003;
	//		timberRail.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0525;
	//		timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + placingZone->cellsFromBeginAtLSide [0][0].perLen + placingZone->cellsFromBeginAtLSide [1][0].perLen + 0.023;
	//		if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.600) < EPS)			timberRail.leftBottomZ += 0.450;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.500) < EPS)		timberRail.leftBottomZ += 0.350;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.450) < EPS)		timberRail.leftBottomZ += 0.300;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.400) < EPS)		timberRail.leftBottomZ += 0.250;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.300) < EPS)		timberRail.leftBottomZ += 0.150;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.200) < EPS)		timberRail.leftBottomZ += 0.150;
	//		timberRail.angX = DegreeToRad (0.0);
	//		timberRail.angY = DegreeToRad (90.0);
	//		timberRail.ang = placingZone->ang + DegreeToRad (180.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = timberRail.leftBottomX;
	//		rotatedPoint.y = timberRail.leftBottomY;
	//		rotatedPoint.z = timberRail.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		timberRail.leftBottomX = unrotatedPoint.x;
	//		timberRail.leftBottomY = unrotatedPoint.y;
	//		timberRail.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (timberRail));
	//	}
	//	if ((placingZone->cellCenterAtLSide [0].objType == PLYWOOD) && (placingZone->cellCenterAtLSide [0].dirLen > EPS)) {
	//		accumDist = 0.0;
	//		for (xx = 0 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)
	//			accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;

	//		timberRail.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) + accumDist - 0.003;
	//		timberRail.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0525;
	//		timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + placingZone->cellsFromBeginAtLSide [0][0].perLen + placingZone->cellsFromBeginAtLSide [1][0].perLen + 0.023 - 0.194;
	//		if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.600) < EPS)			timberRail.leftBottomZ += 0.450;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.500) < EPS)		timberRail.leftBottomZ += 0.350;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.450) < EPS)		timberRail.leftBottomZ += 0.300;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.400) < EPS)		timberRail.leftBottomZ += 0.250;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.300) < EPS)		timberRail.leftBottomZ += 0.150;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.200) < EPS)		timberRail.leftBottomZ += 0.150;
	//		timberRail.angX = DegreeToRad (0.0);
	//		timberRail.angY = DegreeToRad (270.0);
	//		timberRail.ang = placingZone->ang + DegreeToRad (180.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = timberRail.leftBottomX;
	//		rotatedPoint.y = timberRail.leftBottomY;
	//		rotatedPoint.z = timberRail.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		timberRail.leftBottomX = unrotatedPoint.x;
	//		timberRail.leftBottomY = unrotatedPoint.y;
	//		timberRail.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (timberRail));
	//	}
	//}

	//// ����� - ������ 1�� (���� ���� �κ� - ������)
	//if (placingZone->bFillMarginBeginAtSide == true) {
	//	timberRail.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) + 0.003;
	//	timberRail.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0525;
	//	timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + 0.023 - 0.194;
	//	if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.600) < EPS)			timberRail.leftBottomZ += 0.450;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.500) < EPS)		timberRail.leftBottomZ += 0.350;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.450) < EPS)		timberRail.leftBottomZ += 0.300;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.400) < EPS)		timberRail.leftBottomZ += 0.250;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.300) < EPS)		timberRail.leftBottomZ += 0.150;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.200) < EPS)		timberRail.leftBottomZ += 0.150;
	//	timberRail.angX = DegreeToRad (0.0);
	//	timberRail.angY = DegreeToRad (270.0);
	//	timberRail.ang = placingZone->ang + DegreeToRad (0.0);

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = timberRail.leftBottomX;
	//	rotatedPoint.y = timberRail.leftBottomY;
	//	rotatedPoint.z = timberRail.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	timberRail.leftBottomX = unrotatedPoint.x;
	//	timberRail.leftBottomY = unrotatedPoint.y;
	//	timberRail.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (timberRail));
	//}
	//if ((placingZone->cellCenterAtLSide [0].objType == PLYWOOD) && (placingZone->cellCenterAtLSide [0].dirLen > EPS)) {
	//	accumDist = 0.0;
	//	for (xx = 0 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)
	//		accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;

	//	timberRail.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) + accumDist - 0.003;
	//	timberRail.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0525;
	//	timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + 0.023;
	//	if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.600) < EPS)			timberRail.leftBottomZ += 0.450;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.500) < EPS)		timberRail.leftBottomZ += 0.350;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.450) < EPS)		timberRail.leftBottomZ += 0.300;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.400) < EPS)		timberRail.leftBottomZ += 0.250;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.300) < EPS)		timberRail.leftBottomZ += 0.150;
	//	else if (abs (placingZone->cellsFromBeginAtLSide [0][0].perLen - 0.200) < EPS)		timberRail.leftBottomZ += 0.150;
	//	timberRail.angX = DegreeToRad (0.0);
	//	timberRail.angY = DegreeToRad (90.0);
	//	timberRail.ang = placingZone->ang + DegreeToRad (0.0);

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = timberRail.leftBottomX;
	//	rotatedPoint.y = timberRail.leftBottomY;
	//	rotatedPoint.z = timberRail.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	timberRail.leftBottomX = unrotatedPoint.x;
	//	timberRail.leftBottomY = unrotatedPoint.y;
	//	timberRail.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (timberRail));
	//}

	//// ����� - ������ 2�� (���� ���� �κ� - ������)
	//if (placingZone->cellsFromBeginAtLSide [2][0].perLen > EPS) {
	//	if (placingZone->bFillMarginBeginAtSide == true) {
	//		timberRail.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) + 0.003;
	//		timberRail.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0525;
	//		timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + placingZone->cellsFromBeginAtLSide [0][0].perLen + placingZone->cellsFromBeginAtLSide [1][0].perLen + 0.023 - 0.194;
	//		if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.600) < EPS)			timberRail.leftBottomZ += 0.450;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.500) < EPS)		timberRail.leftBottomZ += 0.350;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.450) < EPS)		timberRail.leftBottomZ += 0.300;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.400) < EPS)		timberRail.leftBottomZ += 0.250;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.300) < EPS)		timberRail.leftBottomZ += 0.150;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.200) < EPS)		timberRail.leftBottomZ += 0.150;
	//		timberRail.angX = DegreeToRad (0.0);
	//		timberRail.angY = DegreeToRad (270.0);
	//		timberRail.ang = placingZone->ang + DegreeToRad (0.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = timberRail.leftBottomX;
	//		rotatedPoint.y = timberRail.leftBottomY;
	//		rotatedPoint.z = timberRail.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		timberRail.leftBottomX = unrotatedPoint.x;
	//		timberRail.leftBottomY = unrotatedPoint.y;
	//		timberRail.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (timberRail));
	//	}
	//	if ((placingZone->cellCenterAtLSide [0].objType == PLYWOOD) && (placingZone->cellCenterAtLSide [0].dirLen > EPS)) {
	//		accumDist = 0.0;
	//		for (xx = 0 ; xx < placingZone->nCellsFromBeginAtSide ; ++xx)
	//			accumDist += placingZone->cellsFromBeginAtLSide [0][xx].dirLen;

	//		timberRail.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtSide * marginBeginAtSide) + accumDist - 0.003;
	//		timberRail.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0525;
	//		timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + placingZone->cellsFromBeginAtLSide [0][0].perLen + placingZone->cellsFromBeginAtLSide [1][0].perLen + 0.023;
	//		if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.600) < EPS)			timberRail.leftBottomZ += 0.450;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.500) < EPS)		timberRail.leftBottomZ += 0.350;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.450) < EPS)		timberRail.leftBottomZ += 0.300;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.400) < EPS)		timberRail.leftBottomZ += 0.250;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.300) < EPS)		timberRail.leftBottomZ += 0.150;
	//		else if (abs (placingZone->cellsFromBeginAtLSide [2][0].perLen - 0.200) < EPS)		timberRail.leftBottomZ += 0.150;
	//		timberRail.angX = DegreeToRad (0.0);
	//		timberRail.angY = DegreeToRad (90.0);
	//		timberRail.ang = placingZone->ang + DegreeToRad (0.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = timberRail.leftBottomX;
	//		rotatedPoint.y = timberRail.leftBottomY;
	//		rotatedPoint.z = timberRail.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		timberRail.leftBottomX = unrotatedPoint.x;
	//		timberRail.leftBottomY = unrotatedPoint.y;
	//		timberRail.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (timberRail));
	//	}
	//}

	//// ����� - ������ 1�� (���� �� �κ� - ����)
	//if (placingZone->bFillMarginEndAtSide == true) {
	//	timberRail.leftBottomX = placingZone->begC.x + beamLength - (bFillMarginEndAtSide * marginEndAtSide) - 0.003;
	//	timberRail.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0525;
	//	timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + 0.023 - 0.194;
	//	if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.600) < EPS)				timberRail.leftBottomZ += 0.450;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.500) < EPS)		timberRail.leftBottomZ += 0.350;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.450) < EPS)		timberRail.leftBottomZ += 0.300;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.400) < EPS)		timberRail.leftBottomZ += 0.250;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.300) < EPS)		timberRail.leftBottomZ += 0.150;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.200) < EPS)		timberRail.leftBottomZ += 0.150;
	//	timberRail.angX = DegreeToRad (0.0);
	//	timberRail.angY = DegreeToRad (270.0);
	//	timberRail.ang = placingZone->ang + DegreeToRad (180.0);

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = timberRail.leftBottomX;
	//	rotatedPoint.y = timberRail.leftBottomY;
	//	rotatedPoint.z = timberRail.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	timberRail.leftBottomX = unrotatedPoint.x;
	//	timberRail.leftBottomY = unrotatedPoint.y;
	//	timberRail.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (timberRail));
	//}
	//if ((placingZone->cellCenterAtLSide [0].objType == PLYWOOD) && (placingZone->cellCenterAtLSide [0].dirLen > EPS)) {
	//	accumDist = 0.0;
	//	for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)
	//		accumDist += placingZone->cellsFromEndAtLSide [0][xx].dirLen;

	//	timberRail.leftBottomX = placingZone->begC.x + beamLength - (bFillMarginEndAtSide * marginEndAtSide) - accumDist + 0.003;
	//	timberRail.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0525;
	//	timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + 0.023;
	//	if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.600) < EPS)				timberRail.leftBottomZ += 0.450;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.500) < EPS)		timberRail.leftBottomZ += 0.350;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.450) < EPS)		timberRail.leftBottomZ += 0.300;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.400) < EPS)		timberRail.leftBottomZ += 0.250;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.300) < EPS)		timberRail.leftBottomZ += 0.150;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.200) < EPS)		timberRail.leftBottomZ += 0.150;
	//	timberRail.angX = DegreeToRad (0.0);
	//	timberRail.angY = DegreeToRad (90.0);
	//	timberRail.ang = placingZone->ang + DegreeToRad (180.0);

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = timberRail.leftBottomX;
	//	rotatedPoint.y = timberRail.leftBottomY;
	//	rotatedPoint.z = timberRail.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	timberRail.leftBottomX = unrotatedPoint.x;
	//	timberRail.leftBottomY = unrotatedPoint.y;
	//	timberRail.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (timberRail));
	//}

	//// ����� - ������ 2�� (���� �� �κ� - ����)
	//if (placingZone->cellsFromEndAtLSide [2][0].perLen > EPS) {
	//	if (placingZone->bFillMarginEndAtSide == true) {
	//		timberRail.leftBottomX = placingZone->begC.x + beamLength - (bFillMarginEndAtSide * marginEndAtSide) - 0.003;
	//		timberRail.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0525;
	//		timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + placingZone->cellsFromEndAtLSide [0][0].perLen + placingZone->cellsFromEndAtLSide [1][0].perLen + 0.023 - 0.194;
	//		if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.600) < EPS)				timberRail.leftBottomZ += 0.450;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.500) < EPS)		timberRail.leftBottomZ += 0.350;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.450) < EPS)		timberRail.leftBottomZ += 0.300;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.400) < EPS)		timberRail.leftBottomZ += 0.250;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.300) < EPS)		timberRail.leftBottomZ += 0.150;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.200) < EPS)		timberRail.leftBottomZ += 0.150;
	//		timberRail.angX = DegreeToRad (0.0);
	//		timberRail.angY = DegreeToRad (270.0);
	//		timberRail.ang = placingZone->ang + DegreeToRad (180.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = timberRail.leftBottomX;
	//		rotatedPoint.y = timberRail.leftBottomY;
	//		rotatedPoint.z = timberRail.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		timberRail.leftBottomX = unrotatedPoint.x;
	//		timberRail.leftBottomY = unrotatedPoint.y;
	//		timberRail.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (timberRail));
	//	}
	//	if ((placingZone->cellCenterAtLSide [0].objType == PLYWOOD) && (placingZone->cellCenterAtLSide [0].dirLen > EPS)) {
	//		accumDist = 0.0;
	//		for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)
	//			accumDist += placingZone->cellsFromEndAtLSide [0][xx].dirLen;

	//		timberRail.leftBottomX = placingZone->begC.x + beamLength - (bFillMarginEndAtSide * marginEndAtSide) - accumDist + 0.003;
	//		timberRail.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide + 0.0525;
	//		timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + placingZone->cellsFromEndAtLSide [0][0].perLen + placingZone->cellsFromEndAtLSide [1][0].perLen + 0.023;
	//		if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.600) < EPS)				timberRail.leftBottomZ += 0.450;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.500) < EPS)		timberRail.leftBottomZ += 0.350;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.450) < EPS)		timberRail.leftBottomZ += 0.300;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.400) < EPS)		timberRail.leftBottomZ += 0.250;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.300) < EPS)		timberRail.leftBottomZ += 0.150;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.200) < EPS)		timberRail.leftBottomZ += 0.150;
	//		timberRail.angX = DegreeToRad (0.0);
	//		timberRail.angY = DegreeToRad (90.0);
	//		timberRail.ang = placingZone->ang + DegreeToRad (180.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = timberRail.leftBottomX;
	//		rotatedPoint.y = timberRail.leftBottomY;
	//		rotatedPoint.z = timberRail.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		timberRail.leftBottomX = unrotatedPoint.x;
	//		timberRail.leftBottomY = unrotatedPoint.y;
	//		timberRail.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (timberRail));
	//	}
	//}

	//// ����� - ������ 1�� (���� �� �κ� - ������)
	//if (placingZone->bFillMarginEndAtSide == true) {
	//	timberRail.leftBottomX = placingZone->begC.x + beamLength - (bFillMarginEndAtSide * marginEndAtSide) - 0.003;
	//	timberRail.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0525;
	//	timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + 0.023;
	//	if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.600) < EPS)				timberRail.leftBottomZ += 0.450;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.500) < EPS)		timberRail.leftBottomZ += 0.350;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.450) < EPS)		timberRail.leftBottomZ += 0.300;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.400) < EPS)		timberRail.leftBottomZ += 0.250;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.300) < EPS)		timberRail.leftBottomZ += 0.150;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.200) < EPS)		timberRail.leftBottomZ += 0.150;
	//	timberRail.angX = DegreeToRad (0.0);
	//	timberRail.angY = DegreeToRad (90.0);
	//	timberRail.ang = placingZone->ang + DegreeToRad (0.0);

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = timberRail.leftBottomX;
	//	rotatedPoint.y = timberRail.leftBottomY;
	//	rotatedPoint.z = timberRail.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	timberRail.leftBottomX = unrotatedPoint.x;
	//	timberRail.leftBottomY = unrotatedPoint.y;
	//	timberRail.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (timberRail));
	//}
	//if ((placingZone->cellCenterAtLSide [0].objType == PLYWOOD) && (placingZone->cellCenterAtLSide [0].dirLen > EPS)) {
	//	accumDist = 0.0;
	//	for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)
	//		accumDist += placingZone->cellsFromEndAtLSide [0][xx].dirLen;

	//	timberRail.leftBottomX = placingZone->begC.x + beamLength - (bFillMarginEndAtSide * marginEndAtSide) - accumDist + 0.003;
	//	timberRail.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0525;
	//	timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + 0.023 - 0.194;
	//	if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.600) < EPS)				timberRail.leftBottomZ += 0.450;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.500) < EPS)		timberRail.leftBottomZ += 0.350;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.450) < EPS)		timberRail.leftBottomZ += 0.300;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.400) < EPS)		timberRail.leftBottomZ += 0.250;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.300) < EPS)		timberRail.leftBottomZ += 0.150;
	//	else if (abs (placingZone->cellsFromEndAtLSide [0][0].perLen - 0.200) < EPS)		timberRail.leftBottomZ += 0.150;
	//	timberRail.angX = DegreeToRad (0.0);
	//	timberRail.angY = DegreeToRad (270.0);
	//	timberRail.ang = placingZone->ang + DegreeToRad (0.0);

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = timberRail.leftBottomX;
	//	rotatedPoint.y = timberRail.leftBottomY;
	//	rotatedPoint.z = timberRail.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	timberRail.leftBottomX = unrotatedPoint.x;
	//	timberRail.leftBottomY = unrotatedPoint.y;
	//	timberRail.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (timberRail));
	//}

	//// ����� - ������ 2�� (���� �� �κ� - ������)
	//if (placingZone->cellsFromEndAtLSide [2][0].perLen > EPS) {
	//	if (placingZone->bFillMarginEndAtSide == true) {
	//		timberRail.leftBottomX = placingZone->begC.x + beamLength - (bFillMarginEndAtSide * marginEndAtSide) - 0.003;
	//		timberRail.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0525;
	//		timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + placingZone->cellsFromEndAtLSide [0][0].perLen + placingZone->cellsFromEndAtLSide [1][0].perLen + 0.023;
	//		if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.600) < EPS)				timberRail.leftBottomZ += 0.450;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.500) < EPS)		timberRail.leftBottomZ += 0.350;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.450) < EPS)		timberRail.leftBottomZ += 0.300;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.400) < EPS)		timberRail.leftBottomZ += 0.250;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.300) < EPS)		timberRail.leftBottomZ += 0.150;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.200) < EPS)		timberRail.leftBottomZ += 0.150;
	//		timberRail.angX = DegreeToRad (0.0);
	//		timberRail.angY = DegreeToRad (90.0);
	//		timberRail.ang = placingZone->ang + DegreeToRad (0.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = timberRail.leftBottomX;
	//		rotatedPoint.y = timberRail.leftBottomY;
	//		rotatedPoint.z = timberRail.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		timberRail.leftBottomX = unrotatedPoint.x;
	//		timberRail.leftBottomY = unrotatedPoint.y;
	//		timberRail.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (timberRail));
	//	}
	//	if ((placingZone->cellCenterAtLSide [0].objType == PLYWOOD) && (placingZone->cellCenterAtLSide [0].dirLen > EPS)) {
	//		accumDist = 0.0;
	//		for (xx = 0 ; xx < placingZone->nCellsFromEndAtSide ; ++xx)
	//			accumDist += placingZone->cellsFromEndAtLSide [0][xx].dirLen;

	//		timberRail.leftBottomX = placingZone->begC.x + beamLength - (bFillMarginEndAtSide * marginEndAtSide) - accumDist + 0.003;
	//		timberRail.leftBottomY = placingZone->begC.y - infoBeam.width/2 + infoBeam.offset - placingZone->gapSide - 0.0525;
	//		timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom + placingZone->cellsFromEndAtLSide [0][0].perLen + placingZone->cellsFromEndAtLSide [1][0].perLen + 0.023 - 0.194;
	//		if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.600) < EPS)				timberRail.leftBottomZ += 0.450;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.500) < EPS)		timberRail.leftBottomZ += 0.350;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.450) < EPS)		timberRail.leftBottomZ += 0.300;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.400) < EPS)		timberRail.leftBottomZ += 0.250;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.300) < EPS)		timberRail.leftBottomZ += 0.150;
	//		else if (abs (placingZone->cellsFromEndAtLSide [2][0].perLen - 0.200) < EPS)		timberRail.leftBottomZ += 0.150;
	//		timberRail.angX = DegreeToRad (0.0);
	//		timberRail.angY = DegreeToRad (270.0);
	//		timberRail.ang = placingZone->ang + DegreeToRad (0.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = timberRail.leftBottomX;
	//		rotatedPoint.y = timberRail.leftBottomY;
	//		rotatedPoint.z = timberRail.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		timberRail.leftBottomX = unrotatedPoint.x;
	//		timberRail.leftBottomY = unrotatedPoint.y;
	//		timberRail.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (timberRail));
	//	}
	//}

	//// ����� - ������ 1�� (�Ϻ� ���� �κ�)
	//if (placingZone->bFillMarginBeginAtBottom == true) {
	//	timberRail.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtBottom * marginBeginAtBottom) + 0.003;
	//	timberRail.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide - 0.023 + 0.194;
	//	timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0525;
	//	if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.600) < EPS)			timberRail.leftBottomY -= 0.450;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.500) < EPS)		timberRail.leftBottomY -= 0.350;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.450) < EPS)		timberRail.leftBottomY -= 0.300;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.400) < EPS)		timberRail.leftBottomY -= 0.250;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.300) < EPS)		timberRail.leftBottomY -= 0.150;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.200) < EPS)		timberRail.leftBottomY -= 0.150;
	//	timberRail.angX = DegreeToRad (90.0);
	//	timberRail.angY = DegreeToRad (0.0);
	//	timberRail.ang = placingZone->ang + DegreeToRad (270.0);

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = timberRail.leftBottomX;
	//	rotatedPoint.y = timberRail.leftBottomY;
	//	rotatedPoint.z = timberRail.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	timberRail.leftBottomX = unrotatedPoint.x;
	//	timberRail.leftBottomY = unrotatedPoint.y;
	//	timberRail.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (timberRail));
	//}
	//if ((placingZone->cellCenterAtBottom [0].objType == PLYWOOD) && (placingZone->cellCenterAtBottom [0].dirLen > EPS)) {
	//	accumDist = 0.0;
	//	for (xx = 0 ; xx < placingZone->nCellsFromBeginAtBottom ; ++xx)
	//		accumDist += placingZone->cellsFromBeginAtBottom [0][xx].dirLen;

	//	timberRail.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtBottom * marginBeginAtBottom) + accumDist - 0.003;
	//	timberRail.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide - 0.023;
	//	timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0525;
	//	if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.600) < EPS)			timberRail.leftBottomY -= 0.450;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.500) < EPS)		timberRail.leftBottomY -= 0.350;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.450) < EPS)		timberRail.leftBottomY -= 0.300;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.400) < EPS)		timberRail.leftBottomY -= 0.250;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.300) < EPS)		timberRail.leftBottomY -= 0.150;
	//	else if (abs (placingZone->cellsFromBeginAtBottom [0][0].perLen - 0.200) < EPS)		timberRail.leftBottomY -= 0.150;
	//	timberRail.angX = DegreeToRad (90.0);
	//	timberRail.angY = DegreeToRad (180.0);
	//	timberRail.ang = placingZone->ang + DegreeToRad (270.0);

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = timberRail.leftBottomX;
	//	rotatedPoint.y = timberRail.leftBottomY;
	//	rotatedPoint.z = timberRail.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	timberRail.leftBottomX = unrotatedPoint.x;
	//	timberRail.leftBottomY = unrotatedPoint.y;
	//	timberRail.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (timberRail));
	//}

	//// ����� - ������ 2�� (�Ϻ� ���� �κ�)
	//if (placingZone->cellsFromBeginAtBottom [2][0].perLen > EPS) {
	//	if (placingZone->bFillMarginBeginAtBottom == true) {
	//		timberRail.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtBottom * marginBeginAtBottom) + 0.003;
	//		timberRail.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide - placingZone->cellsFromBeginAtBottom [0][0].perLen - placingZone->cellsFromBeginAtBottom [1][0].perLen - 0.023 + 0.194;
	//		timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0525;
	//		if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.600) < EPS)			timberRail.leftBottomY -= 0.450;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.500) < EPS)		timberRail.leftBottomY -= 0.350;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.450) < EPS)		timberRail.leftBottomY -= 0.300;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.400) < EPS)		timberRail.leftBottomY -= 0.250;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.300) < EPS)		timberRail.leftBottomY -= 0.150;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.200) < EPS)		timberRail.leftBottomY -= 0.150;
	//		timberRail.angX = DegreeToRad (90.0);
	//		timberRail.angY = DegreeToRad (0.0);
	//		timberRail.ang = placingZone->ang + DegreeToRad (270.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = timberRail.leftBottomX;
	//		rotatedPoint.y = timberRail.leftBottomY;
	//		rotatedPoint.z = timberRail.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		timberRail.leftBottomX = unrotatedPoint.x;
	//		timberRail.leftBottomY = unrotatedPoint.y;
	//		timberRail.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (timberRail));
	//	}
	//	if ((placingZone->cellCenterAtBottom [0].objType == PLYWOOD) && (placingZone->cellCenterAtBottom [0].dirLen > EPS)) {
	//		accumDist = 0.0;
	//		for (xx = 0 ; xx < placingZone->nCellsFromBeginAtBottom ; ++xx)
	//			accumDist += placingZone->cellsFromBeginAtBottom [0][xx].dirLen;

	//		timberRail.leftBottomX = placingZone->begC.x + (bFillMarginBeginAtBottom * marginBeginAtBottom) + accumDist - 0.003;
	//		timberRail.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide - placingZone->cellsFromBeginAtBottom [0][0].perLen - placingZone->cellsFromBeginAtBottom [1][0].perLen - 0.023;
	//		timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0525;
	//		if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.600) < EPS)			timberRail.leftBottomY -= 0.450;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.500) < EPS)		timberRail.leftBottomY -= 0.350;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.450) < EPS)		timberRail.leftBottomY -= 0.300;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.400) < EPS)		timberRail.leftBottomY -= 0.250;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.300) < EPS)		timberRail.leftBottomY -= 0.150;
	//		else if (abs (placingZone->cellsFromBeginAtBottom [2][0].perLen - 0.200) < EPS)		timberRail.leftBottomY -= 0.150;
	//		timberRail.angX = DegreeToRad (90.0);
	//		timberRail.angY = DegreeToRad (180.0);
	//		timberRail.ang = placingZone->ang + DegreeToRad (270.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = timberRail.leftBottomX;
	//		rotatedPoint.y = timberRail.leftBottomY;
	//		rotatedPoint.z = timberRail.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		timberRail.leftBottomX = unrotatedPoint.x;
	//		timberRail.leftBottomY = unrotatedPoint.y;
	//		timberRail.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (timberRail));
	//	}
	//}

	//// ����� - ������ 1�� (�Ϻ� �� �κ�)
	//if (placingZone->bFillMarginEndAtBottom == true) {
	//	timberRail.leftBottomX = placingZone->begC.x + beamLength - (bFillMarginEndAtBottom * marginEndAtBottom) - 0.003;
	//	timberRail.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide - 0.023;
	//	timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0525;
	//	if (abs (placingZone->cellsFromEndAtBottom [0][0].perLen - 0.600) < EPS)			timberRail.leftBottomY -= 0.450;
	//	else if (abs (placingZone->cellsFromEndAtBottom [0][0].perLen - 0.500) < EPS)		timberRail.leftBottomY -= 0.350;
	//	else if (abs (placingZone->cellsFromEndAtBottom [0][0].perLen - 0.450) < EPS)		timberRail.leftBottomY -= 0.300;
	//	else if (abs (placingZone->cellsFromEndAtBottom [0][0].perLen - 0.400) < EPS)		timberRail.leftBottomY -= 0.250;
	//	else if (abs (placingZone->cellsFromEndAtBottom [0][0].perLen - 0.300) < EPS)		timberRail.leftBottomY -= 0.150;
	//	else if (abs (placingZone->cellsFromEndAtBottom [0][0].perLen - 0.200) < EPS)		timberRail.leftBottomY -= 0.150;
	//	timberRail.angX = DegreeToRad (90.0);
	//	timberRail.angY = DegreeToRad (180.0);
	//	timberRail.ang = placingZone->ang + DegreeToRad (270.0);

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = timberRail.leftBottomX;
	//	rotatedPoint.y = timberRail.leftBottomY;
	//	rotatedPoint.z = timberRail.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	timberRail.leftBottomX = unrotatedPoint.x;
	//	timberRail.leftBottomY = unrotatedPoint.y;
	//	timberRail.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (timberRail));
	//}
	//if ((placingZone->cellCenterAtBottom [0].objType == PLYWOOD) && (placingZone->cellCenterAtBottom [0].dirLen > EPS)) {
	//	accumDist = 0.0;
	//	for (xx = 0 ; xx < placingZone->nCellsFromEndAtBottom ; ++xx)
	//		accumDist += placingZone->cellsFromEndAtBottom [0][xx].dirLen;

	//	timberRail.leftBottomX = placingZone->begC.x + beamLength - (bFillMarginEndAtBottom * marginEndAtBottom) - accumDist + 0.003;
	//	timberRail.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide - 0.023 + 0.194;
	//	timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0525;
	//	if (abs (placingZone->cellsFromEndAtBottom [0][0].perLen - 0.600) < EPS)			timberRail.leftBottomY -= 0.450;
	//	else if (abs (placingZone->cellsFromEndAtBottom [0][0].perLen - 0.500) < EPS)		timberRail.leftBottomY -= 0.350;
	//	else if (abs (placingZone->cellsFromEndAtBottom [0][0].perLen - 0.450) < EPS)		timberRail.leftBottomY -= 0.300;
	//	else if (abs (placingZone->cellsFromEndAtBottom [0][0].perLen - 0.400) < EPS)		timberRail.leftBottomY -= 0.250;
	//	else if (abs (placingZone->cellsFromEndAtBottom [0][0].perLen - 0.300) < EPS)		timberRail.leftBottomY -= 0.150;
	//	else if (abs (placingZone->cellsFromEndAtBottom [0][0].perLen - 0.200) < EPS)		timberRail.leftBottomY -= 0.150;
	//	timberRail.angX = DegreeToRad (90.0);
	//	timberRail.angY = DegreeToRad (00.0);
	//	timberRail.ang = placingZone->ang + DegreeToRad (270.0);

	//	axisPoint.x = placingZone->begC.x;
	//	axisPoint.y = placingZone->begC.y;
	//	axisPoint.z = placingZone->begC.z;

	//	rotatedPoint.x = timberRail.leftBottomX;
	//	rotatedPoint.y = timberRail.leftBottomY;
	//	rotatedPoint.z = timberRail.leftBottomZ;
	//	unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//	timberRail.leftBottomX = unrotatedPoint.x;
	//	timberRail.leftBottomY = unrotatedPoint.y;
	//	timberRail.leftBottomZ = unrotatedPoint.z;

	//	elemList.Push (placeLibPart (timberRail));
	//}

	//// ����� - ������ 2�� (�Ϻ� �� �κ�)
	//if (placingZone->cellsFromEndAtBottom [2][0].perLen > EPS) {
	//	if (placingZone->bFillMarginEndAtBottom == true) {
	//		timberRail.leftBottomX = placingZone->begC.x + beamLength - (bFillMarginEndAtBottom * marginEndAtBottom) - 0.003;
	//		timberRail.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide - placingZone->cellsFromEndAtBottom [0][0].perLen - placingZone->cellsFromEndAtBottom [1][0].perLen - 0.023;
	//		timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0525;
	//		if (abs (placingZone->cellsFromEndAtBottom [2][0].perLen - 0.600) < EPS)			timberRail.leftBottomY -= 0.450;
	//		else if (abs (placingZone->cellsFromEndAtBottom [2][0].perLen - 0.500) < EPS)		timberRail.leftBottomY -= 0.350;
	//		else if (abs (placingZone->cellsFromEndAtBottom [2][0].perLen - 0.450) < EPS)		timberRail.leftBottomY -= 0.300;
	//		else if (abs (placingZone->cellsFromEndAtBottom [2][0].perLen - 0.400) < EPS)		timberRail.leftBottomY -= 0.250;
	//		else if (abs (placingZone->cellsFromEndAtBottom [2][0].perLen - 0.300) < EPS)		timberRail.leftBottomY -= 0.150;
	//		else if (abs (placingZone->cellsFromEndAtBottom [2][0].perLen - 0.200) < EPS)		timberRail.leftBottomY -= 0.150;
	//		timberRail.angX = DegreeToRad (90.0);
	//		timberRail.angY = DegreeToRad (180.0);
	//		timberRail.ang = placingZone->ang + DegreeToRad (270.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = timberRail.leftBottomX;
	//		rotatedPoint.y = timberRail.leftBottomY;
	//		rotatedPoint.z = timberRail.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		timberRail.leftBottomX = unrotatedPoint.x;
	//		timberRail.leftBottomY = unrotatedPoint.y;
	//		timberRail.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (timberRail));
	//	}
	//	if ((placingZone->cellCenterAtBottom [0].objType == PLYWOOD) && (placingZone->cellCenterAtBottom [0].dirLen > EPS)) {
	//		accumDist = 0.0;
	//		for (xx = 0 ; xx < placingZone->nCellsFromEndAtBottom ; ++xx)
	//			accumDist += placingZone->cellsFromEndAtBottom [0][xx].dirLen;

	//		timberRail.leftBottomX = placingZone->begC.x + beamLength - (bFillMarginEndAtBottom * marginEndAtBottom) - accumDist + 0.003;
	//		timberRail.leftBottomY = placingZone->begC.y + infoBeam.width/2 + infoBeam.offset + placingZone->gapSide - placingZone->cellsFromEndAtBottom [0][0].perLen - placingZone->cellsFromEndAtBottom [1][0].perLen - 0.023 + 0.194;
	//		timberRail.leftBottomZ = placingZone->level - infoBeam.height - placingZone->gapBottom - 0.0525;
	//		if (abs (placingZone->cellsFromEndAtBottom [2][0].perLen - 0.600) < EPS)			timberRail.leftBottomY -= 0.450;
	//		else if (abs (placingZone->cellsFromEndAtBottom [2][0].perLen - 0.500) < EPS)		timberRail.leftBottomY -= 0.350;
	//		else if (abs (placingZone->cellsFromEndAtBottom [2][0].perLen - 0.450) < EPS)		timberRail.leftBottomY -= 0.300;
	//		else if (abs (placingZone->cellsFromEndAtBottom [2][0].perLen - 0.400) < EPS)		timberRail.leftBottomY -= 0.250;
	//		else if (abs (placingZone->cellsFromEndAtBottom [2][0].perLen - 0.300) < EPS)		timberRail.leftBottomY -= 0.150;
	//		else if (abs (placingZone->cellsFromEndAtBottom [2][0].perLen - 0.200) < EPS)		timberRail.leftBottomY -= 0.150;
	//		timberRail.angX = DegreeToRad (90.0);
	//		timberRail.angY = DegreeToRad (00.0);
	//		timberRail.ang = placingZone->ang + DegreeToRad (270.0);

	//		axisPoint.x = placingZone->begC.x;
	//		axisPoint.y = placingZone->begC.y;
	//		axisPoint.z = placingZone->begC.z;

	//		rotatedPoint.x = timberRail.leftBottomX;
	//		rotatedPoint.y = timberRail.leftBottomY;
	//		rotatedPoint.z = timberRail.leftBottomZ;
	//		unrotatedPoint = getUnrotatedPoint (rotatedPoint, axisPoint, RadToDegree (placingZone->ang));

	//		timberRail.leftBottomX = unrotatedPoint.x;
	//		timberRail.leftBottomY = unrotatedPoint.y;
	//		timberRail.leftBottomZ = unrotatedPoint.z;

	//		elemList.Push (placeLibPart (timberRail));
	//	}
	//}

	return	err;
}

// 1�� ��ġ�� ���� ���Ǹ� ��û�ϴ� 1�� ���̾�α�
short DGCALLBACK beamTableformPlacerHandler1 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short		result;
	short		xx;
	double		h1, h2, h3, h4, hRest;	// ������ ���� ����� ���� ����
	API_UCCallbackType	ucb;

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, "���� ��ġ - �� �ܸ�");

			//////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			// Ȯ�� ��ư
			DGSetItemText (dialogID, DG_OK, "Ȯ ��");

			// ��� ��ư
			DGSetItemText (dialogID, DG_CANCEL, "�� ��");

			//////////////////////////////////////////////////////////// ������ ��ġ (������)
			// �� �� üũ�ڽ�
			DGSetItemText (dialogID, LABEL_BEAM_SECTION, "�� �ܸ�");
			DGSetItemText (dialogID, LABEL_BEAM_LEFT_HEIGHT, "�� ���� (L)");
			DGSetItemText (dialogID, LABEL_BEAM_RIGHT_HEIGHT, "�� ���� (R)");
			DGSetItemText (dialogID, LABEL_BEAM_WIDTH, "�� �ʺ�");
			DGSetItemText (dialogID, LABEL_TOTAL_LEFT_HEIGHT, "�� ���� (L)");
			DGSetItemText (dialogID, LABEL_TOTAL_RIGHT_HEIGHT, "�� ���� (R)");
			DGSetItemText (dialogID, LABEL_TOTAL_WIDTH, "�� �ʺ�");

			DGSetItemText (dialogID, LABEL_REST_LSIDE, "������");
			DGSetItemText (dialogID, CHECKBOX_TIMBER_LSIDE, "����");
			DGSetItemText (dialogID, CHECKBOX_T_FORM_LSIDE, "������");
			DGSetItemText (dialogID, CHECKBOX_FILLER_LSIDE, "�ٷ�");
			DGSetItemText (dialogID, CHECKBOX_B_FORM_LSIDE, "������");

			DGSetItemText (dialogID, LABEL_REST_RSIDE, "������");
			DGSetItemText (dialogID, CHECKBOX_TIMBER_RSIDE, "����");
			DGSetItemText (dialogID, CHECKBOX_T_FORM_RSIDE, "������");
			DGSetItemText (dialogID, CHECKBOX_FILLER_RSIDE, "�ٷ�");
			DGSetItemText (dialogID, CHECKBOX_B_FORM_RSIDE, "������");

			DGSetItemText (dialogID, CHECKBOX_L_FORM_BOTTOM, "������");
			DGSetItemText (dialogID, CHECKBOX_FILLER_BOTTOM, "�ٷ�");
			DGSetItemText (dialogID, CHECKBOX_R_FORM_BOTTOM, "������");

			DGSetItemText (dialogID, BUTTON_COPY_TO_RIGHT, "����\n��");

			// ��: ���̾� ����
			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, "���纰 ���̾� ����");
			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM, "������");
			DGSetItemText (dialogID, LABEL_LAYER_PLYWOOD, "����");
			DGSetItemText (dialogID, LABEL_LAYER_TIMBER, "����");
			DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER_ANGLE, "�ƿ��ڳʾޱ�");
			DGSetItemText (dialogID, LABEL_LAYER_FILLERSPACER, "�ٷ������̼�");
			DGSetItemText (dialogID, LABEL_LAYER_RECTPIPE, "���������");
			DGSetItemText (dialogID, LABEL_LAYER_RECTPIPE_HANGER, "�����������");
			DGSetItemText (dialogID, LABEL_LAYER_PINBOLT, "�ɺ�Ʈ��Ʈ");
			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM_HOOK, "������ ��ũ");
			DGSetItemText (dialogID, LABEL_LAYER_BLUE_CLAMP, "���Ŭ����");
			DGSetItemText (dialogID, LABEL_LAYER_BLUE_TIMBER_RAIL, "�����");

			// üũ�ڽ�: ���̾� ����
			DGSetItemText (dialogID, CHECKBOX_LAYER_COUPLING, "���̾� ����");
			DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, TRUE);

			DGSetItemText (dialogID, BUTTON_AUTOSET, "���̾� �ڵ� ����");

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

			ucb.itemID	 = USERCONTROL_LAYER_TIMBER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, 1);

			ucb.itemID	 = USERCONTROL_LAYER_OUTCORNER_ANGLE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, 1);

			ucb.itemID	 = USERCONTROL_LAYER_FILLERSPACER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSPACER, 1);

			ucb.itemID	 = USERCONTROL_LAYER_RECTPIPE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, 1);

			ucb.itemID	 = USERCONTROL_LAYER_RECTPIPE_HANGER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, 1);

			ucb.itemID	 = USERCONTROL_LAYER_PINBOLT;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, 1);

			ucb.itemID	 = USERCONTROL_LAYER_EUROFORM_HOOK;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, 1);

			ucb.itemID	 = USERCONTROL_LAYER_BLUE_CLAMP;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_CLAMP, 1);

			ucb.itemID	 = USERCONTROL_LAYER_BLUE_TIMBER_RAIL;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL, 1);

			// �� ����/�ʺ� ���
			DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_LEFT_HEIGHT, placingZone.areaHeight_Left);		// �� ���� (L)
			DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_RIGHT_HEIGHT, placingZone.areaHeight_Right);		// �� ���� (R)
			DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_WIDTH, placingZone.areaWidth_Bottom);			// �� �ʺ�

			// �� ����/�ʺ� ���
			DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_LEFT_HEIGHT, placingZone.areaHeight_Left + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM));		// �� ���� (L)
			DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_WIDTH, placingZone.areaWidth_Bottom + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_SIDE1) + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_SIDE2));		// �� �ʺ�
			DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_RIGHT_HEIGHT, placingZone.areaHeight_Right + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM));	// �� ���� (R)

			// ���纰 üũ�ڽ�-�԰� ����
			(DGGetItemValLong (dialogID, CHECKBOX_TIMBER_LSIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_TIMBER_LSIDE)	:	DGDisableItem (dialogID, EDITCONTROL_TIMBER_LSIDE);
			(DGGetItemValLong (dialogID, CHECKBOX_T_FORM_LSIDE) == TRUE) ?	DGEnableItem (dialogID, POPUP_T_FORM_LSIDE)			:	DGDisableItem (dialogID, POPUP_T_FORM_LSIDE);
			(DGGetItemValLong (dialogID, CHECKBOX_FILLER_LSIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_FILLER_LSIDE)	:	DGDisableItem (dialogID, EDITCONTROL_FILLER_LSIDE);

			(DGGetItemValLong (dialogID, CHECKBOX_FILLER_BOTTOM) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_FILLER_BOTTOM)	:	DGDisableItem (dialogID, EDITCONTROL_FILLER_BOTTOM);
			(DGGetItemValLong (dialogID, CHECKBOX_R_FORM_BOTTOM) == TRUE) ?	DGEnableItem (dialogID, POPUP_R_FORM_BOTTOM)		:	DGDisableItem (dialogID, POPUP_R_FORM_BOTTOM);

			(DGGetItemValLong (dialogID, CHECKBOX_TIMBER_RSIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_TIMBER_RSIDE)	:	DGDisableItem (dialogID, EDITCONTROL_TIMBER_RSIDE);
			(DGGetItemValLong (dialogID, CHECKBOX_T_FORM_RSIDE) == TRUE) ?	DGEnableItem (dialogID, POPUP_T_FORM_RSIDE)			:	DGDisableItem (dialogID, POPUP_T_FORM_RSIDE);
			(DGGetItemValLong (dialogID, CHECKBOX_FILLER_RSIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_FILLER_RSIDE)	:	DGDisableItem (dialogID, EDITCONTROL_FILLER_RSIDE);

			// ���� 0��, �Ϻ� 0�� ���� ������ ����ؾ� ��
			DGSetItemValLong (dialogID, CHECKBOX_B_FORM_LSIDE, TRUE);
			DGSetItemValLong (dialogID, CHECKBOX_B_FORM_RSIDE, TRUE);
			DGSetItemValLong (dialogID, CHECKBOX_L_FORM_BOTTOM, TRUE);
			DGDisableItem (dialogID, CHECKBOX_B_FORM_LSIDE);
			DGDisableItem (dialogID, CHECKBOX_B_FORM_RSIDE);
			DGDisableItem (dialogID, CHECKBOX_L_FORM_BOTTOM);

			// ������ �� ���
			h1 = 0;
			h2 = 0;
			h3 = 0;
			h4 = 0;
			if (DGGetItemValLong (dialogID, CHECKBOX_TIMBER_LSIDE) == TRUE)		h1 = DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_LSIDE);
			if (DGGetItemValLong (dialogID, CHECKBOX_T_FORM_LSIDE) == TRUE)		h2 = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_LSIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_LSIDE)).ToCStr ()) / 1000.0;
			if (DGGetItemValLong (dialogID, CHECKBOX_FILLER_LSIDE) == TRUE)		h3 = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_LSIDE);
			if (DGGetItemValLong (dialogID, CHECKBOX_B_FORM_LSIDE) == TRUE)		h4 = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_LSIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_LSIDE)).ToCStr ()) / 1000.0;
			hRest = placingZone.areaHeight_Left + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM) - (h1 + h2 + h3 + h4);
			DGSetItemValDouble (dialogID, EDITCONTROL_REST_LSIDE, hRest);

			h1 = 0;
			h2 = 0;
			h3 = 0;
			h4 = 0;
			if (DGGetItemValLong (dialogID, CHECKBOX_TIMBER_RSIDE) == TRUE)		h1 = DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_RSIDE);
			if (DGGetItemValLong (dialogID, CHECKBOX_T_FORM_RSIDE) == TRUE)		h2 = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_RSIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_RSIDE)).ToCStr ()) / 1000.0;
			if (DGGetItemValLong (dialogID, CHECKBOX_FILLER_RSIDE) == TRUE)		h3 = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_RSIDE);
			if (DGGetItemValLong (dialogID, CHECKBOX_B_FORM_RSIDE) == TRUE)		h4 = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_RSIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_RSIDE)).ToCStr ()) / 1000.0;
			hRest = placingZone.areaHeight_Right + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM) - (h1 + h2 + h3 + h4);
			DGSetItemValDouble (dialogID, EDITCONTROL_REST_RSIDE, hRest);

			// ���� �����ؼ��� �� �Ǵ� �׸� ��ױ�
			DGDisableItem (dialogID, EDITCONTROL_GAP_SIDE2);
			DGDisableItem (dialogID, EDITCONTROL_BEAM_LEFT_HEIGHT);
			DGDisableItem (dialogID, EDITCONTROL_BEAM_RIGHT_HEIGHT);
			DGDisableItem (dialogID, EDITCONTROL_BEAM_WIDTH);
			DGDisableItem (dialogID, EDITCONTROL_TOTAL_LEFT_HEIGHT);
			DGDisableItem (dialogID, EDITCONTROL_TOTAL_RIGHT_HEIGHT);
			DGDisableItem (dialogID, EDITCONTROL_TOTAL_WIDTH);
			DGDisableItem (dialogID, EDITCONTROL_REST_LSIDE);
			DGDisableItem (dialogID, EDITCONTROL_REST_RSIDE);

			// ���̾� �ɼ� Ȱ��ȭ/��Ȱ��ȭ
			if ((DGGetItemValLong (dialogID, CHECKBOX_FILLER_LSIDE) == TRUE) || (DGGetItemValLong (dialogID, CHECKBOX_FILLER_RSIDE) == TRUE) || (DGGetItemValLong (dialogID, CHECKBOX_FILLER_BOTTOM) == TRUE)) {
				DGEnableItem (dialogID, LABEL_LAYER_FILLERSPACER);
				DGEnableItem (dialogID, USERCONTROL_LAYER_FILLERSPACER);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_FILLERSPACER);
				DGDisableItem (dialogID, USERCONTROL_LAYER_FILLERSPACER);
			}

			break;
		
		case DG_MSG_CHANGE:
			// �� ����/�ʺ� ���
			DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_LEFT_HEIGHT, placingZone.areaHeight_Left);		// �� ���� (L)
			DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_RIGHT_HEIGHT, placingZone.areaHeight_Right);		// �� ���� (R)
			DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_WIDTH, placingZone.areaWidth_Bottom);			// �� �ʺ�

			// �� ����/�ʺ� ���
			DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_LEFT_HEIGHT, placingZone.areaHeight_Left + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM));		// �� ���� (L)
			DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_WIDTH, placingZone.areaWidth_Bottom + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_SIDE1) + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_SIDE2));		// �� �ʺ�
			DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_RIGHT_HEIGHT, placingZone.areaHeight_Right + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM));	// �� ���� (R)

			// ���纰 üũ�ڽ�-�԰� ����
			(DGGetItemValLong (dialogID, CHECKBOX_TIMBER_LSIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_TIMBER_LSIDE)	:	DGDisableItem (dialogID, EDITCONTROL_TIMBER_LSIDE);
			(DGGetItemValLong (dialogID, CHECKBOX_T_FORM_LSIDE) == TRUE) ?	DGEnableItem (dialogID, POPUP_T_FORM_LSIDE)			:	DGDisableItem (dialogID, POPUP_T_FORM_LSIDE);
			(DGGetItemValLong (dialogID, CHECKBOX_FILLER_LSIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_FILLER_LSIDE)	:	DGDisableItem (dialogID, EDITCONTROL_FILLER_LSIDE);

			(DGGetItemValLong (dialogID, CHECKBOX_FILLER_BOTTOM) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_FILLER_BOTTOM)	:	DGDisableItem (dialogID, EDITCONTROL_FILLER_BOTTOM);
			(DGGetItemValLong (dialogID, CHECKBOX_R_FORM_BOTTOM) == TRUE) ?	DGEnableItem (dialogID, POPUP_R_FORM_BOTTOM)		:	DGDisableItem (dialogID, POPUP_R_FORM_BOTTOM);

			(DGGetItemValLong (dialogID, CHECKBOX_TIMBER_RSIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_TIMBER_RSIDE)	:	DGDisableItem (dialogID, EDITCONTROL_TIMBER_RSIDE);
			(DGGetItemValLong (dialogID, CHECKBOX_T_FORM_RSIDE) == TRUE) ?	DGEnableItem (dialogID, POPUP_T_FORM_RSIDE)			:	DGDisableItem (dialogID, POPUP_T_FORM_RSIDE);
			(DGGetItemValLong (dialogID, CHECKBOX_FILLER_RSIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_FILLER_RSIDE)	:	DGDisableItem (dialogID, EDITCONTROL_FILLER_RSIDE);

			// ������ �� ���
			h1 = 0;
			h2 = 0;
			h3 = 0;
			h4 = 0;
			if (DGGetItemValLong (dialogID, CHECKBOX_TIMBER_LSIDE) == TRUE)		h1 = DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_LSIDE);
			if (DGGetItemValLong (dialogID, CHECKBOX_T_FORM_LSIDE) == TRUE)		h2 = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_LSIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_LSIDE)).ToCStr ()) / 1000.0;
			if (DGGetItemValLong (dialogID, CHECKBOX_FILLER_LSIDE) == TRUE)		h3 = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_LSIDE);
			if (DGGetItemValLong (dialogID, CHECKBOX_B_FORM_LSIDE) == TRUE)		h4 = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_LSIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_LSIDE)).ToCStr ()) / 1000.0;
			hRest = placingZone.areaHeight_Left + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM) - (h1 + h2 + h3 + h4);
			DGSetItemValDouble (dialogID, EDITCONTROL_REST_LSIDE, hRest);

			h1 = 0;
			h2 = 0;
			h3 = 0;
			h4 = 0;
			if (DGGetItemValLong (dialogID, CHECKBOX_TIMBER_RSIDE) == TRUE)		h1 = DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_RSIDE);
			if (DGGetItemValLong (dialogID, CHECKBOX_T_FORM_RSIDE) == TRUE)		h2 = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_RSIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_RSIDE)).ToCStr ()) / 1000.0;
			if (DGGetItemValLong (dialogID, CHECKBOX_FILLER_RSIDE) == TRUE)		h3 = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_RSIDE);
			if (DGGetItemValLong (dialogID, CHECKBOX_B_FORM_RSIDE) == TRUE)		h4 = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_RSIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_RSIDE)).ToCStr ()) / 1000.0;
			hRest = placingZone.areaHeight_Right + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM) - (h1 + h2 + h3 + h4);
			DGSetItemValDouble (dialogID, EDITCONTROL_REST_RSIDE, hRest);

			// ���� ������ �ٲ�� ������ ���ݵ� �����ϰ� �ٲ�
			DGSetItemValDouble (dialogID, EDITCONTROL_GAP_SIDE2, DGGetItemValDouble (dialogID, EDITCONTROL_GAP_SIDE1));

			// ���̾� ���� �ٲ�
			if ((item >= USERCONTROL_LAYER_EUROFORM) && (item <= USERCONTROL_LAYER_BLUE_TIMBER_RAIL)) {
				if (DGGetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING) == 1) {
					long selectedLayer;

					selectedLayer = DGGetItemValLong (dialogID, item);

					for (xx = USERCONTROL_LAYER_EUROFORM ; xx <= USERCONTROL_LAYER_BLUE_TIMBER_RAIL ; ++xx)
						DGSetItemValLong (dialogID, xx, selectedLayer);
				}
			}

			// ���̾� �ɼ� Ȱ��ȭ/��Ȱ��ȭ
			if ((DGGetItemValLong (dialogID, CHECKBOX_FILLER_LSIDE) == TRUE) || (DGGetItemValLong (dialogID, CHECKBOX_FILLER_RSIDE) == TRUE) || (DGGetItemValLong (dialogID, CHECKBOX_FILLER_BOTTOM) == TRUE)) {
				DGEnableItem (dialogID, LABEL_LAYER_FILLERSPACER);
				DGEnableItem (dialogID, USERCONTROL_LAYER_FILLERSPACER);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_FILLERSPACER);
				DGDisableItem (dialogID, USERCONTROL_LAYER_FILLERSPACER);
			}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					///////////////////////////////////////////////////////////////// ���� ���� (������)
					if (DGGetItemValLong (dialogID, CHECKBOX_B_FORM_LSIDE) == TRUE) {
						for (xx = 0 ; xx < 50 ; ++xx) {
							placingZone.cellsAtLSide [0][xx].objType = EUROFORM;
							placingZone.cellsAtLSide [0][xx].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_LSIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_LSIDE)).ToCStr ()) / 1000.0;
						}
					}

					if (DGGetItemValLong (dialogID, CHECKBOX_FILLER_LSIDE) == TRUE) {
						for (xx = 0 ; xx < 50 ; ++xx) {
							placingZone.cellsAtLSide [1][xx].objType = FILLERSP;
							placingZone.cellsAtLSide [1][xx].perLen = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_LSIDE);
						}
					}

					if (DGGetItemValLong (dialogID, CHECKBOX_T_FORM_LSIDE) == TRUE) {
						for (xx = 0 ; xx < 50 ; ++xx) {
							placingZone.cellsAtLSide [2][xx].objType = EUROFORM;
							placingZone.cellsAtLSide [2][xx].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_LSIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_LSIDE)).ToCStr ()) / 1000.0;
						}
					}

					if (DGGetItemValLong (dialogID, CHECKBOX_TIMBER_LSIDE) == TRUE) {
						for (xx = 0 ; xx < 50 ; ++xx) {
							if (DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_LSIDE) >= 0.090 - EPS)
								placingZone.cellsAtLSide [3][xx].objType = PLYWOOD;
							else
								placingZone.cellsAtLSide [3][xx].objType = TIMBER;
							placingZone.cellsAtLSide [3][xx].perLen = DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_LSIDE);
						}
					}

					///////////////////////////////////////////////////////////////// ������ ���� (������)
					if (DGGetItemValLong (dialogID, CHECKBOX_B_FORM_RSIDE) == TRUE) {
						for (xx = 0 ; xx < 50 ; ++xx) {
							placingZone.cellsAtRSide [0][xx].objType = EUROFORM;
							placingZone.cellsAtRSide [0][xx].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_RSIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_RSIDE)).ToCStr ()) / 1000.0;
						}
					}

					if (DGGetItemValLong (dialogID, CHECKBOX_FILLER_RSIDE) == TRUE) {
						for (xx = 0 ; xx < 50 ; ++xx) {
							placingZone.cellsAtRSide [1][xx].objType = FILLERSP;
							placingZone.cellsAtRSide [1][xx].perLen = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_RSIDE);
						}
					}

					if (DGGetItemValLong (dialogID, CHECKBOX_T_FORM_RSIDE) == TRUE) {
						for (xx = 0 ; xx < 50 ; ++xx) {
							placingZone.cellsAtRSide [2][xx].objType = EUROFORM;
							placingZone.cellsAtRSide [2][xx].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_RSIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_RSIDE)).ToCStr ()) / 1000.0;
						}
					}

					if (DGGetItemValLong (dialogID, CHECKBOX_TIMBER_RSIDE) == TRUE) {
						for (xx = 0 ; xx < 50 ; ++xx) {
							if (DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_RSIDE) >= 0.090 - EPS)
								placingZone.cellsAtRSide [3][xx].objType = PLYWOOD;
							else
								placingZone.cellsAtRSide [3][xx].objType = TIMBER;
							placingZone.cellsAtRSide [3][xx].perLen = DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_RSIDE);
						}
					}

					///////////////////////////////////////////////////////////////// �Ϻ�
					if (DGGetItemValLong (dialogID, CHECKBOX_L_FORM_BOTTOM) == TRUE) {
						for (xx = 0 ; xx < 50 ; ++xx) {
							placingZone.cellsAtBottom [0][xx].objType = EUROFORM;
							placingZone.cellsAtBottom [0][xx].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_L_FORM_BOTTOM, DGPopUpGetSelected (dialogID, POPUP_L_FORM_BOTTOM)).ToCStr ()) / 1000.0;
						}
					}

					if (DGGetItemValLong (dialogID, CHECKBOX_FILLER_BOTTOM) == TRUE) {
						for (xx = 0 ; xx < 50 ; ++xx) {
							placingZone.cellsAtBottom [1][xx].objType = FILLERSP;
							placingZone.cellsAtBottom [1][xx].perLen = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_BOTTOM);
						}
					}

					if (DGGetItemValLong (dialogID, CHECKBOX_R_FORM_BOTTOM) == TRUE) {
						for (xx = 0 ; xx < 50 ; ++xx) {
							placingZone.cellsAtBottom [2][xx].objType = EUROFORM;
							placingZone.cellsAtBottom [2][xx].perLen = atof (DGPopUpGetItemText (dialogID, POPUP_R_FORM_BOTTOM, DGPopUpGetSelected (dialogID, POPUP_R_FORM_BOTTOM)).ToCStr ()) / 1000.0;
						}
					}

					// ������ ����
					placingZone.gapSide = DGGetItemValDouble (dialogID, EDITCONTROL_GAP_SIDE1);
					placingZone.gapBottom = DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM);

					// ���̾� ��ȣ ����
					layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM);
					layerInd_Plywood		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD);
					layerInd_Timber			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER);
					layerInd_OutcornerAngle	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE);
					layerInd_Fillerspacer	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSPACER);
					layerInd_Rectpipe		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE);
					layerInd_RectpipeHanger	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER);
					layerInd_Pinbolt		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT);
					layerInd_EuroformHook	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK);
					layerInd_BlueClamp		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_CLAMP);
					layerInd_BlueTimberRail	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL);

					break;

				case BUTTON_AUTOSET:
					item = 0;

					DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, FALSE);

					layerInd_Euroform			= makeTemporaryLayer (structuralObject_forTableformBeam, "UFOM", NULL);
					layerInd_Plywood			= makeTemporaryLayer (structuralObject_forTableformBeam, "PLYW", NULL);
					layerInd_Timber				= makeTemporaryLayer (structuralObject_forTableformBeam, "TIMB", NULL);
					layerInd_OutcornerAngle		= makeTemporaryLayer (structuralObject_forTableformBeam, "OUTA", NULL);
					layerInd_Fillerspacer		= makeTemporaryLayer (structuralObject_forTableformBeam, "FISP", NULL);
					layerInd_Rectpipe			= makeTemporaryLayer (structuralObject_forTableformBeam, "SPIP", NULL);
					layerInd_RectpipeHanger		= makeTemporaryLayer (structuralObject_forTableformBeam, "JOIB", NULL);
					layerInd_Pinbolt			= makeTemporaryLayer (structuralObject_forTableformBeam, "PINB", NULL);
					layerInd_EuroformHook		= makeTemporaryLayer (structuralObject_forTableformBeam, "HOOK", NULL);
					layerInd_BlueClamp			= makeTemporaryLayer (structuralObject_forTableformBeam, "UFCL", NULL);
					layerInd_BlueTimberRail		= makeTemporaryLayer (structuralObject_forTableformBeam, "RAIL", NULL);

					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, layerInd_Euroform);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, layerInd_Plywood);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, layerInd_Timber);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, layerInd_OutcornerAngle);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSPACER, layerInd_Fillerspacer);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, layerInd_Rectpipe);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, layerInd_RectpipeHanger);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, layerInd_Pinbolt);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, layerInd_EuroformHook);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_CLAMP, layerInd_BlueClamp);
					DGSetItemValLong (dialogID, USERCONTROL_LAYER_BLUE_TIMBER_RAIL, layerInd_BlueTimberRail);

					break;

				case DG_CANCEL:
					break;

				case BUTTON_COPY_TO_RIGHT:
					item = 0;

					// üũ�ڽ� ���� ����
					DGSetItemValLong (dialogID, CHECKBOX_B_FORM_RSIDE, DGGetItemValLong (dialogID, CHECKBOX_B_FORM_LSIDE));
					DGSetItemValLong (dialogID, CHECKBOX_FILLER_RSIDE, DGGetItemValLong (dialogID, CHECKBOX_FILLER_LSIDE));
					DGSetItemValLong (dialogID, CHECKBOX_T_FORM_RSIDE, DGGetItemValLong (dialogID, CHECKBOX_T_FORM_LSIDE));
					DGSetItemValLong (dialogID, CHECKBOX_TIMBER_LSIDE, DGGetItemValLong (dialogID, CHECKBOX_TIMBER_LSIDE));

					// �˾� ��Ʈ�� �� Edit��Ʈ�� �� ����
					DGPopUpSelectItem (dialogID, POPUP_B_FORM_RSIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_LSIDE));
					DGSetItemValDouble (dialogID, EDITCONTROL_FILLER_RSIDE, DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_LSIDE));
					DGPopUpSelectItem (dialogID, POPUP_T_FORM_RSIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_LSIDE));
					DGSetItemValDouble (dialogID, EDITCONTROL_TIMBER_RSIDE, DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_LSIDE));

					// �� ����/�ʺ� ���
					DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_LEFT_HEIGHT, placingZone.areaHeight_Left);		// �� ���� (L)
					DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_RIGHT_HEIGHT, placingZone.areaHeight_Right);		// �� ���� (R)
					DGSetItemValDouble (dialogID, EDITCONTROL_BEAM_WIDTH, placingZone.areaWidth_Bottom);			// �� �ʺ�

					// �� ����/�ʺ� ���
					DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_LEFT_HEIGHT, placingZone.areaHeight_Left + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM));		// �� ���� (L)
					DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_WIDTH, placingZone.areaWidth_Bottom + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_SIDE1) + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_SIDE2));		// �� �ʺ�
					DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_RIGHT_HEIGHT, placingZone.areaHeight_Right + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM));	// �� ���� (R)

					// ���纰 üũ�ڽ�-�԰� ����
					(DGGetItemValLong (dialogID, CHECKBOX_TIMBER_LSIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_TIMBER_LSIDE)	:	DGDisableItem (dialogID, EDITCONTROL_TIMBER_LSIDE);
					(DGGetItemValLong (dialogID, CHECKBOX_T_FORM_LSIDE) == TRUE) ?	DGEnableItem (dialogID, POPUP_T_FORM_LSIDE)			:	DGDisableItem (dialogID, POPUP_T_FORM_LSIDE);
					(DGGetItemValLong (dialogID, CHECKBOX_FILLER_LSIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_FILLER_LSIDE)	:	DGDisableItem (dialogID, EDITCONTROL_FILLER_LSIDE);

					(DGGetItemValLong (dialogID, CHECKBOX_FILLER_BOTTOM) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_FILLER_BOTTOM)	:	DGDisableItem (dialogID, EDITCONTROL_FILLER_BOTTOM);
					(DGGetItemValLong (dialogID, CHECKBOX_R_FORM_BOTTOM) == TRUE) ?	DGEnableItem (dialogID, POPUP_R_FORM_BOTTOM)		:	DGDisableItem (dialogID, POPUP_R_FORM_BOTTOM);

					(DGGetItemValLong (dialogID, CHECKBOX_TIMBER_RSIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_TIMBER_RSIDE)	:	DGDisableItem (dialogID, EDITCONTROL_TIMBER_RSIDE);
					(DGGetItemValLong (dialogID, CHECKBOX_T_FORM_RSIDE) == TRUE) ?	DGEnableItem (dialogID, POPUP_T_FORM_RSIDE)			:	DGDisableItem (dialogID, POPUP_T_FORM_RSIDE);
					(DGGetItemValLong (dialogID, CHECKBOX_FILLER_RSIDE) == TRUE) ?	DGEnableItem (dialogID, EDITCONTROL_FILLER_RSIDE)	:	DGDisableItem (dialogID, EDITCONTROL_FILLER_RSIDE);

					// ������ �� ���
					h1 = 0;
					h2 = 0;
					h3 = 0;
					h4 = 0;
					if (DGGetItemValLong (dialogID, CHECKBOX_TIMBER_LSIDE) == TRUE)		h1 = DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_LSIDE);
					if (DGGetItemValLong (dialogID, CHECKBOX_T_FORM_LSIDE) == TRUE)		h2 = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_LSIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_LSIDE)).ToCStr ()) / 1000.0;
					if (DGGetItemValLong (dialogID, CHECKBOX_FILLER_LSIDE) == TRUE)		h3 = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_LSIDE);
					if (DGGetItemValLong (dialogID, CHECKBOX_B_FORM_LSIDE) == TRUE)		h4 = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_LSIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_LSIDE)).ToCStr ()) / 1000.0;
					hRest = placingZone.areaHeight_Left + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM) - (h1 + h2 + h3 + h4);
					DGSetItemValDouble (dialogID, EDITCONTROL_REST_LSIDE, hRest);

					h1 = 0;
					h2 = 0;
					h3 = 0;
					h4 = 0;
					if (DGGetItemValLong (dialogID, CHECKBOX_TIMBER_RSIDE) == TRUE)		h1 = DGGetItemValDouble (dialogID, EDITCONTROL_TIMBER_RSIDE);
					if (DGGetItemValLong (dialogID, CHECKBOX_T_FORM_RSIDE) == TRUE)		h2 = atof (DGPopUpGetItemText (dialogID, POPUP_T_FORM_RSIDE, DGPopUpGetSelected (dialogID, POPUP_T_FORM_RSIDE)).ToCStr ()) / 1000.0;
					if (DGGetItemValLong (dialogID, CHECKBOX_FILLER_RSIDE) == TRUE)		h3 = DGGetItemValDouble (dialogID, EDITCONTROL_FILLER_RSIDE);
					if (DGGetItemValLong (dialogID, CHECKBOX_B_FORM_RSIDE) == TRUE)		h4 = atof (DGPopUpGetItemText (dialogID, POPUP_B_FORM_RSIDE, DGPopUpGetSelected (dialogID, POPUP_B_FORM_RSIDE)).ToCStr ()) / 1000.0;
					hRest = placingZone.areaHeight_Right + DGGetItemValDouble (dialogID, EDITCONTROL_GAP_BOTTOM) - (h1 + h2 + h3 + h4);
					DGSetItemValDouble (dialogID, EDITCONTROL_REST_RSIDE, hRest);

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
short DGCALLBACK beamTableformPlacerHandler2 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;
	short	btnSizeX = 50, btnSizeY = 50;
	short	btnPosX, btnPosY;
	short	xx;
	short	itmPosX, itmPosY;
	std::string		txtButton = "";
	const short		maxCol = 50;		// �� �ִ� ����
	static short	dialogSizeX = 500;	// ���� ���̾�α� ũ�� X
	static short	dialogSizeY = 360;	// ���� ���̾�α� ũ�� Y

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, "���� ��ġ - �� ����");

			//////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			// Ȯ�� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 230, 70, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "Ȯ��");
			DGShowItem (dialogID, DG_OK);

			// ��� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 270, 70, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "���");
			DGShowItem (dialogID, DG_CANCEL);

			// ���� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 310, 70, 25);
			DGSetItemFont (dialogID, DG_PREV, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_PREV, "����");
			DGShowItem (dialogID, DG_PREV);

			// ��: �� ����
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 10, 10, 100, 23);
			DGSetItemFont (dialogID, LABEL_BEAM_SIDE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_BEAM_SIDE, "�� ����");
			DGShowItem (dialogID, LABEL_BEAM_SIDE);

			// ��: �� ����
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 100, 12, 60, 23);
			DGSetItemFont (dialogID, LABEL_TOTAL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_TOTAL_LENGTH, "�� ����");
			DGShowItem (dialogID, LABEL_TOTAL_LENGTH);

			// Edit��Ʈ��: �� ����
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 165, 5, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_TOTAL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, EDITCONTROL_TOTAL_LENGTH);

			// ��: ���� ����
			DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 230, 12, 60, 23);
			DGSetItemFont (dialogID, LABEL_REMAIN_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_REMAIN_LENGTH, "���� ����");
			DGShowItem (dialogID, LABEL_REMAIN_LENGTH);

			// Edit��Ʈ��: ���� ����
			DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 300, 5, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_REMAIN_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, EDITCONTROL_REMAIN_LENGTH);

			// ��ư: �߰�
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 70, 70, 25);
			DGSetItemFont (dialogID, BUTTON_ADD_COL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_ADD_COL, "�߰�");
			DGShowItem (dialogID, BUTTON_ADD_COL);

			// ��ư: ����
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 105, 70, 25);
			DGSetItemFont (dialogID, BUTTON_DEL_COL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_DEL_COL, "����");
			DGShowItem (dialogID, BUTTON_DEL_COL);

			// ���� �� ���� ä��� ���� (üũ�ڽ�)
			placingZone.CHECKBOX_MARGIN_LEFT_END = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, 120, 70, 70, 70);
			DGSetItemFont (dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END, "����\nä���");
			DGShowItem (dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END);
			DGSetItemValLong (dialogID, placingZone.CHECKBOX_MARGIN_LEFT_END, TRUE);
			// ���� �� ���� ���� (Edit��Ʈ��)
			placingZone.EDITCONTROL_MARGIN_LEFT_END = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 120, 140, 70, 25);
			DGShowItem (dialogID, placingZone.EDITCONTROL_MARGIN_LEFT_END);

			// �Ϲ� ��: �⺻���� ������ !!!
			itmPosX = 120+70;
			itmPosY = 70;
			//for (xx = 0 ; xx < placingZone.nCellsInHor ; ++xx) {
			//	// ��ư
			//	placingZone.BUTTON_OBJ [xx] = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 71, 66);
			//	DGSetItemFont (dialogID, placingZone.BUTTON_OBJ [xx], DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, placingZone.BUTTON_OBJ [xx], "���̺���");
			//	DGShowItem (dialogID, placingZone.BUTTON_OBJ [xx]);

			//	// ��ü Ÿ�� (�˾���Ʈ��)
			//	placingZone.POPUP_OBJ_TYPE [xx] = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY - 25, 70, 23);
			//	DGSetItemFont (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_IS_EXTRASMALL | DG_IS_PLAIN);
			//	DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
			//	DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "����");
			//	DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
			//	DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "���̺���");
			//	DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
			//	DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "������");
			//	DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
			//	DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "�ٷ������̼�");
			//	DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
			//	DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "����");
			//	DGPopUpInsertItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM);
			//	DGPopUpSetItemText (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_BOTTOM, "����");
			//	DGPopUpSelectItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx], DG_POPUP_TOP+1);
			//	DGShowItem (dialogID, placingZone.POPUP_OBJ_TYPE [xx]);

			//	// �ʺ� (�˾���Ʈ��)
			//	placingZone.POPUP_WIDTH [xx] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 50, 1, itmPosX, itmPosY + 68, 70, 23);
			//	DGSetItemFont (dialogID, placingZone.POPUP_WIDTH [xx], DG_IS_LARGE | DG_IS_PLAIN);
			//	for (yy = 0 ; yy < sizeof (placingZone.presetWidth_tableform) / sizeof (int) ; ++yy) {
			//		DGPopUpInsertItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
			//		_itoa (placingZone.presetWidth_tableform [yy], numbuf, 10);
			//		DGPopUpSetItemText (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_BOTTOM, numbuf);
			//	}
			//	DGPopUpSelectItem (dialogID, placingZone.POPUP_WIDTH [xx], DG_POPUP_TOP);
			//	DGShowItem (dialogID, placingZone.POPUP_WIDTH [xx]);

			//	// �ʺ� (�˾���Ʈ��) - ó������ ����
			//	placingZone.EDITCONTROL_WIDTH [xx] = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, itmPosY + 68, 70, 23);
			//	DGHideItem (dialogID, placingZone.EDITCONTROL_WIDTH [xx]);

			//	itmPosX += 70;
			//}

			// ������ �� ���� ä��� ���� (üũ�ڽ�)
			//placingZone.CHECKBOX_RINCORNER = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, 135, 70, 70);
			//DGSetItemFont (dialogID, placingZone.CHECKBOX_RINCORNER, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, placingZone.CHECKBOX_RINCORNER, "���ڳ�");
			//DGShowItem (dialogID, placingZone.CHECKBOX_RINCORNER);
			//DGSetItemValLong (dialogID, placingZone.CHECKBOX_RINCORNER, TRUE);
			// ������ �� ���� ���� (Edit��Ʈ��)
			//placingZone.EDITCONTROL_RINCORNER = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, 205, 70, 25);
			//DGShowItem (dialogID, placingZone.EDITCONTROL_RINCORNER);
			//DGSetItemValDouble (dialogID, placingZone.EDITCONTROL_RINCORNER, 0.100);

			// ���̾�α� ũ�� ����
			dialogSizeX = 500;
			dialogSizeY = 360;
			//if (placingZone.nCellsInHor >= 5) {
			//	DGSetDialogSize (dialogID, DG_CLIENT, dialogSizeX + 70 * (placingZone.nCellsInHor - 5), dialogSizeY, DG_TOPLEFT, true);
			//}

			break;

		case DG_MSG_CLICK:

			switch (item) {
				case DG_PREV:
					clickedPrevButton = true;
					break;

				case DG_OK:
					break;

				case DG_CANCEL:
					break;
			}

			//// Ȯ�� ��ư
			//if (item == DG_OK) {
			//	clickedOKButton = true;

			//	// ���� ä��/��� ���� ����
			//	if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_BEGIN_AT_SIDE) == TRUE)
			//		placingZone.bFillMarginBeginAtSide = true;
			//	if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_END_AT_SIDE) == TRUE)
			//		placingZone.bFillMarginEndAtSide = true;
			//	if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_BEGIN_AT_BOTTOM) == TRUE)
			//		placingZone.bFillMarginBeginAtBottom = true;
			//	if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_END_AT_BOTTOM) == TRUE)
			//		placingZone.bFillMarginEndAtBottom = true;

			//	placingZone.centerLengthAtSide = DGGetItemValDouble (dialogID, EDITCONTROL_CENTER_LENGTH_SIDE);

			//	// �� ���� ���� �߻�, ��� ���� ��ġ ���� ������Ʈ
			//	placingZone.alignPlacingZone (&placingZone);
			//}

			//// ��� ��ư
			//if (item == DG_CANCEL) {
			//}

			//// �� �߰�/���� ��ư 8��
			//if (item == ADD_CELLS_FROM_BEGIN_AT_SIDE) {
			//	placingZone.addNewColFromBeginAtSide (&placingZone);
			//}
			//if (item == DEL_CELLS_FROM_BEGIN_AT_SIDE) {
			//	placingZone.delLastColFromBeginAtSide (&placingZone);
			//}
			//if (item == ADD_CELLS_FROM_END_AT_SIDE) {
			//	placingZone.addNewColFromEndAtSide (&placingZone);
			//}
			//if (item == DEL_CELLS_FROM_END_AT_SIDE) {
			//	placingZone.delLastColFromEndAtSide (&placingZone);
			//}
			//if (item == ADD_CELLS_FROM_BEGIN_AT_BOTTOM) {
			//	placingZone.addNewColFromBeginAtBottom (&placingZone);
			//}
			//if (item == DEL_CELLS_FROM_BEGIN_AT_BOTTOM) {
			//	placingZone.delLastColFromBeginAtBottom (&placingZone);
			//}
			//if (item == ADD_CELLS_FROM_END_AT_BOTTOM) {
			//	placingZone.addNewColFromEndAtBottom (&placingZone);
			//}
			//if (item == DEL_CELLS_FROM_END_AT_BOTTOM) {
			//	placingZone.delLastColFromEndAtBottom (&placingZone);
			//}

			//if ( (item == ADD_CELLS_FROM_BEGIN_AT_SIDE) || (item == DEL_CELLS_FROM_BEGIN_AT_SIDE) || (item == ADD_CELLS_FROM_END_AT_SIDE) || (item == DEL_CELLS_FROM_END_AT_SIDE) ||
			//	 (item == ADD_CELLS_FROM_BEGIN_AT_BOTTOM) || (item == DEL_CELLS_FROM_BEGIN_AT_BOTTOM) || (item == ADD_CELLS_FROM_END_AT_BOTTOM) || (item == DEL_CELLS_FROM_END_AT_BOTTOM)) {

			//	item = 0;

			//	// �� ���� ���� �߻�, ��� ���� ��ġ ���� ������Ʈ
			//	placingZone.alignPlacingZone (&placingZone);

			//	// ���� ���ɼ��� �ִ� DG �׸� ��� ����
			//	DGRemoveDialogItems (dialogID, AFTER_ALL);

			//	// ���� ���� �κ� ���� ä�� ���� - bFillMarginBeginAtSide
			//	// ���� ��ư: ���� (ä��)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 111, 90, 110, 70, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "���� ä��");
			//	DGShowItem (dialogID, itmIdx);
			//	MARGIN_FILL_FROM_BEGIN_AT_SIDE = itmIdx;
			//	DGSetItemValLong (dialogID, itmIdx, TRUE);
			//	// ���� ��ư: ���� (���)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 111, 90, 135, 70, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "���� ���");
			//	DGShowItem (dialogID, itmIdx);
			//	MARGIN_EMPTY_FROM_BEGIN_AT_SIDE = itmIdx;

			//	// ���� ���� �κ� ����
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 100, 50, btnSizeX, btnSizeY);
			//	DGShowItem (dialogID, itmIdx);
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 101, 55, 45, 23);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "����");
			//	DGShowItem (dialogID, itmIdx);
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 102, 74, 45, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemValDouble (dialogID, itmIdx, placingZone.marginBeginAtSide);
			//	DGShowItem (dialogID, itmIdx);
			//	DGDisableItem (dialogID, itmIdx);
			//	MARGIN_FROM_BEGIN_AT_SIDE = itmIdx;
			//	btnPosX = 150;
			//	btnPosY = 50;
			//	// ���� ���� �κ�
			//	for (xx = 0 ; xx < placingZone.nCellsFromBeginAtSide ; ++xx) {
			//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
			//		DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//		txtButton = "";
			//		if (placingZone.cellsFromBeginAtRSide [0][xx].objType == NONE) {
			//			txtButton = "NONE";
			//		} else if (placingZone.cellsFromBeginAtRSide [0][xx].objType == EUROFORM) {
			//			txtButton = format_string ("������\n��%.0f", placingZone.cellsFromBeginAtRSide [0][xx].dirLen * 1000);
			//		}
			//		DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// �׸��� ��ư �ؽ�Ʈ ����
			//		DGShowItem (dialogID, itmIdx);
			//		if (xx == 0) START_INDEX_FROM_BEGIN_AT_SIDE = itmIdx;
			//		btnPosX += 50;
			//	}
			//	// ȭ��ǥ �߰�
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnPosX - 5, btnPosY + 52, 30, 20);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "��");
			//	DGShowItem (dialogID, itmIdx);
			//	// �߰�/���� ��ư
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 70, 50, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "�߰�");
			//	DGShowItem (dialogID, itmIdx);
			//	ADD_CELLS_FROM_BEGIN_AT_SIDE = itmIdx;
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 100, 50, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "����");
			//	DGShowItem (dialogID, itmIdx);
			//	DEL_CELLS_FROM_BEGIN_AT_SIDE = itmIdx;
			//	// ���� �߾� �κ�
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	txtButton = "";
			//	if (placingZone.cellCenterAtRSide [0].objType == NONE) {
			//		txtButton = "NONE";
			//	} else if (placingZone.cellCenterAtRSide [0].objType == EUROFORM) {
			//		txtButton = format_string ("������\n��%.0f", placingZone.cellCenterAtRSide [0].dirLen * 1000);
			//	} else if (placingZone.cellCenterAtRSide [0].objType == PLYWOOD) {
			//		txtButton = format_string ("����\n��%.0f", placingZone.cellCenterAtRSide [0].dirLen * 1000);
			//	}
			//	DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// �׸��� ��ư �ؽ�Ʈ ����
			//	DGShowItem (dialogID, itmIdx);
			//	START_INDEX_CENTER_AT_SIDE = itmIdx;
			//	btnPosX += 50;
			//	// ȭ��ǥ �߰�
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnPosX - 5, btnPosY + 52, 30, 20);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "��");
			//	DGShowItem (dialogID, itmIdx);
			//	// �߰�/���� ��ư
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 70, 50, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "�߰�");
			//	DGShowItem (dialogID, itmIdx);
			//	ADD_CELLS_FROM_END_AT_SIDE = itmIdx;
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 100, 50, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "����");
			//	DGShowItem (dialogID, itmIdx);
			//	DEL_CELLS_FROM_END_AT_SIDE = itmIdx;
			//	// ���� �� �κ�
			//	for (xx = placingZone.nCellsFromEndAtSide-1 ; xx >= 0 ; --xx) {
			//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
			//		DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//		txtButton = "";
			//		if (placingZone.cellsFromEndAtRSide [0][xx].objType == NONE) {
			//			txtButton = "NONE";
			//		} else if (placingZone.cellsFromEndAtRSide [0][xx].objType == EUROFORM) {
			//			txtButton = format_string ("������\n��%.0f", placingZone.cellsFromEndAtRSide [0][xx].dirLen * 1000);
			//		}
			//		DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// �׸��� ��ư �ؽ�Ʈ ����
			//		DGShowItem (dialogID, itmIdx);
			//		if (xx == (placingZone.nCellsFromEndAtSide-1)) END_INDEX_FROM_END_AT_SIDE = itmIdx;
			//		btnPosX += 50;
			//	}
			//	// ���� �� �κ� ����
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, btnPosX, 50, btnSizeX, btnSizeY);
			//	DGShowItem (dialogID, itmIdx);
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, btnPosX+1, 55, 45, 23);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "����");
			//	DGShowItem (dialogID, itmIdx);
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnPosX+2, 74, 45, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemValDouble (dialogID, itmIdx, placingZone.marginEndAtSide);
			//	DGShowItem (dialogID, itmIdx);
			//	DGDisableItem (dialogID, itmIdx);
			//	MARGIN_FROM_END_AT_SIDE = itmIdx;

			//	// ���� �� �κ� ���� ä�� ���� - bFillMarginEndAtSide
			//	// ���� ��ư: ���� (ä��)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 112, btnPosX-10, 110, 70, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "���� ä��");
			//	DGShowItem (dialogID, itmIdx);
			//	MARGIN_FILL_FROM_END_AT_SIDE = itmIdx;
			//	DGSetItemValLong (dialogID, itmIdx, TRUE);
			//	// ���� ��ư: ���� (���)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 112, btnPosX-10, 135, 70, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "���� ���");
			//	DGShowItem (dialogID, itmIdx);
			//	MARGIN_EMPTY_FROM_END_AT_SIDE = itmIdx;

			//	// �Ϻ� ���� �κ� ���� ä�� ���� - bFillMarginBeginAtBottom
			//	// ���� ��ư: ���� (ä��)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 113, 90, 270, 70, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "���� ä��");
			//	DGShowItem (dialogID, itmIdx);
			//	MARGIN_FILL_FROM_BEGIN_AT_BOTTOM = itmIdx;
			//	DGSetItemValLong (dialogID, itmIdx, TRUE);
			//	// ���� ��ư: ���� (���)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 113, 90, 295, 70, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "���� ���");
			//	DGShowItem (dialogID, itmIdx);
			//	MARGIN_EMPTY_FROM_BEGIN_AT_BOTTOM = itmIdx;

			//	// �Ϻ� ���� �κ� ����
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 100, 210, btnSizeX, btnSizeY);
			//	DGShowItem (dialogID, itmIdx);
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 101, 215, 45, 23);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "����");
			//	DGShowItem (dialogID, itmIdx);
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 102, 234, 45, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemValDouble (dialogID, itmIdx, placingZone.marginBeginAtBottom);
			//	DGShowItem (dialogID, itmIdx);
			//	DGDisableItem (dialogID, itmIdx);
			//	MARGIN_FROM_BEGIN_AT_BOTTOM = itmIdx;
			//	btnPosX = 150;
			//	btnPosY = 210;
			//	// �Ϻ� ���� �κ�
			//	for (xx = 0 ; xx < placingZone.nCellsFromBeginAtBottom ; ++xx) {
			//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
			//		DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//		txtButton = "";
			//		if (placingZone.cellsFromBeginAtBottom [0][xx].objType == NONE) {
			//			txtButton = "NONE";
			//		} else if (placingZone.cellsFromBeginAtBottom [0][xx].objType == EUROFORM) {
			//			txtButton = format_string ("������\n��%.0f", placingZone.cellsFromBeginAtBottom [0][xx].dirLen * 1000);
			//		}
			//		DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// �׸��� ��ư �ؽ�Ʈ ����
			//		DGShowItem (dialogID, itmIdx);
			//		if (xx == 0) START_INDEX_FROM_BEGIN_AT_BOTTOM = itmIdx;
			//		btnPosX += 50;
			//	}
			//	// ȭ��ǥ �߰�
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnPosX - 5, btnPosY + 52, 30, 20);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "��");
			//	DGShowItem (dialogID, itmIdx);
			//	// �߰�/���� ��ư
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 70, 50, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "�߰�");
			//	DGShowItem (dialogID, itmIdx);
			//	ADD_CELLS_FROM_BEGIN_AT_BOTTOM = itmIdx;
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 100, 50, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "����");
			//	DGShowItem (dialogID, itmIdx);
			//	DEL_CELLS_FROM_BEGIN_AT_BOTTOM = itmIdx;
			//	// �Ϻ� �߾� �κ�
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	txtButton = "";
			//	if (placingZone.cellCenterAtBottom [0].objType == NONE) {
			//		txtButton = "NONE";
			//	} else if (placingZone.cellCenterAtBottom [0].objType == EUROFORM) {
			//		txtButton = format_string ("������\n��%.0f", placingZone.cellCenterAtBottom [0].dirLen * 1000);
			//	} else if (placingZone.cellCenterAtBottom [0].objType == PLYWOOD) {
			//		txtButton = format_string ("����\n��%.0f", placingZone.cellCenterAtBottom [0].dirLen * 1000);
			//	}
			//	DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// �׸��� ��ư �ؽ�Ʈ ����
			//	DGShowItem (dialogID, itmIdx);
			//	START_INDEX_CENTER_AT_BOTTOM = itmIdx;
			//	btnPosX += 50;
			//	// ȭ��ǥ �߰�
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnPosX - 5, btnPosY + 52, 30, 20);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "��");
			//	DGShowItem (dialogID, itmIdx);
			//	// �߰�/���� ��ư
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 70, 50, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "�߰�");
			//	DGShowItem (dialogID, itmIdx);
			//	ADD_CELLS_FROM_END_AT_BOTTOM = itmIdx;
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 100, 50, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "����");
			//	DGShowItem (dialogID, itmIdx);
			//	DEL_CELLS_FROM_END_AT_BOTTOM = itmIdx;
			//	// �Ϻ� �� �κ�
			//	for (xx = placingZone.nCellsFromEndAtBottom-1 ; xx >= 0 ; --xx) {
			//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
			//		DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//		txtButton = "";
			//		if (placingZone.cellsFromEndAtBottom [0][xx].objType == NONE) {
			//			txtButton = "NONE";
			//		} else if (placingZone.cellsFromEndAtBottom [0][xx].objType == EUROFORM) {
			//			txtButton = format_string ("������\n��%.0f", placingZone.cellsFromEndAtBottom [0][xx].dirLen * 1000);
			//		}
			//		DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// �׸��� ��ư �ؽ�Ʈ ����
			//		DGShowItem (dialogID, itmIdx);
			//		if (xx == (placingZone.nCellsFromEndAtBottom-1)) END_INDEX_FROM_END_AT_BOTTOM = itmIdx;
			//		btnPosX += 50;
			//	}
			//	// �Ϻ� �� �κ� ����
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, btnPosX, 210, btnSizeX, btnSizeY);
			//	DGShowItem (dialogID, itmIdx);
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, btnPosX+1, 215, 45, 23);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "����");
			//	DGShowItem (dialogID, itmIdx);
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnPosX+2, 234, 45, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemValDouble (dialogID, itmIdx, placingZone.marginEndAtBottom);
			//	DGShowItem (dialogID, itmIdx);
			//	DGDisableItem (dialogID, itmIdx);
			//	MARGIN_FROM_END_AT_BOTTOM = itmIdx;

			//	// �Ϻ� �� �κ� ���� ä�� ���� - bFillMarginEndAtBottom
			//	// ���� ��ư: ���� (ä��)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 114, btnPosX-10, 270, 70, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "���� ä��");
			//	DGShowItem (dialogID, itmIdx);
			//	MARGIN_FILL_FROM_END_AT_BOTTOM = itmIdx;
			//	DGSetItemValLong (dialogID, itmIdx, TRUE);
			//	// ���� ��ư: ���� (���)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 114, btnPosX-10, 295, 70, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "���� ���");
			//	DGShowItem (dialogID, itmIdx);
			//	MARGIN_EMPTY_FROM_END_AT_BOTTOM = itmIdx;

			//	// ���� ���� �ٴ� �� ���� ���� (����)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 150 + (btnSizeX * placingZone.nCellsFromBeginAtSide), 24, 50, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGShowItem (dialogID, itmIdx);
			//	if (placingZone.cellCenterAtRSide [0].objType == NONE) {
			//		DGEnableItem (dialogID, itmIdx);
			//		DGEnableItem (dialogID, DG_UPDATE_BUTTON);
			//	} else {
			//		DGDisableItem (dialogID, itmIdx);
			//		DGDisableItem (dialogID, DG_UPDATE_BUTTON);
			//	}
			//	EDITCONTROL_CENTER_LENGTH_SIDE = itmIdx;

			//	// ���� â ũ�⸦ ����
			//	dialogSizeX = max<short>(500, 150 + (btnSizeX * (placingZone.nCellsFromBeginAtBottom + placingZone.nCellsFromEndAtBottom + 1)) + 150);
			//	dialogSizeY = 490;
			//	DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);
			//}

			//// [DIALOG] �׸��� ��ư�� ������ Cell�� �����ϱ� ���� ���� â(3��° ���̾�α�)�� ����
			//if ( ((item >= START_INDEX_FROM_BEGIN_AT_SIDE) && (item < START_INDEX_FROM_BEGIN_AT_SIDE + placingZone.nCellsFromBeginAtSide))			// ��ġ ��ư (���� ���� �κ�)
			//	|| (item == START_INDEX_CENTER_AT_SIDE)																								// ��ġ ��ư (���� �߾� �κ�)
			//	|| ((item >= END_INDEX_FROM_END_AT_SIDE) && (item < END_INDEX_FROM_END_AT_SIDE + placingZone.nCellsFromEndAtSide))					// ��ġ ��ư (���� �� �κ�)
			//	|| ((item >= START_INDEX_FROM_BEGIN_AT_BOTTOM) && (item < START_INDEX_FROM_BEGIN_AT_BOTTOM + placingZone.nCellsFromBeginAtBottom))	// ��ġ ��ư (�Ϻ� ���� �κ�)
			//	|| (item == START_INDEX_CENTER_AT_BOTTOM)																							// ��ġ ��ư (�Ϻ� �߾� �κ�)
			//	|| ((item >= END_INDEX_FROM_END_AT_BOTTOM) && (item < END_INDEX_FROM_END_AT_BOTTOM + placingZone.nCellsFromEndAtBottom))			// ��ġ ��ư (�Ϻ� �� �κ�)
			//	) {
			//	clickedBtnItemIdx = item;
			//	result = DGBlankModalDialog (200, 200, DG_DLG_NOGROW, 0, DG_DLG_NORMALFRAME, beamTableformPlacerHandler3, 0);
			//	item = 0;

			//	// ����� ���� ���� ���� ���� ����
			//	if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_BEGIN_AT_SIDE) == TRUE)
			//		placingZone.bFillMarginBeginAtSide = true;
			//	else
			//		placingZone.bFillMarginBeginAtSide = false;

			//	// ����� ���� �� ���� ���� ����
			//	if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_END_AT_SIDE) == TRUE)
			//		placingZone.bFillMarginEndAtSide = true;
			//	else
			//		placingZone.bFillMarginEndAtSide = false;

			//	// ����� �Ϻ� ���� ���� ���� ����
			//	if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_BEGIN_AT_BOTTOM) == TRUE)
			//		placingZone.bFillMarginBeginAtBottom = true;
			//	else
			//		placingZone.bFillMarginBeginAtBottom = false;

			//	// ����� �Ϻ� �� ���� ���� ����
			//	if (DGGetItemValLong (dialogID, MARGIN_FILL_FROM_END_AT_BOTTOM) == TRUE)
			//		placingZone.bFillMarginEndAtBottom = true;
			//	else
			//		placingZone.bFillMarginEndAtBottom = false;

			//	// ���� ���� �ٴ� �� ���� ���� ����
			//	placingZone.centerLengthAtSide = DGGetItemValDouble (dialogID, EDITCONTROL_CENTER_LENGTH_SIDE);

			//	// �� ���� ���� �߻�, ��� ���� ��ġ ���� ������Ʈ
			//	placingZone.alignPlacingZone (&placingZone);

			//	// ���� ���ɼ��� �ִ� DG �׸� ��� ����
			//	DGRemoveDialogItems (dialogID, AFTER_ALL);

			//	// ���� ���� �κ� ���� ä�� ���� - bFillMarginBeginAtSide
			//	// ���� ��ư: ���� (ä��)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 111, 90, 110, 70, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "���� ä��");
			//	DGShowItem (dialogID, itmIdx);
			//	MARGIN_FILL_FROM_BEGIN_AT_SIDE = itmIdx;
			//	DGSetItemValLong (dialogID, itmIdx, TRUE);
			//	// ���� ��ư: ���� (���)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 111, 90, 135, 70, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "���� ���");
			//	DGShowItem (dialogID, itmIdx);
			//	MARGIN_EMPTY_FROM_BEGIN_AT_SIDE = itmIdx;

			//	// ����� ���� ���� ���� ���� �ε�
			//	if (placingZone.bFillMarginBeginAtSide == true) {
			//		DGSetItemValLong (dialogID, MARGIN_FILL_FROM_BEGIN_AT_SIDE, TRUE);
			//		DGSetItemValLong (dialogID, MARGIN_EMPTY_FROM_BEGIN_AT_SIDE, FALSE);
			//	} else {
			//		DGSetItemValLong (dialogID, MARGIN_FILL_FROM_BEGIN_AT_SIDE, FALSE);
			//		DGSetItemValLong (dialogID, MARGIN_EMPTY_FROM_BEGIN_AT_SIDE, TRUE);
			//	}

			//	// ���� ���� �κ� ����
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 100, 50, btnSizeX, btnSizeY);
			//	DGShowItem (dialogID, itmIdx);
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 101, 55, 45, 23);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "����");
			//	DGShowItem (dialogID, itmIdx);
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 102, 74, 45, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemValDouble (dialogID, itmIdx, placingZone.marginBeginAtSide);
			//	DGShowItem (dialogID, itmIdx);
			//	DGDisableItem (dialogID, itmIdx);
			//	MARGIN_FROM_BEGIN_AT_SIDE = itmIdx;
			//	btnPosX = 150;
			//	btnPosY = 50;
			//	// ���� ���� �κ�
			//	for (xx = 0 ; xx < placingZone.nCellsFromBeginAtSide ; ++xx) {
			//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
			//		DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//		txtButton = "";
			//		if (placingZone.cellsFromBeginAtRSide [0][xx].objType == NONE) {
			//			txtButton = "NONE";
			//		} else if (placingZone.cellsFromBeginAtRSide [0][xx].objType == EUROFORM) {
			//			txtButton = format_string ("������\n��%.0f", placingZone.cellsFromBeginAtRSide [0][xx].dirLen * 1000);
			//		}
			//		DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// �׸��� ��ư �ؽ�Ʈ ����
			//		DGShowItem (dialogID, itmIdx);
			//		if (xx == 0) START_INDEX_FROM_BEGIN_AT_SIDE = itmIdx;
			//		btnPosX += 50;
			//	}
			//	// ȭ��ǥ �߰�
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnPosX - 5, btnPosY + 52, 30, 20);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "��");
			//	DGShowItem (dialogID, itmIdx);
			//	// �߰�/���� ��ư
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 70, 50, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "�߰�");
			//	DGShowItem (dialogID, itmIdx);
			//	ADD_CELLS_FROM_BEGIN_AT_SIDE = itmIdx;
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 100, 50, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "����");
			//	DGShowItem (dialogID, itmIdx);
			//	DEL_CELLS_FROM_BEGIN_AT_SIDE = itmIdx;
			//	// ���� �߾� �κ�
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	txtButton = "";
			//	if (placingZone.cellCenterAtRSide [0].objType == NONE) {
			//		txtButton = "NONE";
			//	} else if (placingZone.cellCenterAtRSide [0].objType == EUROFORM) {
			//		txtButton = format_string ("������\n��%.0f", placingZone.cellCenterAtRSide [0].dirLen * 1000);
			//	} else if (placingZone.cellCenterAtRSide [0].objType == PLYWOOD) {
			//		txtButton = format_string ("����\n��%.0f", placingZone.cellCenterAtRSide [0].dirLen * 1000);
			//	}
			//	DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// �׸��� ��ư �ؽ�Ʈ ����
			//	DGShowItem (dialogID, itmIdx);
			//	START_INDEX_CENTER_AT_SIDE = itmIdx;
			//	btnPosX += 50;
			//	// ȭ��ǥ �߰�
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnPosX - 5, btnPosY + 52, 30, 20);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "��");
			//	DGShowItem (dialogID, itmIdx);
			//	// �߰�/���� ��ư
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 70, 50, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "�߰�");
			//	DGShowItem (dialogID, itmIdx);
			//	ADD_CELLS_FROM_END_AT_SIDE = itmIdx;
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 100, 50, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "����");
			//	DGShowItem (dialogID, itmIdx);
			//	DEL_CELLS_FROM_END_AT_SIDE = itmIdx;
			//	// ���� �� �κ�
			//	for (xx = placingZone.nCellsFromEndAtSide-1 ; xx >= 0 ; --xx) {
			//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
			//		DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//		txtButton = "";
			//		if (placingZone.cellsFromEndAtRSide [0][xx].objType == NONE) {
			//			txtButton = "NONE";
			//		} else if (placingZone.cellsFromEndAtRSide [0][xx].objType == EUROFORM) {
			//			txtButton = format_string ("������\n��%.0f", placingZone.cellsFromEndAtRSide [0][xx].dirLen * 1000);
			//		}
			//		DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// �׸��� ��ư �ؽ�Ʈ ����
			//		DGShowItem (dialogID, itmIdx);
			//		if (xx == (placingZone.nCellsFromEndAtSide-1)) END_INDEX_FROM_END_AT_SIDE = itmIdx;
			//		btnPosX += 50;
			//	}
			//	// ���� �� �κ� ����
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, btnPosX, 50, btnSizeX, btnSizeY);
			//	DGShowItem (dialogID, itmIdx);
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, btnPosX+1, 55, 45, 23);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "����");
			//	DGShowItem (dialogID, itmIdx);
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnPosX+2, 74, 45, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemValDouble (dialogID, itmIdx, placingZone.marginEndAtSide);
			//	DGShowItem (dialogID, itmIdx);
			//	DGDisableItem (dialogID, itmIdx);
			//	MARGIN_FROM_END_AT_SIDE = itmIdx;

			//	// ���� �� �κ� ���� ä�� ���� - bFillMarginEndAtSide
			//	// ���� ��ư: ���� (ä��)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 112, btnPosX-10, 110, 70, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "���� ä��");
			//	DGShowItem (dialogID, itmIdx);
			//	MARGIN_FILL_FROM_END_AT_SIDE = itmIdx;
			//	DGSetItemValLong (dialogID, itmIdx, TRUE);
			//	// ���� ��ư: ���� (���)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 112, btnPosX-10, 135, 70, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "���� ���");
			//	DGShowItem (dialogID, itmIdx);
			//	MARGIN_EMPTY_FROM_END_AT_SIDE = itmIdx;

			//	// ����� ���� �� ���� ���� �ε�
			//	if (placingZone.bFillMarginEndAtSide == true) {
			//		DGSetItemValLong (dialogID, MARGIN_FILL_FROM_END_AT_SIDE, TRUE);
			//		DGSetItemValLong (dialogID, MARGIN_EMPTY_FROM_END_AT_SIDE, FALSE);
			//	} else {
			//		DGSetItemValLong (dialogID, MARGIN_FILL_FROM_END_AT_SIDE, FALSE);
			//		DGSetItemValLong (dialogID, MARGIN_EMPTY_FROM_END_AT_SIDE, TRUE);
			//	}

			//	// �Ϻ� ���� �κ� ���� ä�� ���� - bFillMarginBeginAtBottom
			//	// ���� ��ư: ���� (ä��)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 113, 90, 270, 70, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "���� ä��");
			//	DGShowItem (dialogID, itmIdx);
			//	MARGIN_FILL_FROM_BEGIN_AT_BOTTOM = itmIdx;
			//	DGSetItemValLong (dialogID, itmIdx, TRUE);
			//	// ���� ��ư: ���� (���)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 113, 90, 295, 70, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "���� ���");
			//	DGShowItem (dialogID, itmIdx);
			//	MARGIN_EMPTY_FROM_BEGIN_AT_BOTTOM = itmIdx;

			//	// ����� �Ϻ� ���� ���� ���� �ε�
			//	if (placingZone.bFillMarginBeginAtBottom == true) {
			//		DGSetItemValLong (dialogID, MARGIN_FILL_FROM_BEGIN_AT_BOTTOM, TRUE);
			//		DGSetItemValLong (dialogID, MARGIN_EMPTY_FROM_BEGIN_AT_BOTTOM, FALSE);
			//	} else {
			//		DGSetItemValLong (dialogID, MARGIN_FILL_FROM_BEGIN_AT_BOTTOM, FALSE);
			//		DGSetItemValLong (dialogID, MARGIN_EMPTY_FROM_BEGIN_AT_BOTTOM, TRUE);
			//	}

			//	// �Ϻ� ���� �κ� ����
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 100, 210, btnSizeX, btnSizeY);
			//	DGShowItem (dialogID, itmIdx);
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 101, 215, 45, 23);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "����");
			//	DGShowItem (dialogID, itmIdx);
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 102, 234, 45, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemValDouble (dialogID, itmIdx, placingZone.marginBeginAtBottom);
			//	DGShowItem (dialogID, itmIdx);
			//	DGDisableItem (dialogID, itmIdx);
			//	MARGIN_FROM_BEGIN_AT_BOTTOM = itmIdx;
			//	btnPosX = 150;
			//	btnPosY = 210;
			//	// �Ϻ� ���� �κ�
			//	for (xx = 0 ; xx < placingZone.nCellsFromBeginAtBottom ; ++xx) {
			//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
			//		DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//		txtButton = "";
			//		if (placingZone.cellsFromBeginAtBottom [0][xx].objType == NONE) {
			//			txtButton = "NONE";
			//		} else if (placingZone.cellsFromBeginAtBottom [0][xx].objType == EUROFORM) {
			//			txtButton = format_string ("������\n��%.0f", placingZone.cellsFromBeginAtBottom [0][xx].dirLen * 1000);
			//		}
			//		DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// �׸��� ��ư �ؽ�Ʈ ����
			//		DGShowItem (dialogID, itmIdx);
			//		if (xx == 0) START_INDEX_FROM_BEGIN_AT_BOTTOM = itmIdx;
			//		btnPosX += 50;
			//	}
			//	// ȭ��ǥ �߰�
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnPosX - 5, btnPosY + 52, 30, 20);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "��");
			//	DGShowItem (dialogID, itmIdx);
			//	// �߰�/���� ��ư
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 70, 50, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "�߰�");
			//	DGShowItem (dialogID, itmIdx);
			//	ADD_CELLS_FROM_BEGIN_AT_BOTTOM = itmIdx;
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 100, 50, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "����");
			//	DGShowItem (dialogID, itmIdx);
			//	DEL_CELLS_FROM_BEGIN_AT_BOTTOM = itmIdx;
			//	// �Ϻ� �߾� �κ�
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	txtButton = "";
			//	if (placingZone.cellCenterAtBottom [0].objType == NONE) {
			//		txtButton = "NONE";
			//	} else if (placingZone.cellCenterAtBottom [0].objType == EUROFORM) {
			//		txtButton = format_string ("������\n��%.0f", placingZone.cellCenterAtBottom [0].dirLen * 1000);
			//	} else if (placingZone.cellCenterAtBottom [0].objType == PLYWOOD) {
			//		txtButton = format_string ("����\n��%.0f", placingZone.cellCenterAtBottom [0].dirLen * 1000);
			//	}
			//	DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// �׸��� ��ư �ؽ�Ʈ ����
			//	DGShowItem (dialogID, itmIdx);
			//	START_INDEX_CENTER_AT_BOTTOM = itmIdx;
			//	btnPosX += 50;
			//	// ȭ��ǥ �߰�
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, btnPosX - 5, btnPosY + 52, 30, 20);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "��");
			//	DGShowItem (dialogID, itmIdx);
			//	// �߰�/���� ��ư
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 70, 50, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "�߰�");
			//	DGShowItem (dialogID, itmIdx);
			//	ADD_CELLS_FROM_END_AT_BOTTOM = itmIdx;
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX - 25, btnPosY + 100, 50, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "����");
			//	DGShowItem (dialogID, itmIdx);
			//	DEL_CELLS_FROM_END_AT_BOTTOM = itmIdx;
			//	// �Ϻ� �� �κ�
			//	for (xx = placingZone.nCellsFromEndAtBottom-1 ; xx >= 0 ; --xx) {
			//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, btnPosX, btnPosY, btnSizeX, btnSizeY);
			//		DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//		txtButton = "";
			//		if (placingZone.cellsFromEndAtBottom [0][xx].objType == NONE) {
			//			txtButton = "NONE";
			//		} else if (placingZone.cellsFromEndAtBottom [0][xx].objType == EUROFORM) {
			//			txtButton = format_string ("������\n��%.0f", placingZone.cellsFromEndAtBottom [0][xx].dirLen * 1000);
			//		}
			//		DGSetItemText (dialogID, itmIdx, txtButton.c_str ());	// �׸��� ��ư �ؽ�Ʈ ����
			//		DGShowItem (dialogID, itmIdx);
			//		if (xx == (placingZone.nCellsFromEndAtBottom-1)) END_INDEX_FROM_END_AT_BOTTOM = itmIdx;
			//		btnPosX += 50;
			//	}
			//	// �Ϻ� �� �κ� ����
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, btnPosX, 210, btnSizeX, btnSizeY);
			//	DGShowItem (dialogID, itmIdx);
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, btnPosX+1, 215, 45, 23);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "����");
			//	DGShowItem (dialogID, itmIdx);
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, btnPosX+2, 234, 45, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemValDouble (dialogID, itmIdx, placingZone.marginEndAtBottom);
			//	DGShowItem (dialogID, itmIdx);
			//	DGDisableItem (dialogID, itmIdx);
			//	MARGIN_FROM_END_AT_BOTTOM = itmIdx;

			//	// �Ϻ� �� �κ� ���� ä�� ���� - bFillMarginEndAtBottom
			//	// ���� ��ư: ���� (ä��)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 114, btnPosX-10, 270, 70, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "���� ä��");
			//	DGShowItem (dialogID, itmIdx);
			//	MARGIN_FILL_FROM_END_AT_BOTTOM = itmIdx;
			//	DGSetItemValLong (dialogID, itmIdx, TRUE);
			//	// ���� ��ư: ���� (���)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, 114, btnPosX-10, 295, 70, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGSetItemText (dialogID, itmIdx, "���� ���");
			//	DGShowItem (dialogID, itmIdx);
			//	MARGIN_EMPTY_FROM_END_AT_BOTTOM = itmIdx;

			//	// ����� �Ϻ� �� ���� ���� �ε�
			//	if (placingZone.bFillMarginEndAtBottom == true) {
			//		DGSetItemValLong (dialogID, MARGIN_FILL_FROM_END_AT_BOTTOM, TRUE);
			//		DGSetItemValLong (dialogID, MARGIN_EMPTY_FROM_END_AT_BOTTOM, FALSE);
			//	} else {
			//		DGSetItemValLong (dialogID, MARGIN_FILL_FROM_END_AT_BOTTOM, FALSE);
			//		DGSetItemValLong (dialogID, MARGIN_EMPTY_FROM_END_AT_BOTTOM, TRUE);
			//	}

			//	// ���� ���� �ٴ� �� ���� ���� (����)
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 150 + (btnSizeX * placingZone.nCellsFromBeginAtSide), 24, 50, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//	DGShowItem (dialogID, itmIdx);
			//	if (placingZone.cellCenterAtRSide [0].objType == NONE) {
			//		DGEnableItem (dialogID, itmIdx);
			//		DGEnableItem (dialogID, DG_UPDATE_BUTTON);
			//	} else {
			//		DGDisableItem (dialogID, itmIdx);
			//		DGDisableItem (dialogID, DG_UPDATE_BUTTON);
			//	}
			//	EDITCONTROL_CENTER_LENGTH_SIDE = itmIdx;

			//	// ���� ���� �ٴ� �� ���� ���� �ε�
			//	DGSetItemValDouble (dialogID, EDITCONTROL_CENTER_LENGTH_SIDE, placingZone.centerLengthAtSide);

			//	// ���� â ũ�⸦ ����
			//	dialogSizeX = max<short>(500, 150 + (btnSizeX * (placingZone.nCellsFromBeginAtBottom + placingZone.nCellsFromEndAtBottom + 1)) + 150);
			//	dialogSizeY = 490;
			//	DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);
			//}

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
