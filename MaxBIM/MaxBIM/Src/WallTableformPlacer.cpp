#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "WallTableformPlacer.hpp"

using namespace wallTableformPlacerDG;

static WallTableformPlacingZone		placingZone;	// �⺻ ���� ���� ����
static InfoWallForWallTableform		infoWall;		// �� ��ü ����

static short	layerInd_Euroform;		// ���̾� ��ȣ: ������
static short	layerInd_RectPipe;		// ���̾� ��ȣ: ��� ������
static short	layerInd_PinBolt;		// ���̾� ��ȣ: �ɺ�Ʈ ��Ʈ
static short	layerInd_WallTie;		// ���̾� ��ȣ: ��ü Ÿ��
static short	layerInd_Clamp;			// ���̾� ��ȣ: ���� Ŭ����
static short	layerInd_HeadPiece;		// ���̾� ��ȣ: ����ǽ�

const GS::uchar_t*	gsmUFOM = L("������v2.0.gsm");
const GS::uchar_t*	gsmSPIP = L("���������v1.0.gsm");
const GS::uchar_t*	gsmPINB = L("�ɺ�Ʈ��Ʈv1.0.gsm");
const GS::uchar_t*	gsmTIE = L("��ü Ÿ�� v1.0.gsm");
const GS::uchar_t*	gsmCLAM = L("����Ŭ����v1.0.gsm");
const GS::uchar_t*	gsmPUSH = L("RS Push-Pull Props ����ǽ� v2.0 (�ξ�� ����).gsm");

static GS::Array<API_Guid>	elemList;	// �׷�ȭ�� ���� ������ ��������� GUID�� ���� ������

// ���̾�α� ���� ��� �ε��� ��ȣ ����
static short	EDITCONTROL_REMAIN_WIDTH;
static short	POPUP_WIDTH [50];


// ���� ���̺����� ��ġ�ϴ� ���� ��ƾ
GSErrCode	placeTableformOnWall (void)
{
	GSErrCode	err = NoError;
	short		result;
	long		nSel;
	short		xx;
	double		dx, dy;
	double		width;
	double		halfWidth;
	short		tableColumn;

	// Selection Manager ���� ����
	API_SelectionInfo		selectionInfo;
	API_Element				tElem;
	API_Neig				**selNeigs;
	GS::Array<API_Guid>&	walls = GS::Array<API_Guid> ();
	GS::Array<API_Guid>&	morphs = GS::Array<API_Guid> ();
	long					nWalls = 0;
	long					nMorphs = 0;

	// ��ü ���� ��������
	API_Element				elem;
	API_ElementMemo			memo;
	API_ElemInfo3D			info3D;

	// ���� ��ü ����
	InfoMorphForWallTableform	infoMorph;

	// �۾� �� ����
	API_StoryInfo	storyInfo;
	double			workLevel_wall;		// ���� �۾� �� ����


	// ������ ��� ��������
	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err == APIERR_NOPLAN) {
		ACAPI_WriteReport ("���� ������Ʈ â�� �����ϴ�.", true);
	}
	if (err == APIERR_NOSEL) {
		ACAPI_WriteReport ("�ƹ� �͵� �������� �ʾҽ��ϴ�.\n�ʼ� ����: �� (1��), ���� ���� ���� (1��)", true);
	}
	if (err != NoError) {
		BMKillHandle ((GSHandle *) &selNeigs);
		return err;
	}

	// �� 1��, ���� 1�� �����ؾ� ��
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
		}
	}
	BMKillHandle ((GSHandle *) &selNeigs);
	nWalls = walls.GetSize ();
	nMorphs = morphs.GetSize ();

	// ���� 1���ΰ�?
	if (nWalls != 1) {
		ACAPI_WriteReport ("���� 1�� �����ؾ� �մϴ�.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// ������ 1���ΰ�?
	if (nMorphs != 1) {
		ACAPI_WriteReport ("���� ���� ������ 1�� �����ϼž� �մϴ�.", true);
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
	placingZone.leftBottomX		= infoMorph.leftBottomX;
	placingZone.leftBottomY		= infoMorph.leftBottomY;
	placingZone.leftBottomZ		= infoMorph.leftBottomZ;
	placingZone.horLen			= infoMorph.horLen;
	placingZone.verLen			= infoMorph.verLen;
	placingZone.ang				= DegreeToRad (infoMorph.ang);
	
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
	placingZone.leftBottomZ = infoWall.bottomOffset;

	// ���� ���� ����
	API_Elem_Head* headList = new API_Elem_Head [1];
	headList [0] = elem.header;
	err = ACAPI_Element_Delete (&headList, 1);
	delete headList;

	// ���̺��� ���� �ʱ�ȭ
	placingZone.n1800w = 0;
	placingZone.n1850w = 0;
	placingZone.n1900w = 0;
	placingZone.n1950w = 0;
	placingZone.n2000w = 0;
	placingZone.n2050w = 0;
	placingZone.n2100w = 0;
	placingZone.n2150w = 0;
	placingZone.n2200w = 0;
	placingZone.n2250w = 0;
	placingZone.n2300w = 0;

	// ���̺��� ���� ���
	tableColumn = 0;
	width = placingZone.horLen;

	while (width > EPS) {
		if (width + EPS >= 4.600) {
			width -= 2.300;		placingZone.n2300w ++;	tableColumn ++;
		} else if ((width + EPS > 2.300) && (width + EPS < 4.600)) {
			halfWidth = width / 2;
			if (abs (halfWidth - 2.300) < EPS) {
				width -= 2.300 * 2;		placingZone.n2300w += 2;	tableColumn += 2;
			} else if (abs (halfWidth - 2.250) < EPS) {
				width -= 2.250 * 2;		placingZone.n2250w += 2;	tableColumn += 2;
			} else if (abs (halfWidth - 2.200) < EPS) {
				width -= 2.200 * 2;		placingZone.n2200w += 2;	tableColumn += 2;
			} else if (abs (halfWidth - 2.150) < EPS) {
				width -= 2.150 * 2;		placingZone.n2150w += 2;	tableColumn += 2;
			} else if (abs (halfWidth - 2.100) < EPS) {
				width -= 2.100 * 2;		placingZone.n2100w += 2;	tableColumn += 2;
			} else if (abs (halfWidth - 2.050) < EPS) {
				width -= 2.050 * 2;		placingZone.n2050w += 2;	tableColumn += 2;
			} else if (abs (halfWidth - 2.000) < EPS) {
				width -= 2.000 * 2;		placingZone.n2000w += 2;	tableColumn += 2;
			} else if (abs (halfWidth - 1.950) < EPS) {
				width -= 1.950 * 2;		placingZone.n1950w += 2;	tableColumn += 2;
			} else if (abs (halfWidth - 1.900) < EPS) {
				width -= 1.900 * 2;		placingZone.n1900w += 2;	tableColumn += 2;
			} else if (abs (halfWidth - 1.850) < EPS) {
				width -= 1.850 * 2;		placingZone.n1850w += 2;	tableColumn += 2;
			} else if (abs (halfWidth - 1.800) < EPS) {
				width -= 1.800 * 2;		placingZone.n1800w += 2;	tableColumn += 2;
			} else {
				width -= 2.300;			placingZone.n2300w ++;		tableColumn ++;
				width -= 1.800;			placingZone.n1800w ++;		tableColumn ++;
			}
		} else if (width + EPS <= 2.300) {
			if (width + EPS > 2.300) {
				width -= 2.300;		placingZone.n2300w ++;	tableColumn ++;
			} else if (width + EPS > 2.250) {
				width -= 2.250;		placingZone.n2250w ++;	tableColumn ++;
			} else if (width + EPS > 2.200) {
				width -= 2.200;		placingZone.n2200w ++;	tableColumn ++;
			} else if (width + EPS > 2.150) {
				width -= 2.150;		placingZone.n2150w ++;	tableColumn ++;
			} else if (width + EPS > 2.100) {
				width -= 2.100;		placingZone.n2100w ++;	tableColumn ++;
			} else if (width + EPS > 2.050) {
				width -= 2.050;		placingZone.n2050w ++;	tableColumn ++;
			} else if (width + EPS > 2.000) {
				width -= 2.000;		placingZone.n2000w ++;	tableColumn ++;
			} else if (width + EPS > 1.950) {
				width -= 1.950;		placingZone.n1950w ++;	tableColumn ++;
			} else if (width + EPS > 1.900) {
				width -= 1.900;		placingZone.n1900w ++;	tableColumn ++;
			} else if (width + EPS > 1.850) {
				width -= 1.850;		placingZone.n1850w ++;	tableColumn ++;
			} else if (width + EPS > 1.800) {
				width -= 1.800;		placingZone.n1800w ++;	tableColumn ++;
			} else {
				width -= 1.800;		placingZone.n1800w ++;	tableColumn ++;
			}
		}
	}

	// ���� ���� ����
	placingZone.remainWidth = width;

	// �� ���� ����
	placingZone.nCells = tableColumn;

	// [DIALOG] 1��° ���̾�α׿��� �� �ʺ� ������ ���̺��� ���� �� �� ���� �ʺ�/���̸� ������
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32517, ACAPI_GetOwnResModule (), wallTableformPlacerHandler, 0);

	// ������ �������� ���� ���� ������Ʈ
	infoWall.wallThk		+= (placingZone.gap * 2);

	if (result != DG_OK)
		return err;

	// �� ��ġ �� ���� ����
	initCellsForWallTableform (&placingZone);

	// ���̺��� ��ġ�ϱ�
	for (xx = 0 ; xx < placingZone.nCells ; ++xx)
		err = placeTableformOnWall (placingZone.cells [xx]);

	return	err;
}

// Cell �迭�� �ʱ�ȭ��
void	initCellsForWallTableform (WallTableformPlacingZone* placingZone)
{
	short	xx;

	for (xx = 0 ; xx < placingZone->nCells ; ++xx) {
		placingZone->cells [xx].ang = placingZone->ang;
		placingZone->cells [xx].leftBottomX = placingZone->leftBottomX + (placingZone->gap * sin(placingZone->ang)) + (getCellPositionLeftBottomXForWallTableForm (placingZone, xx) * cos(placingZone->ang));
		placingZone->cells [xx].leftBottomY = placingZone->leftBottomY - (placingZone->gap * cos(placingZone->ang)) + (getCellPositionLeftBottomXForWallTableForm (placingZone, xx) * sin(placingZone->ang));
		placingZone->cells [xx].leftBottomZ = placingZone->leftBottomZ;
	}
}

// ���̺��� ��ġ�ϱ�
GSErrCode	placeTableformOnWall (CellForWallTableform cell)
{
	GSErrCode	err = NoError;
	placementInfoForWallTableform	placementInfo;

	short		xx, yy;
	double		width, height;
	double		remainder;		// fmod �Լ��� �� ����
	double		elev_headpiece;

	paramsUFOM_ForWallTableform		params_UFOM;
	paramsSPIP_ForWallTableform		params_SPIP;
	paramsPINB_ForWallTableform		params_PINB;
	paramsTIE_ForWallTableform		params_TIE;
	paramsCLAM_ForWallTableform		params_CLAM;
	paramsPUSH_ForWallTableform		params_PUSH;

	if (abs (cell.horLen - 2.300) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.600;	placementInfo.width [2] = 0.500;	placementInfo.width [3] = 0.600;
	} else if (abs (cell.horLen - 2.250) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.600;	placementInfo.width [2] = 0.450;	placementInfo.width [3] = 0.600;
	} else if (abs (cell.horLen - 2.200) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.600;	placementInfo.width [2] = 0.400;	placementInfo.width [3] = 0.600;
	} else if (abs (cell.horLen - 2.150) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.500;	placementInfo.width [2] = 0.450;	placementInfo.width [3] = 0.600;
	} else if (abs (cell.horLen - 2.100) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.600;	placementInfo.width [2] = 0.300;	placementInfo.width [3] = 0.600;
	} else if (abs (cell.horLen - 2.050) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.450;	placementInfo.width [2] = 0.400;	placementInfo.width [3] = 0.600;
	} else if (abs (cell.horLen - 2.000) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.600;	placementInfo.width [2] = 0.200;	placementInfo.width [3] = 0.600;
	} else if (abs (cell.horLen - 1.950) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.450;	placementInfo.width [2] = 0.300;	placementInfo.width [3] = 0.600;
	} else if (abs (cell.horLen - 1.900) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.500;	placementInfo.width [2] = 0.200;	placementInfo.width [3] = 0.600;
	} else if (abs (cell.horLen - 1.850) < EPS) {
		placementInfo.nHorEuroform = 4;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.450;	placementInfo.width [2] = 0.200;	placementInfo.width [3] = 0.600;
	} else if (abs (cell.horLen - 1.800) < EPS) {
		placementInfo.nHorEuroform = 3;
		placementInfo.width [0] = 0.600;	placementInfo.width [1] = 0.600;	placementInfo.width [2] = 0.600;
	} else {
		placementInfo.nHorEuroform = 0;
	}

	if (abs (cell.verLen - 6.000) < EPS) {
		placementInfo.nVerEuroform = 5;
		placementInfo.height [4] = 1.200;
		placementInfo.height [3] = 1.200;
		placementInfo.height [2] = 1.200;
		placementInfo.height [1] = 1.200;
		placementInfo.height [0] = 1.200;
	} else if (abs (cell.verLen - 5.700) < EPS) {
		placementInfo.nVerEuroform = 5;
		placementInfo.height [4] = 0.900;
		placementInfo.height [3] = 1.200;
		placementInfo.height [2] = 1.200;
		placementInfo.height [1] = 1.200;
		placementInfo.height [0] = 1.200;
	} else if (abs (cell.verLen - 5.400) < EPS) {
		placementInfo.nVerEuroform = 5;
		placementInfo.height [4] = 0.900;
		placementInfo.height [3] = 0.900;
		placementInfo.height [2] = 1.200;
		placementInfo.height [1] = 1.200;
		placementInfo.height [0] = 1.200;
	} else if (abs (cell.verLen - 5.100) < EPS) {
		placementInfo.nVerEuroform = 5;
		placementInfo.height [4] = 0.600;
		placementInfo.height [3] = 0.900;
		placementInfo.height [2] = 1.200;
		placementInfo.height [1] = 1.200;
		placementInfo.height [0] = 1.200;
	} else if (abs (cell.verLen - 4.800) < EPS) {
		placementInfo.nVerEuroform = 4;
		placementInfo.height [3] = 1.200;
		placementInfo.height [2] = 1.200;
		placementInfo.height [1] = 1.200;
		placementInfo.height [0] = 1.200;
	} else if (abs (cell.verLen - 4.500) < EPS) {
		placementInfo.nVerEuroform = 4;
		placementInfo.height [3] = 0.900;
		placementInfo.height [2] = 1.200;
		placementInfo.height [1] = 1.200;
		placementInfo.height [0] = 1.200;
	} else if (abs (cell.verLen - 4.200) < EPS) {
		placementInfo.nVerEuroform = 4;
		placementInfo.height [3] = 0.900;
		placementInfo.height [2] = 0.900;
		placementInfo.height [1] = 1.200;
		placementInfo.height [0] = 1.200;
	} else if (abs (cell.verLen - 3.900) < EPS) {
		placementInfo.nVerEuroform = 4;
		placementInfo.height [3] = 0.600;
		placementInfo.height [2] = 0.900;
		placementInfo.height [1] = 1.200;
		placementInfo.height [0] = 1.200;
	} else if (abs (cell.verLen - 3.600) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [2] = 1.200;
		placementInfo.height [1] = 1.200;
		placementInfo.height [0] = 1.200;
	} else if (abs (cell.verLen - 3.300) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [2] = 0.900;
		placementInfo.height [1] = 1.200;
		placementInfo.height [0] = 1.200;
	} else if (abs (cell.verLen - 3.000) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [2] = 0.600;
		placementInfo.height [1] = 1.200;
		placementInfo.height [0] = 1.200;
	} else if (abs (cell.verLen - 2.700) < EPS) {
		placementInfo.nVerEuroform = 3;
		placementInfo.height [2] = 0.600;
		placementInfo.height [1] = 0.900;
		placementInfo.height [0] = 1.200;
	} else if (abs (cell.verLen - 2.400) < EPS) {
		placementInfo.nVerEuroform = 2;
		placementInfo.height [1] = 1.200;
		placementInfo.height [0] = 1.200;
	} else if (abs (cell.verLen - 2.100) < EPS) {
		placementInfo.nVerEuroform = 2;
		placementInfo.height [1] = 0.900;
		placementInfo.height [0] = 1.200;
	} else if (abs (cell.verLen - 1.800) < EPS) {
		placementInfo.nVerEuroform = 2;
		placementInfo.height [1] = 0.900;
		placementInfo.height [0] = 0.900;
	} else if (abs (cell.verLen - 1.500) < EPS) {
		placementInfo.nVerEuroform = 2;
		placementInfo.height [1] = 0.600;
		placementInfo.height [0] = 0.900;
	} else {
		placementInfo.nVerEuroform = 0;
	}

	// �ʺ� ���̰� 0�̸� �ƹ��͵� ��ġ���� ����
	if ((placementInfo.nHorEuroform == 0) || (placementInfo.nVerEuroform == 0))
		return	NoError;

	//////////////////////////////// �����
	// ������ ��ġ
	params_UFOM.leftBottomX = cell.leftBottomX;
	params_UFOM.leftBottomY = cell.leftBottomY;
	params_UFOM.leftBottomZ = cell.leftBottomZ;
	params_UFOM.ang = cell.ang;

	for (xx = 0 ; xx < placementInfo.nHorEuroform ; ++xx) {
		height = 0.0;
		for (yy = 0 ; yy < placementInfo.nVerEuroform ; ++yy) {
			params_UFOM.width	= placementInfo.width [xx];
			params_UFOM.height	= placementInfo.height [yy];
			height += placementInfo.height [yy];
			elemList.Push (placeUFOM (params_UFOM));	params_UFOM.leftBottomZ = moveZ (params_UFOM.leftBottomZ, placementInfo.height [yy]);
		}
		params_UFOM.leftBottomX = moveXinParallel (params_UFOM.leftBottomX, params_UFOM.ang, placementInfo.width [xx]);
		params_UFOM.leftBottomY = moveYinParallel (params_UFOM.leftBottomY, params_UFOM.ang, placementInfo.width [xx]);
		params_UFOM.leftBottomZ = moveZ (params_UFOM.leftBottomZ, -height);
	}

	// ��� ������ (����) ��ġ
	params_SPIP.leftBottomX = cell.leftBottomX;
	params_SPIP.leftBottomY = cell.leftBottomY;
	params_SPIP.leftBottomZ = cell.leftBottomZ;
	params_SPIP.ang = cell.ang;
	params_SPIP.length = cell.horLen - 0.100;
	params_SPIP.pipeAng = DegreeToRad (0);

	params_SPIP.leftBottomX = moveXinPerpend (params_SPIP.leftBottomX, params_SPIP.ang, -(0.0635 + 0.025));
	params_SPIP.leftBottomY = moveYinPerpend (params_SPIP.leftBottomY, params_SPIP.ang, -(0.0635 + 0.025));
	params_SPIP.leftBottomX = moveXinParallel (params_SPIP.leftBottomX, params_SPIP.ang, 0.050);
	params_SPIP.leftBottomY = moveYinParallel (params_SPIP.leftBottomY, params_SPIP.ang, 0.050);
	params_SPIP.leftBottomZ = moveZ (params_SPIP.leftBottomZ, 0.150 - 0.031);

	for (xx = 0 ; xx <= placementInfo.nVerEuroform ; ++xx) {
		if (xx == 0) {
			// 1��
			elemList.Push (placeSPIP (params_SPIP));
			params_SPIP.leftBottomZ = moveZ (params_SPIP.leftBottomZ, 0.062);
			elemList.Push (placeSPIP (params_SPIP));
			params_SPIP.leftBottomZ = moveZ (params_SPIP.leftBottomZ, -0.031 - 0.150 + placementInfo.height [xx] - 0.031);
		} else if (xx == placementInfo.nVerEuroform) {
			// ������ ��
			params_SPIP.leftBottomZ = moveZ (params_SPIP.leftBottomZ, -0.150);
			elemList.Push (placeSPIP (params_SPIP));
			params_SPIP.leftBottomZ = moveZ (params_SPIP.leftBottomZ, 0.062);
			elemList.Push (placeSPIP (params_SPIP));
		} else {
			// ������ ��
			elemList.Push (placeSPIP (params_SPIP));
			params_SPIP.leftBottomZ = moveZ (params_SPIP.leftBottomZ, 0.062);
			elemList.Push (placeSPIP (params_SPIP));
			params_SPIP.leftBottomZ = moveZ (params_SPIP.leftBottomZ, -0.031 + placementInfo.height [xx] - 0.031);
		}
	}

	// ��� ������ (����) ��ġ
	params_SPIP.leftBottomX = cell.leftBottomX;
	params_SPIP.leftBottomY = cell.leftBottomY;
	params_SPIP.leftBottomZ = cell.leftBottomZ;
	params_SPIP.ang = cell.ang;
	params_SPIP.length = cell.verLen - 0.100;
	params_SPIP.pipeAng = DegreeToRad (90);

	params_SPIP.leftBottomX = moveXinPerpend (params_SPIP.leftBottomX, params_SPIP.ang, -(0.0635 + 0.075));
	params_SPIP.leftBottomY = moveYinPerpend (params_SPIP.leftBottomY, params_SPIP.ang, -(0.0635 + 0.075));
	params_SPIP.leftBottomX = moveXinParallel (params_SPIP.leftBottomX, params_SPIP.ang, 0.450 - 0.031);
	params_SPIP.leftBottomY = moveYinParallel (params_SPIP.leftBottomY, params_SPIP.ang, 0.450 - 0.031);
	params_SPIP.leftBottomZ = moveZ (params_SPIP.leftBottomZ, 0.050);

	// 1��
	elemList.Push (placeSPIP (params_SPIP));	params_SPIP.leftBottomX = moveXinParallel (params_SPIP.leftBottomX, params_SPIP.ang, 0.062);						params_SPIP.leftBottomY = moveYinParallel (params_SPIP.leftBottomY, params_SPIP.ang, 0.062);
	elemList.Push (placeSPIP (params_SPIP));	params_SPIP.leftBottomX = moveXinParallel (params_SPIP.leftBottomX, params_SPIP.ang, cell.horLen - 0.900 - 0.062);	params_SPIP.leftBottomY = moveYinParallel (params_SPIP.leftBottomY, params_SPIP.ang, cell.horLen - 0.900 - 0.062);
	// 2��
	elemList.Push (placeSPIP (params_SPIP));	params_SPIP.leftBottomX = moveXinParallel (params_SPIP.leftBottomX, params_SPIP.ang, 0.062);						params_SPIP.leftBottomY = moveYinParallel (params_SPIP.leftBottomY, params_SPIP.ang, 0.062);
	elemList.Push (placeSPIP (params_SPIP));

	// �ɺ�Ʈ ��ġ (���� - ���ϴ�, �ֻ��)
	params_PINB.leftBottomX = cell.leftBottomX;
	params_PINB.leftBottomY = cell.leftBottomY;
	params_PINB.leftBottomZ = cell.leftBottomZ;
	params_PINB.ang = cell.ang;
	params_PINB.bPinBoltRot90 = TRUE;
	params_PINB.boltLen = 0.100;
	params_PINB.angX = DegreeToRad (270.0);
	params_PINB.angY = DegreeToRad (0.0);

	params_PINB.leftBottomX = moveXinPerpend (params_PINB.leftBottomX, params_PINB.ang, -(0.1635));
	params_PINB.leftBottomY = moveYinPerpend (params_PINB.leftBottomY, params_PINB.ang, -(0.1635));

	// ���ϴ� ��
	params_PINB.leftBottomZ = moveZ (params_PINB.leftBottomZ, 0.150);
	width = 0.0;
	for (xx = 0 ; xx < placementInfo.nHorEuroform - 1 ; ++xx) {
		width += placementInfo.width [xx];
		params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, placementInfo.width [xx]);
		params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, placementInfo.width [xx]);

		elemList.Push (placePINB (params_PINB));
	}
	// �ֻ�� ��
	params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, -width);
	params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, -width);
	params_PINB.leftBottomZ = moveZ (params_PINB.leftBottomZ, cell.verLen - 0.300);
	for (xx = 0 ; xx < placementInfo.nHorEuroform - 1 ; ++xx) {
		params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, placementInfo.width [xx]);
		params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, placementInfo.width [xx]);

		elemList.Push (placePINB (params_PINB));
	}

	// �ɺ�Ʈ ��ġ (���� - ������)
	params_PINB.leftBottomX = cell.leftBottomX;
	params_PINB.leftBottomY = cell.leftBottomY;
	params_PINB.leftBottomZ = cell.leftBottomZ;
	params_PINB.ang = cell.ang;
	params_PINB.bPinBoltRot90 = FALSE;
	params_PINB.boltLen = 0.100;
	params_PINB.angX = DegreeToRad (270.0);
	params_PINB.angY = DegreeToRad (0.0);

	params_PINB.leftBottomX = moveXinPerpend (params_PINB.leftBottomX, params_PINB.ang, -(0.1635));
	params_PINB.leftBottomY = moveYinPerpend (params_PINB.leftBottomY, params_PINB.ang, -(0.1635));

	// 2 ~ [n-1]��
	params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
	params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
	params_PINB.leftBottomZ = moveZ (params_PINB.leftBottomZ, placementInfo.height [0]);
	for (xx = 1 ; xx < placementInfo.nVerEuroform ; ++xx) {
		width = 0.0;
		for (yy = 0 ; yy < placementInfo.nHorEuroform ; ++yy) {
			// 1��
			if (yy == 0) {
				elemList.Push (placePINB (params_PINB));
				params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.450);
				params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.450);
				width += 0.450;
			// ������ ��
			} else if (yy == placementInfo.nHorEuroform - 1) {
				width += 0.450;
				params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.450);
				params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.450);
				elemList.Push (placePINB (params_PINB));
			// ������ ��
			} else {
				width += placementInfo.width [yy];
				if (abs (placementInfo.width [yy] - 0.600) < EPS) {
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
					elemList.Push (placePINB (params_PINB));
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.300);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.300);
					elemList.Push (placePINB (params_PINB));
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
				} else if (abs (placementInfo.width [yy] - 0.500) < EPS) {
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
					elemList.Push (placePINB (params_PINB));
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.200);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.200);
					elemList.Push (placePINB (params_PINB));
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
				} else if (abs (placementInfo.width [yy] - 0.450) < EPS) {
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
					elemList.Push (placePINB (params_PINB));
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
					elemList.Push (placePINB (params_PINB));
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
				} else if (abs (placementInfo.width [yy] - 0.400) < EPS) {
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.100);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.100);
					elemList.Push (placePINB (params_PINB));
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.200);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.200);
					elemList.Push (placePINB (params_PINB));
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.100);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.100);
				} else if (abs (placementInfo.width [yy] - 0.300) < EPS) {
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
					elemList.Push (placePINB (params_PINB));
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
				} else if (abs (placementInfo.width [yy] - 0.200) < EPS) {
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
					elemList.Push (placePINB (params_PINB));
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.050);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.050);
				}
			}
		}
		params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, -width);
		params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, -width);
		params_PINB.leftBottomZ = moveZ (params_PINB.leftBottomZ, placementInfo.height [xx]);
	}

	// �ɺ�Ʈ ��ġ (����)
	params_PINB.leftBottomX = cell.leftBottomX;
	params_PINB.leftBottomY = cell.leftBottomY;
	params_PINB.leftBottomZ = cell.leftBottomZ;
	params_PINB.ang = cell.ang;
	params_PINB.bPinBoltRot90 = FALSE;
	params_PINB.boltLen = 0.150;
	params_PINB.angX = DegreeToRad (270.0);
	params_PINB.angY = DegreeToRad (0.0);

	params_PINB.leftBottomX = moveXinPerpend (params_PINB.leftBottomX, params_PINB.ang, -(0.2135));
	params_PINB.leftBottomY = moveYinPerpend (params_PINB.leftBottomY, params_PINB.ang, -(0.2135));
	params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.450);
	params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.450);
	params_PINB.leftBottomZ = moveZ (params_PINB.leftBottomZ, placementInfo.height [0]);

	// 1��
	height = 0.0;
	for (xx = 1 ; xx < placementInfo.nVerEuroform ; ++xx) {
		elemList.Push (placePINB (params_PINB));
		params_PINB.leftBottomZ = moveZ (params_PINB.leftBottomZ, placementInfo.height [xx]);
		height += placementInfo.height [xx];
	}
	// 2��
	params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, cell.horLen - 0.900);
	params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, cell.horLen - 0.900);
	params_PINB.leftBottomZ = moveZ (params_PINB.leftBottomZ, -height);
	for (xx = 1 ; xx < placementInfo.nVerEuroform ; ++xx) {
		elemList.Push (placePINB (params_PINB));
		params_PINB.leftBottomZ = moveZ (params_PINB.leftBottomZ, placementInfo.height [xx]);
		height += placementInfo.height [xx];
	}

	// ��ü Ÿ��
	params_TIE.leftBottomX = cell.leftBottomX;
	params_TIE.leftBottomY = cell.leftBottomY;
	params_TIE.leftBottomZ = cell.leftBottomZ;
	params_TIE.ang = cell.ang;
	remainder = fmod ((infoWall.wallThk + 0.327), 0.100);
	params_TIE.boltLen = (infoWall.wallThk + 0.327 + (0.100 - remainder));
	params_TIE.pipeBeg = 0.0365 + 0.1635;
	params_TIE.pipeEnd = 0.0365 + 0.1635 + infoWall.wallThk;
	params_TIE.clampBeg = 0.0365;
	params_TIE.clampEnd = 0.0365 + infoWall.wallThk + 0.327;

	params_TIE.leftBottomX = moveXinPerpend (params_TIE.leftBottomX, params_TIE.ang, -(0.1635 + 0.0365));
	params_TIE.leftBottomY = moveYinPerpend (params_TIE.leftBottomY, params_TIE.ang, -(0.1635 + 0.0365));
	params_TIE.leftBottomX = moveXinParallel (params_TIE.leftBottomX, params_TIE.ang, 0.450);
	params_TIE.leftBottomY = moveYinParallel (params_TIE.leftBottomY, params_TIE.ang, 0.450);
	params_TIE.leftBottomZ = moveZ (params_TIE.leftBottomZ, 0.220);

	for (xx = 0 ; xx < 2 ; ++xx) {
		for (yy = 0 ; yy < placementInfo.nVerEuroform ; ++yy) {
			// ������ ��
			if (yy == 0) {
				elemList.Push (placeTIE (params_TIE));
				params_TIE.leftBottomZ = moveZ (params_TIE.leftBottomZ, -0.220 + placementInfo.height [yy] + 0.150);
		
			// �ֻ��� ��
			} else if (yy == placementInfo.nVerEuroform - 1) {
				params_TIE.leftBottomZ = moveZ (params_TIE.leftBottomZ, placementInfo.height [yy] - 0.150 - 0.230);
				elemList.Push (placeTIE (params_TIE));
				params_TIE.leftBottomX = moveXinParallel (params_TIE.leftBottomX, params_TIE.ang, cell.horLen - 0.900);
				params_TIE.leftBottomY = moveYinParallel (params_TIE.leftBottomY, params_TIE.ang, cell.horLen - 0.900);
				params_TIE.leftBottomZ = moveZ (params_TIE.leftBottomZ, 0.230 - cell.verLen + 0.220);
		
			// 2 ~ [n-1]��
			} else {
				elemList.Push (placeTIE (params_TIE));
				params_TIE.leftBottomZ = moveZ (params_TIE.leftBottomZ, placementInfo.height [yy]);
			}
		}
	}

	// ���� Ŭ����
	params_CLAM.leftBottomX = cell.leftBottomX;
	params_CLAM.leftBottomY = cell.leftBottomY;
	params_CLAM.leftBottomZ = cell.leftBottomZ;
	params_CLAM.ang = cell.ang;
	params_CLAM.angX = DegreeToRad (0.0);
	params_CLAM.angY = DegreeToRad (0.0);

	params_CLAM.leftBottomX = moveXinPerpend (params_CLAM.leftBottomX, params_CLAM.ang, -0.1835);
	params_CLAM.leftBottomY = moveYinPerpend (params_CLAM.leftBottomY, params_CLAM.ang, -0.1835);
	params_CLAM.leftBottomX = moveXinParallel (params_CLAM.leftBottomX, params_CLAM.ang, 0.450 - 0.031);
	params_CLAM.leftBottomY = moveYinParallel (params_CLAM.leftBottomY, params_CLAM.ang, 0.450 - 0.031);
	params_CLAM.leftBottomZ = moveZ (params_CLAM.leftBottomZ, 0.099);

	for (xx = 0 ; xx < 2 ; ++xx) {
		elemList.Push (placeCLAM (params_CLAM));
		params_CLAM.leftBottomX = moveXinParallel (params_CLAM.leftBottomX, params_CLAM.ang, 0.062);
		params_CLAM.leftBottomY = moveYinParallel (params_CLAM.leftBottomY, params_CLAM.ang, 0.062);
		elemList.Push (placeCLAM (params_CLAM));
		params_CLAM.leftBottomX = moveXinParallel (params_CLAM.leftBottomX, params_CLAM.ang, -0.031 - 0.450 + cell.horLen - 0.450 - 0.031);
		params_CLAM.leftBottomY = moveYinParallel (params_CLAM.leftBottomY, params_CLAM.ang, -0.031 - 0.450 + cell.horLen - 0.450 - 0.031);
		elemList.Push (placeCLAM (params_CLAM));
		params_CLAM.leftBottomX = moveXinParallel (params_CLAM.leftBottomX, params_CLAM.ang, 0.062);
		params_CLAM.leftBottomY = moveYinParallel (params_CLAM.leftBottomY, params_CLAM.ang, 0.062);
		elemList.Push (placeCLAM (params_CLAM));
		params_CLAM.leftBottomX = moveXinParallel (params_CLAM.leftBottomX, params_CLAM.ang, -0.031 + 0.450 - cell.horLen + 0.450 - 0.031);
		params_CLAM.leftBottomY = moveYinParallel (params_CLAM.leftBottomY, params_CLAM.ang, -0.031 + 0.450 - cell.horLen + 0.450 - 0.031);
		params_CLAM.leftBottomZ = moveZ (params_CLAM.leftBottomZ, -0.099 + cell.verLen - 0.099);	params_CLAM.angY = DegreeToRad (180.0);
	}

	// ��� �ǽ�
	params_PUSH.leftBottomX = cell.leftBottomX;
	params_PUSH.leftBottomY = cell.leftBottomY;
	params_PUSH.leftBottomZ = cell.leftBottomZ;
	params_PUSH.ang = cell.ang;

	params_PUSH.leftBottomX = moveXinPerpend (params_PUSH.leftBottomX, params_CLAM.ang, -0.1725);
	params_PUSH.leftBottomY = moveYinPerpend (params_PUSH.leftBottomY, params_CLAM.ang, -0.1725);
	params_PUSH.leftBottomX = moveXinParallel (params_PUSH.leftBottomX, params_CLAM.ang, 0.450 - 0.100);
	params_PUSH.leftBottomY = moveYinParallel (params_PUSH.leftBottomY, params_CLAM.ang, 0.450 - 0.100);
	params_PUSH.leftBottomZ = moveZ (params_PUSH.leftBottomZ, 0.300);

	// ó�� ��
	elemList.Push (placePUSH (params_PUSH));
	params_PUSH.leftBottomX = moveXinParallel (params_PUSH.leftBottomX, params_PUSH.ang, cell.horLen - 0.900);
	params_PUSH.leftBottomY = moveYinParallel (params_PUSH.leftBottomY, params_PUSH.ang, cell.horLen - 0.900);
	elemList.Push (placePUSH (params_PUSH));
	params_PUSH.leftBottomX = moveXinParallel (params_PUSH.leftBottomX, params_PUSH.ang, 0.900 - cell.horLen);
	params_PUSH.leftBottomY = moveYinParallel (params_PUSH.leftBottomY, params_PUSH.ang, 0.900 - cell.horLen);
	if (cell.verLen > 4.000) {
		elev_headpiece = 4.000 * 0.80;
	} else {
		elev_headpiece = cell.verLen * 0.80;
	}
	params_PUSH.leftBottomZ = moveZ (params_PUSH.leftBottomZ, -0.300 + elev_headpiece);
	// ������ ��
	elemList.Push (placePUSH (params_PUSH));
	params_PUSH.leftBottomX = moveXinParallel (params_PUSH.leftBottomX, params_PUSH.ang, cell.horLen - 0.900);
	params_PUSH.leftBottomY = moveYinParallel (params_PUSH.leftBottomY, params_PUSH.ang, cell.horLen - 0.900);
	elemList.Push (placePUSH (params_PUSH));

	//////////////////////////////// �ݴ��
	cell.leftBottomX = moveXinParallel (cell.leftBottomX, cell.ang, cell.horLen);
	cell.leftBottomY = moveYinParallel (cell.leftBottomY, cell.ang, cell.horLen);
	cell.leftBottomX = moveXinPerpend (cell.leftBottomX, cell.ang, infoWall.wallThk);
	cell.leftBottomY = moveYinPerpend (cell.leftBottomY, cell.ang, infoWall.wallThk);
	cell.ang += DegreeToRad (180.0);

	// ������ ��ġ (�ݴ����� �����)
	params_UFOM.leftBottomX = cell.leftBottomX;
	params_UFOM.leftBottomY = cell.leftBottomY;
	params_UFOM.leftBottomZ = cell.leftBottomZ;
	params_UFOM.ang = cell.ang;

	for (xx = placementInfo.nHorEuroform - 1 ; xx >= 0 ; --xx) {
		height = 0.0;
		for (yy = 0 ; yy < placementInfo.nVerEuroform ; ++yy) {
			params_UFOM.width	= placementInfo.width [xx];
			params_UFOM.height	= placementInfo.height [yy];
			height += placementInfo.height [yy];
			elemList.Push (placeUFOM (params_UFOM));	params_UFOM.leftBottomZ = moveZ (params_UFOM.leftBottomZ, placementInfo.height [yy]);
		}
		params_UFOM.leftBottomX = moveXinParallel (params_UFOM.leftBottomX, params_UFOM.ang, placementInfo.width [xx]);
		params_UFOM.leftBottomY = moveYinParallel (params_UFOM.leftBottomY, params_UFOM.ang, placementInfo.width [xx]);
		params_UFOM.leftBottomZ = moveZ (params_UFOM.leftBottomZ, -height);
	}

	// ��� ������ (����) ��ġ
	params_SPIP.leftBottomX = cell.leftBottomX;
	params_SPIP.leftBottomY = cell.leftBottomY;
	params_SPIP.leftBottomZ = cell.leftBottomZ;
	params_SPIP.ang = cell.ang;
	params_SPIP.length = cell.horLen - 0.100;
	params_SPIP.pipeAng = DegreeToRad (0);

	params_SPIP.leftBottomX = moveXinPerpend (params_SPIP.leftBottomX, params_SPIP.ang, -(0.0635 + 0.025));
	params_SPIP.leftBottomY = moveYinPerpend (params_SPIP.leftBottomY, params_SPIP.ang, -(0.0635 + 0.025));
	params_SPIP.leftBottomX = moveXinParallel (params_SPIP.leftBottomX, params_SPIP.ang, 0.050);
	params_SPIP.leftBottomY = moveYinParallel (params_SPIP.leftBottomY, params_SPIP.ang, 0.050);
	params_SPIP.leftBottomZ = moveZ (params_SPIP.leftBottomZ, 0.150 - 0.031);

	for (xx = 0 ; xx <= placementInfo.nVerEuroform ; ++xx) {
		if (xx == 0) {
			// 1��
			elemList.Push (placeSPIP (params_SPIP));
			params_SPIP.leftBottomZ = moveZ (params_SPIP.leftBottomZ, 0.062);
			elemList.Push (placeSPIP (params_SPIP));
			params_SPIP.leftBottomZ = moveZ (params_SPIP.leftBottomZ, -0.031 - 0.150 + placementInfo.height [xx] - 0.031);
		} else if (xx == placementInfo.nVerEuroform) {
			// ������ ��
			params_SPIP.leftBottomZ = moveZ (params_SPIP.leftBottomZ, -0.150);
			elemList.Push (placeSPIP (params_SPIP));
			params_SPIP.leftBottomZ = moveZ (params_SPIP.leftBottomZ, 0.062);
			elemList.Push (placeSPIP (params_SPIP));
		} else {
			// ������ ��
			elemList.Push (placeSPIP (params_SPIP));
			params_SPIP.leftBottomZ = moveZ (params_SPIP.leftBottomZ, 0.062);
			elemList.Push (placeSPIP (params_SPIP));
			params_SPIP.leftBottomZ = moveZ (params_SPIP.leftBottomZ, -0.031 + placementInfo.height [xx] - 0.031);
		}
	}

	// ��� ������ (����) ��ġ
	params_SPIP.leftBottomX = cell.leftBottomX;
	params_SPIP.leftBottomY = cell.leftBottomY;
	params_SPIP.leftBottomZ = cell.leftBottomZ;
	params_SPIP.ang = cell.ang;
	params_SPIP.length = cell.verLen - 0.100;
	params_SPIP.pipeAng = DegreeToRad (90);

	params_SPIP.leftBottomX = moveXinPerpend (params_SPIP.leftBottomX, params_SPIP.ang, -(0.0635 + 0.075));
	params_SPIP.leftBottomY = moveYinPerpend (params_SPIP.leftBottomY, params_SPIP.ang, -(0.0635 + 0.075));
	params_SPIP.leftBottomX = moveXinParallel (params_SPIP.leftBottomX, params_SPIP.ang, 0.450 - 0.031);
	params_SPIP.leftBottomY = moveYinParallel (params_SPIP.leftBottomY, params_SPIP.ang, 0.450 - 0.031);
	params_SPIP.leftBottomZ = moveZ (params_SPIP.leftBottomZ, 0.050);

	// 1��
	elemList.Push (placeSPIP (params_SPIP));	params_SPIP.leftBottomX = moveXinParallel (params_SPIP.leftBottomX, params_SPIP.ang, 0.062);						params_SPIP.leftBottomY = moveYinParallel (params_SPIP.leftBottomY, params_SPIP.ang, 0.062);
	elemList.Push (placeSPIP (params_SPIP));	params_SPIP.leftBottomX = moveXinParallel (params_SPIP.leftBottomX, params_SPIP.ang, cell.horLen - 0.900 - 0.062);	params_SPIP.leftBottomY = moveYinParallel (params_SPIP.leftBottomY, params_SPIP.ang, cell.horLen - 0.900 - 0.062);
	// 2��
	elemList.Push (placeSPIP (params_SPIP));	params_SPIP.leftBottomX = moveXinParallel (params_SPIP.leftBottomX, params_SPIP.ang, 0.062);						params_SPIP.leftBottomY = moveYinParallel (params_SPIP.leftBottomY, params_SPIP.ang, 0.062);
	elemList.Push (placeSPIP (params_SPIP));

	// �ɺ�Ʈ ��ġ (���� - ���ϴ�, �ֻ��) (�ݴ����� �����)
	params_PINB.leftBottomX = cell.leftBottomX;
	params_PINB.leftBottomY = cell.leftBottomY;
	params_PINB.leftBottomZ = cell.leftBottomZ;
	params_PINB.ang = cell.ang;
	params_PINB.bPinBoltRot90 = TRUE;
	params_PINB.boltLen = 0.100;
	params_PINB.angX = DegreeToRad (270.0);
	params_PINB.angY = DegreeToRad (0.0);

	params_PINB.leftBottomX = moveXinPerpend (params_PINB.leftBottomX, params_PINB.ang, -(0.1635));
	params_PINB.leftBottomY = moveYinPerpend (params_PINB.leftBottomY, params_PINB.ang, -(0.1635));

	// ���ϴ� ��
	params_PINB.leftBottomZ = moveZ (params_PINB.leftBottomZ, 0.150);
	width = 0.0;
	for (xx = placementInfo.nHorEuroform - 1 ; xx > 0 ; --xx) {
		width += placementInfo.width [xx];
		params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, placementInfo.width [xx]);
		params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, placementInfo.width [xx]);

		elemList.Push (placePINB (params_PINB));
	}
	// �ֻ�� ��
	params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, -width);
	params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, -width);
	params_PINB.leftBottomZ = moveZ (params_PINB.leftBottomZ, cell.verLen - 0.300);
	for (xx = placementInfo.nHorEuroform - 1 ; xx > 0 ; --xx) {
		params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, placementInfo.width [xx]);
		params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, placementInfo.width [xx]);

		elemList.Push (placePINB (params_PINB));
	}

	// �ɺ�Ʈ ��ġ (���� - ������) (�ݴ����� �����)
	params_PINB.leftBottomX = cell.leftBottomX;
	params_PINB.leftBottomY = cell.leftBottomY;
	params_PINB.leftBottomZ = cell.leftBottomZ;
	params_PINB.ang = cell.ang;
	params_PINB.bPinBoltRot90 = FALSE;
	params_PINB.boltLen = 0.100;
	params_PINB.angX = DegreeToRad (270.0);
	params_PINB.angY = DegreeToRad (0.0);

	params_PINB.leftBottomX = moveXinPerpend (params_PINB.leftBottomX, params_PINB.ang, -(0.1635));
	params_PINB.leftBottomY = moveYinPerpend (params_PINB.leftBottomY, params_PINB.ang, -(0.1635));

	// 2 ~ [n-1]��
	params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
	params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
	params_PINB.leftBottomZ = moveZ (params_PINB.leftBottomZ, placementInfo.height [0]);
	for (xx = 1 ; xx < placementInfo.nVerEuroform ; ++xx) {
		width = 0.0;
		for (yy = placementInfo.nHorEuroform - 1 ; yy >= 0 ; --yy) {
			// 1��
			if (yy == placementInfo.nHorEuroform - 1) {
				elemList.Push (placePINB (params_PINB));
				params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.450);
				params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.450);
				width += 0.450;
			// ������ ��
			} else if (yy == 0) {
				width += 0.450;
				params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.450);
				params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.450);
				elemList.Push (placePINB (params_PINB));
			// ������ ��
			} else {
				width += placementInfo.width [yy];
				if (abs (placementInfo.width [yy] - 0.600) < EPS) {
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
					elemList.Push (placePINB (params_PINB));
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.300);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.300);
					elemList.Push (placePINB (params_PINB));
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
				} else if (abs (placementInfo.width [yy] - 0.500) < EPS) {
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
					elemList.Push (placePINB (params_PINB));
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.200);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.200);
					elemList.Push (placePINB (params_PINB));
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
				} else if (abs (placementInfo.width [yy] - 0.450) < EPS) {
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
					elemList.Push (placePINB (params_PINB));
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
					elemList.Push (placePINB (params_PINB));
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
				} else if (abs (placementInfo.width [yy] - 0.400) < EPS) {
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.100);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.100);
					elemList.Push (placePINB (params_PINB));
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.200);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.200);
					elemList.Push (placePINB (params_PINB));
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.100);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.100);
				} else if (abs (placementInfo.width [yy] - 0.300) < EPS) {
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
					elemList.Push (placePINB (params_PINB));
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
				} else if (abs (placementInfo.width [yy] - 0.200) < EPS) {
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.050);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.050);
					elemList.Push (placePINB (params_PINB));
					params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.150);
					params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.150);
				}
			}
		}
		params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, -width);
		params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, -width);
		params_PINB.leftBottomZ = moveZ (params_PINB.leftBottomZ, placementInfo.height [xx]);
	}

	// �ɺ�Ʈ ��ġ (����)
	params_PINB.leftBottomX = cell.leftBottomX;
	params_PINB.leftBottomY = cell.leftBottomY;
	params_PINB.leftBottomZ = cell.leftBottomZ;
	params_PINB.ang = cell.ang;
	params_PINB.bPinBoltRot90 = FALSE;
	params_PINB.boltLen = 0.150;
	params_PINB.angX = DegreeToRad (270.0);
	params_PINB.angY = DegreeToRad (0.0);

	params_PINB.leftBottomX = moveXinPerpend (params_PINB.leftBottomX, params_PINB.ang, -(0.2135));
	params_PINB.leftBottomY = moveYinPerpend (params_PINB.leftBottomY, params_PINB.ang, -(0.2135));
	params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, 0.450);
	params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, 0.450);
	params_PINB.leftBottomZ = moveZ (params_PINB.leftBottomZ, placementInfo.height [0]);

	// 1��
	height = 0.0;
	for (xx = 1 ; xx < placementInfo.nVerEuroform ; ++xx) {
		elemList.Push (placePINB (params_PINB));
		params_PINB.leftBottomZ = moveZ (params_PINB.leftBottomZ, placementInfo.height [xx]);
		height += placementInfo.height [xx];
	}
	// 2��
	params_PINB.leftBottomX = moveXinParallel (params_PINB.leftBottomX, params_PINB.ang, cell.horLen - 0.900);
	params_PINB.leftBottomY = moveYinParallel (params_PINB.leftBottomY, params_PINB.ang, cell.horLen - 0.900);
	params_PINB.leftBottomZ = moveZ (params_PINB.leftBottomZ, -height);
	for (xx = 1 ; xx < placementInfo.nVerEuroform ; ++xx) {
		elemList.Push (placePINB (params_PINB));
		params_PINB.leftBottomZ = moveZ (params_PINB.leftBottomZ, placementInfo.height [xx]);
		height += placementInfo.height [xx];
	}

	// ��ü Ÿ�� (����鿡�� �����Ƿ� ����)

	// ���� Ŭ����
	params_CLAM.leftBottomX = cell.leftBottomX;
	params_CLAM.leftBottomY = cell.leftBottomY;
	params_CLAM.leftBottomZ = cell.leftBottomZ;
	params_CLAM.ang = cell.ang;
	params_CLAM.angX = DegreeToRad (0.0);
	params_CLAM.angY = DegreeToRad (0.0);

	params_CLAM.leftBottomX = moveXinPerpend (params_CLAM.leftBottomX, params_CLAM.ang, -0.1835);
	params_CLAM.leftBottomY = moveYinPerpend (params_CLAM.leftBottomY, params_CLAM.ang, -0.1835);
	params_CLAM.leftBottomX = moveXinParallel (params_CLAM.leftBottomX, params_CLAM.ang, 0.450 - 0.031);
	params_CLAM.leftBottomY = moveYinParallel (params_CLAM.leftBottomY, params_CLAM.ang, 0.450 - 0.031);
	params_CLAM.leftBottomZ = moveZ (params_CLAM.leftBottomZ, 0.099);

	for (xx = 0 ; xx < 2 ; ++xx) {
		elemList.Push (placeCLAM (params_CLAM));
		params_CLAM.leftBottomX = moveXinParallel (params_CLAM.leftBottomX, params_CLAM.ang, 0.062);
		params_CLAM.leftBottomY = moveYinParallel (params_CLAM.leftBottomY, params_CLAM.ang, 0.062);
		elemList.Push (placeCLAM (params_CLAM));
		params_CLAM.leftBottomX = moveXinParallel (params_CLAM.leftBottomX, params_CLAM.ang, -0.031 - 0.450 + cell.horLen - 0.450 - 0.031);
		params_CLAM.leftBottomY = moveYinParallel (params_CLAM.leftBottomY, params_CLAM.ang, -0.031 - 0.450 + cell.horLen - 0.450 - 0.031);
		elemList.Push (placeCLAM (params_CLAM));
		params_CLAM.leftBottomX = moveXinParallel (params_CLAM.leftBottomX, params_CLAM.ang, 0.062);
		params_CLAM.leftBottomY = moveYinParallel (params_CLAM.leftBottomY, params_CLAM.ang, 0.062);
		elemList.Push (placeCLAM (params_CLAM));
		params_CLAM.leftBottomX = moveXinParallel (params_CLAM.leftBottomX, params_CLAM.ang, -0.031 + 0.450 - cell.horLen + 0.450 - 0.031);
		params_CLAM.leftBottomY = moveYinParallel (params_CLAM.leftBottomY, params_CLAM.ang, -0.031 + 0.450 - cell.horLen + 0.450 - 0.031);
		params_CLAM.leftBottomZ = moveZ (params_CLAM.leftBottomZ, -0.099 + cell.verLen - 0.099);	params_CLAM.angY = DegreeToRad (180.0);
	}

	// ��� �ǽ�
	params_PUSH.leftBottomX = cell.leftBottomX;
	params_PUSH.leftBottomY = cell.leftBottomY;
	params_PUSH.leftBottomZ = cell.leftBottomZ;
	params_PUSH.ang = cell.ang;

	params_PUSH.leftBottomX = moveXinPerpend (params_PUSH.leftBottomX, params_CLAM.ang, -0.1725);
	params_PUSH.leftBottomY = moveYinPerpend (params_PUSH.leftBottomY, params_CLAM.ang, -0.1725);
	params_PUSH.leftBottomX = moveXinParallel (params_PUSH.leftBottomX, params_CLAM.ang, 0.450 - 0.100);
	params_PUSH.leftBottomY = moveYinParallel (params_PUSH.leftBottomY, params_CLAM.ang, 0.450 - 0.100);
	params_PUSH.leftBottomZ = moveZ (params_PUSH.leftBottomZ, 0.300);

	// ó�� ��
	elemList.Push (placePUSH (params_PUSH));
	params_PUSH.leftBottomX = moveXinParallel (params_PUSH.leftBottomX, params_PUSH.ang, cell.horLen - 0.900);
	params_PUSH.leftBottomY = moveYinParallel (params_PUSH.leftBottomY, params_PUSH.ang, cell.horLen - 0.900);
	elemList.Push (placePUSH (params_PUSH));
	params_PUSH.leftBottomX = moveXinParallel (params_PUSH.leftBottomX, params_PUSH.ang, 0.900 - cell.horLen);
	params_PUSH.leftBottomY = moveYinParallel (params_PUSH.leftBottomY, params_PUSH.ang, 0.900 - cell.horLen);
	if (cell.verLen > 4.000) {
		elev_headpiece = 4.000 * 0.80;
	} else {
		elev_headpiece = cell.verLen * 0.80;
	}
	params_PUSH.leftBottomZ = moveZ (params_PUSH.leftBottomZ, -0.300 + elev_headpiece);
	// ������ ��
	elemList.Push (placePUSH (params_PUSH));
	params_PUSH.leftBottomX = moveXinParallel (params_PUSH.leftBottomX, params_PUSH.ang, cell.horLen - 0.900);
	params_PUSH.leftBottomY = moveYinParallel (params_PUSH.leftBottomY, params_PUSH.ang, cell.horLen - 0.900);
	elemList.Push (placePUSH (params_PUSH));

	// �׷�ȭ�ϱ�
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

	return	err;
}

// �ش� ���� ���ϴ� ��ǥX ��ġ�� ����
double	getCellPositionLeftBottomXForWallTableForm (WallTableformPlacingZone *placingZone, short idx)
{
	double		distance = 0.0;
	short		xx;

	for (xx = 0 ; xx < idx ; ++xx) {
		distance += placingZone->cells [xx].horLen;
	}

	return distance;
}

// ���̺��� ��ġ�� ���� ���Ǹ� ��û�ϴ� ���̾�α�
short DGCALLBACK wallTableformPlacerHandler (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	API_UCCallbackType	ucb;

	short	xx;
	short	buttonPosX;		// ������ ��ư ��ġ
	short	itmIdx;
	double	width;

	char	labelStr [32];

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, "���̺��� ���� ��ġ");

			//////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			// ���� ��ư
			DGSetItemText (dialogID, DG_OK, "Ȯ ��");

			// ���� ��ư
			DGSetItemText (dialogID, DG_CANCEL, "�� ��");

			//////////////////////////////////////////////////////////// ������ ��ġ (������)
			DGSetItemText (dialogID, LABEL_HEIGHT, "����");
			DGSetItemText (dialogID, LABEL_WIDTH, "�ʺ�");
			DGSetItemText (dialogID, LABEL_ERR_MESSAGE, "���̴� ���� ġ���� ������\n1500, 1800, 2100, 2400, 2700, 3000, 3300, 3600, 3900\n4200, 4500, 4800, 5100, 5400, 5700, 6000");
			DGSetItemText (dialogID, LABEL_GAP_LENGTH, "������ ����");

			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, "���纰 ���̾� ����");
			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM, "������");
			DGSetItemText (dialogID, LABEL_LAYER_RECTPIPE, "��� ������");
			DGSetItemText (dialogID, LABEL_LAYER_PINBOLT, "�ɺ�Ʈ ��Ʈ");
			DGSetItemText (dialogID, LABEL_LAYER_WALLTIE, "��ü Ÿ��");
			DGSetItemText (dialogID, LABEL_LAYER_CLAMP, "���� Ŭ����");
			DGSetItemText (dialogID, LABEL_LAYER_HEADPIECE, "����ǽ�");

			// ���� ��Ʈ�� �ʱ�ȭ
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_LAYER_EUROFORM;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, 1);

			ucb.itemID	 = USERCONTROL_LAYER_RECTPIPE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, 1);

			ucb.itemID	 = USERCONTROL_LAYER_PINBOLT;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, 1);

			ucb.itemID	 = USERCONTROL_LAYER_WALLTIE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, 1);

			ucb.itemID	 = USERCONTROL_LAYER_CLAMP;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_CLAMP, 1);

			ucb.itemID	 = USERCONTROL_LAYER_HEADPIECE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, 1);

			// ���� �ʺ�
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 280, 20, 70, 23);
			DGSetItemText (dialogID, itmIdx, "���� �ʺ�");
			DGShowItem (dialogID, itmIdx);

			EDITCONTROL_REMAIN_WIDTH = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 350, 13, 50, 25);
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_WIDTH, placingZone.remainWidth);
			DGDisableItem (dialogID, EDITCONTROL_REMAIN_WIDTH);
			DGShowItem (dialogID, EDITCONTROL_REMAIN_WIDTH);

			// ���̾�α� �ʺ� ����
			DGSetDialogSize (dialogID, DG_CLIENT, 350 + (placingZone.nCells * 100), 500, DG_TOPLEFT, true);
			
			// ������
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 170, 45, placingZone.nCells * 100 + 50, 110);
			DGShowItem (dialogID, itmIdx);

			buttonPosX = 195;
			for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, buttonPosX, 50, 99, 100);
				DGShowItem (dialogID, itmIdx);

				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, buttonPosX + 20, 60, 60, 23);
				sprintf (labelStr, "�ʺ� (%d)", xx + 1);
				DGSetItemText (dialogID, itmIdx, labelStr);
				DGShowItem (dialogID, itmIdx);

				// �ʺ� �ش��ϴ� �޺��ڽ�
				POPUP_WIDTH [xx] = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 25, 5, buttonPosX + 20, 100, 60, 25);
				DGSetItemFont (dialogID, POPUP_WIDTH [xx], DG_IS_LARGE | DG_IS_PLAIN);
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "2300");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "2250");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "2200");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "2150");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "2100");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "2050");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "2000");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "1950");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "1900");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "1850");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "1800");
				DGPopUpInsertItem (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_WIDTH [xx], DG_POPUP_BOTTOM, "0");
				DGShowItem (dialogID, POPUP_WIDTH [xx]);

				if (placingZone.n2300w > 0) {
					width = 2.300;
					placingZone.n2300w --;
				} else if (placingZone.n2250w > 0) {
					width = 2.250;
					placingZone.n2250w --;
				} else if (placingZone.n2200w > 0) {
					width = 2.200;
					placingZone.n2200w --;
				} else if (placingZone.n2150w > 0) {
					width = 2.150;
					placingZone.n2150w --;
				} else if (placingZone.n2100w > 0) {
					width = 2.100;
					placingZone.n2100w --;
				} else if (placingZone.n2050w > 0) {
					width = 2.050;
					placingZone.n2050w --;
				} else if (placingZone.n2000w > 0) {
					width = 2.000;
					placingZone.n2000w --;
				} else if (placingZone.n1950w > 0) {
					width = 1.950;
					placingZone.n1950w --;
				} else if (placingZone.n1900w > 0) {
					width = 1.900;
					placingZone.n1900w --;
				} else if (placingZone.n1850w > 0) {
					width = 1.850;
					placingZone.n1850w --;
				} else if (placingZone.n1800w > 0) {
					width = 1.800;
					placingZone.n1800w --;
				} else
					width = 0.0;

				// �޺��ڽ��� �� ����
				if (width > EPS) {
					if (width + EPS > 2.300)		DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 1);
					else if (width  + EPS> 2.250)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 2);
					else if (width + EPS > 2.200)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 3);
					else if (width + EPS > 2.150)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 4);
					else if (width + EPS > 2.100)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 5);
					else if (width + EPS > 2.050)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 6);
					else if (width + EPS > 2.000)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 7);
					else if (width + EPS > 1.950)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 8);
					else if (width + EPS > 1.900)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 9);
					else if (width + EPS > 1.850)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 10);
					else if (width + EPS > 1.800)	DGPopUpSelectItem (dialogID, POPUP_WIDTH [xx], 11);
				}
				buttonPosX += 100;
			}

			// ���� ���
			DGSetItemValDouble (dialogID, EDITCONTROL_HEIGHT, placingZone.verLen);
			DGDisableItem (dialogID, EDITCONTROL_HEIGHT);

			// ���� ���� �޾Ƶ��� �� �ִ� ���ΰ�?
			if ( (abs (placingZone.verLen - 1.500) < EPS) || (abs (placingZone.verLen - 1.800) < EPS) || (abs (placingZone.verLen - 2.100) < EPS) || (abs (placingZone.verLen - 2.400) < EPS) ||
				(abs (placingZone.verLen - 2.700) < EPS) || (abs (placingZone.verLen - 3.000) < EPS) || (abs (placingZone.verLen - 3.300) < EPS) || (abs (placingZone.verLen - 3.600) < EPS) ||
				(abs (placingZone.verLen - 3.900) < EPS) || (abs (placingZone.verLen - 4.200) < EPS) || (abs (placingZone.verLen - 4.500) < EPS) || (abs (placingZone.verLen - 4.800) < EPS) ||
				(abs (placingZone.verLen - 5.100) < EPS) || (abs (placingZone.verLen - 5.400) < EPS) || (abs (placingZone.verLen - 5.700) < EPS) || (abs (placingZone.verLen - 6.000) < EPS) ) {

				DGHideItem (dialogID, LABEL_ERR_MESSAGE);
			} else {
				DGShowItem (dialogID, LABEL_ERR_MESSAGE);
			}

			// �ʺ� ���
			DGSetItemValDouble (dialogID, EDITCONTROL_WIDTH, placingZone.horLen);
			DGDisableItem (dialogID, EDITCONTROL_WIDTH);

			break;

		case DG_MSG_CHANGE:

			width = 0;
			for (xx = 0 ; xx < placingZone.nCells ; ++xx)
				width += atof (DGPopUpGetItemText (dialogID, POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, POPUP_WIDTH [xx])).ToCStr ()) / 1000.0;
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_WIDTH, placingZone.horLen - width);

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// ���� �ʺ�/���� ����
					for (xx = 0 ; xx < placingZone.nCells ; ++xx) {
						placingZone.cells [xx].horLen = atof (DGPopUpGetItemText (dialogID, POPUP_WIDTH [xx], DGPopUpGetSelected (dialogID, POPUP_WIDTH [xx])).ToCStr ()) / 1000.0;
						
						if ((6.000 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 6.000;
						else if ((5.700 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 5.700;
						else if ((5.400 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 5.400;
						else if ((5.100 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 5.100;
						else if ((4.800 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 4.800;
						else if ((4.500 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 4.500;
						else if ((4.200 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 4.200;
						else if ((3.900 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 3.900;
						else if ((3.600 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 3.600;
						else if ((3.300 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 3.300;
						else if ((3.000 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 3.000;
						else if ((2.700 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 2.700;
						else if ((2.400 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 2.400;
						else if ((2.100 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 2.100;
						else if ((1.800 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 1.800;
						else if ((1.500 - placingZone.verLen) < EPS)
							placingZone.cells [xx].verLen = 1.500;
						else
							placingZone.cells [xx].verLen = 0;
					}

					// ������ ����
					placingZone.gap = DGGetItemValDouble (dialogID, EDITCONTROL_GAP_LENGTH);

					// ���̾� ��ȣ ����
					layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM);
					layerInd_RectPipe		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE);
					layerInd_PinBolt		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT);
					layerInd_WallTie		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE);
					layerInd_Clamp			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_CLAMP);
					layerInd_HeadPiece		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE);
	
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

// �̵� ���� X ��ǥ�� �˷��� (Z ȸ������ ���) - ���� ������ �������� �̵�
double		moveXinParallel (double prevPosX, double ang, double offset)
{
	return	prevPosX + (offset * cos(ang));
}

// �̵� ���� Y ��ǥ�� �˷��� (Z ȸ������ ���) - ���� ������ �������� �̵�
double		moveYinParallel (double prevPosY, double ang, double offset)
{
	return	prevPosY + (offset * sin(ang));
}

// �̵� ���� X ��ǥ�� �˷��� (Z ȸ������ ���) - ���� ������ �������� �̵�
double		moveXinPerpend (double prevPosX, double ang, double offset)
{
	return	prevPosX - (offset * sin(ang));
}

// �̵� ���� Y ��ǥ�� �˷��� (Z ȸ������ ���) - ���� ������ �������� �̵�
double		moveYinPerpend (double prevPosY, double ang, double offset)
{
	return	prevPosY + (offset * cos(ang));
}

// �̵� ���� Z ��ǥ�� �˷��� (Z ȸ������ ���)
double		moveZ (double prevPosZ, double offset)
{
	return	prevPosZ + offset;
}

// ��ġ: ������
API_Guid	placeUFOM (paramsUFOM_ForWallTableform	params)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = gsmUFOM;
	double				aParam;
	double				bParam;
	Int32				addParNum;

	std::string			tempStr;
	
	// ��ü �ε�
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return elem.header.guid;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

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
	elem.header.floorInd = infoWall.floorInd;

	// ���̾�
	elem.header.layer = layerInd_Euroform;

	memo.params [0][27].value.real = TRUE;	// �԰���

	// �ʺ�
	tempStr = format_string ("%.0f", params.width * 1000);
	GS::ucscpy (memo.params [0][28].value.uStr, GS::UniString (tempStr.c_str ()).ToUStr ().Get ());

	// ����
	tempStr = format_string ("%.0f", params.height * 1000);
	GS::ucscpy (memo.params [0][29].value.uStr, GS::UniString (tempStr.c_str ()).ToUStr ().Get ());

	// ��ġ����
	tempStr = "�������";
	GS::ucscpy (memo.params [0][32].value.uStr, GS::UniString (tempStr.c_str ()).ToUStr ().Get ());
		
	memo.params [0][33].value.real = DegreeToRad (90.0);	// ȸ��X

	// ��ü ��ġ
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}

// ��ġ: ��� ������
API_Guid	placeSPIP (paramsSPIP_ForWallTableform	params)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = gsmSPIP;
	double				aParam;
	double				bParam;
	Int32				addParNum;

	std::string			tempStr;

	// ��ü �ε�
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return elem.header.guid;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

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
	elem.header.floorInd = infoWall.floorInd;

	// ���̾�
	elem.header.layer = layerInd_RectPipe;

	// �簢������
	tempStr = "�簢������";
	GS::ucscpy (memo.params [0][24].value.uStr, GS::UniString (tempStr.c_str ()).ToUStr ().Get ());

	memo.params [0][27].value.real = params.length;		// ����
	memo.params [0][28].value.real = params.pipeAng;	// ����

	// ��ü ��ġ
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}

// ��ġ: �ɺ�Ʈ ��Ʈ
API_Guid	placePINB (paramsPINB_ForWallTableform	params)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = gsmPINB;
	double				aParam;
	double				bParam;
	Int32				addParNum;

	std::string			tempStr;

	// ��ü �ε�
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return elem.header.guid;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

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
	elem.header.floorInd = infoWall.floorInd;

	// ���̾�
	elem.header.layer = layerInd_PinBolt;

	// �ɺ�Ʈ 90�� ȸ��
	if (params.bPinBoltRot90)
		memo.params [0][9].value.real = TRUE;
	else
		memo.params [0][9].value.real = FALSE;

	memo.params [0][10].value.real = params.boltLen;	// ��Ʈ ����
	memo.params [0][11].value.real = 0.010;				// ��Ʈ ����
	memo.params [0][12].value.real = 0.050;				// �ͼ� ��ġ
	memo.params [0][13].value.real = 0.100;				// �ͼ� ũ��
	memo.params [0][17].value.real = params.angX;		// X�� ȸ��
	memo.params [0][18].value.real = params.angY;		// Y�� ȸ��

	// ��ü ��ġ
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}

// ��ġ: ��ü Ÿ��
API_Guid	placeTIE (paramsTIE_ForWallTableform	params)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = gsmTIE;
	double				aParam;
	double				bParam;
	Int32				addParNum;

	std::string			tempStr;

	// ��ü �ε�
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return elem.header.guid;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

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
	elem.object.angle = params.ang + DegreeToRad (90.0);
	elem.header.floorInd = infoWall.floorInd;

	// ���̾�
	elem.header.layer = layerInd_WallTie;

	memo.params [0][9].value.real = params.boltLen;		// ��Ʈ ���� (�� �β� + 327mm �ʰ��̸� 100 ������ �������� ���� ���� ��)
	memo.params [0][10].value.real = 0.012;		// ��Ʈ ����
	memo.params [0][11].value.real = TRUE;		// �簢�ͻ�
	memo.params [0][12].value.real = 0.100;		// �簢�ͻ� ũ��
	
	// ��Ʈ Ÿ��
	tempStr = "Ÿ�� 1";
	GS::ucscpy (memo.params [0][13].value.uStr, GS::UniString (tempStr.c_str ()).ToUStr ().Get ());
	
	memo.params [0][14].value.real = TRUE;		// ��ü ���� ������
	memo.params [0][16].value.real = 0.012;		// ������ ����
	memo.params [0][17].value.real = 0.002;		// ������ �β�
	
	// ������ ������, ���� (�� �β���ŭ ����)
	memo.params [0][18].value.real = params.pipeBeg;
	memo.params [0][19].value.real = params.pipeEnd;
	
	// ��,���� ���Ӽ� ��ġ (�� �β� + 327mm ��ŭ ����)
	memo.params [0][20].value.real = params.clampBeg;
	memo.params [0][21].value.real = params.clampEnd;
	
	memo.params [0][22].value.real = DegreeToRad (0.0);		// ȸ�� Y

	// ��ü ��ġ
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}

// ��ġ: ���� Ŭ����
API_Guid	placeCLAM (paramsCLAM_ForWallTableform	params)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = gsmCLAM;
	double				aParam;
	double				bParam;
	Int32				addParNum;

	std::string			tempStr;

	// ��ü �ε�
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return elem.header.guid;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

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
	elem.header.floorInd = infoWall.floorInd;

	// ���̾�
	elem.header.layer = layerInd_WallTie;

	memo.params [0][9].value.real = params.angX;	// ��ü ȸ�� (X)
	memo.params [0][10].value.real = params.angY;	// ��ü ȸ�� (Y)
	memo.params [0][11].value.real = 0.018;			// ������Ʈ ���̱�

	// ��ü ��ġ
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}

// ��ġ: ����ǽ�
API_Guid	placePUSH (paramsPUSH_ForWallTableform	params)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = gsmPUSH;
	double				aParam;
	double				bParam;
	Int32				addParNum;

	std::string			tempStr;

	// ��ü �ε�
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return elem.header.guid;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

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
	elem.header.floorInd = infoWall.floorInd;

	// ���̾�
	elem.header.layer = layerInd_HeadPiece;

	memo.params [0][9].value.real = 0.009;					// ö�� �β�
	memo.params [0][10].value.real = DegreeToRad (0.0);		// ȸ��X
	memo.params [0][11].value.real = DegreeToRad (0.0);		// ȸ��Y

	// ��ü ��ġ
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}
