#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "LibraryConvert.hpp"

using namespace libraryConvertDG;

short	floorInd;		// ���� �������� �� �ε��� ����

// �ش� ���̾ ����ϴ��� ����
static bool		bLayerInd_Euroform;			// ���̾� ��ȣ: ������
static bool		bLayerInd_RectPipe;			// ���̾� ��ȣ: ��� ������
static bool		bLayerInd_PinBolt;			// ���̾� ��ȣ: �ɺ�Ʈ ��Ʈ
static bool		bLayerInd_WallTie;			// ���̾� ��ȣ: ��ü Ÿ��
static bool		bLayerInd_HeadPiece;		// ���̾� ��ȣ: ����ǽ�
static bool		bLayerInd_Join;				// ���̾� ��ȣ: ����ö��

static bool		bLayerInd_SlabTableform;	// ���̾� ��ȣ: ������ ���̺���
static bool		bLayerInd_Profile;			// ���̾� ��ȣ: KS��������

static bool		bLayerInd_Steelform;		// ���̾� ��ȣ: ��ƿ��
static bool		bLayerInd_Plywood;			// ���̾� ��ȣ: ����
static bool		bLayerInd_Fillersp;			// ���̾� ��ȣ: �ٷ������̼�
static bool		bLayerInd_OutcornerAngle;	// ���̾� ��ȣ: �ƿ��ڳʾޱ�
static bool		bLayerInd_OutcornerPanel;	// ���̾� ��ȣ: �ƿ��ڳʾޱ�
static bool		bLayerInd_IncornerPanel;	// ���̾� ��ȣ: ���ڳʾޱ�
static bool		bLayerInd_RectpipeHanger;	// ���̾� ��ȣ: �������� ���
static bool		bLayerInd_EuroformHook;		// ���̾� ��ȣ: ������ ��ũ
static bool		bLayerInd_Hidden;			// ���̾� ��ȣ: ����

// �ش� ���̾��� ��ȣ
static short	layerInd_Euroform;			// ���̾� ��ȣ: ������
static short	layerInd_RectPipe;			// ���̾� ��ȣ: ��� ������
static short	layerInd_PinBolt;			// ���̾� ��ȣ: �ɺ�Ʈ ��Ʈ
static short	layerInd_WallTie;			// ���̾� ��ȣ: ��ü Ÿ��
static short	layerInd_HeadPiece;			// ���̾� ��ȣ: ����ǽ�
static short	layerInd_Join;				// ���̾� ��ȣ: ����ö��

static short	layerInd_SlabTableform;		// ���̾� ��ȣ: ������ ���̺���
static short	layerInd_Profile;			// ���̾� ��ȣ: KS��������

static short	layerInd_Steelform;			// ���̾� ��ȣ: ��ƿ��
static short	layerInd_Plywood;			// ���̾� ��ȣ: ����
static short	layerInd_Fillersp;			// ���̾� ��ȣ: �ٷ������̼�
static short	layerInd_OutcornerAngle;	// ���̾� ��ȣ: �ƿ��ڳʾޱ�
static short	layerInd_OutcornerPanel;	// ���̾� ��ȣ: �ƿ��ڳʾޱ�
static short	layerInd_IncornerPanel;		// ���̾� ��ȣ: ���ڳʾޱ�
static short	layerInd_RectpipeHanger;	// ���̾� ��ȣ: �������� ���
static short	layerInd_EuroformHook;		// ���̾� ��ȣ: ������ ��ũ
static short	layerInd_Hidden;			// ���̾� ��ȣ: ����

static GS::Array<API_Guid>	elemList;		// �׷�ȭ�� ���� ������ ��������� GUID�� ���� ������
static GS::Array<API_Guid>	elemListBack;	// �׷�ȭ�� ���� ������ ��������� GUID�� ���� ������

bool	bDoubleSide;	// ���̺���(��): ���(true), �ܸ�(false)
double	wallThk;		// �� �β�


// ��� ���� ������(TCO: Temporary Construction Object)�� ���� ������� ��ȯ��
GSErrCode	convertVirtualTCO (void)
{
	GSErrCode	err = NoError;

	API_Element				elem;
	API_ElementMemo			memo;
	API_Elem_Head*			headList;

	short	xx, yy;
	short	result;
	long	nSel;

	const char*		tempStr;
	char			productName [16];	// ���� ������
	char			objType [32];		// ���̺���(��) Ÿ��1, ���̺���(��) Ÿ��2, ���̺���(������), ������, ��ƿ��, ����, �ٷ������̼�, �ƿ��ڳʾޱ�, �ƿ��ڳ��ǳ�, ���ڳ��ǳ�
	char			dir [16];			// �������, �ٴڱ��, �ٴڵ���
	char			coverSide [8];		// ���, �ܸ�
	double			oppSideOffset;		// �ݴ������� �Ÿ�
	bool			leftSide;			// ����(1), ������(0)
	bool			bRegularSize;		// ���� ũ��(1)
	double unit_A, unit_B, unit_ZZYZX;	// ���� ����
	int num_A, num_B, num_ZZYZX;		// ���� ����


	// ������ ��Ұ� ������ ����
	API_SelectionInfo		selectionInfo;
	API_Neig				**selNeigs;
	API_Element				tElem;
	GS::Array<API_Guid>		objects;
	GS::Array<API_Guid>		objectsRetry;


	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err != NoError) {
		BMKillHandle ((GSHandle *) &selNeigs);
		ACAPI_WriteReport ("��ҵ��� �����ؾ� �մϴ�.", true);
		return err;
	}

	// ��ü Ÿ���� ��� GUID�� ������
	if (selectionInfo.typeID != API_SelEmpty) {
		nSel = BMGetHandleSize ((GSHandle) selNeigs) / sizeof (API_Neig);
		for (xx = 0 ; xx < nSel && err == NoError ; ++xx) {
			tElem.header.typeID = Neig_To_ElemID ((*selNeigs)[xx].neigID);

			tElem.header.guid = (*selNeigs)[xx].guid;
			if (ACAPI_Element_Get (&tElem) != NoError)	// ������ �� �ִ� ����ΰ�?
				continue;

			if (tElem.header.typeID == API_ObjectID) {	// ��ü Ÿ�� ����ΰ�?
				objects.Push (tElem.header.guid);
				objectsRetry.Push (tElem.header.guid);
			}
		}
	}
	BMKillHandle ((GSHandle *) &selNeigs);

	// ����� ���� Ȯ��
	bLayerInd_Euroform = false;
	bLayerInd_RectPipe = false;
	bLayerInd_PinBolt = false;
	bLayerInd_WallTie = false;
	bLayerInd_HeadPiece = false;
	bLayerInd_Join = false;

	bLayerInd_SlabTableform = false;
	bLayerInd_Profile = false;

	bLayerInd_Steelform = false;
	bLayerInd_Plywood = false;
	bLayerInd_Fillersp = false;
	bLayerInd_OutcornerAngle = false;
	bLayerInd_OutcornerPanel = false;
	bLayerInd_IncornerPanel = false;
	bLayerInd_RectpipeHanger = false;
	bLayerInd_EuroformHook = false;
	bLayerInd_Hidden = false;

	// �̸� ���� �����縦 Ȯ���Ͽ� �����ؾ� �� ���̾ ������ ��
	while (objects.GetSize () > 0) {
		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		elem.header.guid = objects.Pop ();
		err = ACAPI_Element_Get (&elem);
		err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

		tempStr = getParameterStringByName (&memo, "productName");
		if (tempStr == NULL) {
			ACAPI_DisposeElemMemoHdls (&memo);
			continue;
		}
		strncpy (productName, tempStr, strlen (tempStr));
		productName [strlen (tempStr)] = '\0';

		if (my_strcmp (productName, "���� ������") == 0) {
			
			// ���� �������� �Ķ���� �� �ҷ�����
			tempStr = getParameterStringByName (&memo, "objType");
			strncpy (objType, tempStr, strlen (tempStr));
			objType [strlen (tempStr)] = '\0';

			if (my_strcmp (objType, "���̺���(��) Ÿ��1") == 0) {
				bLayerInd_Euroform = true;
				bLayerInd_RectPipe = true;
				bLayerInd_PinBolt = true;
				bLayerInd_HeadPiece = true;
				bLayerInd_Join = true;
			}
			if (my_strcmp (objType, "���̺���(��) Ÿ��2") == 0) {
				bLayerInd_Euroform = true;
				bLayerInd_RectPipe = true;
				bLayerInd_RectpipeHanger = true;
				bLayerInd_EuroformHook = true;
				bLayerInd_HeadPiece = true;
				bLayerInd_Join = true;
				bLayerInd_Hidden = true;
			}
			if (my_strcmp (objType, "���̺���(������)") == 0) {
				bLayerInd_SlabTableform = true;
				bLayerInd_Profile = true;
				bLayerInd_Join = true;
			}
			if (my_strcmp (objType, "������") == 0) {
				bLayerInd_Euroform = true;

			}
			if (my_strcmp (objType, "��ƿ��") == 0) {
				bLayerInd_Steelform = true;

			}
			if (my_strcmp (objType, "����") == 0) {
				bLayerInd_Plywood = true;

			}
			if (my_strcmp (objType, "�ٷ������̼�") == 0) {
				bLayerInd_Fillersp = true;

			}
			if (my_strcmp (objType, "�ƿ��ڳʾޱ�") == 0) {
				bLayerInd_OutcornerAngle = true;

			}
			if (my_strcmp (objType, "�ƿ��ڳ��ǳ�") == 0) {
				bLayerInd_OutcornerPanel = true;

			}
			if (my_strcmp (objType, "���ڳ��ǳ�") == 0) {
				bLayerInd_IncornerPanel = true;
			}
		}

		ACAPI_DisposeElemMemoHdls (&memo);
	}
	
	// [DIALOG] ���̾� ����
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32519, ACAPI_GetOwnResModule (), convertVirtualTCOHandler1, 0);

	// ���� �����縸 �߷����� ��ġ��
	while (objectsRetry.GetSize () > 0) {
		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		elem.header.guid = objectsRetry.Pop ();
		err = ACAPI_Element_Get (&elem);
		err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

		tempStr = getParameterStringByName (&memo, "productName");
		if (tempStr == NULL) {
			ACAPI_DisposeElemMemoHdls (&memo);
			continue;
		}
		strncpy (productName, tempStr, strlen (tempStr));
		productName [strlen (tempStr)] = '\0';

		if (my_strcmp (productName, "���� ������") == 0) {

			// ���� �������� �� �ε��� ������
			floorInd = elem.header.floorInd;

			tempStr = getParameterStringByName (&memo, "objType");						// ��ü ����
			strncpy (objType, tempStr, strlen (tempStr));
			objType [strlen (tempStr)] = '\0';
			tempStr = getParameterStringByName (&memo, "dir");							// �������, ��������, �ٴڱ��, �ٴڵ���
			strncpy (dir, tempStr, strlen (tempStr));
			dir [strlen (tempStr)] = '\0';
			tempStr = getParameterStringByName (&memo, "coverSide");					// ���, �ܸ�
			strncpy (coverSide, tempStr, strlen (tempStr));
			coverSide [strlen (tempStr)] = '\0';
			oppSideOffset = getParameterValueByName (&memo, "oppSideOffset");			// �ݴ������� �Ÿ�
			if (getParameterValueByName (&memo, "leftSide") > 0)						// ����(1), ������(0)
				leftSide = true;
			else
				leftSide = false;
			if (getParameterValueByName (&memo, "bRegularSize") > 0)					// ���� ũ��(1)
				bRegularSize = true;
			else
				bRegularSize = false;
			unit_A = getParameterValueByName (&memo, "unit_A");							// ���� ���� (A)
			unit_B = getParameterValueByName (&memo, "unit_B");							// ���� ���� (B)
			unit_ZZYZX = getParameterValueByName (&memo, "unit_ZZYZX");					// ���� ���� (ZZYZX)
			num_A = (int)round (getParameterValueByName (&memo, "num_A"), 0);			// ���� ���� (A)
			num_B = (int)round (getParameterValueByName (&memo, "num_B"), 0);			// ���� ���� (B)
			num_ZZYZX = (int)round (getParameterValueByName (&memo, "num_ZZYZX"), 0);	// ���� ���� (ZZYZX)

			// ���� ������ ����
			headList = new API_Elem_Head [1];
			headList [0] = elem.header;
			err = ACAPI_Element_Delete (&headList, 1);
			delete headList;

			// ���� �������� ������ ���� ���� �������� �����͸� ������
			WallTableform	walltableform;
			SlabTableform	slabtableform;
			Euroform		euroform;
			Plywood			plywood;
			FillerSpacer	fillersp;
			OutcornerAngle	outcornerAngle;
			OutcornerPanel	outcornerPanel;
			IncornerPanel	incornerPanel;

			// ���� �����縦 ��ġ��
			if (strncmp (objType, "���̺���(��)", strlen ("���̺���(��)")) == 0) {
				
				walltableform.leftBottomX = elem.object.pos.x;
				walltableform.leftBottomY = elem.object.pos.y;
				walltableform.leftBottomZ = elem.object.level;
				walltableform.ang = elem.object.angle;
				walltableform.width = unit_A;
				walltableform.height = unit_ZZYZX;

				// �� �β�
				wallThk = oppSideOffset;

				if (my_strcmp (coverSide, "���") == 0) {
					bDoubleSide = true;
				} else {
					bDoubleSide = false;
				}

				if (my_strcmp (dir, "�������") == 0) {
					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							if (my_strcmp (objType, "���̺���(��) Ÿ��1") == 0)
								placeTableformOnWall_portrait_Type1 (walltableform);
							else if (my_strcmp (objType, "���̺���(��) Ÿ��2") == 0)
								placeTableformOnWall_portrait_Type2 (walltableform);
							moveIn3D ('x', elem.object.angle, unit_A, &walltableform.leftBottomX, &walltableform.leftBottomY, &walltableform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &walltableform.leftBottomX, &walltableform.leftBottomY, &walltableform.leftBottomZ);
						moveIn3D ('z', elem.object.angle, unit_ZZYZX, &walltableform.leftBottomX, &walltableform.leftBottomY, &walltableform.leftBottomZ);
					}
				} else if (my_strcmp (dir, "��������") == 0) {
					for (xx = 0 ; xx < num_A ; ++xx) {
						for (yy = 0 ; yy < num_ZZYZX ; ++yy) {
							if (my_strcmp (objType, "���̺���(��) Ÿ��1") == 0)
								placeTableformOnWall_landscape_Type1 (walltableform);
							else if (my_strcmp (objType, "���̺���(��) Ÿ��2") == 0)
								placeTableformOnWall_landscape_Type2 (walltableform);
							moveIn3D ('x', elem.object.angle, unit_ZZYZX, &walltableform.leftBottomX, &walltableform.leftBottomY, &walltableform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_ZZYZX * num_ZZYZX, &walltableform.leftBottomX, &walltableform.leftBottomY, &walltableform.leftBottomZ);
						moveIn3D ('z', elem.object.angle, unit_A, &walltableform.leftBottomX, &walltableform.leftBottomY, &walltableform.leftBottomZ);
					}
				}
			} else if (my_strcmp (objType, "���̺���(������)") == 0) {

				slabtableform.leftBottomX = elem.object.pos.x;
				slabtableform.leftBottomY = elem.object.pos.y;
				slabtableform.leftBottomZ = elem.object.level;
				slabtableform.ang = elem.object.angle;

				slabtableform.direction = true;
				slabtableform.horLen = unit_A;
				slabtableform.verLen = unit_B;
				sprintf (slabtableform.type, "%.0f x %.0f", round (unit_A * 1000, 0), round (unit_B * 1000, 0));
				
				if (my_strcmp (dir, "�ٴڱ��") == 0) {

					// �������� ��ġ
					for (xx = 0 ; xx < num_B ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							elemList.Push (placeTableformOnSlabBottom (slabtableform));
							moveIn3D ('x', elem.object.angle, unit_A, &slabtableform.leftBottomX, &slabtableform.leftBottomY, &slabtableform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &slabtableform.leftBottomX, &slabtableform.leftBottomY, &slabtableform.leftBottomZ);
						moveIn3D ('y', elem.object.angle, unit_B, &slabtableform.leftBottomX, &slabtableform.leftBottomY, &slabtableform.leftBottomZ);
					}
				}

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
			} else if (my_strcmp (objType, "������") == 0) {

				euroform.ang = elem.object.angle;
				euroform.eu_stan_onoff = bRegularSize;
				if (bRegularSize == true) {
					euroform.eu_wid = unit_A;
					euroform.eu_hei = unit_ZZYZX;
				} else {
					euroform.eu_wid2 = unit_A;
					euroform.eu_hei2 = unit_ZZYZX;
				}

				if (my_strcmp (dir, "�������") == 0) {
					euroform.leftBottomX = elem.object.pos.x;
					euroform.leftBottomY = elem.object.pos.y;
					euroform.leftBottomZ = elem.object.level;
					euroform.u_ins_wall = true;					// true: �������, false: ��������
					euroform.ang_x = DegreeToRad (90.0);		// ��(90)

					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							elemList.Push (placeEuroform (euroform));
							moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('z', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					}

					// ���
					if (my_strcmp (coverSide, "���") == 0) {
						euroform.leftBottomX = elem.object.pos.x;
						euroform.leftBottomY = elem.object.pos.y;
						euroform.leftBottomZ = elem.object.level;
						euroform.ang = elem.object.angle + DegreeToRad (180.0);
						moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('y', elem.object.angle, oppSideOffset, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						
						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							for (yy = 0 ; yy < num_A ; ++yy) {
								elemListBack.Push (placeEuroform (euroform));
								moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
							}
							moveIn3D ('x', elem.object.angle, -unit_A * num_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
					}
				} else if (my_strcmp (dir, "��������") == 0) {
					euroform.leftBottomX = elem.object.pos.x;
					euroform.leftBottomY = elem.object.pos.y;
					euroform.leftBottomZ = elem.object.level;
					moveIn3D ('x', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					euroform.u_ins_wall = false;				// true: �������, false: ��������
					euroform.ang_x = DegreeToRad (90.0);		// ��(90)

					for (xx = 0 ; xx < num_A ; ++xx) {
						for (yy = 0 ; yy < num_ZZYZX ; ++yy) {
							elemList.Push (placeEuroform (euroform));
							moveIn3D ('x', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_ZZYZX * num_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('z', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					}

					// ���
					if (my_strcmp (coverSide, "���") == 0) {
						euroform.leftBottomX = elem.object.pos.x;
						euroform.leftBottomY = elem.object.pos.y;
						euroform.leftBottomZ = elem.object.level;
						euroform.ang = elem.object.angle + DegreeToRad (180.0);
						moveIn3D ('y', elem.object.angle, oppSideOffset, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						
						for (xx = 0 ; xx < num_A ; ++xx) {
							for (yy = 0 ; yy < num_ZZYZX ; ++yy) {
								elemListBack.Push (placeEuroform (euroform));
								moveIn3D ('x', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
							}
							moveIn3D ('x', elem.object.angle, -unit_ZZYZX * num_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
							moveIn3D ('z', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
					}
				} else if (my_strcmp (dir, "�ٴڱ��") == 0) {
					euroform.leftBottomX = elem.object.pos.x;
					euroform.leftBottomY = elem.object.pos.y;
					euroform.leftBottomZ = elem.object.level;
					moveIn3D ('y', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					euroform.u_ins_wall = true;					// true: �������, false: ��������
					euroform.ang_x = DegreeToRad (0.0);			// õ��(0)
					
					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							elemList.Push (placeEuroform (euroform));
							moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('y', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					}
				} else if (my_strcmp (dir, "�ٴڵ���") == 0) {
					euroform.leftBottomX = elem.object.pos.x;
					euroform.leftBottomY = elem.object.pos.y;
					euroform.leftBottomZ = elem.object.level;
					euroform.u_ins_wall = true;					// true: �������, false: ��������
					euroform.ang_x = DegreeToRad (180.0);		// �ٴ�(180)

					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							elemList.Push (placeEuroform (euroform));
							moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('y', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					}
				}

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

				// �׷�ȭ�ϱ�
				if (!elemListBack.IsEmpty ()) {
					GSSize nElems = elemListBack.GetSize ();
					API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
					if (elemHead != NULL) {
						for (GSIndex i = 0; i < nElems; i++)
							(*elemHead)[i].guid = elemListBack[i];

						ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

						BMKillHandle ((GSHandle *) &elemHead);
					}
				}
				elemListBack.Clear (false);
			} else if (my_strcmp (objType, "��ƿ��") == 0) {

				euroform.ang = elem.object.angle;
				euroform.eu_stan_onoff = bRegularSize;
				if (bRegularSize == true) {
					euroform.eu_wid = unit_A;
					euroform.eu_hei = unit_ZZYZX;
				} else {
					euroform.eu_wid2 = unit_A;
					euroform.eu_hei2 = unit_ZZYZX;
				}

				if (my_strcmp (dir, "�������") == 0) {
					euroform.leftBottomX = elem.object.pos.x;
					euroform.leftBottomY = elem.object.pos.y;
					euroform.leftBottomZ = elem.object.level;
					euroform.u_ins_wall = true;					// true: �������, false: ��������
					euroform.ang_x = DegreeToRad (90.0);		// ��(90)

					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							elemList.Push (placeSteelform (euroform));
							moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('z', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					}

					// ���
					if (my_strcmp (coverSide, "���") == 0) {
						euroform.leftBottomX = elem.object.pos.x;
						euroform.leftBottomY = elem.object.pos.y;
						euroform.leftBottomZ = elem.object.level;
						euroform.ang = elem.object.angle + DegreeToRad (180.0);
						moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('y', elem.object.angle, oppSideOffset, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						
						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							for (yy = 0 ; yy < num_A ; ++yy) {
								elemListBack.Push (placeSteelform (euroform));
								moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
							}
							moveIn3D ('x', elem.object.angle, -unit_A * num_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
					}
				} else if (my_strcmp (dir, "��������") == 0) {
					euroform.leftBottomX = elem.object.pos.x;
					euroform.leftBottomY = elem.object.pos.y;
					euroform.leftBottomZ = elem.object.level;
					moveIn3D ('x', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					euroform.u_ins_wall = false;				// true: �������, false: ��������
					euroform.ang_x = DegreeToRad (90.0);		// ��(90)

					for (xx = 0 ; xx < num_A ; ++xx) {
						for (yy = 0 ; yy < num_ZZYZX ; ++yy) {
							elemList.Push (placeSteelform (euroform));
							moveIn3D ('x', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_ZZYZX * num_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('z', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					}

					// ���
					if (my_strcmp (coverSide, "���") == 0) {
						euroform.leftBottomX = elem.object.pos.x;
						euroform.leftBottomY = elem.object.pos.y;
						euroform.leftBottomZ = elem.object.level;
						euroform.ang = elem.object.angle + DegreeToRad (180.0);
						moveIn3D ('y', elem.object.angle, oppSideOffset, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						
						for (xx = 0 ; xx < num_A ; ++xx) {
							for (yy = 0 ; yy < num_ZZYZX ; ++yy) {
								elemListBack.Push (placeSteelform (euroform));
								moveIn3D ('x', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
							}
							moveIn3D ('x', elem.object.angle, -unit_ZZYZX * num_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
							moveIn3D ('z', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
					}
				} else if (my_strcmp (dir, "�ٴڱ��") == 0) {
					euroform.leftBottomX = elem.object.pos.x;
					euroform.leftBottomY = elem.object.pos.y;
					euroform.leftBottomZ = elem.object.level;
					moveIn3D ('y', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					euroform.u_ins_wall = true;					// true: �������, false: ��������
					euroform.ang_x = DegreeToRad (0.0);			// õ��(0)
					
					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							elemList.Push (placeSteelform (euroform));
							moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('y', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					}
				} else if (my_strcmp (dir, "�ٴڵ���") == 0) {
					euroform.leftBottomX = elem.object.pos.x;
					euroform.leftBottomY = elem.object.pos.y;
					euroform.leftBottomZ = elem.object.level;
					euroform.u_ins_wall = true;					// true: �������, false: ��������
					euroform.ang_x = DegreeToRad (180.0);		// �ٴ�(180)

					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							elemList.Push (placeSteelform (euroform));
							moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('y', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					}
				}

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

				// �׷�ȭ�ϱ�
				if (!elemListBack.IsEmpty ()) {
					GSSize nElems = elemListBack.GetSize ();
					API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
					if (elemHead != NULL) {
						for (GSIndex i = 0; i < nElems; i++)
							(*elemHead)[i].guid = elemListBack[i];

						ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

						BMKillHandle ((GSHandle *) &elemHead);
					}
				}
				elemListBack.Clear (false);
			} else if (my_strcmp (objType, "����") == 0) {

				plywood.ang = elem.object.angle;
				plywood.leftBottomX = elem.object.pos.x;
				plywood.leftBottomY = elem.object.pos.y;
				plywood.leftBottomZ = elem.object.level;
				plywood.p_wid = unit_A;
				plywood.p_leng = unit_ZZYZX;

				if (my_strcmp (dir, "�������") == 0) {
					plywood.w_dir = 1;

					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							elemList.Push (placePlywood (plywood));
							moveIn3D ('x', elem.object.angle, unit_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						moveIn3D ('z', elem.object.angle, unit_ZZYZX, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
					}

					// ���
					if (my_strcmp (coverSide, "���") == 0) {
						plywood.leftBottomX = elem.object.pos.x;
						plywood.leftBottomY = elem.object.pos.y;
						plywood.leftBottomZ = elem.object.level;
						plywood.ang = elem.object.angle + DegreeToRad (180.0);
						moveIn3D ('x', elem.object.angle, unit_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						moveIn3D ('y', elem.object.angle, oppSideOffset, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							for (yy = 0 ; yy < num_A ; ++yy) {
								elemListBack.Push (placePlywood (plywood));
								moveIn3D ('x', elem.object.angle, unit_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
							}
							moveIn3D ('x', elem.object.angle, -unit_A * num_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						}
					}
				} else if (my_strcmp (dir, "��������") == 0) {
					plywood.w_dir = 2;

					for (xx = 0 ; xx < num_A ; ++xx) {
						for (yy = 0 ; yy < num_ZZYZX ; ++yy) {
							elemList.Push (placePlywood (plywood));
							moveIn3D ('x', elem.object.angle, unit_ZZYZX, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_ZZYZX * num_ZZYZX, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						moveIn3D ('z', elem.object.angle, unit_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
					}

					// ���
					if (my_strcmp (coverSide, "���") == 0) {
						plywood.leftBottomX = elem.object.pos.x;
						plywood.leftBottomY = elem.object.pos.y;
						plywood.leftBottomZ = elem.object.level;
						plywood.ang = elem.object.angle + DegreeToRad (180.0);
						moveIn3D ('x', elem.object.angle, unit_ZZYZX, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						moveIn3D ('y', elem.object.angle, oppSideOffset, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);

						for (xx = 0 ; xx < num_A ; ++xx) {
							for (yy = 0 ; yy < num_ZZYZX ; ++yy) {
								elemListBack.Push (placePlywood (plywood));
								moveIn3D ('x', elem.object.angle, unit_ZZYZX, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
							}
							moveIn3D ('x', elem.object.angle, -unit_ZZYZX * num_ZZYZX, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
							moveIn3D ('z', elem.object.angle, unit_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						}
					}
				} else if (my_strcmp (dir, "�ٴڱ��") == 0) {
					plywood.w_dir = 3;

					moveIn3D ('x', elem.object.angle, unit_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
					plywood.ang = elem.object.angle + DegreeToRad (90.0);

					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							elemList.Push (placePlywood (plywood));
							moveIn3D ('x', elem.object.angle, unit_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						moveIn3D ('y', elem.object.angle, unit_ZZYZX, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
					}
				} else if (my_strcmp (dir, "�ٴڵ���") == 0) {
					plywood.w_dir = 4;

					moveIn3D ('x', elem.object.angle, unit_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
					plywood.ang = elem.object.angle + DegreeToRad (90.0);

					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							elemList.Push (placePlywood (plywood));
							moveIn3D ('x', elem.object.angle, unit_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						moveIn3D ('y', elem.object.angle, unit_ZZYZX, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
					}
				}

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

				// �׷�ȭ�ϱ�
				if (!elemListBack.IsEmpty ()) {
					GSSize nElems = elemListBack.GetSize ();
					API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
					if (elemHead != NULL) {
						for (GSIndex i = 0; i < nElems; i++)
							(*elemHead)[i].guid = elemListBack[i];

						ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

						BMKillHandle ((GSHandle *) &elemHead);
					}
				}
				elemListBack.Clear (false);
			} else if (my_strcmp (objType, "�ٷ������̼�") == 0) {

				fillersp.ang = elem.object.angle;
				fillersp.leftBottomX = elem.object.pos.x;
				fillersp.leftBottomY = elem.object.pos.y;
				fillersp.leftBottomZ = elem.object.level;
				fillersp.f_thk = unit_A;
				fillersp.f_leng = unit_ZZYZX;

				if (my_strcmp (dir, "�������") == 0) {
					fillersp.f_ang = DegreeToRad (90.0);
					fillersp.f_rota = DegreeToRad (0.0);
					moveIn3D ('x', elem.object.angle, unit_A, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);

					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							elemList.Push (placeFillersp (fillersp));
							moveIn3D ('x', elem.object.angle, unit_A, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
						moveIn3D ('z', elem.object.angle, unit_ZZYZX, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
					}

					// ���
					if (my_strcmp (coverSide, "���") == 0) {
						fillersp.leftBottomX = elem.object.pos.x;
						fillersp.leftBottomY = elem.object.pos.y;
						fillersp.leftBottomZ = elem.object.level;
						fillersp.ang = elem.object.angle + DegreeToRad (180.0);
						moveIn3D ('y', elem.object.angle, oppSideOffset, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							for (yy = 0 ; yy < num_A ; ++yy) {
								elemListBack.Push (placeFillersp (fillersp));
								moveIn3D ('x', elem.object.angle, unit_A, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
							}
							moveIn3D ('x', elem.object.angle, -unit_A * num_A, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
						}
					}
				} else if (my_strcmp (dir, "��������") == 0) {
					fillersp.f_ang = DegreeToRad (0.0);
					fillersp.f_rota = DegreeToRad (0.0);

					for (xx = 0 ; xx < num_A ; ++xx) {
						for (yy = 0 ; yy < num_ZZYZX ; ++yy) {
							elemList.Push (placeFillersp (fillersp));
							moveIn3D ('x', elem.object.angle, unit_ZZYZX, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_ZZYZX * num_ZZYZX, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
						moveIn3D ('z', elem.object.angle, unit_A, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
					}

					// ���
					if (my_strcmp (coverSide, "���") == 0) {
						fillersp.leftBottomX = elem.object.pos.x;
						fillersp.leftBottomY = elem.object.pos.y;
						fillersp.leftBottomZ = elem.object.level;
						fillersp.ang = elem.object.angle + DegreeToRad (180.0);
						moveIn3D ('x', elem.object.angle, unit_ZZYZX, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
						moveIn3D ('y', elem.object.angle, oppSideOffset, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);

						for (xx = 0 ; xx < num_A ; ++xx) {
							for (yy = 0 ; yy < num_ZZYZX ; ++yy) {
								elemListBack.Push (placeFillersp (fillersp));
								moveIn3D ('x', elem.object.angle, unit_ZZYZX, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
							}
							moveIn3D ('x', elem.object.angle, -unit_ZZYZX * num_ZZYZX, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
							moveIn3D ('z', elem.object.angle, unit_A, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
						}
					}
				}

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

				// �׷�ȭ�ϱ�
				if (!elemListBack.IsEmpty ()) {
					GSSize nElems = elemListBack.GetSize ();
					API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
					if (elemHead != NULL) {
						for (GSIndex i = 0; i < nElems; i++)
							(*elemHead)[i].guid = elemListBack[i];

						ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

						BMKillHandle ((GSHandle *) &elemHead);
					}
				}
				elemListBack.Clear (false);
			} else if (my_strcmp (objType, "�ƿ��ڳʾޱ�") == 0) {

				outcornerAngle.ang = elem.object.angle;
				outcornerAngle.leftBottomX = elem.object.pos.x;
				outcornerAngle.leftBottomY = elem.object.pos.y;
				outcornerAngle.leftBottomZ = elem.object.level;
				outcornerAngle.a_leng = unit_ZZYZX;
				
				if (leftSide == true) {
					if (my_strcmp (dir, "�������") == 0) {
						moveIn3D ('x', outcornerAngle.ang, unit_A, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						outcornerAngle.a_ang = DegreeToRad (90.0);

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							outcornerAngle.ang = elem.object.angle + DegreeToRad (180.0);
							elemList.Push (placeOutcornerAngle (outcornerAngle));
							outcornerAngle.ang = elem.object.angle;
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						}

						// ���
						if (my_strcmp (coverSide, "���") == 0) {
							outcornerAngle.leftBottomX = elem.object.pos.x;
							outcornerAngle.leftBottomY = elem.object.pos.y;
							outcornerAngle.leftBottomZ = elem.object.level;
							outcornerAngle.a_ang = DegreeToRad (90.0);
							moveIn3D ('x', outcornerAngle.ang, unit_A, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
							moveIn3D ('y', elem.object.angle, oppSideOffset, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);

							for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
								outcornerAngle.ang = elem.object.angle + DegreeToRad (90.0);
								elemListBack.Push (placeOutcornerAngle (outcornerAngle));
								outcornerAngle.ang = elem.object.angle;
								moveIn3D ('z', elem.object.angle, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
							}
						}
					} else if (my_strcmp (dir, "�ٴڱ��") == 0) {
						moveIn3D ('x', outcornerAngle.ang, unit_A, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						outcornerAngle.a_ang = DegreeToRad (0.0);

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							outcornerAngle.ang = elem.object.angle + DegreeToRad (90.0);
							elemList.Push (placeOutcornerAngle (outcornerAngle));
							outcornerAngle.ang = elem.object.angle;
							moveIn3D ('y', elem.object.angle, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						}
					} else if (my_strcmp (dir, "�ٴڵ���") == 0) {
						moveIn3D ('x', elem.object.angle, unit_A, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						moveIn3D ('y', elem.object.angle, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						outcornerAngle.a_ang = DegreeToRad (180.0);

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							outcornerAngle.ang = elem.object.angle + DegreeToRad (90.0);
							elemList.Push (placeOutcornerAngle (outcornerAngle));
							outcornerAngle.ang = elem.object.angle;
							moveIn3D ('y', elem.object.angle, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						}
					}
				} else {
					if (my_strcmp (dir, "�������") == 0) {
						outcornerAngle.a_ang = DegreeToRad (90.0);

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							outcornerAngle.ang = elem.object.angle + DegreeToRad (270.0);
							elemList.Push (placeOutcornerAngle (outcornerAngle));
							outcornerAngle.ang = elem.object.angle;
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						}

						// ���
						if (my_strcmp (coverSide, "���") == 0) {
							outcornerAngle.leftBottomX = elem.object.pos.x;
							outcornerAngle.leftBottomY = elem.object.pos.y;
							outcornerAngle.leftBottomZ = elem.object.level;
							outcornerAngle.a_ang = DegreeToRad (90.0);
							moveIn3D ('y', elem.object.angle, oppSideOffset, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);

							for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
								outcornerAngle.ang = elem.object.angle;
								elemListBack.Push (placeOutcornerAngle (outcornerAngle));
								moveIn3D ('z', elem.object.angle, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
							}
						}
					} else if (my_strcmp (dir, "�ٴڱ��") == 0) {
						moveIn3D ('y', outcornerAngle.ang, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						outcornerAngle.a_ang = DegreeToRad (0.0);

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							outcornerAngle.ang = elem.object.angle + DegreeToRad (270.0);
							elemList.Push (placeOutcornerAngle (outcornerAngle));
							outcornerAngle.ang = elem.object.angle;
							moveIn3D ('y', elem.object.angle, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						}
					} else if (my_strcmp (dir, "�ٴڵ���") == 0) {
						outcornerAngle.a_ang = DegreeToRad (180.0);

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							outcornerAngle.ang = elem.object.angle + DegreeToRad (270.0);
							elemList.Push (placeOutcornerAngle (outcornerAngle));
							outcornerAngle.ang = elem.object.angle;
							moveIn3D ('y', elem.object.angle, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						}
					}
				}

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

				// �׷�ȭ�ϱ�
				if (!elemListBack.IsEmpty ()) {
					GSSize nElems = elemListBack.GetSize ();
					API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
					if (elemHead != NULL) {
						for (GSIndex i = 0; i < nElems; i++)
							(*elemHead)[i].guid = elemListBack[i];

						ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

						BMKillHandle ((GSHandle *) &elemHead);
					}
				}
				elemListBack.Clear (false);
			} else if (my_strcmp (objType, "�ƿ��ڳ��ǳ�") == 0) {

				outcornerPanel.ang = elem.object.angle;
				outcornerPanel.leftBottomX = elem.object.pos.x;
				outcornerPanel.leftBottomY = elem.object.pos.y;
				outcornerPanel.leftBottomZ = elem.object.level;

				if (leftSide == true) {
					if (my_strcmp (dir, "�������") == 0) {
						outcornerPanel.wid_s = unit_A;
						outcornerPanel.leng_s = unit_B;
						outcornerPanel.hei_s = unit_ZZYZX;

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							elemList.Push (placeOutcornerPanel (outcornerPanel));
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &outcornerPanel.leftBottomX, &outcornerPanel.leftBottomY, &outcornerPanel.leftBottomZ);
						}

						// ���
						if (my_strcmp (coverSide, "���") == 0) {
							outcornerPanel.leftBottomX = elem.object.pos.x;
							outcornerPanel.leftBottomY = elem.object.pos.y;
							outcornerPanel.leftBottomZ = elem.object.level;

							outcornerPanel.wid_s = unit_B;
							outcornerPanel.leng_s = unit_A;
							outcornerPanel.hei_s = unit_ZZYZX;

							moveIn3D ('y', elem.object.angle, oppSideOffset, &outcornerPanel.leftBottomX, &outcornerPanel.leftBottomY, &outcornerPanel.leftBottomZ);

							for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
								outcornerPanel.ang = elem.object.angle - DegreeToRad (90.0);
								elemListBack.Push (placeOutcornerPanel (outcornerPanel));
								outcornerPanel.ang = elem.object.angle;
								moveIn3D ('z', elem.object.angle, unit_ZZYZX, &outcornerPanel.leftBottomX, &outcornerPanel.leftBottomY, &outcornerPanel.leftBottomZ);
							}
						}
					}
				} else {
					if (my_strcmp (dir, "�������") == 0) {
						outcornerPanel.wid_s = unit_B;
						outcornerPanel.leng_s = unit_A;
						outcornerPanel.hei_s = unit_ZZYZX;

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							outcornerPanel.ang = elem.object.angle + DegreeToRad (90.0);
							elemList.Push (placeOutcornerPanel (outcornerPanel));
							outcornerPanel.ang = elem.object.angle;
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &outcornerPanel.leftBottomX, &outcornerPanel.leftBottomY, &outcornerPanel.leftBottomZ);
						}

						// ���
						if (my_strcmp (coverSide, "���") == 0) {
							outcornerPanel.leftBottomX = elem.object.pos.x;
							outcornerPanel.leftBottomY = elem.object.pos.y;
							outcornerPanel.leftBottomZ = elem.object.level;

							outcornerPanel.wid_s = unit_A;
							outcornerPanel.leng_s = unit_B;
							outcornerPanel.hei_s = unit_ZZYZX;

							moveIn3D ('y', elem.object.angle, oppSideOffset, &outcornerPanel.leftBottomX, &outcornerPanel.leftBottomY, &outcornerPanel.leftBottomZ);

							for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
								outcornerPanel.ang = elem.object.angle + DegreeToRad (180.0);
								elemListBack.Push (placeOutcornerPanel (outcornerPanel));
								outcornerPanel.ang = elem.object.angle;
								moveIn3D ('z', elem.object.angle, unit_ZZYZX, &outcornerPanel.leftBottomX, &outcornerPanel.leftBottomY, &outcornerPanel.leftBottomZ);
							}
						}
					}
				}

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

				// �׷�ȭ�ϱ�
				if (!elemListBack.IsEmpty ()) {
					GSSize nElems = elemListBack.GetSize ();
					API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
					if (elemHead != NULL) {
						for (GSIndex i = 0; i < nElems; i++)
							(*elemHead)[i].guid = elemListBack[i];

						ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

						BMKillHandle ((GSHandle *) &elemHead);
					}
				}
				elemListBack.Clear (false);
			} else if (my_strcmp (objType, "���ڳ��ǳ�") == 0) {

				incornerPanel.ang = elem.object.angle;
				incornerPanel.leftBottomX = elem.object.pos.x;
				incornerPanel.leftBottomY = elem.object.pos.y;
				incornerPanel.leftBottomZ = elem.object.level;

				if (leftSide == true) {
					if (my_strcmp (dir, "�������") == 0) {
						incornerPanel.wid_s = unit_B;
						incornerPanel.leng_s = unit_A;
						incornerPanel.hei_s = unit_ZZYZX;

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							incornerPanel.ang = elem.object.angle + DegreeToRad (270.0);
							elemList.Push (placeIncornerPanel (incornerPanel));
							incornerPanel.ang = elem.object.angle;
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &incornerPanel.leftBottomX, &incornerPanel.leftBottomY, &incornerPanel.leftBottomZ);
						}

						// ���
						if (my_strcmp (coverSide, "���") == 0) {
							incornerPanel.leftBottomX = elem.object.pos.x;
							incornerPanel.leftBottomY = elem.object.pos.y;
							incornerPanel.leftBottomZ = elem.object.level;

							incornerPanel.wid_s = unit_A;
							incornerPanel.leng_s = unit_B;
							incornerPanel.hei_s = unit_ZZYZX;

							moveIn3D ('y', elem.object.angle, oppSideOffset, &incornerPanel.leftBottomX, &incornerPanel.leftBottomY, &incornerPanel.leftBottomZ);

							for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
								incornerPanel.ang = elem.object.angle;
								elemListBack.Push (placeIncornerPanel (incornerPanel));
								moveIn3D ('z', elem.object.angle, unit_ZZYZX, &incornerPanel.leftBottomX, &incornerPanel.leftBottomY, &incornerPanel.leftBottomZ);
							}
						}
					}
				} else {
					if (my_strcmp (dir, "�������") == 0) {
						incornerPanel.wid_s = unit_A;
						incornerPanel.leng_s = unit_B;
						incornerPanel.hei_s = unit_ZZYZX;

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							incornerPanel.ang = elem.object.angle + DegreeToRad (180.0);
							elemList.Push (placeIncornerPanel (incornerPanel));
							incornerPanel.ang = elem.object.angle;
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &incornerPanel.leftBottomX, &incornerPanel.leftBottomY, &incornerPanel.leftBottomZ);
						}

						// ���
						if (my_strcmp (coverSide, "���") == 0) {
							incornerPanel.leftBottomX = elem.object.pos.x;
							incornerPanel.leftBottomY = elem.object.pos.y;
							incornerPanel.leftBottomZ = elem.object.level;

							incornerPanel.wid_s = unit_B;
							incornerPanel.leng_s = unit_A;
							incornerPanel.hei_s = unit_ZZYZX;

							moveIn3D ('y', elem.object.angle, oppSideOffset, &incornerPanel.leftBottomX, &incornerPanel.leftBottomY, &incornerPanel.leftBottomZ);

							for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
								incornerPanel.ang = elem.object.angle + DegreeToRad (90.0);
								elemListBack.Push (placeIncornerPanel (incornerPanel));
								incornerPanel.ang = elem.object.angle;
								moveIn3D ('z', elem.object.angle, unit_ZZYZX, &incornerPanel.leftBottomX, &incornerPanel.leftBottomY, &incornerPanel.leftBottomZ);
							}
						}
					}
				}

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

				// �׷�ȭ�ϱ�
				if (!elemListBack.IsEmpty ()) {
					GSSize nElems = elemListBack.GetSize ();
					API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
					if (elemHead != NULL) {
						for (GSIndex i = 0; i < nElems; i++)
							(*elemHead)[i].guid = elemListBack[i];

						ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

						BMKillHandle ((GSHandle *) &elemHead);
					}
				}
				elemListBack.Clear (false);
			}
		}

		ACAPI_DisposeElemMemoHdls (&memo);
	}

	// ȭ�� ���ΰ�ħ
	ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	bool	regenerate = true;
	ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

	return	err;
}

// ���̺���(��) ��ġ (�������) Ÿ��1
GSErrCode	placeTableformOnWall_portrait_Type1 (WallTableform params)
{
	GSErrCode	err = NoError;

	short	nHorEuroform;			// ���� ���� ������ ����
	short	nVerEuroform;			// ���� ���� ������ ����
	double	width [7];				// ���� ���� �� ������ �ʺ�
	double	height [7];				// ���� ���� �� ������ ����

	short		xx, yy;
	double		width_t, height_t;
	double		elev_headpiece;
	double		horizontalGap = 0.050;	// ������ ���� �̰ݰŸ�

	Euroform					euroform;
	SquarePipe					sqrPipe;
	PinBoltSet					pinbolt;
	//WallTie						walltie;
	HeadpieceOfPushPullProps	headpiece;
	MetalFittingsWithRectWasher	fittings;

	nHorEuroform = 0;
	nVerEuroform = 0;
	for (xx = 0 ; xx < 7 ; ++xx) {
		width [xx] = 0.0;
		height [xx] = 0.0;
	}

	if (abs (params.width - 2.300) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.600;	width [2] = 0.500;	width [3] = 0.600;
		horizontalGap = 0.050;
	} else if (abs (params.width - 2.250) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.600;	width [2] = 0.450;	width [3] = 0.600;
		horizontalGap = 0.025;
	} else if (abs (params.width - 2.200) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.600;	width [2] = 0.400;	width [3] = 0.600;
		horizontalGap = 0.050;
	} else if (abs (params.width - 2.150) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.500;	width [2] = 0.450;	width [3] = 0.600;
		horizontalGap = 0.025;
	} else if (abs (params.width - 2.100) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.600;	width [2] = 0.300;	width [3] = 0.600;
		horizontalGap = 0.050;
	} else if (abs (params.width - 2.050) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.450;	width [2] = 0.400;	width [3] = 0.600;
		horizontalGap = 0.025;
	} else if (abs (params.width - 2.000) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.600;	width [2] = 0.200;	width [3] = 0.600;
		horizontalGap = 0.050;
	} else if (abs (params.width - 1.950) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.450;	width [2] = 0.300;	width [3] = 0.600;
		horizontalGap = 0.025;
	} else if (abs (params.width - 1.900) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.500;	width [2] = 0.200;	width [3] = 0.600;
		horizontalGap = 0.050;
	} else if (abs (params.width - 1.850) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.450;	width [2] = 0.200;	width [3] = 0.600;
		horizontalGap = 0.025;
	} else if (abs (params.width - 1.800) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.600;	width [1] = 0.600;	width [2] = 0.600;	width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (params.width - 1.750) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.200;	width [2] = 0.450;	width [3] = 0.500;
		horizontalGap = 0.025;
	} else if (abs (params.width - 1.700) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.600;	width [1] = 0.500;	width [2] = 0.600;	width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (params.width - 1.650) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.600;	width [1] = 0.450;	width [2] = 0.600;	width [3] = 0.0;
		horizontalGap = 0.025;
	} else if (abs (params.width - 1.600) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.600;	width [1] = 0.400;	width [2] = 0.600;	width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (params.width - 1.550) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.600;	width [1] = 0.450;	width [2] = 0.500;	width [3] = 0.0;
		horizontalGap = 0.025;
	} else if (abs (params.width - 1.500) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.600;	width [1] = 0.300;	width [2] = 0.600;	width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (params.width - 1.450) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.500;	width [1] = 0.450;	width [2] = 0.500;	width [3] = 0.0;
		horizontalGap = 0.025;
	} else if (abs (params.width - 1.400) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.500;	width [1] = 0.400;	width [2] = 0.500;	width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (params.width - 1.350) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.600;	width [1] = 0.300;	width [2] = 0.450;	width [3] = 0.0;
		horizontalGap = 0.025;
	} else if (abs (params.width - 1.300) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.600;	width [1] = 0.200;	width [2] = 0.500;	width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (params.width - 1.250) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.600;	width [1] = 0.200;	width [2] = 0.450;	width [3] = 0.0;
		horizontalGap = 0.025;
	} else if (abs (params.width - 1.200) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.600;	width [1] = 0.600;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (params.width - 1.150) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.450;	width [1] = 0.300;	width [2] = 0.400;	width [3] = 0.0;
		horizontalGap = 0.025;
	} else if (abs (params.width - 1.100) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.400;	width [1] = 0.300;	width [2] = 0.400;	width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (params.width - 1.050) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.450;	width [1] = 0.300;	width [2] = 0.300;	width [3] = 0.0;
		horizontalGap = 0.025;
	} else if (abs (params.width - 1.000) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.600;	width [1] = 0.400;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (params.width - 0.950) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.450;	width [1] = 0.500;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.025;
	} else if (abs (params.width - 0.900) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.600;	width [1] = 0.300;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (params.width - 0.850) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.400;	width [1] = 0.450;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.025;
	} else if (abs (params.width - 0.800) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.400;	width [1] = 0.400;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (params.width - 0.750) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.450;	width [1] = 0.300;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.025;
	} else if (abs (params.width - 0.700) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.400;	width [1] = 0.300;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (params.width - 0.650) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.450;	width [1] = 0.200;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.025;
	} else if (abs (params.width - 0.600) < EPS) {
		nHorEuroform = 1;
		width [0] = 0.600;	width [1] = 0.0;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (params.width - 0.500) < EPS) {
		nHorEuroform = 1;
		width [0] = 0.500;	width [1] = 0.0;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (params.width - 0.450) < EPS) {
		nHorEuroform = 1;
		width [0] = 0.450;	width [1] = 0.0;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.025;
	} else if (abs (params.width - 0.400) < EPS) {
		nHorEuroform = 1;
		width [0] = 0.400;	width [1] = 0.0;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.050;
	} else {
		nHorEuroform = 0;
		width [0] = 0.0;	width [1] = 0.0;	width [2] = 0.0;	width [3] = 0.0;
	}

	if (abs (params.height - 6.000) < EPS) {
		nVerEuroform = 5;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 1.200;
		height [3] = 1.200;
		height [4] = 1.200;
	} else if (abs (params.height - 5.700) < EPS) {
		nVerEuroform = 5;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 1.200;
		height [3] = 1.200;
		height [4] = 0.900;
	} else if (abs (params.height - 5.400) < EPS) {
		nVerEuroform = 5;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 1.200;
		height [3] = 0.900;
		height [4] = 0.900;
	} else if (abs (params.height - 5.100) < EPS) {
		nVerEuroform = 5;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 1.200;
		height [3] = 0.900;
		height [4] = 0.600;
	} else if (abs (params.height - 4.800) < EPS) {
		nVerEuroform = 4;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 1.200;
		height [3] = 1.200;
		height [4] = 0.0;
	} else if (abs (params.height - 4.500) < EPS) {
		nVerEuroform = 4;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 1.200;
		height [3] = 0.900;
		height [4] = 0.0;
	} else if (abs (params.height - 4.200) < EPS) {
		nVerEuroform = 4;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 0.900;
		height [3] = 0.900;
		height [4] = 0.0;
	} else if (abs (params.height - 3.900) < EPS) {
		nVerEuroform = 4;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 0.900;
		height [3] = 0.600;
		height [4] = 0.0;
	} else if (abs (params.height - 3.600) < EPS) {
		nVerEuroform = 3;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 1.200;
		height [3] = 0.0;
		height [4] = 0.0;
	} else if (abs (params.height - 3.300) < EPS) {
		nVerEuroform = 3;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 0.900;
		height [3] = 0.0;
		height [4] = 0.0;
	} else if (abs (params.height - 3.000) < EPS) {
		nVerEuroform = 3;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 0.600;
		height [3] = 0.0;
		height [4] = 0.0;
	} else if (abs (params.height - 2.700) < EPS) {
		nVerEuroform = 3;
		height [0] = 1.200;
		height [1] = 0.900;
		height [2] = 0.600;
		height [3] = 0.0;
		height [4] = 0.0;
	} else if (abs (params.height - 2.400) < EPS) {
		nVerEuroform = 2;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 0.0;
		height [3] = 0.0;
		height [4] = 0.0;
	} else if (abs (params.height - 2.100) < EPS) {
		nVerEuroform = 2;
		height [0] = 1.200;
		height [1] = 0.900;
		height [2] = 0.0;
		height [3] = 0.0;
		height [4] = 0.0;
	} else if (abs (params.height - 1.800) < EPS) {
		nVerEuroform = 2;
		height [0] = 0.900;
		height [1] = 0.900;
		height [2] = 0.0;
		height [3] = 0.0;
		height [4] = 0.0;
	} else if (abs (params.height - 1.500) < EPS) {
		nVerEuroform = 2;
		height [0] = 0.900;
		height [1] = 0.600;
		height [2] = 0.0;
		height [3] = 0.0;
		height [4] = 0.0;
	} else {
		nVerEuroform = 0;
		height [0] = 0.0;
		height [1] = 0.0;
		height [2] = 0.0;
		height [3] = 0.0;
		height [4] = 0.0;
	}

	// �ʺ� ���̰� 0�̸� �ƹ��͵� ��ġ���� ����
	if ((nHorEuroform == 0) || (nVerEuroform == 0))
		return	NoError;

	//////////////////////////////////////////////////////////////// �����
	// ������ ��ġ
	euroform.leftBottomX = params.leftBottomX;
	euroform.leftBottomY = params.leftBottomY;
	euroform.leftBottomZ = params.leftBottomZ;
	euroform.ang = params.ang;
	euroform.eu_stan_onoff = true;
	euroform.u_ins_wall = true;
	euroform.ang_x = DegreeToRad (90.0);

	for (xx = 0 ; xx < nHorEuroform ; ++xx) {
		height_t = 0.0;
		for (yy = 0 ; yy < nVerEuroform ; ++yy) {
			euroform.eu_wid = euroform.width = width [xx];
			euroform.eu_hei = euroform.height = height [yy];
			height_t += height [yy];
			elemList.Push (placeEuroform (euroform));
			moveIn3D ('z', euroform.ang, height [yy], &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
		}
		moveIn3D ('x', euroform.ang, width [xx], &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
		moveIn3D ('z', euroform.ang, -height_t, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
	}

	// ��� ������ (����) ��ġ
	sqrPipe.leftBottomX = params.leftBottomX;
	sqrPipe.leftBottomY = params.leftBottomY;
	sqrPipe.leftBottomZ = params.leftBottomZ;
	sqrPipe.ang = params.ang;
	sqrPipe.length = params.width - (horizontalGap * 2);
	sqrPipe.pipeAng = DegreeToRad (0);

	moveIn3D ('x', sqrPipe.ang, horizontalGap, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	moveIn3D ('y', sqrPipe.ang, -(0.0635 + 0.025), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	moveIn3D ('z', sqrPipe.ang, 0.150 - 0.031, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

	for (xx = 0 ; xx <= nVerEuroform ; ++xx) {
		if (xx == 0) {
			// 1��
			elemList.Push (placeSqrPipe (sqrPipe));
			moveIn3D ('z', sqrPipe.ang, 0.062, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
			elemList.Push (placeSqrPipe (sqrPipe));
			moveIn3D ('z', sqrPipe.ang, -0.031 - 0.150 + height [xx] - 0.031, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		} else if (xx == nVerEuroform) {
			// ������ ��
			moveIn3D ('z', sqrPipe.ang, -0.150, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
			elemList.Push (placeSqrPipe (sqrPipe));
			moveIn3D ('z', sqrPipe.ang, 0.062, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
			elemList.Push (placeSqrPipe (sqrPipe));
		} else {
			// ������ ��
			elemList.Push (placeSqrPipe (sqrPipe));
			moveIn3D ('z', sqrPipe.ang, 0.062, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
			elemList.Push (placeSqrPipe (sqrPipe));
			moveIn3D ('z', sqrPipe.ang, -0.031 + height [xx] - 0.031, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		}
	}

	// ��� ������ (����) ��ġ
	sqrPipe.leftBottomX = params.leftBottomX;
	sqrPipe.leftBottomY = params.leftBottomY;
	sqrPipe.leftBottomZ = params.leftBottomZ;
	sqrPipe.ang = params.ang;
	sqrPipe.length = params.height - 0.100;
	sqrPipe.pipeAng = DegreeToRad (90);

	moveIn3D ('x', sqrPipe.ang, width [0] - 0.150 - 0.035, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	moveIn3D ('y', sqrPipe.ang, -(0.0635 + 0.075), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	moveIn3D ('z', sqrPipe.ang, 0.050, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

	// 1��
	elemList.Push (placeSqrPipe (sqrPipe));	moveIn3D ('x', sqrPipe.ang, 0.070, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	elemList.Push (placeSqrPipe (sqrPipe));	moveIn3D ('x', sqrPipe.ang, -0.070 - (width [0] - 0.150) + params.width + (-width [nHorEuroform-1] + 0.150), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	// 2��
	elemList.Push (placeSqrPipe (sqrPipe));	moveIn3D ('x', sqrPipe.ang, 0.070, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	elemList.Push (placeSqrPipe (sqrPipe));

	// �ɺ�Ʈ ��ġ (���� - ���ϴ�, �ֻ��)
	pinbolt.leftBottomX = params.leftBottomX;
	pinbolt.leftBottomY = params.leftBottomY;
	pinbolt.leftBottomZ = params.leftBottomZ;
	pinbolt.ang = params.ang;
	pinbolt.bPinBoltRot90 = TRUE;
	pinbolt.boltLen = 0.100;
	pinbolt.angX = DegreeToRad (270.0);
	pinbolt.angY = DegreeToRad (0.0);

	moveIn3D ('y', pinbolt.ang, -(0.1635), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

	// ���ϴ� ��
	moveIn3D ('z', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
	width_t = 0.0;
	for (xx = 0 ; xx < nHorEuroform - 1 ; ++xx) {
		width_t += width [xx];
		moveIn3D ('x', pinbolt.ang, width [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

		elemList.Push (placePinbolt (pinbolt));
	}
	// �ֻ�� ��
	moveIn3D ('x', pinbolt.ang, -width_t, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
	moveIn3D ('z', pinbolt.ang, params.height - 0.300, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
	for (xx = 0 ; xx < nHorEuroform - 1 ; ++xx) {
		moveIn3D ('x', pinbolt.ang, width [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

		elemList.Push (placePinbolt (pinbolt));
	}

	// �ɺ�Ʈ ��ġ (���� - ������)
	pinbolt.leftBottomX = params.leftBottomX;
	pinbolt.leftBottomY = params.leftBottomY;
	pinbolt.leftBottomZ = params.leftBottomZ;
	pinbolt.ang = params.ang;
	pinbolt.bPinBoltRot90 = FALSE;
	pinbolt.boltLen = 0.100;
	pinbolt.angX = DegreeToRad (270.0);
	pinbolt.angY = DegreeToRad (0.0);

	moveIn3D ('y', pinbolt.ang, -(0.1635), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

	// 2 ~ [n-1]��
	if (nHorEuroform >= 3) {
		moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		moveIn3D ('z', pinbolt.ang, height [0], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		for (xx = 1 ; xx < nVerEuroform ; ++xx) {
			width_t = 0.0;
			for (yy = 0 ; yy < nHorEuroform ; ++yy) {
				// 1��
				if (yy == 0) {
					elemList.Push (placePinbolt (pinbolt));
					moveIn3D ('x', pinbolt.ang, width [0] - 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
					width_t += width [0] - 0.150;
				// ������ ��
				} else if (yy == nHorEuroform - 1) {
					width_t += width [nHorEuroform-1] - 0.150;
					moveIn3D ('x', pinbolt.ang, width [nHorEuroform-1] - 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
					elemList.Push (placePinbolt (pinbolt));
				// ������ ��
				} else {
					width_t += width [yy];
					if (abs (width [yy] - 0.600) < EPS) {
						moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('x', pinbolt.ang, 0.300, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
					} else if (abs (width [yy] - 0.500) < EPS) {
						moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('x', pinbolt.ang, 0.200, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
					} else if (abs (width [yy] - 0.450) < EPS) {
						moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
					} else if (abs (width [yy] - 0.400) < EPS) {
						moveIn3D ('x', pinbolt.ang, 0.100, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('x', pinbolt.ang, 0.200, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('x', pinbolt.ang, 0.100, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
					} else if (abs (width [yy] - 0.300) < EPS) {
						moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
					} else if (abs (width [yy] - 0.200) < EPS) {
						moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('x', pinbolt.ang, 0.050, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
					}
				}
			}
			moveIn3D ('x', pinbolt.ang, -width_t, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
			moveIn3D ('z', pinbolt.ang, height [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		}
	}

	// �ɺ�Ʈ ��ġ (����)
	pinbolt.leftBottomX = params.leftBottomX;
	pinbolt.leftBottomY = params.leftBottomY;
	pinbolt.leftBottomZ = params.leftBottomZ;
	pinbolt.ang = params.ang;
	pinbolt.bPinBoltRot90 = FALSE;
	pinbolt.boltLen = 0.150;
	pinbolt.angX = DegreeToRad (270.0);
	pinbolt.angY = DegreeToRad (0.0);

	moveIn3D ('x', pinbolt.ang, width [0] - 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
	moveIn3D ('y', pinbolt.ang, -(0.2135), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
	moveIn3D ('z', pinbolt.ang, height [0], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

	// 1��
	height_t = 0.0;
	for (xx = 1 ; xx < nVerEuroform ; ++xx) {
		elemList.Push (placePinbolt (pinbolt));
		moveIn3D ('z', pinbolt.ang, height [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		height_t += height [xx];
	}
	// 2��
	moveIn3D ('x', pinbolt.ang, -(width [0] - 0.150) + params.width + (-width [nHorEuroform-1] + 0.150), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
	moveIn3D ('z', pinbolt.ang, -height_t, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
	for (xx = 1 ; xx < nVerEuroform ; ++xx) {
		elemList.Push (placePinbolt (pinbolt));
		moveIn3D ('z', pinbolt.ang, height [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		height_t += height [xx];
	}

	// ��ü Ÿ��
	//if (bDoubleSide == true) {
	//	walltie.leftBottomX = params.leftBottomX;
	//	walltie.leftBottomY = params.leftBottomY;
	//	walltie.leftBottomZ = params.leftBottomZ;
	//	walltie.ang = params.ang;
	//	remainder = fmod ((wallThk + 0.327), 0.100);
	//	walltie.boltLen = (wallThk + 0.327 + (0.100 - remainder));
	//	walltie.pipeBeg = 0.0365 + 0.1635;
	//	walltie.pipeEnd = 0.0365 + 0.1635 + wallThk;
	//	walltie.clampBeg = 0.0365;
	//	walltie.clampEnd = 0.0365 + wallThk + 0.327;

	//	moveIn3D ('x', walltie.ang, width [0] - 0.150, &walltie.leftBottomX, &walltie.leftBottomY, &walltie.leftBottomZ);
	//	moveIn3D ('y', walltie.ang, -(0.1635 + 0.0365), &walltie.leftBottomX, &walltie.leftBottomY, &walltie.leftBottomZ);
	//	moveIn3D ('z', walltie.ang, 0.350, &walltie.leftBottomX, &walltie.leftBottomY, &walltie.leftBottomZ);

	//	for (xx = 0 ; xx < 2 ; ++xx) {
	//		for (yy = 0 ; yy < nVerEuroform ; ++yy) {
	//			// ������ ��
	//			if (yy == 0) {
	//				elemList.Push (placeWalltie (walltie));
	//				moveIn3D ('z', walltie.ang, height [yy], &walltie.leftBottomX, &walltie.leftBottomY, &walltie.leftBottomZ);
	//	
	//			// �ֻ��� ��
	//			} else if (yy == nVerEuroform - 1) {
	//				moveIn3D ('z', walltie.ang, height [yy] - 0.350*2, &walltie.leftBottomX, &walltie.leftBottomY, &walltie.leftBottomZ);
	//				elemList.Push (placeWalltie (walltie));
	//				moveIn3D ('x', walltie.ang, -(width [0] - 0.150) + params.width + (-width [nHorEuroform-1] + 0.150), &walltie.leftBottomX, &walltie.leftBottomY, &walltie.leftBottomZ);
	//				moveIn3D ('z', walltie.ang, 0.350 - params.height + 0.350, &walltie.leftBottomX, &walltie.leftBottomY, &walltie.leftBottomZ);
	//			
	//			// 2 ~ [n-1]��
	//			} else {
	//				elemList.Push (placeWalltie (walltie));
	//				moveIn3D ('z', walltie.ang, height [yy], &walltie.leftBottomX, &walltie.leftBottomY, &walltie.leftBottomZ);
	//			}
	//		}
	//	}
	//}

	// ��� �ǽ�
	headpiece.leftBottomX = params.leftBottomX;
	headpiece.leftBottomY = params.leftBottomY;
	headpiece.leftBottomZ = params.leftBottomZ;
	headpiece.ang = params.ang;

	moveIn3D ('x', headpiece.ang, width [0] - 0.150 - 0.100, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	moveIn3D ('y', headpiece.ang, -0.1725, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	moveIn3D ('z', headpiece.ang, 0.231, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);

	// ó�� ��
	elemList.Push (placeHeadpiece_ver (headpiece));
	moveIn3D ('x', headpiece.ang, -(width [0] - 0.150) + params.width + (-width [nHorEuroform-1] + 0.150), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	elemList.Push (placeHeadpiece_ver (headpiece));
	moveIn3D ('x', headpiece.ang, (width [0] - 0.150) - params.width - (-width [nHorEuroform-1] + 0.150), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	elev_headpiece = 2.100;
	moveIn3D ('z', headpiece.ang, -0.231 + elev_headpiece, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	// ������ ��
	elemList.Push (placeHeadpiece_ver (headpiece));
	moveIn3D ('x', headpiece.ang, -(width [0] - 0.150) + params.width + (-width [nHorEuroform-1] + 0.150), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	elemList.Push (placeHeadpiece_ver (headpiece));
	
	// ����ö��
	fittings.leftBottomX = params.leftBottomX;
	fittings.leftBottomY = params.leftBottomY;
	fittings.leftBottomZ = params.leftBottomZ;
	fittings.ang = params.ang;

	fittings.angX = DegreeToRad (0.0);
	fittings.angY = DegreeToRad (0.0);
	fittings.bolt_len = 0.150;
	fittings.bolt_dia = 0.012;
	fittings.bWasher1 = true;
	fittings.bWasher2 = true;
	fittings.washer_pos1 = 0.0;
	fittings.washer_pos2 = 0.108;
	fittings.washer_size = 0.100;
	strncpy (fittings.nutType, "������Ʈ", strlen ("������Ʈ"));

	moveIn3D ('x', fittings.ang, width [0] - 0.150, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	moveIn3D ('y', fittings.ang, -0.0455, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	moveIn3D ('z', fittings.ang, 0.150, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);

	// ó�� ��
	fittings.ang += DegreeToRad (180.0);
	elemList.Push (placeFittings (fittings));
	fittings.ang -= DegreeToRad (180.0);
	moveIn3D ('x', fittings.ang, -(width [0] - 0.150) + params.width + (-width [nHorEuroform-1] + 0.150), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	fittings.ang += DegreeToRad (180.0);
	elemList.Push (placeFittings (fittings));
	fittings.ang -= DegreeToRad (180.0);
	moveIn3D ('x', fittings.ang, (width [0] - 0.150) - params.width - (-width [nHorEuroform-1] + 0.150), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	moveIn3D ('z', fittings.ang, params.height - 0.300, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);

	// ������ ��
	fittings.ang += DegreeToRad (180.0);
	elemList.Push (placeFittings (fittings));
	fittings.ang -= DegreeToRad (180.0);
	moveIn3D ('x', fittings.ang, -(width [0] - 0.150) + params.width + (-width [nHorEuroform-1] + 0.150), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	fittings.ang += DegreeToRad (180.0);
	elemList.Push (placeFittings (fittings));
	fittings.ang -= DegreeToRad (180.0);

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

	//////////////////////////////////////////////////////////////// �ݴ��
	if (bDoubleSide == true) {
		moveIn3D ('x', params.ang, params.width, &params.leftBottomX, &params.leftBottomY, &params.leftBottomY);
		moveIn3D ('y', params.ang, wallThk, &params.leftBottomX, &params.leftBottomY, &params.leftBottomY);
		params.ang += DegreeToRad (180.0);

		// ������ ��ġ (�ݴ����� �����)
		euroform.leftBottomX = params.leftBottomX;
		euroform.leftBottomY = params.leftBottomY;
		euroform.leftBottomZ = params.leftBottomZ;
		euroform.ang = params.ang;

		for (xx = nHorEuroform - 1 ; xx >= 0 ; --xx) {
			height_t = 0.0;
			for (yy = 0 ; yy < nVerEuroform ; ++yy) {
				euroform.eu_wid = euroform.width = width [xx];
				euroform.eu_hei = euroform.height = height [yy];
				height_t += height [yy];
				elemList.Push (placeEuroform (euroform));
				moveIn3D ('z', euroform.ang, height [yy], &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
			}
			moveIn3D ('x', euroform.ang, width [xx], &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
			moveIn3D ('z', euroform.ang, -height_t, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
		}

		// ��� ������ (����) ��ġ
		sqrPipe.leftBottomX = params.leftBottomX;
		sqrPipe.leftBottomY = params.leftBottomY;
		sqrPipe.leftBottomZ = params.leftBottomZ;
		sqrPipe.ang = params.ang;
		sqrPipe.length = params.width - (horizontalGap * 2);
		sqrPipe.pipeAng = DegreeToRad (0);

		moveIn3D ('x', sqrPipe.ang, horizontalGap, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		moveIn3D ('y', sqrPipe.ang, -(0.0635 + 0.025), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		moveIn3D ('z', sqrPipe.ang, 0.150 - 0.031, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

		for (xx = 0 ; xx <= nVerEuroform ; ++xx) {
			if (xx == 0) {
				// 1��
				elemList.Push (placeSqrPipe (sqrPipe));
				moveIn3D ('z', sqrPipe.ang, 0.062, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
				elemList.Push (placeSqrPipe (sqrPipe));
				moveIn3D ('z', sqrPipe.ang, -0.031 - 0.150 + height [xx] - 0.031, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
			} else if (xx == nVerEuroform) {
				// ������ ��
				moveIn3D ('z', sqrPipe.ang, -0.150, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
				elemList.Push (placeSqrPipe (sqrPipe));
				moveIn3D ('z', sqrPipe.ang, 0.062, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
				elemList.Push (placeSqrPipe (sqrPipe));
			} else {
				// ������ ��
				elemList.Push (placeSqrPipe (sqrPipe));
				moveIn3D ('z', sqrPipe.ang, 0.062, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
				elemList.Push (placeSqrPipe (sqrPipe));
				moveIn3D ('z', sqrPipe.ang, -0.031 + height [xx] - 0.031, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
			}
		}

		// ��� ������ (����) ��ġ
		sqrPipe.leftBottomX = params.leftBottomX;
		sqrPipe.leftBottomY = params.leftBottomY;
		sqrPipe.leftBottomZ = params.leftBottomZ;
		sqrPipe.ang = params.ang;
		sqrPipe.length = params.height - 0.100;
		sqrPipe.pipeAng = DegreeToRad (90);

		moveIn3D ('x', sqrPipe.ang, width [nHorEuroform-1] - 0.150 - 0.035, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		moveIn3D ('y', sqrPipe.ang, -(0.0635 + 0.075), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		moveIn3D ('z', sqrPipe.ang, 0.050, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

		// 1��
		elemList.Push (placeSqrPipe (sqrPipe));	moveIn3D ('x', sqrPipe.ang, 0.070, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		elemList.Push (placeSqrPipe (sqrPipe));	moveIn3D ('x', sqrPipe.ang, -0.070 - (width [nHorEuroform-1] - 0.150) + params.width + (-width [0] + 0.150), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		// 2��
		elemList.Push (placeSqrPipe (sqrPipe));	moveIn3D ('x', sqrPipe.ang, 0.070, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		elemList.Push (placeSqrPipe (sqrPipe));

		// �ɺ�Ʈ ��ġ (���� - ���ϴ�, �ֻ��) (�ݴ����� �����)
		pinbolt.leftBottomX = params.leftBottomX;
		pinbolt.leftBottomY = params.leftBottomY;
		pinbolt.leftBottomZ = params.leftBottomZ;
		pinbolt.ang = params.ang;
		pinbolt.bPinBoltRot90 = TRUE;
		pinbolt.boltLen = 0.100;
		pinbolt.angX = DegreeToRad (270.0);
		pinbolt.angY = DegreeToRad (0.0);

		moveIn3D ('y', pinbolt.ang, -(0.1635), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

		// ���ϴ� ��
		moveIn3D ('z', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		width_t = 0.0;
		for (xx = nHorEuroform - 1 ; xx > 0 ; --xx) {
			width_t += width [xx];
			moveIn3D ('x', pinbolt.ang, width [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

			elemList.Push (placePinbolt (pinbolt));
		}
		// �ֻ�� ��
		moveIn3D ('x', pinbolt.ang, -width_t, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		moveIn3D ('z', pinbolt.ang, params.height - 0.300, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		for (xx = nHorEuroform - 1 ; xx > 0 ; --xx) {
			moveIn3D ('x', pinbolt.ang, width [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

			elemList.Push (placePinbolt (pinbolt));
		}

		// �ɺ�Ʈ ��ġ (���� - ������) (�ݴ����� �����)
		pinbolt.leftBottomX = params.leftBottomX;
		pinbolt.leftBottomY = params.leftBottomY;
		pinbolt.leftBottomZ = params.leftBottomZ;
		pinbolt.ang = params.ang;
		pinbolt.bPinBoltRot90 = FALSE;
		pinbolt.boltLen = 0.100;
		pinbolt.angX = DegreeToRad (270.0);
		pinbolt.angY = DegreeToRad (0.0);

		moveIn3D ('y', pinbolt.ang, -(0.1635), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

		if (nHorEuroform >= 3) {
			moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
			moveIn3D ('z', pinbolt.ang, height [0], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
			for (xx = 1 ; xx < nVerEuroform ; ++xx) {
				width_t = 0.0;
				for (yy = nHorEuroform - 1 ; yy >= 0 ; --yy) {
					// 1��
					if (yy == nHorEuroform - 1) {
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('x', pinbolt.ang, width [nHorEuroform-1] - 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						width_t += width [nHorEuroform-1] - 0.150;
					// ������ ��
					} else if (yy == 0) {
						width_t += width [0] - 0.150;
						moveIn3D ('x', pinbolt.ang, width [0] - 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
					// ������ ��
					} else {
						width_t += width [yy];
						if (abs (width [yy] - 0.600) < EPS) {
							moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('x', pinbolt.ang, 0.300, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						} else if (abs (width [yy] - 0.500) < EPS) {
							moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('x', pinbolt.ang, 0.200, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						} else if (abs (width [yy] - 0.450) < EPS) {
							moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						} else if (abs (width [yy] - 0.400) < EPS) {
							moveIn3D ('x', pinbolt.ang, 0.100, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('x', pinbolt.ang, 0.200, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('x', pinbolt.ang, 0.100, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						} else if (abs (width [yy] - 0.300) < EPS) {
							moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						} else if (abs (width [yy] - 0.200) < EPS) {
							moveIn3D ('x', pinbolt.ang, 0.050, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						}
					}
				}
				moveIn3D ('x', pinbolt.ang, -width_t, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
				moveIn3D ('z', pinbolt.ang, height [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
			}
		}

		// �ɺ�Ʈ ��ġ (����)
		pinbolt.leftBottomX = params.leftBottomX;
		pinbolt.leftBottomY = params.leftBottomY;
		pinbolt.leftBottomZ = params.leftBottomZ;
		pinbolt.ang = params.ang;
		pinbolt.bPinBoltRot90 = FALSE;
		pinbolt.boltLen = 0.150;
		pinbolt.angX = DegreeToRad (270.0);
		pinbolt.angY = DegreeToRad (0.0);

		moveIn3D ('x', pinbolt.ang, width [nHorEuroform-1] - 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		moveIn3D ('y', pinbolt.ang, -(0.2135), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		moveIn3D ('z', pinbolt.ang, height [0], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

		// 1��
		height_t = 0.0;
		for (xx = 1 ; xx < nVerEuroform ; ++xx) {
			elemList.Push (placePinbolt (pinbolt));
			moveIn3D ('z', pinbolt.ang, height [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
			height_t += height [xx];
		}
		// 2��
		moveIn3D ('x', pinbolt.ang, -(width [nHorEuroform-1] - 0.150) + params.width + (-width [0] + 0.150), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		moveIn3D ('z', pinbolt.ang, -height_t, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		for (xx = 1 ; xx < nVerEuroform ; ++xx) {
			elemList.Push (placePinbolt (pinbolt));
			moveIn3D ('z', pinbolt.ang, height [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
			height_t += height [xx];
		}

		// ��� �ǽ�
		headpiece.leftBottomX = params.leftBottomX;
		headpiece.leftBottomY = params.leftBottomY;
		headpiece.leftBottomZ = params.leftBottomZ;
		headpiece.ang = params.ang;

		moveIn3D ('x', headpiece.ang, width [nHorEuroform-1] - 0.150 - 0.100, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		moveIn3D ('y', headpiece.ang, -0.1725, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		moveIn3D ('z', headpiece.ang, 0.231, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);

		// ó�� ��
		elemList.Push (placeHeadpiece_hor (headpiece));
		moveIn3D ('x', headpiece.ang, -(width [nHorEuroform-1] - 0.150) + params.width + (-width [0] + 0.150), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		elemList.Push (placeHeadpiece_hor (headpiece));
		moveIn3D ('x', headpiece.ang, (width [nHorEuroform-1] - 0.150) - params.width - (-width [0] + 0.150), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		elev_headpiece = 2.100;
		moveIn3D ('z', headpiece.ang, -0.231 + elev_headpiece, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		// ������ ��
		elemList.Push (placeHeadpiece_hor (headpiece));
		moveIn3D ('x', headpiece.ang, -(width [nHorEuroform-1] - 0.150) + params.width + (-width [0] + 0.150), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		elemList.Push (placeHeadpiece_hor (headpiece));

		// ����ö��
		fittings.leftBottomX = params.leftBottomX;
		fittings.leftBottomY = params.leftBottomY;
		fittings.leftBottomZ = params.leftBottomZ;
		fittings.ang = params.ang;

		fittings.angX = DegreeToRad (0.0);
		fittings.angY = DegreeToRad (0.0);
		fittings.bolt_len = 0.150;
		fittings.bolt_dia = 0.012;
		fittings.bWasher1 = true;
		fittings.bWasher2 = true;
		fittings.washer_pos1 = 0.0;
		fittings.washer_pos2 = 0.108;
		fittings.washer_size = 0.100;
		strncpy (fittings.nutType, "������Ʈ", strlen ("������Ʈ"));

		moveIn3D ('x', fittings.ang, width [nHorEuroform-1] - 0.150, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		moveIn3D ('y', fittings.ang, -0.0455, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		moveIn3D ('z', fittings.ang, 0.150, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);

		// ó�� ��
		fittings.ang += DegreeToRad (180.0);
		elemList.Push (placeFittings (fittings));
		fittings.ang -= DegreeToRad (180.0);
		moveIn3D ('x', fittings.ang, -(width [nHorEuroform-1] - 0.150) + params.width + (-width [0] + 0.150), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		fittings.ang += DegreeToRad (180.0);
		elemList.Push (placeFittings (fittings));
		fittings.ang -= DegreeToRad (180.0);
		moveIn3D ('x', fittings.ang, (width [nHorEuroform-1] - 0.150) - params.width - (-width [0] + 0.150), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		moveIn3D ('z', fittings.ang, params.height - 0.300, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);

		// ������ ��
		fittings.ang += DegreeToRad (180.0);
		elemList.Push (placeFittings (fittings));
		fittings.ang -= DegreeToRad (180.0);
		moveIn3D ('x', fittings.ang, -(width [nHorEuroform-1] - 0.150) + params.width + (-width [0] + 0.150), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		fittings.ang += DegreeToRad (180.0);
		elemList.Push (placeFittings (fittings));
		fittings.ang -= DegreeToRad (180.0);

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
	}

	return	err;
}

// ���̺���(��) ��ġ (��������) Ÿ��1
GSErrCode	placeTableformOnWall_landscape_Type1 (WallTableform params)
{
	// ����, ���ΰ� �ڹٲ��� ��
	exchangeDoubles (&params.width, &params.height);

	GSErrCode	err = NoError;

	short	nHorEuroform;			// ���� ���� ������ ����
	short	nVerEuroform;			// ���� ���� ������ ����
	double	width [7];				// ���� ���� �� ������ �ʺ�
	double	height [7];				// ���� ���� �� ������ ����

	short		xx, yy;
	double		width_t, height_t;
	double		elev_headpiece;
	double		verticalGap = 0.050;	// ������ ���� �̰ݰŸ�

	Euroform					euroform;
	SquarePipe					sqrPipe;
	PinBoltSet					pinbolt;
	//WallTie						walltie;
	HeadpieceOfPushPullProps	headpiece;
	MetalFittingsWithRectWasher	fittings;

	nHorEuroform = 0;
	nVerEuroform = 0;
	for (xx = 0 ; xx < 7 ; ++xx) {
		width [xx] = 0.0;
		height [xx] = 0.0;
	}

	if (abs (params.height - 2.300) < EPS) {
		nVerEuroform = 4;
		//height [0] = 0.600;		height [1] = 0.600;		height [2] = 0.500;		height [3] = 0.600;
		height [0] = 0.500;		height [1] = 0.600;		height [2] = 0.600;		height [3] = 0.600;
		verticalGap = 0.050;
	} else if (abs (params.height - 2.250) < EPS) {
		nVerEuroform = 4;
		height [0] = 0.600;		height [1] = 0.600;		height [2] = 0.450;		height [3] = 0.600;
		verticalGap = 0.025;
	} else if (abs (params.height - 2.200) < EPS) {
		nVerEuroform = 4;
		//height [0] = 0.600;		height [1] = 0.600;		height [2] = 0.400;		height [3] = 0.600;
		height [0] = 0.400;		height [1] = 0.600;		height [2] = 0.600;		height [3] = 0.600;
		verticalGap = 0.050;
	} else if (abs (params.height - 2.150) < EPS) {
		nVerEuroform = 4;
		//height [0] = 0.600;		height [1] = 0.500;		height [2] = 0.450;		height [3] = 0.600;
		height [0] = 0.500;		height [1] = 0.600;		height [2] = 0.450;		height [3] = 0.600;
		verticalGap = 0.025;
	} else if (abs (params.height - 2.100) < EPS) {
		nVerEuroform = 4;
		height [0] = 0.600;		height [1] = 0.600;		height [2] = 0.300;		height [3] = 0.600;
		verticalGap = 0.050;
	} else if (abs (params.height - 2.050) < EPS) {
		nVerEuroform = 4;
		//height [0] = 0.600;		height [1] = 0.450;		height [2] = 0.400;		height [3] = 0.600;
		height [0] = 0.400;		height [1] = 0.450;		height [2] = 0.600;		height [3] = 0.600;
		verticalGap = 0.025;
	} else if (abs (params.height - 2.000) < EPS) {
		nVerEuroform = 4;
		//height [0] = 0.600;		height [1] = 0.600;		height [2] = 0.200;		height [3] = 0.600;
		height [0] = 0.200;		height [1] = 0.600;		height [2] = 0.600;		height [3] = 0.600;
		verticalGap = 0.050;
	} else if (abs (params.height - 1.950) < EPS) {
		nVerEuroform = 4;
		height [0] = 0.600;		height [1] = 0.450;		height [2] = 0.300;		height [3] = 0.600;
		verticalGap = 0.025;
	} else if (abs (params.height - 1.900) < EPS) {
		nVerEuroform = 4;
		//height [0] = 0.600;		height [1] = 0.500;		height [2] = 0.200;		height [3] = 0.600;
		height [0] = 0.500;		height [1] = 0.200;		height [2] = 0.600;		height [3] = 0.600;
		verticalGap = 0.050;
	} else if (abs (params.height - 1.850) < EPS) {
		nVerEuroform = 4;
		//height [0] = 0.600;		height [1] = 0.450;		height [2] = 0.200;		height [3] = 0.600;
		height [0] = 0.200;		height [1] = 0.450;		height [2] = 0.600;		height [3] = 0.600;
		verticalGap = 0.025;
	} else if (abs (params.height - 1.800) < EPS) {
		nVerEuroform = 3;
		height [0] = 0.600;		height [1] = 0.600;		height [2] = 0.600;		height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (params.height - 1.750) < EPS) {
		nVerEuroform = 4;
		//height [0] = 0.600;		height [1] = 0.200;		height [2] = 0.450;		height [3] = 0.500;
		height [0] = 0.500;		height [1] = 0.200;		height [2] = 0.450;		height [3] = 0.600;
		verticalGap = 0.025;
	} else if (abs (params.height - 1.700) < EPS) {
		nVerEuroform = 3;
		//height [0] = 0.600;		height [1] = 0.500;		height [2] = 0.600;		height [3] = 0.0;
		height [0] = 0.500;		height [1] = 0.600;		height [2] = 0.600;		height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (params.height - 1.650) < EPS) {
		nVerEuroform = 3;
		height [0] = 0.600;		height [1] = 0.450;		height [2] = 0.600;		height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (params.height - 1.600) < EPS) {
		nVerEuroform = 3;
		//height [0] = 0.600;		height [1] = 0.400;		height [2] = 0.600;		height [3] = 0.0;
		height [0] = 0.400;		height [1] = 0.600;		height [2] = 0.600;		height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (params.height - 1.550) < EPS) {
		nVerEuroform = 3;
		//height [0] = 0.600;		height [1] = 0.450;		height [2] = 0.500;		height [3] = 0.0;
		height [0] = 0.500;		height [1] = 0.450;		height [2] = 0.600;		height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (params.height - 1.500) < EPS) {
		nVerEuroform = 3;
		height [0] = 0.600;		height [1] = 0.300;		height [2] = 0.600;		height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (params.height - 1.450) < EPS) {
		nVerEuroform = 3;
		//height [0] = 0.500;		height [1] = 0.450;		height [2] = 0.500;		height [3] = 0.0;
		height [0] = 0.400;		height [1] = 0.450;		height [2] = 0.600;		height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (params.height - 1.400) < EPS) {
		nVerEuroform = 3;
		//height [0] = 0.500;		height [1] = 0.400;		height [2] = 0.500;		height [3] = 0.0;
		height [0] = 0.500;		height [1] = 0.300;		height [2] = 0.600;		height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (params.height - 1.350) < EPS) {
		nVerEuroform = 3;
		height [0] = 0.600;		height [1] = 0.300;		height [2] = 0.450;		height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (params.height - 1.300) < EPS) {
		nVerEuroform = 3;
		//height [0] = 0.600;		height [1] = 0.200;		height [2] = 0.500;		height [3] = 0.0;
		height [0] = 0.500;		height [1] = 0.200;		height [2] = 0.600;		height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (params.height - 1.250) < EPS) {
		nVerEuroform = 3;
		//height [0] = 0.600;		height [1] = 0.200;		height [2] = 0.450;		height [3] = 0.0;
		height [0] = 0.200;		height [1] = 0.600;		height [2] = 0.450;		height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (params.height - 1.200) < EPS) {
		nVerEuroform = 2;
		height [0] = 0.600;		height [1] = 0.600;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (params.height - 1.150) < EPS) {
		nVerEuroform = 3;
		//height [0] = 0.450;		height [1] = 0.300;		height [2] = 0.400;		height [3] = 0.0;
		height [0] = 0.400;		height [1] = 0.300;		height [2] = 0.450;		height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (params.height - 1.100) < EPS) {
		//nVerEuroform = 3;
		nVerEuroform = 2;
		//height [0] = 0.400;		height [1] = 0.300;		height [2] = 0.400;		height [3] = 0.0;
		height [0] = 0.500;		height [1] = 0.600;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (params.height - 1.050) < EPS) {
		nVerEuroform = 3;
		height [0] = 0.450;		height [1] = 0.300;		height [2] = 0.300;		height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (params.height - 1.000) < EPS) {
		nVerEuroform = 2;
		//height [0] = 0.600;		height [1] = 0.400;		height [2] = 0.0;		height [3] = 0.0;
		height [0] = 0.400;		height [1] = 0.600;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (params.height - 0.950) < EPS) {
		nVerEuroform = 2;
		//height [0] = 0.450;		height [1] = 0.500;		height [2] = 0.0;		height [3] = 0.0;
		height [0] = 0.500;		height [1] = 0.450;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (params.height - 0.900) < EPS) {
		nVerEuroform = 2;
		height [0] = 0.600;		height [1] = 0.300;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (params.height - 0.850) < EPS) {
		nVerEuroform = 2;
		height [0] = 0.400;		height [1] = 0.450;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (params.height - 0.800) < EPS) {
		nVerEuroform = 2;
		height [0] = 0.400;		height [1] = 0.400;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (params.height - 0.750) < EPS) {
		nVerEuroform = 2;
		height [0] = 0.450;		height [1] = 0.300;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (params.height - 0.700) < EPS) {
		nVerEuroform = 2;
		height [0] = 0.400;		height [1] = 0.300;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (params.height - 0.650) < EPS) {
		nVerEuroform = 2;
		//height [0] = 0.450;		height [1] = 0.200;		height [2] = 0.0;		height [3] = 0.0;
		height [0] = 0.200;		height [1] = 0.450;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (params.height - 0.600) < EPS) {
		nVerEuroform = 1;
		height [0] = 0.600;		height [1] = 0.0;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (params.height - 0.500) < EPS) {
		nVerEuroform = 1;
		height [0] = 0.500;		height [1] = 0.0;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (params.height - 0.450) < EPS) {
		nVerEuroform = 1;
		height [0] = 0.450;		height [1] = 0.0;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (params.height - 0.400) < EPS) {
		nVerEuroform = 1;
		height [0] = 0.400;		height [1] = 0.0;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.050;
	} else {
		nVerEuroform = 0;
		height [0] = 0.0;		height [1] = 0.0;		height [2] = 0.0;		height [3] = 0.0;
	}

	if (abs (params.width - 6.000) < EPS) {
		nHorEuroform = 5;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 1.200;
		width [3] = 1.200;
		width [4] = 1.200;
	} else if (abs (params.width - 5.700) < EPS) {
		nHorEuroform = 5;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 1.200;
		width [3] = 1.200;
		width [4] = 0.900;
	} else if (abs (params.width - 5.400) < EPS) {
		nHorEuroform = 5;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 1.200;
		width [3] = 0.900;
		width [4] = 0.900;
	} else if (abs (params.width - 5.100) < EPS) {
		nHorEuroform = 5;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 1.200;
		width [3] = 0.900;
		width [4] = 0.600;
	} else if (abs (params.width - 4.800) < EPS) {
		nHorEuroform = 4;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 1.200;
		width [3] = 1.200;
		width [4] = 0.0;
	} else if (abs (params.width - 4.500) < EPS) {
		nHorEuroform = 4;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 1.200;
		width [3] = 0.900;
		width [4] = 0.0;
	} else if (abs (params.width - 4.200) < EPS) {
		nHorEuroform = 4;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 0.900;
		width [3] = 0.900;
		width [4] = 0.0;
	} else if (abs (params.width - 3.900) < EPS) {
		nHorEuroform = 4;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 0.900;
		width [3] = 0.600;
		width [4] = 0.0;
	} else if (abs (params.width - 3.600) < EPS) {
		nHorEuroform = 3;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 1.200;
		width [3] = 0.0;
		width [4] = 0.0;
	} else if (abs (params.width - 3.300) < EPS) {
		nHorEuroform = 3;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 0.900;
		width [3] = 0.0;
		width [4] = 0.0;
	} else if (abs (params.width - 3.000) < EPS) {
		nHorEuroform = 3;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 0.600;
		width [3] = 0.0;
		width [4] = 0.0;
	} else if (abs (params.width - 2.700) < EPS) {
		nHorEuroform = 3;
		width [0] = 1.200;
		width [1] = 0.900;
		width [2] = 0.600;
		width [3] = 0.0;
		width [4] = 0.0;
	} else if (abs (params.width - 2.400) < EPS) {
		nHorEuroform = 2;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 0.0;
		width [3] = 0.0;
		width [4] = 0.0;
	} else if (abs (params.width - 2.100) < EPS) {
		nHorEuroform = 2;
		width [0] = 1.200;
		width [1] = 0.900;
		width [2] = 0.0;
		width [3] = 0.0;
		width [4] = 0.0;
	} else if (abs (params.width - 1.800) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.900;
		width [1] = 0.900;
		width [2] = 0.0;
		width [3] = 0.0;
		width [4] = 0.0;
	} else if (abs (params.width - 1.500) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.900;
		width [1] = 0.600;
		width [2] = 0.0;
		width [3] = 0.0;
		width [4] = 0.0;
	} else {
		nHorEuroform = 0;
		width [0] = 0.0;
		width [1] = 0.0;
		width [2] = 0.0;
		width [3] = 0.0;
		width [4] = 0.0;
	}

	// �ʺ� ���̰� 0�̸� �ƹ��͵� ��ġ���� ����
	if ((nHorEuroform == 0) || (nVerEuroform == 0))
		return	NoError;

	//////////////////////////////////////////////////////////////// �����
	// ������ ��ġ
	euroform.leftBottomX = params.leftBottomX;
	euroform.leftBottomY = params.leftBottomY;
	euroform.leftBottomZ = params.leftBottomZ;
	euroform.ang = params.ang;
	euroform.eu_stan_onoff = true;
	euroform.u_ins_wall = false;
	euroform.ang_x = DegreeToRad (90.0);

	for (xx = 0 ; xx < nHorEuroform ; ++xx) {
		height_t = 0.0;
		for (yy = nVerEuroform-1 ; yy >= 0 ; --yy) {
			euroform.eu_wid = euroform.width	= height [yy];
			euroform.eu_hei = euroform.height	= width [xx];
			height_t += height [yy];
			elemList.Push (placeEuroform (euroform));
			moveIn3D ('z', euroform.ang, height [yy], &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
		}
		moveIn3D ('x', euroform.ang, width [xx], &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
		moveIn3D ('z', euroform.ang, -height_t, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
	}

	// ��� ������ (����) ��ġ
	sqrPipe.leftBottomX = params.leftBottomX;
	sqrPipe.leftBottomY = params.leftBottomY;
	sqrPipe.leftBottomZ = params.leftBottomZ;
	sqrPipe.ang = params.ang;
	sqrPipe.length = params.height - (verticalGap * 2);
	sqrPipe.pipeAng = DegreeToRad (90.0);

	moveIn3D ('z', sqrPipe.ang, verticalGap, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	moveIn3D ('y', sqrPipe.ang, -(0.0635 + 0.025), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	moveIn3D ('x', sqrPipe.ang, 0.150 - 0.031, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

	for (xx = 0 ; xx <= nHorEuroform ; ++xx) {
		if (xx == 0) {
			// 1��
			elemList.Push (placeSqrPipe (sqrPipe));
			moveIn3D ('x', sqrPipe.ang, 0.062, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
			elemList.Push (placeSqrPipe (sqrPipe));
			moveIn3D ('x', sqrPipe.ang, -0.031 - 0.150 + width [xx] - 0.031, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		} else if (xx == nHorEuroform) {
			// ������ ��
			moveIn3D ('x', sqrPipe.ang, -0.150, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
			elemList.Push (placeSqrPipe (sqrPipe));
			moveIn3D ('x', sqrPipe.ang, 0.062, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
			elemList.Push (placeSqrPipe (sqrPipe));
		} else {
			// ������ ��
			elemList.Push (placeSqrPipe (sqrPipe));
			moveIn3D ('x', sqrPipe.ang, 0.062, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
			elemList.Push (placeSqrPipe (sqrPipe));
			moveIn3D ('x', sqrPipe.ang, -0.031 + width [xx] - 0.031, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		}
	}

	// ��� ������ (����) ��ġ
	sqrPipe.leftBottomX = params.leftBottomX;
	sqrPipe.leftBottomY = params.leftBottomY;
	sqrPipe.leftBottomZ = params.leftBottomZ;
	sqrPipe.ang = params.ang;
	sqrPipe.length = params.width - 0.100;
	sqrPipe.pipeAng = DegreeToRad (0);

	height_t = 0.0;
	for (xx = nVerEuroform-1 ; xx >= 0 ; --xx) {
		height_t += height [xx];
	}
	moveIn3D ('z', sqrPipe.ang, height_t - height [0] + 0.150 + 0.035, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	moveIn3D ('y', sqrPipe.ang, -(0.0635 + 0.075), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	moveIn3D ('x', sqrPipe.ang, 0.050, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

	// 1��
	elemList.Push (placeSqrPipe (sqrPipe));	moveIn3D ('z', sqrPipe.ang, -0.070, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	elemList.Push (placeSqrPipe (sqrPipe));	moveIn3D ('z', sqrPipe.ang, 0.070 + (height [nVerEuroform-1] - 0.150) - params.height - (-height [0] + 0.150), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	// 2��
	elemList.Push (placeSqrPipe (sqrPipe));	moveIn3D ('z', sqrPipe.ang, -0.070, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	elemList.Push (placeSqrPipe (sqrPipe));

	// �ɺ�Ʈ ��ġ (���� - ���ϴ�, �ֻ��)
	pinbolt.leftBottomX = params.leftBottomX;
	pinbolt.leftBottomY = params.leftBottomY;
	pinbolt.leftBottomZ = params.leftBottomZ;
	pinbolt.ang = params.ang;
	pinbolt.bPinBoltRot90 = FALSE;
	pinbolt.boltLen = 0.100;
	pinbolt.angX = DegreeToRad (270.0);
	pinbolt.angY = DegreeToRad (0.0);

	moveIn3D ('y', pinbolt.ang, -(0.1635), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

	// ���ϴ� ��
	moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
	height_t = 0.0;
	for (xx = 0 ; xx < nVerEuroform ; ++xx) {
		height_t += height [xx];
	}
	moveIn3D ('z', pinbolt.ang, height_t, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
	height_t = 0.0;
	for (xx = 0 ; xx < nVerEuroform-1 ; ++xx) {
		height_t += height [xx];
		moveIn3D ('z', pinbolt.ang, -height [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

		elemList.Push (placePinbolt (pinbolt));
	}
	// �ֻ�� ��
	moveIn3D ('z', pinbolt.ang, height_t, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
	moveIn3D ('x', pinbolt.ang, params.width - 0.300, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
	for (xx = 0 ; xx < nVerEuroform-1 ; ++xx) {
		moveIn3D ('z', pinbolt.ang, -height [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

		elemList.Push (placePinbolt (pinbolt));
	}

	// �ɺ�Ʈ ��ġ (���� - ������)
	pinbolt.leftBottomX = params.leftBottomX;
	pinbolt.leftBottomY = params.leftBottomY;
	pinbolt.leftBottomZ = params.leftBottomZ;
	pinbolt.ang = params.ang;
	pinbolt.bPinBoltRot90 = TRUE;
	pinbolt.boltLen = 0.100;
	pinbolt.angX = DegreeToRad (270.0);
	pinbolt.angY = DegreeToRad (0.0);

	moveIn3D ('y', pinbolt.ang, -(0.1635), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

	// 2 ~ [n-1]��
	if (nVerEuroform >= 3) {
		height_t = 0.0;
		for (xx = 0 ; xx < nVerEuroform ; ++xx) {
			height_t += height [xx];
		}
		moveIn3D ('z', pinbolt.ang, height_t - 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		moveIn3D ('x', pinbolt.ang, width [0], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		for (xx = 1 ; xx < nHorEuroform ; ++xx) {
			height_t = 0.0;
			for (yy = 0 ; yy < nVerEuroform ; ++yy) {
				// 1��
				if (yy == 0) {
					elemList.Push (placePinbolt (pinbolt));
					moveIn3D ('z', pinbolt.ang, -(height [0] - 0.150), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
					height_t += height [0] - 0.150;
				// ������ ��
				} else if (yy == nVerEuroform - 1) {
					height_t += height [nVerEuroform-1] - 0.150;
					moveIn3D ('z', pinbolt.ang, -(height [nVerEuroform-1] - 0.150), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
					elemList.Push (placePinbolt (pinbolt));
				// ������ ��
				} else {
					height_t += height [yy];
					if (abs (height [yy] - 0.600) < EPS) {
						moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('z', pinbolt.ang, -0.300, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
					} else if (abs (height [yy] - 0.500) < EPS) {
						moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('z', pinbolt.ang, -0.200, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
					} else if (abs (height [yy] - 0.450) < EPS) {
						moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
					} else if (abs (height [yy] - 0.400) < EPS) {
						moveIn3D ('z', pinbolt.ang, -0.100, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('z', pinbolt.ang, -0.200, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('z', pinbolt.ang, -0.100, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
					} else if (abs (height [yy] - 0.300) < EPS) {
						moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
					} else if (abs (height [yy] - 0.200) < EPS) {
						moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('z', pinbolt.ang, -0.050, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
					}
				}
			}
			moveIn3D ('z', pinbolt.ang, height_t, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
			moveIn3D ('x', pinbolt.ang, width [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		}
	}

	// �ɺ�Ʈ ��ġ (����)
	pinbolt.leftBottomX = params.leftBottomX;
	pinbolt.leftBottomY = params.leftBottomY;
	pinbolt.leftBottomZ = params.leftBottomZ;
	pinbolt.ang = params.ang;
	pinbolt.bPinBoltRot90 = TRUE;
	pinbolt.boltLen = 0.150;
	pinbolt.angX = DegreeToRad (270.0);
	pinbolt.angY = DegreeToRad (0.0);

	height_t = 0.0;
	for (xx = 0 ; xx < nVerEuroform ; ++xx) {
		height_t += height [xx];
	}
	moveIn3D ('z', pinbolt.ang, height_t - (height [0] - 0.150), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
	moveIn3D ('y', pinbolt.ang, -(0.2135), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
	moveIn3D ('x', pinbolt.ang, width [0], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

	// 1��
	width_t = 0.0;
	for (xx = 1 ; xx < nHorEuroform ; ++xx) {
		elemList.Push (placePinbolt (pinbolt));
		moveIn3D ('x', pinbolt.ang, width [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		width_t += width [xx];
	}
	// 2��
	moveIn3D ('z', pinbolt.ang, (height [0] - 0.150) - params.height - (-height [nVerEuroform-1] + 0.150), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
	moveIn3D ('x', pinbolt.ang, -width_t, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
	for (xx = 1 ; xx < nHorEuroform ; ++xx) {
		elemList.Push (placePinbolt (pinbolt));
		moveIn3D ('x', pinbolt.ang, width [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		width_t += width [xx];
	}

	// ��ü Ÿ��
	//if (bDoubleSide == true) {
	//	walltie.leftBottomX = params.leftBottomX;
	//	walltie.leftBottomY = params.leftBottomY;
	//	walltie.leftBottomZ = params.leftBottomZ;
	//	walltie.ang = params.ang;
	//	remainder = fmod ((wallThk + 0.327), 0.100);
	//	walltie.boltLen = (wallThk + 0.327 + (0.100 - remainder));
	//	walltie.pipeBeg = 0.0365 + 0.1635;
	//	walltie.pipeEnd = 0.0365 + 0.1635 + wallThk;
	//	walltie.clampBeg = 0.0365;
	//	walltie.clampEnd = 0.0365 + wallThk + 0.327;

	//	height_t = 0.0;
	//	for (xx = 0 ; xx < nVerEuroform ; ++xx) {
	//		height_t += height [xx];
	//	}
	//	moveIn3D ('z', walltie.ang, height_t - (height [0] - 0.150), &walltie.leftBottomX, &walltie.leftBottomY, &walltie.leftBottomZ);
	//	moveIn3D ('y', walltie.ang, -(0.1635 + 0.0365), &walltie.leftBottomX, &walltie.leftBottomY, &walltie.leftBottomZ);
	//	moveIn3D ('x', walltie.ang, 0.350, &walltie.leftBottomX, &walltie.leftBottomY, &walltie.leftBottomZ);

	//	for (xx = 0 ; xx < 2 ; ++xx) {
	//		for (yy = 0 ; yy < nHorEuroform ; ++yy) {
	//			// ������ ��
	//			if (yy == 0) {
	//				elemList.Push (placeWalltie (walltie));
	//				moveIn3D ('x', walltie.ang, width [yy], &walltie.leftBottomX, &walltie.leftBottomY, &walltie.leftBottomZ);
	//	
	//			// �ֻ��� ��
	//			} else if (yy == nHorEuroform - 1) {
	//				moveIn3D ('x', walltie.ang, width [yy] - 0.350*2, &walltie.leftBottomX, &walltie.leftBottomY, &walltie.leftBottomZ);
	//				elemList.Push (placeWalltie (walltie));
	//				moveIn3D ('z', walltie.ang, (height [0] - 0.150) - params.height - (-height [nVerEuroform-1] + 0.150), &walltie.leftBottomX, &walltie.leftBottomY, &walltie.leftBottomZ);
	//				moveIn3D ('x', walltie.ang, (0.350 - params.width + 0.350), &walltie.leftBottomX, &walltie.leftBottomY, &walltie.leftBottomZ);
	//			
	//			// 2 ~ [n-1]��
	//			} else {
	//				elemList.Push (placeWalltie (walltie));
	//				moveIn3D ('x', walltie.ang, width [yy], &walltie.leftBottomX, &walltie.leftBottomY, &walltie.leftBottomZ);
	//			}
	//		}
	//	}
	//}

	// ��� �ǽ�
	headpiece.leftBottomX = params.leftBottomX;
	headpiece.leftBottomY = params.leftBottomY;
	headpiece.leftBottomZ = params.leftBottomZ;
	headpiece.ang = params.ang;

	height_t = 0.0;
	for (xx = 0 ; xx < nVerEuroform ; ++xx) {
		height_t += height [xx];
	}
	width_t = 0.0;
	for (xx = 0 ; xx < nHorEuroform ; ++xx) {
		width_t += width [xx];
	}
	moveIn3D ('z', headpiece.ang, height_t - (height [0] - 0.150 - 0.100) - 0.200, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	moveIn3D ('y', headpiece.ang, -0.1725, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	moveIn3D ('x', headpiece.ang, 0.600, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);

	// ó�� ��
	elemList.Push (placeHeadpiece_hor (headpiece));
	moveIn3D ('z', headpiece.ang, (height [0] - 0.150) - params.height - (-height [nVerEuroform-1] + 0.150), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	elemList.Push (placeHeadpiece_hor (headpiece));
	moveIn3D ('z', headpiece.ang, -(height [nVerEuroform-1] - 0.150) + params.height + (-height [0] + 0.150), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	elev_headpiece = width_t - 0.800;
	moveIn3D ('x', headpiece.ang, -0.600 + elev_headpiece, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	// ������ ��
	elemList.Push (placeHeadpiece_hor (headpiece));
	moveIn3D ('z', headpiece.ang, (height [nVerEuroform-1] - 0.150) - params.height - (-height [0] + 0.150), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	elemList.Push (placeHeadpiece_hor (headpiece));

	// ����ö��
	fittings.leftBottomX = params.leftBottomX;
	fittings.leftBottomY = params.leftBottomY;
	fittings.leftBottomZ = params.leftBottomZ;
	fittings.ang = params.ang;

	fittings.angX = DegreeToRad (0.0);
	fittings.angY = DegreeToRad (0.0);
	fittings.bolt_len = 0.150;
	fittings.bolt_dia = 0.012;
	fittings.bWasher1 = true;
	fittings.bWasher2 = true;
	fittings.washer_pos1 = 0.0;
	fittings.washer_pos2 = 0.108;
	fittings.washer_size = 0.100;
	strncpy (fittings.nutType, "������Ʈ", strlen ("������Ʈ"));

	height_t = 0.0;
	for (xx = 0 ; xx < nVerEuroform ; ++xx) {
		height_t += height [xx];
	}
	moveIn3D ('z', fittings.ang, height_t - (height [0] - 0.150), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	moveIn3D ('y', fittings.ang, -0.0455, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	moveIn3D ('x', fittings.ang, 0.150, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);

	// ó�� ��
	fittings.ang += DegreeToRad (180.0);
	elemList.Push (placeFittings (fittings));
	fittings.ang -= DegreeToRad (180.0);
	moveIn3D ('z', fittings.ang, (height [0] - 0.150) - params.height - (-height [nVerEuroform-1] + 0.150), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	fittings.ang += DegreeToRad (180.0);
	elemList.Push (placeFittings (fittings));
	fittings.ang -= DegreeToRad (180.0);
	moveIn3D ('z', fittings.ang, -(height [nVerEuroform-1] - 0.150) + params.height + (-height [0] + 0.150), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	moveIn3D ('x', fittings.ang, params.width - 0.300, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);

	// ������ ��
	fittings.ang += DegreeToRad (180.0);
	elemList.Push (placeFittings (fittings));
	fittings.ang -= DegreeToRad (180.0);
	moveIn3D ('z', fittings.ang, (height [nVerEuroform-1] - 0.150) - params.height - (-height [0] + 0.150), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	fittings.ang += DegreeToRad (180.0);
	elemList.Push (placeFittings (fittings));
	fittings.ang -= DegreeToRad (180.0);

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

	//////////////////////////////////////////////////////////////// �ݴ��
	if (bDoubleSide == true) {
		moveIn3D ('x', params.ang, params.width, &params.leftBottomX, &params.leftBottomY, &params.leftBottomY);
		moveIn3D ('y', params.ang, wallThk, &params.leftBottomX, &params.leftBottomY, &params.leftBottomY);
		params.ang += DegreeToRad (180.0);

		// ������ ��ġ (�ݴ����� �����)
		euroform.leftBottomX = params.leftBottomX;
		euroform.leftBottomY = params.leftBottomY;
		euroform.leftBottomZ = params.leftBottomZ;
		euroform.ang = params.ang;

		for (xx = nHorEuroform-1 ; xx >= 0 ; --xx) {
			height_t = 0.0;
			for (yy = nVerEuroform-1 ; yy >= 0 ; --yy) {
				euroform.eu_wid = euroform.width	= height [yy];
				euroform.eu_hei = euroform.height	= width [xx];
				height_t += height [yy];
				elemList.Push (placeEuroform (euroform));
				moveIn3D ('z', euroform.ang, height [yy], &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
			}
			moveIn3D ('x', euroform.ang, width [xx], &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
			moveIn3D ('z', euroform.ang, -height_t, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
		}

		// ��� ������ (����) ��ġ
		sqrPipe.leftBottomX = params.leftBottomX;
		sqrPipe.leftBottomY = params.leftBottomY;
		sqrPipe.leftBottomZ = params.leftBottomZ;
		sqrPipe.ang = params.ang;
		sqrPipe.length = params.height - (verticalGap * 2);
		sqrPipe.pipeAng = DegreeToRad (90.0);

		moveIn3D ('z', sqrPipe.ang, verticalGap, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		moveIn3D ('y', sqrPipe.ang, -(0.0635 + 0.025), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		moveIn3D ('x', sqrPipe.ang, params.width - (0.150 - 0.031), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

		for (xx = 0 ; xx <= nHorEuroform ; ++xx) {
			if (xx == 0) {
				// 1��
				elemList.Push (placeSqrPipe (sqrPipe));
				moveIn3D ('x', sqrPipe.ang, -0.062, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
				elemList.Push (placeSqrPipe (sqrPipe));
				moveIn3D ('x', sqrPipe.ang, 0.031 + 0.150 - width [xx] + 0.031, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
			} else if (xx == nHorEuroform) {
				// ������ ��
				moveIn3D ('x', sqrPipe.ang, 0.150, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
				elemList.Push (placeSqrPipe (sqrPipe));
				moveIn3D ('x', sqrPipe.ang, -0.062, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
				elemList.Push (placeSqrPipe (sqrPipe));
			} else {
				// ������ ��
				elemList.Push (placeSqrPipe (sqrPipe));
				moveIn3D ('x', sqrPipe.ang, -0.062, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
				elemList.Push (placeSqrPipe (sqrPipe));
				moveIn3D ('x', sqrPipe.ang, 0.031 - width [xx] + 0.031, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
			}
		}

		// ��� ������ (����) ��ġ
		sqrPipe.leftBottomX = params.leftBottomX;
		sqrPipe.leftBottomY = params.leftBottomY;
		sqrPipe.leftBottomZ = params.leftBottomZ;
		sqrPipe.ang = params.ang;
		sqrPipe.length = params.width - 0.100;
		sqrPipe.pipeAng = DegreeToRad (0);

		height_t = 0.0;
		for (xx = nVerEuroform-1 ; xx >= 0 ; --xx) {
			height_t += height [xx];
		}
		moveIn3D ('z', sqrPipe.ang, height_t - height [0] + 0.150 + 0.035, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		moveIn3D ('y', sqrPipe.ang, -(0.0635 + 0.075), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		moveIn3D ('x', sqrPipe.ang, 0.050, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

		// 1��
		elemList.Push (placeSqrPipe (sqrPipe));	moveIn3D ('z', sqrPipe.ang, -0.070, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		elemList.Push (placeSqrPipe (sqrPipe));	moveIn3D ('z', sqrPipe.ang, 0.070 + (height [nVerEuroform-1] - 0.150) - params.height - (-height [0] + 0.150), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		// 2��
		elemList.Push (placeSqrPipe (sqrPipe));	moveIn3D ('z', sqrPipe.ang, -0.070, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		elemList.Push (placeSqrPipe (sqrPipe));

		// �ɺ�Ʈ ��ġ (���� - ���ϴ�, �ֻ��)
		pinbolt.leftBottomX = params.leftBottomX;
		pinbolt.leftBottomY = params.leftBottomY;
		pinbolt.leftBottomZ = params.leftBottomZ;
		pinbolt.ang = params.ang;
		pinbolt.bPinBoltRot90 = FALSE;
		pinbolt.boltLen = 0.100;
		pinbolt.angX = DegreeToRad (270.0);
		pinbolt.angY = DegreeToRad (0.0);

		moveIn3D ('y', pinbolt.ang, -(0.1635), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

		// ���ϴ� ��
		moveIn3D ('x', pinbolt.ang, params.width - 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		height_t = 0.0;
		for (xx = 0 ; xx < nVerEuroform ; ++xx) {
			height_t += height [xx];
		}
		moveIn3D ('z', pinbolt.ang, height_t, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		height_t = 0.0;
		for (xx = 0 ; xx < nVerEuroform-1 ; ++xx) {
			height_t += height [xx];
			moveIn3D ('z', pinbolt.ang, -height [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

			elemList.Push (placePinbolt (pinbolt));
		}
		// �ֻ�� ��
		moveIn3D ('z', pinbolt.ang, height_t, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		moveIn3D ('x', pinbolt.ang, -params.width + 0.300, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		for (xx = 0 ; xx < nVerEuroform-1 ; ++xx) {
			moveIn3D ('z', pinbolt.ang, -height [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

			elemList.Push (placePinbolt (pinbolt));
		}

		// �ɺ�Ʈ ��ġ (���� - ������)
		pinbolt.leftBottomX = params.leftBottomX;
		pinbolt.leftBottomY = params.leftBottomY;
		pinbolt.leftBottomZ = params.leftBottomZ;
		pinbolt.ang = params.ang;
		pinbolt.bPinBoltRot90 = TRUE;
		pinbolt.boltLen = 0.100;
		pinbolt.angX = DegreeToRad (270.0);
		pinbolt.angY = DegreeToRad (0.0);

		moveIn3D ('y', pinbolt.ang, -(0.1635), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

		// 2 ~ [n-1]��
		if (nVerEuroform >= 3) {
			height_t = 0.0;
			for (xx = 0 ; xx < nVerEuroform ; ++xx) {
				height_t += height [xx];
			}
			moveIn3D ('z', pinbolt.ang, height_t - 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
			moveIn3D ('x', pinbolt.ang, params.width - width [0], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
			for (xx = 1 ; xx < nHorEuroform ; ++xx) {
				height_t = 0.0;
				for (yy = 0 ; yy < nVerEuroform ; ++yy) {
					// 1��
					if (yy == 0) {
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('z', pinbolt.ang, -(height [0] - 0.150), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						height_t += height [0] - 0.150;
					// ������ ��
					} else if (yy == nVerEuroform - 1) {
						height_t += height [nVerEuroform-1] - 0.150;
						moveIn3D ('z', pinbolt.ang, -(height [nVerEuroform-1] - 0.150), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
					// ������ ��
					} else {
						height_t += height [yy];
						if (abs (height [yy] - 0.600) < EPS) {
							moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('z', pinbolt.ang, -0.300, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						} else if (abs (height [yy] - 0.500) < EPS) {
							moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('z', pinbolt.ang, -0.200, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						} else if (abs (height [yy] - 0.450) < EPS) {
							moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						} else if (abs (height [yy] - 0.400) < EPS) {
							moveIn3D ('z', pinbolt.ang, -0.100, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('z', pinbolt.ang, -0.200, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('z', pinbolt.ang, -0.100, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						} else if (abs (height [yy] - 0.300) < EPS) {
							moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						} else if (abs (height [yy] - 0.200) < EPS) {
							moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('z', pinbolt.ang, -0.050, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						}
					}
				}
				moveIn3D ('z', pinbolt.ang, height_t, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
				moveIn3D ('x', pinbolt.ang, -width [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
			}
		}

		// �ɺ�Ʈ ��ġ (����)
		pinbolt.leftBottomX = params.leftBottomX;
		pinbolt.leftBottomY = params.leftBottomY;
		pinbolt.leftBottomZ = params.leftBottomZ;
		pinbolt.ang = params.ang;
		pinbolt.bPinBoltRot90 = TRUE;
		pinbolt.boltLen = 0.150;
		pinbolt.angX = DegreeToRad (270.0);
		pinbolt.angY = DegreeToRad (0.0);

		height_t = 0.0;
		for (xx = 0 ; xx < nVerEuroform ; ++xx) {
			height_t += height [xx];
		}
		moveIn3D ('z', pinbolt.ang, height_t - (height [0] - 0.150), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		moveIn3D ('y', pinbolt.ang, -(0.2135), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		moveIn3D ('x', pinbolt.ang, params.width - width [0], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

		// 1��
		width_t = 0.0;
		for (xx = 1 ; xx < nHorEuroform ; ++xx) {
			elemList.Push (placePinbolt (pinbolt));
			moveIn3D ('x', pinbolt.ang, -width [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
			width_t += width [xx];
		}
		// 2��
		moveIn3D ('z', pinbolt.ang, (height [0] - 0.150) - params.height - (-height [nVerEuroform-1] + 0.150), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		moveIn3D ('x', pinbolt.ang, width_t, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		for (xx = 1 ; xx < nHorEuroform ; ++xx) {
			elemList.Push (placePinbolt (pinbolt));
			moveIn3D ('x', pinbolt.ang, -width [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
			width_t += width [xx];
		}

		// ��ü Ÿ�� (����鿡�� �����Ƿ� ����)

		// ��� �ǽ�
		headpiece.leftBottomX = params.leftBottomX;
		headpiece.leftBottomY = params.leftBottomY;
		headpiece.leftBottomZ = params.leftBottomZ;
		headpiece.ang = params.ang;

		height_t = 0.0;
		for (xx = 0 ; xx < nVerEuroform ; ++xx) {
			height_t += height [xx];
		}
		width_t = 0.0;
		for (xx = 0 ; xx < nHorEuroform ; ++xx) {
			width_t += width [xx];
		}
		moveIn3D ('z', headpiece.ang, height_t - (height [0] - 0.150 - 0.100) - 0.200, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		moveIn3D ('y', headpiece.ang, -0.1725, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		moveIn3D ('x', headpiece.ang, params.width - 0.800, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);

		// ó�� ��
		elemList.Push (placeHeadpiece_hor (headpiece));
		moveIn3D ('z', headpiece.ang, (height [0] - 0.150) - params.height - (-height [nVerEuroform-1] + 0.150), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		elemList.Push (placeHeadpiece_hor (headpiece));
		moveIn3D ('z', headpiece.ang, -(height [nVerEuroform-1] - 0.150) + params.height + (-height [0] + 0.150), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		elev_headpiece = width_t - 0.800;
		moveIn3D ('x', headpiece.ang, 0.600 - elev_headpiece, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		// ������ ��
		elemList.Push (placeHeadpiece_hor (headpiece));
		moveIn3D ('z', headpiece.ang, (height [nVerEuroform-1] - 0.150) - params.height - (-height [0] + 0.150), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		elemList.Push (placeHeadpiece_hor (headpiece));

		// ����ö��
		fittings.leftBottomX = params.leftBottomX;
		fittings.leftBottomY = params.leftBottomY;
		fittings.leftBottomZ = params.leftBottomZ;
		fittings.ang = params.ang;

		fittings.angX = DegreeToRad (0.0);
		fittings.angY = DegreeToRad (0.0);
		fittings.bolt_len = 0.150;
		fittings.bolt_dia = 0.012;
		fittings.bWasher1 = true;
		fittings.bWasher2 = true;
		fittings.washer_pos1 = 0.0;
		fittings.washer_pos2 = 0.108;
		fittings.washer_size = 0.100;
		strncpy (fittings.nutType, "������Ʈ", strlen ("������Ʈ"));

		height_t = 0.0;
		for (xx = 0 ; xx < nVerEuroform ; ++xx) {
			height_t += height [xx];
		}
		moveIn3D ('z', fittings.ang, height_t - (height [0] - 0.150), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		moveIn3D ('y', fittings.ang, -0.0455, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		moveIn3D ('x', fittings.ang, params.width - 0.150, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);

		// ó�� ��
		fittings.ang += DegreeToRad (180.0);
		elemList.Push (placeFittings (fittings));
		fittings.ang -= DegreeToRad (180.0);
		moveIn3D ('z', fittings.ang, (height [0] - 0.150) - params.height - (-height [nVerEuroform-1] + 0.150), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		fittings.ang += DegreeToRad (180.0);
		elemList.Push (placeFittings (fittings));
		fittings.ang -= DegreeToRad (180.0);
		moveIn3D ('z', fittings.ang, -(height [nVerEuroform-1] - 0.150) + params.height + (-height [0] + 0.150), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		moveIn3D ('x', fittings.ang, -params.width + 0.300, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);

		// ������ ��
		fittings.ang += DegreeToRad (180.0);
		elemList.Push (placeFittings (fittings));
		fittings.ang -= DegreeToRad (180.0);
		moveIn3D ('z', fittings.ang, (height [nVerEuroform-1] - 0.150) - params.height - (-height [0] + 0.150), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		fittings.ang += DegreeToRad (180.0);
		elemList.Push (placeFittings (fittings));
		fittings.ang -= DegreeToRad (180.0);

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
	}

	return	err;
}

// ���̺���(��) ��ġ (�������) Ÿ��2
GSErrCode	placeTableformOnWall_portrait_Type2 (WallTableform params)
{
	GSErrCode	err = NoError;

	short	nHorEuroform;			// ���� ���� ������ ����
	short	nVerEuroform;			// ���� ���� ������ ����
	double	width [7];				// ���� ���� �� ������ �ʺ�
	double	height [7];				// ���� ���� �� ������ ����

	short	nVerticalBar;
	double	verticalBarLeftOffset;
	double	verticalBarRightOffset;

	short		xx, yy;
	double		width_t, height_t;
	double		elev_headpiece;
	double		horizontalGap = 0.050;	// ������ ���� �̰ݰŸ�
	API_Guid	tempGuid;
	Cylinder	cylinder;

	Euroform		euroform;
	SquarePipe		sqrPipe;
	HeadpieceOfPushPullProps	headpiece;
	MetalFittings	fittings;
	EuroformHook	hook;
	RectPipeHanger	hanger;

	nHorEuroform = 0;
	nVerEuroform = 0;
	for (xx = 0 ; xx < 7 ; ++xx) {
		width [xx] = 0.0;
		height [xx] = 0.0;
	}

	if (abs (params.width - 2.300) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.600;	width [2] = 0.500;	width [3] = 0.600;
		horizontalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 2.250) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.600;	width [2] = 0.450;	width [3] = 0.600;
		horizontalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 2.200) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.600;	width [2] = 0.400;	width [3] = 0.600;
		horizontalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 2.150) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.500;	width [2] = 0.450;	width [3] = 0.600;
		horizontalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 2.100) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.600;	width [2] = 0.300;	width [3] = 0.600;
		horizontalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 2.050) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.450;	width [2] = 0.400;	width [3] = 0.600;
		horizontalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 2.000) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.600;	width [2] = 0.200;	width [3] = 0.600;
		horizontalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.950) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.450;	width [2] = 0.300;	width [3] = 0.600;
		horizontalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.900) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.500;	width [2] = 0.200;	width [3] = 0.600;
		horizontalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.850) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.450;	width [2] = 0.200;	width [3] = 0.600;
		horizontalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.800) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.600;	width [1] = 0.600;	width [2] = 0.600;	width [3] = 0.0;
		horizontalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.750) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.200;	width [2] = 0.450;	width [3] = 0.500;
		horizontalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.700) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.600;	width [1] = 0.500;	width [2] = 0.600;	width [3] = 0.0;
		horizontalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.650) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.600;	width [1] = 0.450;	width [2] = 0.600;	width [3] = 0.0;
		horizontalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.600) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.600;	width [1] = 0.400;	width [2] = 0.600;	width [3] = 0.0;
		horizontalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.550) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.600;	width [1] = 0.450;	width [2] = 0.500;	width [3] = 0.0;
		horizontalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.500) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.600;	width [1] = 0.300;	width [2] = 0.600;	width [3] = 0.0;
		horizontalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.450) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.500;	width [1] = 0.450;	width [2] = 0.500;	width [3] = 0.0;
		horizontalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.400) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.500;	width [1] = 0.400;	width [2] = 0.500;	width [3] = 0.0;
		horizontalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.350) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.600;	width [1] = 0.300;	width [2] = 0.450;	width [3] = 0.0;
		horizontalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.300) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.600;	width [1] = 0.200;	width [2] = 0.500;	width [3] = 0.0;
		horizontalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.250) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.600;	width [1] = 0.200;	width [2] = 0.450;	width [3] = 0.0;
		horizontalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.200) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.600;	width [1] = 0.600;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.150) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.450;	width [1] = 0.300;	width [2] = 0.400;	width [3] = 0.0;
		horizontalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.100) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.400;	width [1] = 0.300;	width [2] = 0.400;	width [3] = 0.0;
		horizontalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.050) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.450;	width [1] = 0.300;	width [2] = 0.300;	width [3] = 0.0;
		horizontalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.000) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.600;	width [1] = 0.400;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 0.950) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.450;	width [1] = 0.500;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 0.900) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.600;	width [1] = 0.300;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 0.850) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.400;	width [1] = 0.450;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 0.800) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.400;	width [1] = 0.400;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 0.750) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.450;	width [1] = 0.300;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.200;
	} else if (abs (params.width - 0.700) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.400;	width [1] = 0.300;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.150;
	} else if (abs (params.width - 0.650) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.450;	width [1] = 0.200;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.200;
		verticalBarRightOffset = 0.150;
	} else if (abs (params.width - 0.600) < EPS) {
		nHorEuroform = 1;
		width [0] = 0.600;	width [1] = 0.0;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.150;
		verticalBarRightOffset = 0.150;
	} else if (abs (params.width - 0.500) < EPS) {
		nHorEuroform = 1;
		width [0] = 0.500;	width [1] = 0.0;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.050;
		nVerticalBar = 1;
		verticalBarLeftOffset = 0.200;
		verticalBarRightOffset = 0.200;
	} else if (abs (params.width - 0.450) < EPS) {
		nHorEuroform = 1;
		width [0] = 0.450;	width [1] = 0.0;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.025;
		nVerticalBar = 1;
		verticalBarLeftOffset = 0.200;
		verticalBarRightOffset = 0.200;
	} else if (abs (params.width - 0.400) < EPS) {
		nHorEuroform = 1;
		width [0] = 0.400;	width [1] = 0.0;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.050;
		nVerticalBar = 1;
		verticalBarLeftOffset = 0.150;
		verticalBarRightOffset = 0.150;
	} else {
		nHorEuroform = 0;
		width [0] = 0.0;	width [1] = 0.0;	width [2] = 0.0;	width [3] = 0.0;
		nVerticalBar = 0;
		verticalBarLeftOffset = 0.0;
		verticalBarRightOffset = 0.0;
	}

	if (abs (params.height - 6.000) < EPS) {
		nVerEuroform = 5;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 1.200;
		height [3] = 1.200;
		height [4] = 1.200;
	} else if (abs (params.height - 5.700) < EPS) {
		nVerEuroform = 5;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 1.200;
		height [3] = 1.200;
		height [4] = 0.900;
	} else if (abs (params.height - 5.400) < EPS) {
		nVerEuroform = 5;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 1.200;
		height [3] = 0.900;
		height [4] = 0.900;
	} else if (abs (params.height - 5.100) < EPS) {
		nVerEuroform = 5;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 1.200;
		height [3] = 0.900;
		height [4] = 0.600;
	} else if (abs (params.height - 4.800) < EPS) {
		nVerEuroform = 4;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 1.200;
		height [3] = 1.200;
		height [4] = 0.0;
	} else if (abs (params.height - 4.500) < EPS) {
		nVerEuroform = 4;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 1.200;
		height [3] = 0.900;
		height [4] = 0.0;
	} else if (abs (params.height - 4.200) < EPS) {
		nVerEuroform = 4;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 0.900;
		height [3] = 0.900;
		height [4] = 0.0;
	} else if (abs (params.height - 3.900) < EPS) {
		nVerEuroform = 4;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 0.900;
		height [3] = 0.600;
		height [4] = 0.0;
	} else if (abs (params.height - 3.600) < EPS) {
		nVerEuroform = 3;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 1.200;
		height [3] = 0.0;
		height [4] = 0.0;
	} else if (abs (params.height - 3.300) < EPS) {
		nVerEuroform = 3;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 0.900;
		height [3] = 0.0;
		height [4] = 0.0;
	} else if (abs (params.height - 3.000) < EPS) {
		nVerEuroform = 3;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 0.600;
		height [3] = 0.0;
		height [4] = 0.0;
	} else if (abs (params.height - 2.700) < EPS) {
		nVerEuroform = 3;
		height [0] = 1.200;
		height [1] = 0.900;
		height [2] = 0.600;
		height [3] = 0.0;
		height [4] = 0.0;
	} else if (abs (params.height - 2.400) < EPS) {
		nVerEuroform = 2;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 0.0;
		height [3] = 0.0;
		height [4] = 0.0;
	} else if (abs (params.height - 2.100) < EPS) {
		nVerEuroform = 2;
		height [0] = 1.200;
		height [1] = 0.900;
		height [2] = 0.0;
		height [3] = 0.0;
		height [4] = 0.0;
	} else if (abs (params.height - 1.800) < EPS) {
		nVerEuroform = 2;
		height [0] = 0.900;
		height [1] = 0.900;
		height [2] = 0.0;
		height [3] = 0.0;
		height [4] = 0.0;
	} else if (abs (params.height - 1.500) < EPS) {
		nVerEuroform = 2;
		height [0] = 0.900;
		height [1] = 0.600;
		height [2] = 0.0;
		height [3] = 0.0;
		height [4] = 0.0;
	} else {
		nVerEuroform = 0;
		height [0] = 0.0;
		height [1] = 0.0;
		height [2] = 0.0;
		height [3] = 0.0;
		height [4] = 0.0;
	}

	// �ʺ� ���̰� 0�̸� �ƹ��͵� ��ġ���� ����
	if ((nHorEuroform == 0) || (nVerEuroform == 0))
		return	NoError;

	//////////////////////////////////////////////////////////////// �����
	// ������ ��ġ
	euroform.leftBottomX = params.leftBottomX;
	euroform.leftBottomY = params.leftBottomY;
	euroform.leftBottomZ = params.leftBottomZ;
	euroform.ang = params.ang;
	euroform.eu_stan_onoff = true;
	euroform.u_ins_wall = true;
	euroform.ang_x = DegreeToRad (90.0);

	for (xx = 0 ; xx < nHorEuroform ; ++xx) {
		height_t = 0.0;
		for (yy = 0 ; yy < nVerEuroform ; ++yy) {
			euroform.eu_wid	= euroform.width = width [xx];
			euroform.eu_hei	= euroform.height = height [yy];
			height_t += height [yy];
			elemList.Push (placeEuroform (euroform));
			moveIn3D ('z', euroform.ang, height [yy], &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
		}
		moveIn3D ('x', euroform.ang, width [xx], &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
		moveIn3D ('z', euroform.ang, -height_t, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
	}

	// ��� ������ (����) ��ġ
	sqrPipe.leftBottomX = params.leftBottomX;
	sqrPipe.leftBottomY = params.leftBottomY;
	sqrPipe.leftBottomZ = params.leftBottomZ;
	sqrPipe.ang = params.ang;
	sqrPipe.length = params.width - (horizontalGap * 2);
	sqrPipe.pipeAng = DegreeToRad (0);

	moveIn3D ('x', sqrPipe.ang, horizontalGap, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	moveIn3D ('y', sqrPipe.ang, -(0.0635 + 0.025), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	moveIn3D ('z', sqrPipe.ang, 0.150 + 0.031, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

	cylinder.angleFromPlane = DegreeToRad (90.0);
	cylinder.length = 0.050;
	cylinder.radius = 0.013/2;
	cylinder.ang = params.ang;
	cylinder.leftBottomX = sqrPipe.leftBottomX;
	cylinder.leftBottomY = sqrPipe.leftBottomY;
	cylinder.leftBottomZ = sqrPipe.leftBottomZ;

	for (xx = 0 ; xx <= nVerEuroform ; ++xx) {
		if (xx == 0) {
			// 1��
			tempGuid = placeSqrPipe (sqrPipe);
			elemList.Push (tempGuid);
			cylinder.leftBottomX = sqrPipe.leftBottomX;
			cylinder.leftBottomY = sqrPipe.leftBottomY;
			cylinder.leftBottomZ = sqrPipe.leftBottomZ;
			moveIn3D ('z', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (yy = 0 ; yy < 6 ; ++yy) {
				moveIn3D ('x', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHole (tempGuid, cylinder));
			}
			moveIn3D ('x', cylinder.ang, -0.300 + params.width - (horizontalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (yy = 0 ; yy < 6 ; ++yy) {
				moveIn3D ('x', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHole (tempGuid, cylinder));
			}
			moveIn3D ('z', sqrPipe.ang, -0.031 - 0.150 + height [xx], &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
			moveIn3D ('x', cylinder.ang, 0.300 - params.width + (horizontalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
		} else if (xx == nVerEuroform) {
			// ������ ��
			moveIn3D ('z', sqrPipe.ang, -0.150 + 0.031, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
			tempGuid = placeSqrPipe (sqrPipe);
			elemList.Push (tempGuid);
			cylinder.leftBottomX = sqrPipe.leftBottomX;
			cylinder.leftBottomY = sqrPipe.leftBottomY;
			cylinder.leftBottomZ = sqrPipe.leftBottomZ;
			moveIn3D ('z', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (yy = 0 ; yy < 6 ; ++yy) {
				moveIn3D ('x', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHole (tempGuid, cylinder));
			}
			moveIn3D ('x', cylinder.ang, -0.300 + params.width - (horizontalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (yy = 0 ; yy < 6 ; ++yy) {
				moveIn3D ('x', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHole (tempGuid, cylinder));
			}
		} else {
			// ������ ��
			tempGuid = placeSqrPipe (sqrPipe);
			elemList.Push (tempGuid);
			cylinder.leftBottomX = sqrPipe.leftBottomX;
			cylinder.leftBottomY = sqrPipe.leftBottomY;
			cylinder.leftBottomZ = sqrPipe.leftBottomZ;
			moveIn3D ('z', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (yy = 0 ; yy < 6 ; ++yy) {
				moveIn3D ('x', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHole (tempGuid, cylinder));
			}
			moveIn3D ('x', cylinder.ang, -0.300 + params.width - (horizontalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (yy = 0 ; yy < 6 ; ++yy) {
				moveIn3D ('x', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHole (tempGuid, cylinder));
			}
			moveIn3D ('z', sqrPipe.ang, height [xx], &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
			moveIn3D ('x', cylinder.ang, 0.300 - params.width + (horizontalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
		}
	}

	// ��� ������ (����) ��ġ
	sqrPipe.leftBottomX = params.leftBottomX;
	sqrPipe.leftBottomY = params.leftBottomY;
	sqrPipe.leftBottomZ = params.leftBottomZ;
	sqrPipe.ang = params.ang;
	sqrPipe.length = params.height - 0.100;
	sqrPipe.pipeAng = DegreeToRad (90);

	moveIn3D ('x', sqrPipe.ang, horizontalGap + verticalBarLeftOffset, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	moveIn3D ('y', sqrPipe.ang, -(0.0635 + 0.075), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	moveIn3D ('z', sqrPipe.ang, 0.050, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

	// 1��
	cylinder.angleFromPlane = DegreeToRad (0.0);
	cylinder.length = 0.050;
	cylinder.radius = 0.013/2;
	cylinder.ang = params.ang;
	cylinder.leftBottomX = sqrPipe.leftBottomX;
	cylinder.leftBottomY = sqrPipe.leftBottomY;
	cylinder.leftBottomZ = sqrPipe.leftBottomZ;

	tempGuid = placeSqrPipe (sqrPipe);
	elemList.Push (tempGuid);
	moveIn3D ('x', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
	for (xx = 0 ; xx < 6 ; ++xx) {
		moveIn3D ('z', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
		elemList.Push (placeHole (tempGuid, cylinder));
	}
	moveIn3D ('z', cylinder.ang, -0.300 + params.height - 0.100, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
	for (xx = 0 ; xx < 6 ; ++xx) {
		moveIn3D ('z', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
		elemList.Push (placeHole (tempGuid, cylinder));
	}
	moveIn3D ('x', sqrPipe.ang, -(horizontalGap + verticalBarLeftOffset) + params.width - (horizontalGap + verticalBarRightOffset), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

	if (nVerticalBar > 1) {
		// 2��
		tempGuid = placeSqrPipe (sqrPipe);
		elemList.Push (tempGuid);
		cylinder.leftBottomX = sqrPipe.leftBottomX;
		cylinder.leftBottomY = sqrPipe.leftBottomY;
		cylinder.leftBottomZ = sqrPipe.leftBottomZ;
		moveIn3D ('x', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
		for (xx = 0 ; xx < 6 ; ++xx) {
			moveIn3D ('z', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			elemList.Push (placeHole (tempGuid, cylinder));
		}
		moveIn3D ('z', cylinder.ang, -0.300 + params.height - 0.100, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
		for (xx = 0 ; xx < 6 ; ++xx) {
			moveIn3D ('z', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			elemList.Push (placeHole (tempGuid, cylinder));
		}
	}

	// ������ ��ũ ��ġ (���� - ���ϴ�, �ֻ��)
	hook.leftBottomX = params.leftBottomX;
	hook.leftBottomY = params.leftBottomY;
	hook.leftBottomZ = params.leftBottomZ;
	hook.ang = params.ang;
	hook.iHookType = 2;
	hook.iHookShape = 2;
	hook.angX = DegreeToRad (0.0);
	hook.angY = DegreeToRad (90.0);

	moveIn3D ('y', hook.ang, -0.0885, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	if (nHorEuroform >= 2) {
		moveIn3D ('x', hook.ang, width [0], &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
		moveIn3D ('z', hook.ang, 0.030 + 0.150, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
		// 1��
		width_t = 0.0;
		for (xx = 1 ; xx < nHorEuroform ; ++xx) {
			width_t += width [xx];
			elemList.Push (placeEuroformHook (hook));
			moveIn3D ('x', hook.ang, width [xx], &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
		}
		moveIn3D ('x', hook.ang, -width_t, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
		moveIn3D ('z', hook.ang, -0.150 + params.height - 0.150, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

		// ������ ��
		for (xx = 1 ; xx < nHorEuroform ; ++xx) {
			width_t += width [xx];
			elemList.Push (placeEuroformHook (hook));
			moveIn3D ('x', hook.ang, width [xx], &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
		}
	}

	// ����������� ��ġ (���� - ���ϴ�, �ֻ���� ������ ������)
	hanger.leftBottomX = params.leftBottomX;
	hanger.leftBottomY = params.leftBottomY;
	hanger.leftBottomZ = params.leftBottomZ;
	hanger.ang = params.ang;
	hanger.angX = DegreeToRad (0.0);
	hanger.angY = DegreeToRad (270.0);

	moveIn3D ('y', hanger.ang, -0.0635, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);

	// 2 ~ [n-1]��
	if (nHorEuroform >= 2) {
		moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
		moveIn3D ('z', hanger.ang, height [0], &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
		for (xx = 1 ; xx < nVerEuroform ; ++xx) {
			width_t = 0.0;
			for (yy = 0 ; yy < nHorEuroform ; ++yy) {
				// 1��
				if (yy == 0) {
					elemList.Push (placeRectpipeHanger (hanger));
					moveIn3D ('x', hanger.ang, width [0] - 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
					width_t += width [0] - 0.150;
				// ������ ��
				} else if (yy == nHorEuroform - 1) {
					width_t += width [nHorEuroform-1] - 0.150;
					moveIn3D ('x', hanger.ang, width [nHorEuroform-1] - 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
					elemList.Push (placeRectpipeHanger (hanger));
				// ������ ��
				} else {
					width_t += width [yy];
					if (abs (width [yy] - 0.600) < EPS) {
						moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('x', hanger.ang, 0.300, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
					} else if (abs (width [yy] - 0.500) < EPS) {
						moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('x', hanger.ang, 0.200, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
					} else if (abs (width [yy] - 0.450) < EPS) {
						moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
					} else if (abs (width [yy] - 0.400) < EPS) {
						moveIn3D ('x', hanger.ang, 0.100, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('x', hanger.ang, 0.200, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('x', hanger.ang, 0.100, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
					} else if (abs (width [yy] - 0.300) < EPS) {
						moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
					} else if (abs (width [yy] - 0.200) < EPS) {
						moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('x', hanger.ang, 0.050, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
					}
				}
			}
			moveIn3D ('x', hanger.ang, -width_t, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
			moveIn3D ('z', hanger.ang, height [xx], &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
		}
	}

	// ��� �ǽ�
	headpiece.leftBottomX = params.leftBottomX;
	headpiece.leftBottomY = params.leftBottomY;
	headpiece.leftBottomZ = params.leftBottomZ;
	headpiece.ang = params.ang;

	moveIn3D ('x', headpiece.ang, horizontalGap + verticalBarLeftOffset - 0.0475, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	moveIn3D ('y', headpiece.ang, -0.2685, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	moveIn3D ('z', headpiece.ang, 0.291, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);

	// ó�� ��
	elemList.Push (placeJointHeadpeace_hor (headpiece));
	moveIn3D ('x', headpiece.ang, -(horizontalGap + verticalBarLeftOffset - 0.0475) + params.width - (horizontalGap + verticalBarRightOffset + 0.0475), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	if (nVerticalBar > 1)
		elemList.Push (placeJointHeadpeace_ver (headpiece));
	moveIn3D ('x', headpiece.ang, (horizontalGap + verticalBarLeftOffset - 0.0475) - params.width + (horizontalGap + verticalBarRightOffset + 0.0475), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	//if (cell.verLen > 4.000) {
	//	elev_headpiece = 4.000 * 0.80;
	//} else {
	//	elev_headpiece = cell.verLen * 0.80;
	//}
	elev_headpiece = 1.900;
	moveIn3D ('z', headpiece.ang, elev_headpiece, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	// ������ ��
		elemList.Push (placeJointHeadpeace_ver (headpiece));
		moveIn3D ('x', headpiece.ang, -(horizontalGap + verticalBarLeftOffset - 0.0475) + params.width - (horizontalGap + verticalBarRightOffset + 0.0475), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	if (nVerticalBar > 1)
		elemList.Push (placeJointHeadpeace_ver (headpiece));

	// ����ö��
	fittings.leftBottomX = params.leftBottomX;
	fittings.leftBottomY = params.leftBottomY;
	fittings.leftBottomZ = params.leftBottomZ;
	fittings.ang = params.ang;
	fittings.angX = DegreeToRad (180.0);
	fittings.angY = DegreeToRad (0.0);
	
	moveIn3D ('x', fittings.ang, horizontalGap + verticalBarLeftOffset - 0.081, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	moveIn3D ('y', fittings.ang, -0.1155, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	moveIn3D ('z', fittings.ang, 0.230, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);

	// ó�� ��
	for (xx = 0 ; xx <= nVerEuroform ; ++xx) {
		// 1��
		if (xx == 0) {
			elemList.Push (placeFittings (fittings));
			moveIn3D ('z', fittings.ang, height [xx] - 0.180, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		// ������ ��
		} else if (xx == nVerEuroform) {
			moveIn3D ('z', fittings.ang, -0.120, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
			elemList.Push (placeFittings (fittings));
		// ������ ��
		} else {
			elemList.Push (placeFittings (fittings));
			moveIn3D ('z', fittings.ang, height [xx], &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		}
	}
	moveIn3D ('x', fittings.ang, -(horizontalGap + verticalBarLeftOffset - 0.081) + params.width - (horizontalGap + verticalBarRightOffset + 0.081), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	moveIn3D ('z', fittings.ang, 0.300 - params.height, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);

	if (nVerticalBar > 1) {
		// ������ ��
		for (xx = 0 ; xx <= nVerEuroform ; ++xx) {
			// 1��
			if (xx == 0) {
				elemList.Push (placeFittings (fittings));
				moveIn3D ('z', fittings.ang, height [xx] - 0.180, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
			// ������ ��
			} else if (xx == nVerEuroform) {
				moveIn3D ('z', fittings.ang, -0.120, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
				elemList.Push (placeFittings (fittings));
			// ������ ��
			} else {
				elemList.Push (placeFittings (fittings));
				moveIn3D ('z', fittings.ang, height [xx], &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
			}
		}
	}

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

	//////////////////////////////////////////////////////////////// �ݴ��
	if (bDoubleSide) {
		moveIn3D ('x', params.ang, params.width, &params.leftBottomX, &params.leftBottomY, &params.leftBottomY);
		moveIn3D ('y', params.ang, wallThk, &params.leftBottomX, &params.leftBottomY, &params.leftBottomY);
		params.ang += DegreeToRad (180.0);

		// ������ ��ġ (�ݴ����� �����)
		euroform.leftBottomX = params.leftBottomX;
		euroform.leftBottomY = params.leftBottomY;
		euroform.leftBottomZ = params.leftBottomZ;
		euroform.ang = params.ang;
		euroform.u_ins_wall = true;

		for (xx = nHorEuroform - 1 ; xx >= 0 ; --xx) {
			height_t = 0.0;
			for (yy = 0 ; yy < nVerEuroform ; ++yy) {
				euroform.eu_wid = euroform.width	= width [xx];
				euroform.eu_hei = euroform.height	= height [yy];
				height_t += height [yy];
				elemList.Push (placeEuroform (euroform));
				moveIn3D ('z', euroform.ang, height [yy], &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
			}
			moveIn3D ('x', euroform.ang, width [xx], &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
			moveIn3D ('z', euroform.ang, -height_t, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
		}

		// ��� ������ (����) ��ġ
		sqrPipe.leftBottomX = params.leftBottomX;
		sqrPipe.leftBottomY = params.leftBottomY;
		sqrPipe.leftBottomZ = params.leftBottomZ;
		sqrPipe.ang = params.ang;
		sqrPipe.length = params.width - (horizontalGap * 2);
		sqrPipe.pipeAng = DegreeToRad (0);

		moveIn3D ('x', sqrPipe.ang, horizontalGap, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		moveIn3D ('y', sqrPipe.ang, -(0.0635 + 0.025), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		moveIn3D ('z', sqrPipe.ang, 0.150 + 0.031, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

		cylinder.angleFromPlane = DegreeToRad (90.0);
		cylinder.length = 0.050;
		cylinder.radius = 0.013/2;
		cylinder.ang = params.ang;
		cylinder.leftBottomX = sqrPipe.leftBottomX;
		cylinder.leftBottomY = sqrPipe.leftBottomY;
		cylinder.leftBottomZ = sqrPipe.leftBottomZ;
		moveIn3D ('z', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);

		for (xx = 0 ; xx <= nVerEuroform ; ++xx) {
			if (xx == 0) {
				// 1��
				tempGuid = placeSqrPipe (sqrPipe);
				elemList.Push (tempGuid);
				cylinder.leftBottomX = sqrPipe.leftBottomX;
				cylinder.leftBottomY = sqrPipe.leftBottomY;
				cylinder.leftBottomZ = sqrPipe.leftBottomZ;
				moveIn3D ('z', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				for (yy = 0 ; yy < 6 ; ++yy) {
					moveIn3D ('x', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
					elemList.Push (placeHole (tempGuid, cylinder));
				}
				moveIn3D ('x', cylinder.ang, -0.300 + params.width - (horizontalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				for (yy = 0 ; yy < 6 ; ++yy) {
					moveIn3D ('x', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
					elemList.Push (placeHole (tempGuid, cylinder));
				}
				moveIn3D ('z', sqrPipe.ang, -0.031 - 0.150 + height [xx], &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
				moveIn3D ('x', cylinder.ang, 0.300 - params.width + (horizontalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			} else if (xx == nVerEuroform) {
				// ������ ��
				moveIn3D ('z', sqrPipe.ang, -0.150 + 0.031, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
				tempGuid = placeSqrPipe (sqrPipe);
				elemList.Push (tempGuid);
				cylinder.leftBottomX = sqrPipe.leftBottomX;
				cylinder.leftBottomY = sqrPipe.leftBottomY;
				cylinder.leftBottomZ = sqrPipe.leftBottomZ;
				moveIn3D ('z', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				for (yy = 0 ; yy < 6 ; ++yy) {
					moveIn3D ('x', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
					elemList.Push (placeHole (tempGuid, cylinder));
				}
				moveIn3D ('x', cylinder.ang, -0.300 + params.width - (horizontalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				for (yy = 0 ; yy < 6 ; ++yy) {
					moveIn3D ('x', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
					elemList.Push (placeHole (tempGuid, cylinder));
				}
			} else {
				// ������ ��
				tempGuid = placeSqrPipe (sqrPipe);
				elemList.Push (tempGuid);
				cylinder.leftBottomX = sqrPipe.leftBottomX;
				cylinder.leftBottomY = sqrPipe.leftBottomY;
				cylinder.leftBottomZ = sqrPipe.leftBottomZ;
				moveIn3D ('z', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				for (yy = 0 ; yy < 6 ; ++yy) {
					moveIn3D ('x', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
					elemList.Push (placeHole (tempGuid, cylinder));
				}
				moveIn3D ('x', cylinder.ang, -0.300 + params.width - (horizontalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				for (yy = 0 ; yy < 6 ; ++yy) {
					moveIn3D ('x', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
					elemList.Push (placeHole (tempGuid, cylinder));
				}
				moveIn3D ('z', sqrPipe.ang, height [xx], &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
				moveIn3D ('x', cylinder.ang, 0.300 - params.width + (horizontalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			}
		}

		// ��� ������ (����) ��ġ
		sqrPipe.leftBottomX = params.leftBottomX;
		sqrPipe.leftBottomY = params.leftBottomY;
		sqrPipe.leftBottomZ = params.leftBottomZ;
		sqrPipe.ang = params.ang;
		sqrPipe.length = params.height - 0.100;
		sqrPipe.pipeAng = DegreeToRad (90);

		moveIn3D ('x', sqrPipe.ang, horizontalGap + verticalBarRightOffset, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		moveIn3D ('y', sqrPipe.ang, -(0.0635 + 0.075), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		moveIn3D ('z', sqrPipe.ang, 0.050, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

		// 1��
		cylinder.angleFromPlane = DegreeToRad (0.0);
		cylinder.length = 0.050;
		cylinder.radius = 0.013/2;
		cylinder.ang = params.ang;
		cylinder.leftBottomX = sqrPipe.leftBottomX;
		cylinder.leftBottomY = sqrPipe.leftBottomY;
		cylinder.leftBottomZ = sqrPipe.leftBottomZ;

		tempGuid = placeSqrPipe (sqrPipe);
		elemList.Push (tempGuid);
		moveIn3D ('x', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
		for (xx = 0 ; xx < 6 ; ++xx) {
			moveIn3D ('z', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			elemList.Push (placeHole (tempGuid, cylinder));
		}
		moveIn3D ('z', cylinder.ang, -0.300 + params.height - 0.100, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
		for (xx = 0 ; xx < 6 ; ++xx) {
			moveIn3D ('z', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			elemList.Push (placeHole (tempGuid, cylinder));
		}
		moveIn3D ('x', sqrPipe.ang, -(horizontalGap + verticalBarRightOffset) + params.width - (horizontalGap + verticalBarLeftOffset), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

		if (nVerticalBar > 1) {
			// 2��
			tempGuid = placeSqrPipe (sqrPipe);
			elemList.Push (tempGuid);
			cylinder.leftBottomX = sqrPipe.leftBottomX;
			cylinder.leftBottomY = sqrPipe.leftBottomY;
			cylinder.leftBottomZ = sqrPipe.leftBottomZ;
			moveIn3D ('x', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (xx = 0 ; xx < 6 ; ++xx) {
				moveIn3D ('z', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHole (tempGuid, cylinder));
			}
			moveIn3D ('z', cylinder.ang, -0.300 + params.height - 0.100, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (xx = 0 ; xx < 6 ; ++xx) {
				moveIn3D ('z', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHole (tempGuid, cylinder));
			}
		}

		// ������ ��ũ ��ġ (���� - ���ϴ�, �ֻ��)
		hook.leftBottomX = params.leftBottomX;
		hook.leftBottomY = params.leftBottomY;
		hook.leftBottomZ = params.leftBottomZ;
		hook.ang = params.ang;
		hook.iHookType = 2;
		hook.iHookShape = 2;
		hook.angX = DegreeToRad (0.0);
		hook.angY = DegreeToRad (90.0);

		moveIn3D ('y', hook.ang, -0.0885, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

		if (nHorEuroform >= 2) {
			moveIn3D ('x', hook.ang, width [nHorEuroform-1], &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
			moveIn3D ('z', hook.ang, 0.030 + 0.150, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
			// 1��
			width_t = 0.0;
			for (xx = nHorEuroform-2 ; xx >= 0 ; --xx) {
				width_t += width [xx];
				elemList.Push (placeEuroformHook (hook));
				moveIn3D ('x', hook.ang, width [xx], &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
			}
			moveIn3D ('x', hook.ang, -width_t, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
			moveIn3D ('z', hook.ang, -0.150 + params.height - 0.150, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

			// ������ ��
			for (xx = nHorEuroform-2 ; xx >= 0 ; --xx) {
				width_t += width [xx];
				elemList.Push (placeEuroformHook (hook));
				moveIn3D ('x', hook.ang, width [xx], &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
			}
		}

		// ����������� ��ġ (���� - ���ϴ�, �ֻ���� ������ ������)
		hanger.leftBottomX = params.leftBottomX;
		hanger.leftBottomY = params.leftBottomY;
		hanger.leftBottomZ = params.leftBottomZ;
		hanger.ang = params.ang;
		hanger.angX = DegreeToRad (0.0);
		hanger.angY = DegreeToRad (270.0);

		moveIn3D ('y', hanger.ang, -0.0635, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);

		// 2 ~ [n-1]��
		if (nHorEuroform >= 2) {
			moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
			moveIn3D ('z', hanger.ang, height [0], &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
			for (xx = 1 ; xx < nVerEuroform ; ++xx) {
				width_t = 0.0;
				for (yy = nHorEuroform-1 ; yy >= 0 ; --yy) {
					// 1��
					if (yy == nHorEuroform-1) {
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('x', hanger.ang, width [nHorEuroform-1] - 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						width_t += width [0] - 0.150;
					// ������ ��
					} else if (yy == 0) {
						width_t += width [nHorEuroform-1] - 0.150;
						moveIn3D ('x', hanger.ang, width [0] - 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
					// ������ ��
					} else {
						width_t += width [yy];
						if (abs (width [yy] - 0.600) < EPS) {
							moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('x', hanger.ang, 0.300, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						} else if (abs (width [yy] - 0.500) < EPS) {
							moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('x', hanger.ang, 0.200, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						} else if (abs (width [yy] - 0.450) < EPS) {
							moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						} else if (abs (width [yy] - 0.400) < EPS) {
							moveIn3D ('x', hanger.ang, 0.100, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('x', hanger.ang, 0.200, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('x', hanger.ang, 0.100, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						} else if (abs (width [yy] - 0.300) < EPS) {
							moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						} else if (abs (width [yy] - 0.200) < EPS) {
							moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('x', hanger.ang, 0.050, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						}
					}
				}
				moveIn3D ('x', hanger.ang, -width_t, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
				moveIn3D ('z', hanger.ang, height [xx], &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
			}
		}

		// ��� �ǽ�
		headpiece.leftBottomX = params.leftBottomX;
		headpiece.leftBottomY = params.leftBottomY;
		headpiece.leftBottomZ = params.leftBottomZ;
		headpiece.ang = params.ang;

		moveIn3D ('x', headpiece.ang, horizontalGap + verticalBarRightOffset - 0.0475, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		moveIn3D ('y', headpiece.ang, -0.2685, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		moveIn3D ('z', headpiece.ang, 0.291, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);

		// ó�� ��
		elemList.Push (placeJointHeadpeace_hor (headpiece));
		moveIn3D ('x', headpiece.ang, -(horizontalGap + verticalBarRightOffset - 0.0475) + params.width - (horizontalGap + verticalBarLeftOffset + 0.0475), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		if (nVerticalBar > 1)
			elemList.Push (placeJointHeadpeace_hor (headpiece));
		moveIn3D ('x', headpiece.ang, (horizontalGap + verticalBarRightOffset - 0.0475) - params.width + (horizontalGap + verticalBarLeftOffset + 0.0475), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		//if (cell.verLen > 4.000) {
		//	elev_headpiece = 4.000 * 0.80;
		//} else {
		//	elev_headpiece = cell.verLen * 0.80;
		//}
		elev_headpiece = 1.900;
		moveIn3D ('z', headpiece.ang, elev_headpiece, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		// ������ ��
		elemList.Push (placeJointHeadpeace_hor (headpiece));
		moveIn3D ('x', headpiece.ang, -(horizontalGap + verticalBarRightOffset - 0.0475) + params.width - (horizontalGap + verticalBarLeftOffset + 0.0475), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		if (nVerticalBar > 1)
			elemList.Push (placeJointHeadpeace_hor (headpiece));

		// ����ö��
		fittings.leftBottomX = params.leftBottomX;
		fittings.leftBottomY = params.leftBottomY;
		fittings.leftBottomZ = params.leftBottomZ;
		fittings.ang = params.ang;
		fittings.angX = DegreeToRad (180.0);
		fittings.angY = DegreeToRad (0.0);
	
		moveIn3D ('x', fittings.ang, horizontalGap + verticalBarRightOffset - 0.081, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		moveIn3D ('y', fittings.ang, -0.1155, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		moveIn3D ('z', fittings.ang, 0.230, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);

		// ó�� ��
		for (xx = 0 ; xx <= nVerEuroform ; ++xx) {
			// 1��
			if (xx == 0) {
				elemList.Push (placeFittings (fittings));
				moveIn3D ('z', fittings.ang, height [xx] - 0.180, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
			// ������ ��
			} else if (xx == nVerEuroform) {
				moveIn3D ('z', fittings.ang, -0.120, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
				elemList.Push (placeFittings (fittings));
			// ������ ��
			} else {
				elemList.Push (placeFittings (fittings));
				moveIn3D ('z', fittings.ang, height [xx], &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
			}
		}

		moveIn3D ('x', fittings.ang, -(horizontalGap + verticalBarRightOffset - 0.081) + params.width - (horizontalGap + verticalBarLeftOffset + 0.081), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		moveIn3D ('z', fittings.ang, 0.300 - params.height, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);

		if (nVerticalBar > 1) {
			// ������ ��
			for (xx = 0 ; xx <= nVerEuroform ; ++xx) {
				// 1��
				if (xx == 0) {
					elemList.Push (placeFittings (fittings));
					moveIn3D ('z', fittings.ang, height [xx] - 0.180, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
				// ������ ��
				} else if (xx == nVerEuroform) {
					moveIn3D ('z', fittings.ang, -0.120, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
					elemList.Push (placeFittings (fittings));
				// ������ ��
				} else {
					elemList.Push (placeFittings (fittings));
					moveIn3D ('z', fittings.ang, height [xx], &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
				}
			}
		}

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
	}

	return	err;
}

// ���̺���(��) ��ġ (��������) Ÿ��2
GSErrCode	placeTableformOnWall_landscape_Type2 (WallTableform params)
{
	// ����, ���ΰ� �ڹٲ��� ��
	exchangeDoubles (&params.width, &params.height);

	GSErrCode	err = NoError;

	short	nHorEuroform;			// ���� ���� ������ ����
	short	nVerEuroform;			// ���� ���� ������ ����
	double	width [7];				// ���� ���� �� ������ �ʺ�
	double	height [7];				// ���� ���� �� ������ ����

	short	nVerticalBar;
	double	verticalBarLeftOffset;
	double	verticalBarRightOffset;

	short		xx, yy;
	double		height_t;
	double		elev_headpiece;
	double		verticalGap = 0.050;	// ������ ���� �̰ݰŸ�
	API_Guid	tempGuid;
	Cylinder	cylinder;

	Euroform		euroform;
	SquarePipe		sqrPipe;
	HeadpieceOfPushPullProps	headpiece;
	MetalFittings	fittings;
	EuroformHook	hook;
	RectPipeHanger	hanger;

	nHorEuroform = 0;
	nVerEuroform = 0;
	for (xx = 0 ; xx < 7 ; ++xx) {
		width [xx] = 0.0;
		height [xx] = 0.0;
	}

	if (abs (params.height - 2.300) < EPS) {
		nVerEuroform = 4;
		//height [0] = 0.600;		height [1] = 0.600;		height [2] = 0.500;		height [3] = 0.600;
		height [0] = 0.500;		height [1] = 0.600;		height [2] = 0.600;		height [3] = 0.600;
		verticalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 2.250) < EPS) {
		nVerEuroform = 4;
		height [0] = 0.600;		height [1] = 0.600;		height [2] = 0.450;		height [3] = 0.600;
		verticalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 2.200) < EPS) {
		nVerEuroform = 4;
		//height [0] = 0.600;		height [1] = 0.600;		height [2] = 0.400;		height [3] = 0.600;
		height [0] = 0.400;		height [1] = 0.600;		height [2] = 0.600;		height [3] = 0.600;
		verticalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 2.150) < EPS) {
		nVerEuroform = 4;
		//height [0] = 0.600;		height [1] = 0.500;		height [2] = 0.450;		height [3] = 0.600;
		height [0] = 0.500;		height [1] = 0.600;		height [2] = 0.450;		height [3] = 0.600;
		verticalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 2.100) < EPS) {
		nVerEuroform = 4;
		height [0] = 0.600;		height [1] = 0.600;		height [2] = 0.300;		height [3] = 0.600;
		verticalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 2.050) < EPS) {
		nVerEuroform = 4;
		//height [0] = 0.600;		height [1] = 0.450;		height [2] = 0.400;		height [3] = 0.600;
		height [0] = 0.400;		height [1] = 0.450;		height [2] = 0.600;		height [3] = 0.600;
		verticalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 2.000) < EPS) {
		nVerEuroform = 4;
		//height [0] = 0.600;		height [1] = 0.600;		height [2] = 0.200;		height [3] = 0.600;
		height [0] = 0.200;		height [1] = 0.600;		height [2] = 0.600;		height [3] = 0.600;
		verticalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.950) < EPS) {
		nVerEuroform = 4;
		height [0] = 0.600;		height [1] = 0.450;		height [2] = 0.300;		height [3] = 0.600;
		verticalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.900) < EPS) {
		nVerEuroform = 4;
		//height [0] = 0.600;		height [1] = 0.500;		height [2] = 0.200;		height [3] = 0.600;
		height [0] = 0.500;		height [1] = 0.200;		height [2] = 0.600;		height [3] = 0.600;
		verticalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.850) < EPS) {
		nVerEuroform = 4;
		//height [0] = 0.600;		height [1] = 0.450;		height [2] = 0.200;		height [3] = 0.600;
		height [0] = 0.200;		height [1] = 0.450;		height [2] = 0.600;		height [3] = 0.600;
		verticalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.800) < EPS) {
		nVerEuroform = 3;
		height [0] = 0.600;		height [1] = 0.600;		height [2] = 0.600;		height [3] = 0.0;
		verticalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.750) < EPS) {
		nVerEuroform = 4;
		//height [0] = 0.600;		height [1] = 0.200;		height [2] = 0.450;		height [3] = 0.500;
		height [0] = 0.500;		height [1] = 0.200;		height [2] = 0.450;		height [3] = 0.600;
		verticalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.700) < EPS) {
		nVerEuroform = 3;
		//height [0] = 0.600;		height [1] = 0.500;		height [2] = 0.600;		height [3] = 0.0;
		height [0] = 0.500;		height [1] = 0.600;		height [2] = 0.600;		height [3] = 0.0;
		verticalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.650) < EPS) {
		nVerEuroform = 3;
		height [0] = 0.600;		height [1] = 0.450;		height [2] = 0.600;		height [3] = 0.0;
		verticalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.600) < EPS) {
		nVerEuroform = 3;
		//height [0] = 0.600;		height [1] = 0.400;		height [2] = 0.600;		height [3] = 0.0;
		height [0] = 0.400;		height [1] = 0.600;		height [2] = 0.600;		height [3] = 0.0;
		verticalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.550) < EPS) {
		nVerEuroform = 3;
		//height [0] = 0.600;		height [1] = 0.450;		height [2] = 0.500;		height [3] = 0.0;
		height [0] = 0.500;		height [1] = 0.450;		height [2] = 0.600;		height [3] = 0.0;
		verticalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.500) < EPS) {
		nVerEuroform = 3;
		height [0] = 0.600;		height [1] = 0.300;		height [2] = 0.600;		height [3] = 0.0;
		verticalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.450) < EPS) {
		nVerEuroform = 3;
		//height [0] = 0.500;		height [1] = 0.450;		height [2] = 0.500;		height [3] = 0.0;
		height [0] = 0.400;		height [1] = 0.450;		height [2] = 0.600;		height [3] = 0.0;
		verticalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.400) < EPS) {
		nVerEuroform = 3;
		//height [0] = 0.500;		height [1] = 0.400;		height [2] = 0.500;		height [3] = 0.0;
		height [0] = 0.500;		height [1] = 0.300;		height [2] = 0.600;		height [3] = 0.0;
		verticalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.350) < EPS) {
		nVerEuroform = 3;
		height [0] = 0.600;		height [1] = 0.300;		height [2] = 0.450;		height [3] = 0.0;
		verticalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.300) < EPS) {
		nVerEuroform = 3;
		//height [0] = 0.600;		height [1] = 0.200;		height [2] = 0.500;		height [3] = 0.0;
		height [0] = 0.500;		height [1] = 0.200;		height [2] = 0.600;		height [3] = 0.0;
		verticalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.250) < EPS) {
		nVerEuroform = 3;
		//height [0] = 0.600;		height [1] = 0.200;		height [2] = 0.450;		height [3] = 0.0;
		height [0] = 0.200;		height [1] = 0.600;		height [2] = 0.450;		height [3] = 0.0;
		verticalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.200) < EPS) {
		nVerEuroform = 2;
		height [0] = 0.600;		height [1] = 0.600;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.150) < EPS) {
		nVerEuroform = 3;
		//height [0] = 0.450;		height [1] = 0.300;		height [2] = 0.400;		height [3] = 0.0;
		height [0] = 0.400;		height [1] = 0.300;		height [2] = 0.450;		height [3] = 0.0;
		verticalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.100) < EPS) {
		//nVerEuroform = 3;
		nVerEuroform = 2;
		//height [0] = 0.400;		height [1] = 0.300;		height [2] = 0.400;		height [3] = 0.0;
		height [0] = 0.500;		height [1] = 0.600;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.050) < EPS) {
		nVerEuroform = 3;
		height [0] = 0.450;		height [1] = 0.300;		height [2] = 0.300;		height [3] = 0.0;
		verticalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.000) < EPS) {
		nVerEuroform = 2;
		//height [0] = 0.600;		height [1] = 0.400;		height [2] = 0.0;		height [3] = 0.0;
		height [0] = 0.400;		height [1] = 0.600;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 0.950) < EPS) {
		nVerEuroform = 2;
		//height [0] = 0.450;		height [1] = 0.500;		height [2] = 0.0;		height [3] = 0.0;
		height [0] = 0.500;		height [1] = 0.450;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 0.900) < EPS) {
		nVerEuroform = 2;
		height [0] = 0.600;		height [1] = 0.300;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 0.850) < EPS) {
		nVerEuroform = 2;
		height [0] = 0.400;		height [1] = 0.450;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 0.800) < EPS) {
		nVerEuroform = 2;
		height [0] = 0.400;		height [1] = 0.400;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 0.750) < EPS) {
		nVerEuroform = 2;
		height [0] = 0.450;		height [1] = 0.300;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.200;
	} else if (abs (params.height - 0.700) < EPS) {
		nVerEuroform = 2;
		height [0] = 0.400;		height [1] = 0.300;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.150;
	} else if (abs (params.height - 0.650) < EPS) {
		nVerEuroform = 2;
		//height [0] = 0.450;		height [1] = 0.200;		height [2] = 0.0;		height [3] = 0.0;
		height [0] = 0.200;		height [1] = 0.450;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.200;
		verticalBarRightOffset = 0.150;
	} else if (abs (params.height - 0.600) < EPS) {
		nVerEuroform = 1;
		height [0] = 0.600;		height [1] = 0.0;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.150;
		verticalBarRightOffset = 0.150;
	} else if (abs (params.height - 0.500) < EPS) {
		nVerEuroform = 1;
		height [0] = 0.500;		height [1] = 0.0;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.050;
		nVerticalBar = 1;
		verticalBarLeftOffset = 0.200;
		verticalBarRightOffset = 0.200;
	} else if (abs (params.height - 0.450) < EPS) {
		nVerEuroform = 1;
		height [0] = 0.450;		height [1] = 0.0;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.025;
		nVerticalBar = 1;
		verticalBarLeftOffset = 0.200;
		verticalBarRightOffset = 0.200;
	} else if (abs (params.height - 0.400) < EPS) {
		nVerEuroform = 1;
		height [0] = 0.400;		height [1] = 0.0;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.050;
		nVerticalBar = 1;
		verticalBarLeftOffset = 0.150;
		verticalBarRightOffset = 0.150;
	} else {
		nVerEuroform = 0;
		height [0] = 0.0;		height [1] = 0.0;		height [2] = 0.0;		height [3] = 0.0;
		nVerticalBar = 0;
		verticalBarLeftOffset = 0.0;
		verticalBarRightOffset = 0.0;
	}

	if (abs (params.width - 6.000) < EPS) {
		nHorEuroform = 5;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 1.200;
		width [3] = 1.200;
		width [4] = 1.200;
	} else if (abs (params.width - 5.700) < EPS) {
		nHorEuroform = 5;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 1.200;
		width [3] = 1.200;
		width [4] = 0.900;
	} else if (abs (params.width - 5.400) < EPS) {
		nHorEuroform = 5;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 1.200;
		width [3] = 0.900;
		width [4] = 0.900;
	} else if (abs (params.width - 5.100) < EPS) {
		nHorEuroform = 5;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 1.200;
		width [3] = 0.900;
		width [4] = 0.600;
	} else if (abs (params.width - 4.800) < EPS) {
		nHorEuroform = 4;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 1.200;
		width [3] = 1.200;
		width [4] = 0.0;
	} else if (abs (params.width - 4.500) < EPS) {
		nHorEuroform = 4;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 1.200;
		width [3] = 0.900;
		width [4] = 0.0;
	} else if (abs (params.width - 4.200) < EPS) {
		nHorEuroform = 4;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 0.900;
		width [3] = 0.900;
		width [4] = 0.0;
	} else if (abs (params.width - 3.900) < EPS) {
		nHorEuroform = 4;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 0.900;
		width [3] = 0.600;
		width [4] = 0.0;
	} else if (abs (params.width - 3.600) < EPS) {
		nHorEuroform = 3;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 1.200;
		width [3] = 0.0;
		width [4] = 0.0;
	} else if (abs (params.width - 3.300) < EPS) {
		nHorEuroform = 3;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 0.900;
		width [3] = 0.0;
		width [4] = 0.0;
	} else if (abs (params.width - 3.000) < EPS) {
		nHorEuroform = 3;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 0.600;
		width [3] = 0.0;
		width [4] = 0.0;
	} else if (abs (params.width - 2.700) < EPS) {
		nHorEuroform = 3;
		width [0] = 1.200;
		width [1] = 0.900;
		width [2] = 0.600;
		width [3] = 0.0;
		width [4] = 0.0;
	} else if (abs (params.width - 2.400) < EPS) {
		nHorEuroform = 2;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 0.0;
		width [3] = 0.0;
		width [4] = 0.0;
	} else if (abs (params.width - 2.100) < EPS) {
		nHorEuroform = 2;
		width [0] = 1.200;
		width [1] = 0.900;
		width [2] = 0.0;
		width [3] = 0.0;
		width [4] = 0.0;
	} else if (abs (params.width - 1.800) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.900;
		width [1] = 0.900;
		width [2] = 0.0;
		width [3] = 0.0;
		width [4] = 0.0;
	} else if (abs (params.width - 1.500) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.900;
		width [1] = 0.600;
		width [2] = 0.0;
		width [3] = 0.0;
		width [4] = 0.0;
	} else {
		nHorEuroform = 0;
		width [0] = 0.0;
		width [1] = 0.0;
		width [2] = 0.0;
		width [3] = 0.0;
		width [4] = 0.0;
	}

	// �ʺ� ���̰� 0�̸� �ƹ��͵� ��ġ���� ����
	if ((nHorEuroform == 0) || (nVerEuroform == 0))
		return	NoError;

	//////////////////////////////////////////////////////////////// �����
	// ������ ��ġ
	euroform.leftBottomX = params.leftBottomX;
	euroform.leftBottomY = params.leftBottomY;
	euroform.leftBottomZ = params.leftBottomZ;
	euroform.ang = params.ang;
	euroform.eu_stan_onoff = true;
	euroform.u_ins_wall = false;
	euroform.ang_x = DegreeToRad (90.0);

	for (xx = 0 ; xx < nHorEuroform ; ++xx) {
		height_t = 0.0;
		for (yy = nVerEuroform-1 ; yy >= 0 ; --yy) {
			euroform.eu_wid = euroform.width	= height [yy];
			euroform.eu_hei = euroform.height	= width [xx];
			height_t += height [yy];
			elemList.Push (placeEuroform (euroform));
			moveIn3D ('z', euroform.ang, height [yy], &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
		}
		moveIn3D ('x', euroform.ang, width [xx], &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
		moveIn3D ('z', euroform.ang, -height_t, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
	}

	// ��� ������ (����) ��ġ
	sqrPipe.leftBottomX = params.leftBottomX;
	sqrPipe.leftBottomY = params.leftBottomY;
	sqrPipe.leftBottomZ = params.leftBottomZ;
	sqrPipe.ang = params.ang;
	sqrPipe.length = params.height - (verticalGap * 2);
	sqrPipe.pipeAng = DegreeToRad (90.0);

	moveIn3D ('z', sqrPipe.ang, verticalGap, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	moveIn3D ('y', sqrPipe.ang, -(0.0635 + 0.025), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	moveIn3D ('x', sqrPipe.ang, 0.150 + 0.031, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

	cylinder.angleFromPlane = DegreeToRad (0.0);
	cylinder.length = 0.050;
	cylinder.radius = 0.013/2;
	cylinder.ang = params.ang;
	cylinder.leftBottomX = sqrPipe.leftBottomX;
	cylinder.leftBottomY = sqrPipe.leftBottomY;
	cylinder.leftBottomZ = sqrPipe.leftBottomZ;

	for (xx = 0 ; xx <= nHorEuroform ; ++xx) {
		if (xx == 0) {
			// 1��
			tempGuid = placeSqrPipe (sqrPipe);
			elemList.Push (tempGuid);
			cylinder.leftBottomX = sqrPipe.leftBottomX;
			cylinder.leftBottomY = sqrPipe.leftBottomY;
			cylinder.leftBottomZ = sqrPipe.leftBottomZ;
			moveIn3D ('x', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (yy = 0 ; yy < 6 ; ++yy) {
				moveIn3D ('z', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHole (tempGuid, cylinder));
			}
			moveIn3D ('z', cylinder.ang, -0.300 + params.height - (verticalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (yy = 0 ; yy < 6 ; ++yy) {
				moveIn3D ('z', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHole (tempGuid, cylinder));
			}
			moveIn3D ('x', sqrPipe.ang, -0.031 - 0.150 + width [xx], &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		} else if (xx == nHorEuroform) {
			// ������ ��
			moveIn3D ('x', sqrPipe.ang, -0.150 + 0.031, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
			tempGuid = placeSqrPipe (sqrPipe);
			elemList.Push (tempGuid);
			cylinder.leftBottomX = sqrPipe.leftBottomX;
			cylinder.leftBottomY = sqrPipe.leftBottomY;
			cylinder.leftBottomZ = sqrPipe.leftBottomZ;
			moveIn3D ('x', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (yy = 0 ; yy < 6 ; ++yy) {
				moveIn3D ('z', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHole (tempGuid, cylinder));
			}
			moveIn3D ('z', cylinder.ang, -0.300 + params.height - (verticalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (yy = 0 ; yy < 6 ; ++yy) {
				moveIn3D ('z', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHole (tempGuid, cylinder));
			}
		} else {
			// ������ ��
			tempGuid = placeSqrPipe (sqrPipe);
			elemList.Push (tempGuid);
			cylinder.leftBottomX = sqrPipe.leftBottomX;
			cylinder.leftBottomY = sqrPipe.leftBottomY;
			cylinder.leftBottomZ = sqrPipe.leftBottomZ;
			moveIn3D ('x', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (yy = 0 ; yy < 6 ; ++yy) {
				moveIn3D ('z', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHole (tempGuid, cylinder));
			}
			moveIn3D ('z', cylinder.ang, -0.300 + params.height - (verticalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (yy = 0 ; yy < 6 ; ++yy) {
				moveIn3D ('z', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHole (tempGuid, cylinder));
			}
			moveIn3D ('x', sqrPipe.ang, width [xx], &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		}
	}

	// ��� ������ (����) ��ġ
	sqrPipe.leftBottomX = params.leftBottomX;
	sqrPipe.leftBottomY = params.leftBottomY;
	sqrPipe.leftBottomZ = params.leftBottomZ;
	sqrPipe.ang = params.ang;
	sqrPipe.length = params.width - 0.100;
	sqrPipe.pipeAng = DegreeToRad (0);

	moveIn3D ('z', sqrPipe.ang, verticalGap + verticalBarLeftOffset, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	moveIn3D ('y', sqrPipe.ang, -(0.0635 + 0.075), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	moveIn3D ('x', sqrPipe.ang, 0.050, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

	// 1��
	cylinder.angleFromPlane = DegreeToRad (90.0);
	cylinder.length = 0.050;
	cylinder.radius = 0.013/2;
	cylinder.ang = params.ang;
	cylinder.leftBottomX = sqrPipe.leftBottomX;
	cylinder.leftBottomY = sqrPipe.leftBottomY;
	cylinder.leftBottomZ = sqrPipe.leftBottomZ;

	tempGuid = placeSqrPipe (sqrPipe);
	elemList.Push (tempGuid);
	moveIn3D ('z', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
	for (xx = 0 ; xx < 6 ; ++xx) {
		moveIn3D ('x', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
		elemList.Push (placeHole (tempGuid, cylinder));
	}
	moveIn3D ('x', cylinder.ang, -0.300 + params.width - 0.100, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
	for (xx = 0 ; xx < 6 ; ++xx) {
		moveIn3D ('x', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
		elemList.Push (placeHole (tempGuid, cylinder));
	}
	moveIn3D ('z', sqrPipe.ang, -(verticalGap + verticalBarLeftOffset) + params.height - (verticalGap + verticalBarRightOffset), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

	if (nVerticalBar > 1) {
		// 2��
		tempGuid = placeSqrPipe (sqrPipe);
		elemList.Push (tempGuid);
		cylinder.leftBottomX = sqrPipe.leftBottomX;
		cylinder.leftBottomY = sqrPipe.leftBottomY;
		cylinder.leftBottomZ = sqrPipe.leftBottomZ;
		moveIn3D ('z', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
		for (xx = 0 ; xx < 6 ; ++xx) {
			moveIn3D ('x', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			elemList.Push (placeHole (tempGuid, cylinder));
		}
		moveIn3D ('x', cylinder.ang, -0.300 + params.width - 0.100, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
		for (xx = 0 ; xx < 6 ; ++xx) {
			moveIn3D ('x', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			elemList.Push (placeHole (tempGuid, cylinder));
		}
	}

	// ������ ��ũ ��ġ (���� - ���ϴ�, �ֻ��)
	hook.leftBottomX = params.leftBottomX;
	hook.leftBottomY = params.leftBottomY;
	hook.leftBottomZ = params.leftBottomZ;
	hook.ang = params.ang;
	hook.iHookType = 2;
	hook.iHookShape = 2;
	hook.angX = DegreeToRad (0.0);
	hook.angY = DegreeToRad (0.0);

	moveIn3D ('y', hook.ang, -0.0885, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	if (nVerEuroform >= 2) {
		moveIn3D ('z', hook.ang, params.height - height [0], &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
		moveIn3D ('x', hook.ang, 0.030 + 0.150, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
		// 1��
		height_t = 0.0;
		for (xx = 1 ; xx < nVerEuroform ; ++xx) {
			height_t += height [xx];
			elemList.Push (placeEuroformHook (hook));
			moveIn3D ('z', hook.ang, -height [xx], &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
		}
		moveIn3D ('z', hook.ang, height_t, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
		moveIn3D ('x', hook.ang, -0.150 + params.width - 0.150, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

		// ������ ��
		for (xx = 1 ; xx < nVerEuroform ; ++xx) {
			height_t += height [xx];
			elemList.Push (placeEuroformHook (hook));
			moveIn3D ('z', hook.ang, -height [xx], &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
		}
	}

	// ����������� ��ġ (���� - ���ϴ�, �ֻ���� ������ ������)
	hanger.leftBottomX = params.leftBottomX;
	hanger.leftBottomY = params.leftBottomY;
	hanger.leftBottomZ = params.leftBottomZ;
	hanger.ang = params.ang;
	hanger.angX = DegreeToRad (270.0);
	hanger.angY = DegreeToRad (270.0);

	moveIn3D ('y', hanger.ang, -0.0635, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);

	// 2 ~ [n-1]��
	if (nVerEuroform >= 2) {
		moveIn3D ('z', hanger.ang, params.height - 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
		moveIn3D ('x', hanger.ang, width [0], &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
		for (xx = 1 ; xx < nHorEuroform ; ++xx) {
			height_t = 0.0;
			for (yy = 0 ; yy < nVerEuroform ; ++yy) {
				// 1��
				if (yy == 0) {
					elemList.Push (placeRectpipeHanger (hanger));
					moveIn3D ('z', hanger.ang, -(height [0] - 0.150), &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
					height_t += height [0] - 0.150;
				// ������ ��
				} else if (yy == nVerEuroform - 1) {
					height_t += height [nVerEuroform-1] - 0.150;
					moveIn3D ('z', hanger.ang, -(height [nVerEuroform-1] - 0.150), &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
					elemList.Push (placeRectpipeHanger (hanger));
				// ������ ��
				} else {
					height_t += height [yy];
					if (abs (height [yy] - 0.600) < EPS) {
						moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('z', hanger.ang, -0.300, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
					} else if (abs (height [yy] - 0.500) < EPS) {
						moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('z', hanger.ang, -0.200, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
					} else if (abs (height [yy] - 0.450) < EPS) {
						moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
					} else if (abs (height [yy] - 0.400) < EPS) {
						moveIn3D ('z', hanger.ang, -0.100, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('z', hanger.ang, -0.200, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('z', hanger.ang, -0.100, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
					} else if (abs (height [yy] - 0.300) < EPS) {
						moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
					} else if (abs (height [yy] - 0.200) < EPS) {
						moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('z', hanger.ang, -0.050, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
					}
				}
			}
			moveIn3D ('z', hanger.ang, height_t, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
			moveIn3D ('x', hanger.ang, width [xx], &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
		}
	}

	// ��� �ǽ�
	headpiece.leftBottomX = params.leftBottomX;
	headpiece.leftBottomY = params.leftBottomY;
	headpiece.leftBottomZ = params.leftBottomZ;
	headpiece.ang = params.ang;

	moveIn3D ('z', headpiece.ang, params.height - (verticalGap + verticalBarRightOffset - 0.0475) - 0.0975, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	moveIn3D ('y', headpiece.ang, -0.2685, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	moveIn3D ('x', headpiece.ang, 0.291, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);

	// ó�� ��
	elemList.Push (placeJointHeadpeace_hor (headpiece));
	moveIn3D ('z', headpiece.ang, (verticalGap + verticalBarRightOffset - 0.0475) - params.height + (verticalGap + verticalBarLeftOffset + 0.0475), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	if (nVerticalBar > 1)
		elemList.Push (placeJointHeadpeace_hor (headpiece));
	moveIn3D ('z', headpiece.ang, -(verticalGap + verticalBarRightOffset - 0.0475) + params.height - (verticalGap + verticalBarLeftOffset + 0.0475), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	//if (cell.verLen > 4.000) {
	//	elev_headpiece = 4.000 * 0.80;
	//} else {
	//	elev_headpiece = cell.verLen * 0.80;
	//}
	elev_headpiece = 1.900;
	moveIn3D ('x', headpiece.ang, elev_headpiece, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	// ������ ��
		elemList.Push (placeJointHeadpeace_hor (headpiece));
	moveIn3D ('z', headpiece.ang, (verticalGap + verticalBarRightOffset - 0.0475) - params.height + (verticalGap + verticalBarLeftOffset + 0.0475), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	if (nVerticalBar > 1)
		elemList.Push (placeJointHeadpeace_hor (headpiece));

	// ����ö��
	fittings.leftBottomX = params.leftBottomX;
	fittings.leftBottomY = params.leftBottomY;
	fittings.leftBottomZ = params.leftBottomZ;
	fittings.ang = params.ang;
	fittings.angX = DegreeToRad (180.0);
	fittings.angY = DegreeToRad (90.0);
	
	moveIn3D ('z', fittings.ang, params.height - (verticalGap + verticalBarRightOffset + 0.081), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	moveIn3D ('y', fittings.ang, -0.1155, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	moveIn3D ('x', fittings.ang, 0.130, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);

	// ó�� ��
	for (xx = 0 ; xx <= nHorEuroform ; ++xx) {
		// 1��
		if (xx == 0) {
			elemList.Push (placeFittings (fittings));
			moveIn3D ('x', fittings.ang, width [xx] - 0.180, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		// ������ ��
		} else if (xx == nHorEuroform) {
			moveIn3D ('x', fittings.ang, -0.120, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
			elemList.Push (placeFittings (fittings));
		// ������ ��
		} else {
			elemList.Push (placeFittings (fittings));
			moveIn3D ('x', fittings.ang, width [xx], &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		}
	}
	moveIn3D ('z', fittings.ang, (verticalGap + verticalBarRightOffset + 0.081) - params.height + (verticalGap + verticalBarLeftOffset + 0.081) - 0.162, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	moveIn3D ('x', fittings.ang, 0.300 - params.width, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);

	if (nVerticalBar > 1) {
		// ������ ��
		for (xx = 0 ; xx <= nHorEuroform ; ++xx) {
			// 1��
			if (xx == 0) {
				elemList.Push (placeFittings (fittings));
				moveIn3D ('x', fittings.ang, width [xx] - 0.180, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
			// ������ ��
			} else if (xx == nHorEuroform) {
				moveIn3D ('x', fittings.ang, -0.120, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
				elemList.Push (placeFittings (fittings));
			// ������ ��
			} else {
				elemList.Push (placeFittings (fittings));
				moveIn3D ('x', fittings.ang, width [xx], &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
			}
		}
	}

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

	////////////////////////////////////////////////////////////////// �ݴ��

	if (bDoubleSide) {
		moveIn3D ('x', params.ang, params.width, &params.leftBottomX, &params.leftBottomY, &params.leftBottomY);
		moveIn3D ('y', params.ang, wallThk, &params.leftBottomX, &params.leftBottomY, &params.leftBottomY);
		params.ang += DegreeToRad (180.0);

		// ������ ��ġ
		euroform.leftBottomX = params.leftBottomX;
		euroform.leftBottomY = params.leftBottomY;
		euroform.leftBottomZ = params.leftBottomZ;
		euroform.ang = params.ang;
		euroform.u_ins_wall = false;

		for (xx = nHorEuroform-1 ; xx >= 0  ; --xx) {
			height_t = 0.0;
			for (yy = nVerEuroform-1 ; yy >= 0 ; --yy) {
				euroform.eu_wid = euroform.width	= height [yy];
				euroform.eu_hei = euroform.height	= width [xx];
				height_t += height [yy];
				elemList.Push (placeEuroform (euroform));
				moveIn3D ('z', euroform.ang, height [yy], &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
			}
			moveIn3D ('x', euroform.ang, width [xx], &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
			moveIn3D ('z', euroform.ang, -height_t, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
		}

		// ��� ������ (����) ��ġ
		sqrPipe.leftBottomX = params.leftBottomX;
		sqrPipe.leftBottomY = params.leftBottomY;
		sqrPipe.leftBottomZ = params.leftBottomZ;
		sqrPipe.ang = params.ang;
		sqrPipe.length = params.height - (verticalGap * 2);
		sqrPipe.pipeAng = DegreeToRad (90.0);

		moveIn3D ('z', sqrPipe.ang, verticalGap, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		moveIn3D ('y', sqrPipe.ang, -(0.0635 + 0.025), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		moveIn3D ('x', sqrPipe.ang, params.width - (0.150 + 0.031), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

		cylinder.angleFromPlane = DegreeToRad (0.0);
		cylinder.length = 0.050;
		cylinder.radius = 0.013/2;
		cylinder.ang = params.ang;
		cylinder.leftBottomX = sqrPipe.leftBottomX;
		cylinder.leftBottomY = sqrPipe.leftBottomY;
		cylinder.leftBottomZ = sqrPipe.leftBottomZ;

		for (xx = 0 ; xx <= nHorEuroform ; ++xx) {
			if (xx == 0) {
				// 1��
				tempGuid = placeSqrPipe (sqrPipe);
				elemList.Push (tempGuid);
				cylinder.leftBottomX = sqrPipe.leftBottomX;
				cylinder.leftBottomY = sqrPipe.leftBottomY;
				cylinder.leftBottomZ = sqrPipe.leftBottomZ;
				moveIn3D ('x', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				for (yy = 0 ; yy < 6 ; ++yy) {
					moveIn3D ('z', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
					elemList.Push (placeHole (tempGuid, cylinder));
				}
				moveIn3D ('z', cylinder.ang, -0.300 + params.height - (verticalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				for (yy = 0 ; yy < 6 ; ++yy) {
					moveIn3D ('z', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
					elemList.Push (placeHole (tempGuid, cylinder));
				}
				moveIn3D ('x', sqrPipe.ang, (0.150 + 0.031) - width [xx], &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
			} else if (xx == nHorEuroform) {
				// ������ ��
				moveIn3D ('x', sqrPipe.ang, 0.150 - 0.031, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
				tempGuid = placeSqrPipe (sqrPipe);
				elemList.Push (tempGuid);
				cylinder.leftBottomX = sqrPipe.leftBottomX;
				cylinder.leftBottomY = sqrPipe.leftBottomY;
				cylinder.leftBottomZ = sqrPipe.leftBottomZ;
				moveIn3D ('x', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				for (yy = 0 ; yy < 6 ; ++yy) {
					moveIn3D ('z', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
					elemList.Push (placeHole (tempGuid, cylinder));
				}
				moveIn3D ('z', cylinder.ang, -0.300 + params.height - (verticalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				for (yy = 0 ; yy < 6 ; ++yy) {
					moveIn3D ('z', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
					elemList.Push (placeHole (tempGuid, cylinder));
				}
			} else {
				// ������ ��
				tempGuid = placeSqrPipe (sqrPipe);
				elemList.Push (tempGuid);
				cylinder.leftBottomX = sqrPipe.leftBottomX;
				cylinder.leftBottomY = sqrPipe.leftBottomY;
				cylinder.leftBottomZ = sqrPipe.leftBottomZ;
				moveIn3D ('x', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				for (yy = 0 ; yy < 6 ; ++yy) {
					moveIn3D ('z', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
					elemList.Push (placeHole (tempGuid, cylinder));
				}
				moveIn3D ('z', cylinder.ang, -0.300 + params.height - (verticalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				for (yy = 0 ; yy < 6 ; ++yy) {
					moveIn3D ('z', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
					elemList.Push (placeHole (tempGuid, cylinder));
				}
				moveIn3D ('x', sqrPipe.ang, -width [xx], &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
			}
		}

		// ��� ������ (����) ��ġ
		sqrPipe.leftBottomX = params.leftBottomX;
		sqrPipe.leftBottomY = params.leftBottomY;
		sqrPipe.leftBottomZ = params.leftBottomZ;
		sqrPipe.ang = params.ang;
		sqrPipe.length = params.width - 0.100;
		sqrPipe.pipeAng = DegreeToRad (0);

		moveIn3D ('z', sqrPipe.ang, verticalGap + verticalBarLeftOffset, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		moveIn3D ('y', sqrPipe.ang, -(0.0635 + 0.075), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		moveIn3D ('x', sqrPipe.ang, 0.050, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

		// 1��
		cylinder.angleFromPlane = DegreeToRad (90.0);
		cylinder.length = 0.050;
		cylinder.radius = 0.013/2;
		cylinder.ang = params.ang;
		cylinder.leftBottomX = sqrPipe.leftBottomX;
		cylinder.leftBottomY = sqrPipe.leftBottomY;
		cylinder.leftBottomZ = sqrPipe.leftBottomZ;

		tempGuid = placeSqrPipe (sqrPipe);
		elemList.Push (tempGuid);
		moveIn3D ('z', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
		for (xx = 0 ; xx < 6 ; ++xx) {
			moveIn3D ('x', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			elemList.Push (placeHole (tempGuid, cylinder));
		}
		moveIn3D ('x', cylinder.ang, -0.300 + params.width - 0.100, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
		for (xx = 0 ; xx < 6 ; ++xx) {
			moveIn3D ('x', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			elemList.Push (placeHole (tempGuid, cylinder));
		}
		moveIn3D ('z', sqrPipe.ang, -(verticalGap + verticalBarLeftOffset) + params.height - (verticalGap + verticalBarRightOffset), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

		if (nVerticalBar > 1) {
			// 2��
			tempGuid = placeSqrPipe (sqrPipe);
			elemList.Push (tempGuid);
			cylinder.leftBottomX = sqrPipe.leftBottomX;
			cylinder.leftBottomY = sqrPipe.leftBottomY;
			cylinder.leftBottomZ = sqrPipe.leftBottomZ;
			moveIn3D ('z', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (xx = 0 ; xx < 6 ; ++xx) {
				moveIn3D ('x', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHole (tempGuid, cylinder));
			}
			moveIn3D ('x', cylinder.ang, -0.300 + params.width - 0.100, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (xx = 0 ; xx < 6 ; ++xx) {
				moveIn3D ('x', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHole (tempGuid, cylinder));
			}
		}

		// ������ ��ũ ��ġ (���� - ���ϴ�, �ֻ��)
		hook.leftBottomX = params.leftBottomX;
		hook.leftBottomY = params.leftBottomY;
		hook.leftBottomZ = params.leftBottomZ;
		hook.ang = params.ang;
		hook.iHookType = 2;
		hook.iHookShape = 2;
		hook.angX = DegreeToRad (0.0);
		hook.angY = DegreeToRad (180.0);

		moveIn3D ('y', hook.ang, -0.0885, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

		if (nVerEuroform >= 2) {
			moveIn3D ('z', hook.ang, params.height - height [0], &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
			moveIn3D ('x', hook.ang, 0.030 + 0.150 - 0.061, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
			// 1��
			height_t = 0.0;
			for (xx = 1 ; xx < nVerEuroform ; ++xx) {
				height_t += height [xx];
				elemList.Push (placeEuroformHook (hook));
				moveIn3D ('z', hook.ang, -height [xx], &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
			}
			moveIn3D ('z', hook.ang, height_t, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
			moveIn3D ('x', hook.ang, -0.150 + params.width - 0.150, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

			// ������ ��
			for (xx = 1 ; xx < nVerEuroform ; ++xx) {
				height_t += height [xx];
				elemList.Push (placeEuroformHook (hook));
				moveIn3D ('z', hook.ang, -height [xx], &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
			}
		}

		// ����������� ��ġ (���� - ���ϴ�, �ֻ���� ������ ������)
		hanger.leftBottomX = params.leftBottomX;
		hanger.leftBottomY = params.leftBottomY;
		hanger.leftBottomZ = params.leftBottomZ;
		hanger.ang = params.ang;
		hanger.angX = DegreeToRad (90.0);
		hanger.angY = DegreeToRad (270.0);

		moveIn3D ('y', hanger.ang, -0.0635, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);

		// 2 ~ [n-1]��
		if (nVerEuroform >= 2) {
			moveIn3D ('z', hanger.ang, params.height - 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
			moveIn3D ('x', hanger.ang, params.width - width [0], &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
			for (xx = 1 ; xx < nHorEuroform ; ++xx) {
				height_t = 0.0;
				for (yy = 0 ; yy < nVerEuroform ; ++yy) {
					// 1��
					if (yy == 0) {
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('z', hanger.ang, -(height [0] - 0.150), &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						height_t += height [0] - 0.150;
					// ������ ��
					} else if (yy == nVerEuroform - 1) {
						height_t += height [nVerEuroform-1] - 0.150;
						moveIn3D ('z', hanger.ang, -(height [nVerEuroform-1] - 0.150), &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
					// ������ ��
					} else {
						height_t += height [yy];
						if (abs (height [yy] - 0.600) < EPS) {
							moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('z', hanger.ang, -0.300, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						} else if (abs (height [yy] - 0.500) < EPS) {
							moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('z', hanger.ang, -0.200, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						} else if (abs (height [yy] - 0.450) < EPS) {
							moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						} else if (abs (height [yy] - 0.400) < EPS) {
							moveIn3D ('z', hanger.ang, -0.100, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('z', hanger.ang, -0.200, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('z', hanger.ang, -0.100, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						} else if (abs (height [yy] - 0.300) < EPS) {
							moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						} else if (abs (height [yy] - 0.200) < EPS) {
							moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('z', hanger.ang, -0.050, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						}
					}
				}
				moveIn3D ('z', hanger.ang, height_t, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
				moveIn3D ('x', hanger.ang, -width [xx], &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
			}
		}

		// ��� �ǽ�
		headpiece.leftBottomX = params.leftBottomX;
		headpiece.leftBottomY = params.leftBottomY;
		headpiece.leftBottomZ = params.leftBottomZ;
		headpiece.ang = params.ang;

		moveIn3D ('z', headpiece.ang, params.height - (verticalGap + verticalBarRightOffset - 0.0475) - 0.0975, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		moveIn3D ('y', headpiece.ang, -0.2685, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		moveIn3D ('x', headpiece.ang, params.width - 0.291 - 0.095, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);

		// ó�� ��
		elemList.Push (placeJointHeadpeace_hor (headpiece));
		moveIn3D ('z', headpiece.ang, (verticalGap + verticalBarRightOffset - 0.0475) - params.height + (verticalGap + verticalBarLeftOffset + 0.0475), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		if (nVerticalBar > 1)
			elemList.Push (placeJointHeadpeace_hor (headpiece));
		moveIn3D ('z', headpiece.ang, -(verticalGap + verticalBarRightOffset - 0.0475) + params.height - (verticalGap + verticalBarLeftOffset + 0.0475), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		//if (cell.verLen > 4.000) {
		//	elev_headpiece = 4.000 * 0.80;
		//} else {
		//	elev_headpiece = cell.verLen * 0.80;
		//}
		elev_headpiece = 1.900;
		moveIn3D ('x', headpiece.ang, -elev_headpiece, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		// ������ ��
			elemList.Push (placeJointHeadpeace_hor (headpiece));
		moveIn3D ('z', headpiece.ang, (verticalGap + verticalBarRightOffset - 0.0475) - params.height + (verticalGap + verticalBarLeftOffset + 0.0475), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		if (nVerticalBar > 1)
			elemList.Push (placeJointHeadpeace_hor (headpiece));

		// ����ö��
		fittings.leftBottomX = params.leftBottomX;
		fittings.leftBottomY = params.leftBottomY;
		fittings.leftBottomZ = params.leftBottomZ;
		fittings.ang = params.ang;
		fittings.angX = DegreeToRad (180.0);
		fittings.angY = DegreeToRad (90.0);
	
		moveIn3D ('z', fittings.ang, params.height - (verticalGap + verticalBarRightOffset + 0.081), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		moveIn3D ('y', fittings.ang, -0.1155, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		moveIn3D ('x', fittings.ang, params.width - 0.130 - 0.100, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);

		// ó�� ��
		for (xx = 0 ; xx <= nHorEuroform ; ++xx) {
			// 1��
			if (xx == 0) {
				elemList.Push (placeFittings (fittings));
				moveIn3D ('x', fittings.ang, -(width [xx] - 0.180), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
			// ������ ��
			} else if (xx == nHorEuroform) {
				moveIn3D ('x', fittings.ang, 0.120, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
				elemList.Push (placeFittings (fittings));
			// ������ ��
			} else {
				elemList.Push (placeFittings (fittings));
				moveIn3D ('x', fittings.ang, -width [xx], &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
			}
		}
		moveIn3D ('z', fittings.ang, (verticalGap + verticalBarRightOffset + 0.081) - params.height + (verticalGap + verticalBarLeftOffset + 0.081) - 0.162, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		moveIn3D ('x', fittings.ang, -(0.300 - params.width), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);

		if (nVerticalBar > 1) {
			// ������ ��
			for (xx = 0 ; xx <= nHorEuroform ; ++xx) {
				// 1��
				if (xx == 0) {
					elemList.Push (placeFittings (fittings));
					moveIn3D ('x', fittings.ang, -(width [xx] - 0.180), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
				// ������ ��
				} else if (xx == nHorEuroform) {
					moveIn3D ('x', fittings.ang, 0.120, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
					elemList.Push (placeFittings (fittings));
				// ������ ��
				} else {
					elemList.Push (placeFittings (fittings));
					moveIn3D ('x', fittings.ang, -width [xx], &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
				}
			}
		}

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
	}

	return	err;
}

// ���̺���(������) ��ġ
API_Guid	placeTableformOnSlabBottom (SlabTableform params)
{
	GSErrCode	err = NoError;

	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	double	aParam;
	double	bParam;
	Int32	addParNum;

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

	// ��ü �ε�
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, L("������ ���̺��� (���ǳ�) v1.0.gsm"));
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
	element.object.pos.x = params.leftBottomX;
	element.object.pos.y = params.leftBottomY;
	element.object.level = params.leftBottomZ;
	element.object.xRatio = params.horLen;
	element.object.yRatio = params.verLen;
	element.object.angle = params.ang;
	element.header.floorInd = floorInd;
	element.header.layer = layerInd_SlabTableform;

	// Ÿ��
	setParameterByName (&memo, "type", params.type);

	// ��ü ��ġ
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	// ==================== ������ʹ� ������ ���̺����� �μ�ǰ�� ��ġ�� ====================
	double	marginEnds;

	// �̵��Ͽ� ��ġ �ٷ����
	element.object.pos.x += ( params.verLen * sin(params.ang) );
	element.object.pos.y -= ( params.verLen * cos(params.ang) );

	// C���� ��ġ
	KSProfile	profile;

	profile.leftBottomX = params.leftBottomX;
	profile.leftBottomY = params.leftBottomY;
	profile.leftBottomZ = params.leftBottomZ;
	profile.ang = params.ang - DegreeToRad (90.0);
	profile.len = floor (params.horLen * 10) / 10;
	profile.angX = DegreeToRad (270.0);
	profile.angY = DegreeToRad (0.0);

	marginEnds = params.horLen - profile.len;

	moveIn3D ('x', profile.ang, -(0.300 - 0.006 - 0.020), &profile.leftBottomX, &profile.leftBottomY, &profile.leftBottomZ);
	moveIn3D ('y', profile.ang, marginEnds / 2, &profile.leftBottomX, &profile.leftBottomY, &profile.leftBottomZ);
	moveIn3D ('z', profile.ang, -0.0615, &profile.leftBottomX, &profile.leftBottomY, &profile.leftBottomZ);
	elemList.Push (placeProfile (profile));
	moveIn3D ('x', profile.ang, -(-(0.300 - 0.006 - 0.020) + params.verLen - (0.300 + 0.006 + 0.020)), &profile.leftBottomX, &profile.leftBottomY, &profile.leftBottomZ);
	elemList.Push (placeProfile (profile));

	profile.ang = params.ang + DegreeToRad (90.0);
	profile.leftBottomX = params.leftBottomX;
	profile.leftBottomY = params.leftBottomY;
	moveIn3D ('x', profile.ang, params.verLen, &profile.leftBottomX, &profile.leftBottomY, &profile.leftBottomZ);
	moveIn3D ('y', profile.ang, -params.horLen, &profile.leftBottomX, &profile.leftBottomY, &profile.leftBottomZ);
	moveIn3D ('x', profile.ang, -(0.300 - 0.006 - 0.020), &profile.leftBottomX, &profile.leftBottomY, &profile.leftBottomZ);
	moveIn3D ('y', profile.ang, marginEnds / 2, &profile.leftBottomX, &profile.leftBottomY, &profile.leftBottomZ);
	elemList.Push (placeProfile (profile));
	moveIn3D ('x', profile.ang, -(-(0.300 - 0.006 - 0.020) + params.verLen - (0.300 + 0.006 + 0.020)), &profile.leftBottomX, &profile.leftBottomY, &profile.leftBottomZ);
	elemList.Push (placeProfile (profile));

	// ����ö�� (�簢�ͼ�Ȱ��) ��ġ
	MetalFittingsWithRectWasher	fittings;

	fittings.leftBottomX = params.leftBottomX;
	fittings.leftBottomY = params.leftBottomY;
	fittings.leftBottomZ = params.leftBottomZ;
	fittings.ang = params.ang;
	fittings.angX = DegreeToRad (270.0);
	fittings.angY = DegreeToRad (0.0);
	fittings.bolt_len = 0.150;
	fittings.bolt_dia = 0.012;
	fittings.bWasher1 = false;
	fittings.washer_pos1 = 0.0;
	fittings.bWasher2 = true;
	fittings.washer_pos2 = 0.0766;
	fittings.washer_size = 0.100;
	strncpy (fittings.nutType, "������Ʈ", strlen ("������Ʈ"));

	moveIn3D ('y', fittings.ang, params.verLen, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);

	moveIn3D ('x', fittings.ang, 0.328, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	moveIn3D ('y', fittings.ang, -0.300, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	moveIn3D ('z', fittings.ang, -0.0499, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	elemList.Push (placeFittings (fittings));
	moveIn3D ('y', fittings.ang, 0.300 - params.verLen + 0.300, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	elemList.Push (placeFittings (fittings));
	moveIn3D ('x', fittings.ang, -0.328 + params.horLen - 0.328, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	elemList.Push (placeFittings (fittings));
	moveIn3D ('y', fittings.ang, -0.300 + params.verLen - 0.300, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	elemList.Push (placeFittings (fittings));
	
	return	element.header.guid;
}

// ������ ��ġ
API_Guid	placeEuroform (Euroform params)
{
	GSErrCode	err = NoError;

	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	double	aParam;
	double	bParam;
	Int32	addParNum;

	char	tempString [20];

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

	// ��ü �ε�
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, L("������v2.0.gsm"));
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
	element.object.pos.x = params.leftBottomX;
	element.object.pos.y = params.leftBottomY;
	element.object.level = params.leftBottomZ;
	element.object.xRatio = aParam;
	element.object.yRatio = bParam;
	element.object.angle = params.ang;
	element.header.floorInd = floorInd;
	element.header.layer = layerInd_Euroform;

	// ������
	setParameterByName (&memo, "u_comp", "������");

	// �԰�ǰ�� ���,
	if (params.eu_stan_onoff == true) {
		setParameterByName (&memo, "eu_stan_onoff", 1.0);	// �԰��� On/Off

		// �ʺ�
		sprintf (tempString, "%.0f", params.eu_wid * 1000);
		setParameterByName (&memo, "eu_wid", tempString);

		// ����
		sprintf (tempString, "%.0f", params.eu_hei * 1000);
		setParameterByName (&memo, "eu_hei", tempString);

	// ��԰�ǰ�� ���,
	} else {
		setParameterByName (&memo, "eu_stan_onoff", 0.0);		// �԰��� On/Off
		setParameterByName (&memo, "eu_wid2", params.eu_wid2);	// �ʺ�
		setParameterByName (&memo, "eu_hei2", params.eu_hei2);	// ����
	}

	// ��ġ����
	if (params.u_ins_wall == true) {
		strcpy (tempString, "�������");
	} else {
		strcpy (tempString, "��������");
		moveIn2D ('x', params.ang, params.height, &element.object.pos.x, &element.object.pos.y);
	}
	setParameterByName (&memo, "u_ins", tempString);
	setParameterByName (&memo, "ang_x", params.ang_x);	// ȸ��X

	// ��ü ��ġ
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	element.header.guid;
}

// ��ƿ�� ��ġ
API_Guid	placeSteelform (Euroform params)
{
	GSErrCode	err = NoError;

	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	double	aParam;
	double	bParam;
	Int32	addParNum;

	char	tempString [20];

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

	// ��ü �ε�
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, L("������v2.0.gsm"));
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
	element.object.pos.x = params.leftBottomX;
	element.object.pos.y = params.leftBottomY;
	element.object.level = params.leftBottomZ;
	element.object.xRatio = aParam;
	element.object.yRatio = bParam;
	element.object.angle = params.ang;
	element.header.floorInd = floorInd;
	element.header.layer = layerInd_Steelform;

	// ��ƿ��
	setParameterByName (&memo, "u_comp", "��ƿ��");

	// �԰�ǰ�� ���,
	if (params.eu_stan_onoff == true) {
		setParameterByName (&memo, "eu_stan_onoff", 1.0);	// �԰��� On/Off

		// �ʺ�
		sprintf (tempString, "%.0f", params.eu_wid * 1000);
		setParameterByName (&memo, "eu_wid", tempString);

		// ����
		sprintf (tempString, "%.0f", params.eu_hei * 1000);
		setParameterByName (&memo, "eu_hei", tempString);

	// ��԰�ǰ�� ���,
	} else {
		setParameterByName (&memo, "eu_stan_onoff", 0.0);		// �԰��� On/Off
		setParameterByName (&memo, "eu_wid2", params.eu_wid2);	// �ʺ�
		setParameterByName (&memo, "eu_hei2", params.eu_hei2);	// ����
	}

	// ��ġ����
	if (params.u_ins_wall == true) {
		strcpy (tempString, "�������");
	} else {
		strcpy (tempString, "��������");
	}
	setParameterByName (&memo, "u_ins", tempString);
	setParameterByName (&memo, "ang_x", params.ang_x);	// ȸ��X

	// ��ü ��ġ
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	element.header.guid;
}

// ���� ��ġ
API_Guid	placePlywood (Plywood params)
{
	GSErrCode	err = NoError;

	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	double	aParam;
	double	bParam;
	Int32	addParNum;

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

	// ��ü �ε�
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, L("����v1.0.gsm"));
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
	element.object.pos.x = params.leftBottomX;
	element.object.pos.y = params.leftBottomY;
	element.object.level = params.leftBottomZ;
	element.object.xRatio = aParam;
	element.object.yRatio = bParam;
	element.object.angle = params.ang;
	element.header.floorInd = floorInd;
	element.header.layer = layerInd_Plywood;

	// ����
	setParameterByName (&memo, "g_comp", "����");

	// �԰�
	setParameterByName (&memo, "p_stan", "��԰�");

	// ��ġ����
	if (params.w_dir == 1)
		setParameterByName (&memo, "w_dir", "�������");
	else if (params.w_dir == 2)
		setParameterByName (&memo, "w_dir", "��������");
	else if (params.w_dir == 3)
		setParameterByName (&memo, "w_dir", "�ٴڱ��");
	else if (params.w_dir == 4)
		setParameterByName (&memo, "w_dir", "�ٴڵ���");

	// �β�
	setParameterByName (&memo, "p_thk", "11.5T");

	// ����
	setParameterByName (&memo, "p_wid", params.p_wid);

	// ����
	setParameterByName (&memo, "p_leng", params.p_leng);

	// ����Ʋ
	setParameterByName (&memo, "sogak", 0.0);
	setParameterByName (&memo, "prof", "�Ұ�");

	// ��ü ��ġ
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	element.header.guid;
}

// �ٷ������̼� ��ġ
API_Guid	placeFillersp (FillerSpacer params)
{
	GSErrCode	err = NoError;

	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	double	aParam;
	double	bParam;
	Int32	addParNum;

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

	// ��ü �ε�
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, L("�ٷ������̼�v1.0.gsm"));
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
	element.object.pos.x = params.leftBottomX;
	element.object.pos.y = params.leftBottomY;
	element.object.level = params.leftBottomZ;
	element.object.xRatio = aParam;
	element.object.yRatio = bParam;
	element.object.angle = params.ang;
	element.header.floorInd = floorInd;
	element.header.layer = layerInd_Fillersp;

	// �β�
	setParameterByName (&memo, "f_thk", params.f_thk);

	// ����
	setParameterByName (&memo, "f_leng", params.f_leng);

	// ����
	setParameterByName (&memo, "f_ang", params.f_ang);

	// ȸ��
	setParameterByName (&memo, "f_rota", params.f_rota);

	// ��ü ��ġ
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	element.header.guid;
}

// �ƿ��ڳʾޱ� ��ġ
API_Guid	placeOutcornerAngle (OutcornerAngle params)
{
	GSErrCode	err = NoError;

	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	double	aParam;
	double	bParam;
	Int32	addParNum;

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

	// ��ü �ε�
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, L("�ƿ��ڳʾޱ�v1.0.gsm"));
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
	element.object.pos.x = params.leftBottomX;
	element.object.pos.y = params.leftBottomY;
	element.object.level = params.leftBottomZ;
	element.object.xRatio = aParam;
	element.object.yRatio = bParam;
	element.object.angle = params.ang;
	element.header.floorInd = floorInd;
	element.header.layer = layerInd_OutcornerAngle;

	// ����
	setParameterByName (&memo, "a_leng", params.a_leng);

	// ����
	setParameterByName (&memo, "a_ang", params.a_ang);

	// ��ü ��ġ
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	element.header.guid;
}

// �ƿ��ڳ��ǳ� ��ġ
API_Guid	placeOutcornerPanel (OutcornerPanel params)
{
	GSErrCode	err = NoError;

	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	double	aParam;
	double	bParam;
	Int32	addParNum;

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

	// ��ü �ε�
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, L("�ƿ��ڳ��ǳ�v1.0.gsm"));
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
	element.object.pos.x = params.leftBottomX;
	element.object.pos.y = params.leftBottomY;
	element.object.level = params.leftBottomZ;
	element.object.xRatio = aParam;
	element.object.yRatio = bParam;
	element.object.angle = params.ang;
	element.header.floorInd = floorInd;
	element.header.layer = layerInd_OutcornerPanel;

	setParameterByName (&memo, "wid_s", params.wid_s);		// ����
	setParameterByName (&memo, "leng_s", params.leng_s);	// ����
	setParameterByName (&memo, "hei_s", params.hei_s);		// ����
	setParameterByName (&memo, "dir_s", "�����");			// ��ġ����

	// ��ü ��ġ
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	element.header.guid;
}

// ���ڳ��ǳ� ��ġ
API_Guid	placeIncornerPanel (IncornerPanel params)
{
	GSErrCode	err = NoError;

	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	double	aParam;
	double	bParam;
	Int32	addParNum;

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

	// ��ü �ε�
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, L("���ڳ��ǳ�v1.0.gsm"));
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
	element.object.pos.x = params.leftBottomX;
	element.object.pos.y = params.leftBottomY;
	element.object.level = params.leftBottomZ;
	element.object.xRatio = aParam;
	element.object.yRatio = bParam;
	element.object.angle = params.ang;
	element.header.floorInd = floorInd;
	element.header.layer = layerInd_IncornerPanel;

	setParameterByName (&memo, "wid_s", params.wid_s);		// ����
	setParameterByName (&memo, "leng_s", params.leng_s);	// ����
	setParameterByName (&memo, "hei_s", params.hei_s);		// ����
	setParameterByName (&memo, "dir_s", "�����");			// ��ġ����

	// ��ü ��ġ
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	element.header.guid;
}

// KS�������� ��ġ
API_Guid	placeProfile (KSProfile params)
{
	GSErrCode	err = NoError;

	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	double	aParam;
	double	bParam;
	Int32	addParNum;

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

	// ��ü �ε�
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, L("KS��������v1.0.gsm"));
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
	element.object.pos.x = params.leftBottomX;
	element.object.pos.y = params.leftBottomY;
	element.object.level = params.leftBottomZ;
	element.object.xRatio = aParam;
	element.object.yRatio = bParam;
	element.object.angle = params.ang;
	element.header.floorInd = floorInd;
	element.header.layer = layerInd_Profile;

	setParameterByName (&memo, "type", "��");				// �з�
	setParameterByName (&memo, "shape", "C����");			// ����
	setParameterByName (&memo, "iAnchor", 8);				// ��Ŀ ����Ʈ (8, �ϴ�)
	setParameterByName (&memo, "nom", "75 x 40 x 5 x 7");	// �԰�

	setParameterByName (&memo, "len", params.len);			// ����
	setParameterByName (&memo, "ZZYZX", params.len);		// ����
	setParameterByName (&memo, "angX", params.angX);		// ȸ��X
	setParameterByName (&memo, "angY", params.angY);		// ȸ��Y

	// ��ü ��ġ
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	element.header.guid;
}

// ����ö�� (�簢�ͼ�Ȱ��) ��ġ
API_Guid	placeFittings (MetalFittingsWithRectWasher params)
{
	GSErrCode	err = NoError;

	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	double	aParam;
	double	bParam;
	Int32	addParNum;

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

	// ��ü �ε�
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, L("����ö�� (�簢�ͼ�Ȱ��) v1.0.gsm"));
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
	element.object.pos.x = params.leftBottomX;
	element.object.pos.y = params.leftBottomY;
	element.object.level = params.leftBottomZ;
	element.object.xRatio = aParam;
	element.object.yRatio = bParam;
	element.object.angle = params.ang;
	element.header.floorInd = floorInd;
	element.header.layer = layerInd_Join;

	setParameterByName (&memo, "angX", params.angX);	// ȸ��X
	setParameterByName (&memo, "angY", params.angY);	// ȸ��Y

	setParameterByName (&memo, "bolt_len", params.bolt_len);			// ��Ʈ ����
	setParameterByName (&memo, "bolt_dia", params.bolt_dia);			// ��Ʈ ����
	setParameterByName (&memo, "bWasher1", params.bWasher1);			// �ͼ�1 On/Off
	setParameterByName (&memo, "washer_pos1", params.washer_pos1);		// �ͼ�1 ��ġ
	setParameterByName (&memo, "bWasher2", params.bWasher2);			// �ͼ�2 On/Off
	setParameterByName (&memo, "washer_pos2", params.washer_pos2);		// �ͼ�2 ��ġ
	setParameterByName (&memo, "washer_size", params.washer_size);		// �ͼ� ũ��
	setParameterByName (&memo, "nutType", params.nutType);				// ��Ʈ Ÿ��

	// ��ü ��ġ
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	element.header.guid;
}

// ��� ������ ��ġ
API_Guid	placeSqrPipe (SquarePipe params)
{
	GSErrCode	err = NoError;

	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	double	aParam;
	double	bParam;
	Int32	addParNum;

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

	// ��ü �ε�
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, L("���������v1.0.gsm"));
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
	element.object.pos.x = params.leftBottomX;
	element.object.pos.y = params.leftBottomY;
	element.object.level = params.leftBottomZ;
	element.object.xRatio = aParam;
	element.object.yRatio = bParam;
	element.object.angle = params.ang;
	element.header.floorInd = floorInd;
	element.header.layer = layerInd_RectPipe;

	setParameterByName (&memo, "p_comp", "�簢������");		// �簢������
	setParameterByName (&memo, "p_leng", params.length);	// ����
	setParameterByName (&memo, "p_ang", params.pipeAng);	// ����

	// ��ü ��ġ
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	element.header.guid;
}

// �ɺ�Ʈ ��Ʈ ��ġ
API_Guid	placePinbolt (PinBoltSet params)
{
	GSErrCode	err = NoError;

	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	double	aParam;
	double	bParam;
	Int32	addParNum;

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

	// ��ü �ε�
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, L("�ɺ�Ʈ��Ʈv1.0.gsm"));
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
	element.object.pos.x = params.leftBottomX;
	element.object.pos.y = params.leftBottomY;
	element.object.level = params.leftBottomZ;
	element.object.xRatio = aParam;
	element.object.yRatio = bParam;
	element.object.angle = params.ang;
	element.header.floorInd = floorInd;
	element.header.layer = layerInd_PinBolt;

	// �ɺ�Ʈ 90�� ȸ��
	if (params.bPinBoltRot90)
		setParameterByName (&memo, "bRotated", 1.0);
	else
		setParameterByName (&memo, "bRotated", 0.0);

	setParameterByName (&memo, "bolt_len", params.boltLen);		// ��Ʈ ����
	setParameterByName (&memo, "bolt_dia", 0.010);				// ��Ʈ ����
	setParameterByName (&memo, "washer_pos", 0.050);			// �ͼ� ��ġ
	setParameterByName (&memo, "washer_size", 0.100);			// �ͼ� ũ��
	setParameterByName (&memo, "angX", params.angX);			// X�� ȸ��
	setParameterByName (&memo, "angY", params.angY);			// Y�� ȸ��

	// ��ü ��ġ
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	element.header.guid;
}

// ��ü Ÿ�� ��ġ
API_Guid	placeWalltie (WallTie params)
{
	GSErrCode	err = NoError;

	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	double	aParam;
	double	bParam;
	Int32	addParNum;

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

	// ��ü �ε�
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, L("��ü Ÿ�� v1.0.gsm"));
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
	element.object.pos.x = params.leftBottomX;
	element.object.pos.y = params.leftBottomY;
	element.object.level = params.leftBottomZ;
	element.object.xRatio = aParam;
	element.object.yRatio = bParam;
	element.object.angle = params.ang + DegreeToRad (90.0);
	element.header.floorInd = floorInd;
	element.header.layer = layerInd_WallTie;

	setParameterByName (&memo, "boltLen", params.boltLen);		// ��Ʈ ���� (�� �β� + 327mm �ʰ��̸� 100 ������ �������� ���� ���� ��)
	setParameterByName (&memo, "boltDia", 0.012);				// ��Ʈ ����
	setParameterByName (&memo, "bSqrWasher", 1.0);				// �簢�ͻ�
	setParameterByName (&memo, "washer_size", 0.100);			// �簢�ͻ� ũ��
	setParameterByName (&memo, "nutType", "Ÿ�� 1");			// ��Ʈ Ÿ��
	setParameterByName (&memo, "bEmbedPipe", 1.0);				// ��ü ���� ������
	setParameterByName (&memo, "pipeInnerDia", 0.012);			// ������ ����
	setParameterByName (&memo, "pipeThk", 0.002);				// ������ �β�
	
	// ������ ������, ���� (�� �β���ŭ ����)
	setParameterByName (&memo, "pipeBeginPos", params.pipeBeg);
	setParameterByName (&memo, "pipeEndPos", params.pipeEnd);
	
	// ��,���� ���Ӽ� ��ġ (�� �β� + 327mm ��ŭ ����)
	setParameterByName (&memo, "posLClamp", params.clampBeg);
	setParameterByName (&memo, "posRClamp", params.clampEnd);
	
	setParameterByName (&memo, "angY", DegreeToRad (0.0));		// ȸ�� Y

	// ��ü ��ġ
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	element.header.guid;
}

// ����ǽ� ��ġ (���� ����: Ÿ�� A)
API_Guid	placeHeadpiece_ver (HeadpieceOfPushPullProps params)
{
	GSErrCode	err = NoError;

	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	double	aParam;
	double	bParam;
	Int32	addParNum;

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

	// ��ü �ε�
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, L("RS Push-Pull Props ����ǽ� v2.0 (�ξ�� ����).gsm"));
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
	element.object.pos.x = params.leftBottomX;
	element.object.pos.y = params.leftBottomY;
	element.object.level = params.leftBottomZ;
	element.object.xRatio = aParam;
	element.object.yRatio = bParam;
	element.object.angle = params.ang;
	element.header.floorInd = floorInd;
	element.header.layer = layerInd_HeadPiece;

	setParameterByName (&memo, "type", "Ÿ�� A");			// Ÿ��
	setParameterByName (&memo, "plateThk", 0.009);			// ö�� �β�
	setParameterByName (&memo, "angX", DegreeToRad (0.0));	// ȸ��X
	setParameterByName (&memo, "angY", DegreeToRad (0.0));	// ȸ��Y

	// ��ü ��ġ
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	element.header.guid;
}

// ����ǽ� ��ġ (���� ����: Ÿ�� B)
API_Guid	placeHeadpiece_hor (HeadpieceOfPushPullProps params)
{
	GSErrCode	err = NoError;

	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	double	aParam;
	double	bParam;
	Int32	addParNum;

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

	// ��ü �ε�
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, L("RS Push-Pull Props ����ǽ� v2.0 (�ξ�� ����).gsm"));
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
	element.object.pos.x = params.leftBottomX;
	element.object.pos.y = params.leftBottomY;
	element.object.level = params.leftBottomZ;
	element.object.xRatio = aParam;
	element.object.yRatio = bParam;
	element.object.angle = params.ang;
	element.header.floorInd = floorInd;
	element.header.layer = layerInd_HeadPiece;

	setParameterByName (&memo, "type", "Ÿ�� B");			// Ÿ��
	setParameterByName (&memo, "plateThk", 0.009);			// ö�� �β�
	setParameterByName (&memo, "angX", DegreeToRad (0.0));	// ȸ��X
	setParameterByName (&memo, "angY", DegreeToRad (90.0));	// ȸ��Y
	element.object.level += 0.200;

	// ��ü ��ġ
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	element.header.guid;
}

// �簢������ ����ö�� ��ġ
API_Guid	placeFittings (MetalFittings params)
{
	GSErrCode	err = NoError;

	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	double	aParam;
	double	bParam;
	Int32	addParNum;

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

	// ��ü �ε�
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, L("�簢������ ����ö�� v1.0.gsm"));
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
	element.object.pos.x = params.leftBottomX;
	element.object.pos.y = params.leftBottomY;
	element.object.level = params.leftBottomZ;
	element.object.xRatio = aParam;
	element.object.yRatio = bParam;
	element.object.angle = params.ang;
	element.header.floorInd = floorInd;
	element.header.layer = layerInd_Join;

	setParameterByName (&memo, "angX", params.angX);	// ȸ��X
	setParameterByName (&memo, "angY", params.angY);	// ȸ��Y

	// ��ü ��ġ
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	element.header.guid;
}

// ������Ʈ�� Push-Pull Props ��ġ (���� ����: Ÿ�� A)
API_Guid	placeJointHeadpeace_ver (HeadpieceOfPushPullProps params)
{
	GSErrCode	err = NoError;

	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	double	aParam;
	double	bParam;
	Int32	addParNum;

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

	// ��ü �ε�
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, L("������Ʈ�� Push-Pull Props ����ǽ� v1.0.gsm"));
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
	element.object.pos.x = params.leftBottomX;
	element.object.pos.y = params.leftBottomY;
	element.object.level = params.leftBottomZ;
	element.object.xRatio = aParam;
	element.object.yRatio = bParam;
	element.object.angle = params.ang;
	element.header.floorInd = floorInd;
	element.header.layer = layerInd_HeadPiece;

	// Ÿ�� ����
	setParameterByName (&memo, "type", "Ÿ�� A");

	// ��ü ��ġ
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	element.header.guid;
}

// ������Ʈ�� Push-Pull Props ��ġ (���� ����: Ÿ�� B)
API_Guid	placeJointHeadpeace_hor (HeadpieceOfPushPullProps params)
{
	GSErrCode	err = NoError;

	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	double	aParam;
	double	bParam;
	Int32	addParNum;

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

	// ��ü �ε�
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, L("������Ʈ�� Push-Pull Props ����ǽ� v1.0.gsm"));
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
	element.object.pos.x = params.leftBottomX;
	element.object.pos.y = params.leftBottomY;
	element.object.level = params.leftBottomZ;
	element.object.xRatio = aParam;
	element.object.yRatio = bParam;
	element.object.angle = params.ang;
	element.header.floorInd = floorInd;
	element.header.layer = layerInd_HeadPiece;

	// Ÿ�� ����
	setParameterByName (&memo, "type", "Ÿ�� B");

	// ��ü ��ġ
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	element.header.guid;
}

// ������ ��ũ ��ġ
API_Guid	placeEuroformHook (EuroformHook params)
{
	GSErrCode	err = NoError;

	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	double	aParam;
	double	bParam;
	Int32	addParNum;

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

	// ��ü �ε�
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, L("������ ��ũ.gsm"));
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
	element.object.pos.x = params.leftBottomX;
	element.object.pos.y = params.leftBottomY;
	element.object.level = params.leftBottomZ;
	element.object.xRatio = aParam;
	element.object.yRatio = bParam;
	element.object.angle = params.ang + DegreeToRad (180.0);
	element.header.floorInd = floorInd;
	element.header.layer = layerInd_EuroformHook;

	setParameterByName (&memo, "rotationX", params.angX);			// X�� ȸ��
	setParameterByName (&memo, "rotationY", params.angY);			// Y�� ȸ��
	setParameterByName (&memo, "iHookType", params.iHookType);		// (1)����-��, (2)����-��
	setParameterByName (&memo, "iHookShape", params.iHookShape);	// (1)����, (2)�簢

	// ��ü ��ġ
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	element.header.guid;
}

// �������� ��� ��ġ
API_Guid	placeRectpipeHanger (RectPipeHanger params)
{
	GSErrCode	err = NoError;

	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	double	aParam;
	double	bParam;
	Int32	addParNum;

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

	// ��ü �ε�
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, L("�����������.gsm"));
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
	element.object.pos.x = params.leftBottomX;
	element.object.pos.y = params.leftBottomY;
	element.object.level = params.leftBottomZ;
	element.object.xRatio = aParam;
	element.object.yRatio = bParam;
	element.object.angle = params.ang - DegreeToRad (90);
	element.header.floorInd = floorInd;
	element.header.layer = layerInd_RectpipeHanger;

	setParameterByName (&memo, "m_type", "�����������");	// ǰ��
	setParameterByName (&memo, "angX", params.angX);		// ȸ��X
	setParameterByName (&memo, "angY", params.angY);		// ȸ��Y

	// ��ü ��ġ
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	element.header.guid;
}

// Ÿ���� ���� ��� ��ü�� ��ġ�ϰ� ����, "���� 19" ��ü�� �̿���
API_Guid	placeHole (API_Guid guid_Target, Cylinder operator_Object)
{
	GSErrCode	err = NoError;

	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	double	aParam;
	double	bParam;
	Int32	addParNum;

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

	// ��ü �ε�
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, L("���� 19.gsm"));
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
	element.object.pos.x = operator_Object.leftBottomX;
	element.object.pos.y = operator_Object.leftBottomY;
	element.object.level = operator_Object.leftBottomZ;
	element.object.xRatio = aParam;
	element.object.yRatio = bParam;
	element.object.angle = operator_Object.ang;
	element.header.floorInd = floorInd;
	element.header.layer = layerInd_Hidden;

	// ���� ���� "����-����" ����
	setParameterByName (&memo, "edit_mode", "����-����");
	setParameterByName (&memo, "end_mode", "����");
	setParameterByName (&memo, "gamma", operator_Object.angleFromPlane);
	setParameterByName (&memo, "length", operator_Object.length);
	setParameterByName (&memo, "radius_1", operator_Object.radius);

	// ��ü ��ġ
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	// guid_Target Ÿ���ϱ�
	ACAPI_Element_SolidLink_Create (guid_Target, element.header.guid, APISolid_Substract, 0);

	return	element.header.guid;
}


// ��ü�� ���̾ �����ϱ� ���� ���̾�α�
short DGCALLBACK convertVirtualTCOHandler1 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	API_UCCallbackType	ucb;

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, "������ ���̾� �����ϱ�");

			//////////////////////////////////////////////////////////// ������ ��ġ (�⺻ ��ư)
			// ���� ��ư
			DGSetItemText (dialogID, DG_OK, "Ȯ ��");

			// ���� ��ư
			DGSetItemText (dialogID, DG_CANCEL, "�� ��");

			// üũ�ڽ�: ���̾� ����
			DGSetItemText (dialogID, CHECKBOX_LAYER_COUPLING, "���̾� ����");
			DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, TRUE);

			// ���̾� ���� ��
			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, "���纰 ���̾� ����");
			DGSetItemText (dialogID, LABEL_LAYER_SLABTABLEFORM, "������ ���̺���");
			DGSetItemText (dialogID, LABEL_LAYER_PROFILE, "C����");
			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM, "������");
			DGSetItemText (dialogID, LABEL_LAYER_RECTPIPE, "��� ������");
			DGSetItemText (dialogID, LABEL_LAYER_PINBOLT, "�ɺ�Ʈ ��Ʈ");
			DGSetItemText (dialogID, LABEL_LAYER_WALLTIE, "��ü Ÿ��");
			DGSetItemText (dialogID, LABEL_LAYER_JOIN, "����ö��");
			DGSetItemText (dialogID, LABEL_LAYER_HEADPIECE, "����ǽ�");
			DGSetItemText (dialogID, LABEL_LAYER_STEELFORM, "��ƿ��");
			DGSetItemText (dialogID, LABEL_LAYER_PLYWOOD, "����");
			DGSetItemText (dialogID, LABEL_LAYER_FILLERSP, "�ٷ������̼�");
			DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER_ANGLE, "�ƿ��ڳʾޱ�");
			DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER_PANEL, "�ƿ��ڳ��ǳ�");
			DGSetItemText (dialogID, LABEL_LAYER_INCORNER_PANEL, "���ڳ��ǳ�");
			DGSetItemText (dialogID, LABEL_LAYER_RECTPIPE_HANGER, "�����������");
			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM_HOOK, "������ ��ũ");
			DGSetItemText (dialogID, LABEL_LAYER_HIDDEN, "����");

			// ���� ��Ʈ�� �ʱ�ȭ
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_LAYER_SLABTABLEFORM;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, 1);
			if (bLayerInd_SlabTableform == true) {
				DGEnableItem (dialogID, LABEL_LAYER_SLABTABLEFORM);
				DGEnableItem (dialogID, USERCONTROL_LAYER_SLABTABLEFORM);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_SLABTABLEFORM);
				DGDisableItem (dialogID, USERCONTROL_LAYER_SLABTABLEFORM);
			}

			ucb.itemID	 = USERCONTROL_LAYER_PROFILE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, 1);
			if (bLayerInd_Profile == true) {
				DGEnableItem (dialogID, LABEL_LAYER_PROFILE);
				DGEnableItem (dialogID, USERCONTROL_LAYER_PROFILE);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_PROFILE);
				DGDisableItem (dialogID, USERCONTROL_LAYER_PROFILE);
			}

			ucb.itemID	 = USERCONTROL_LAYER_EUROFORM;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, 1);
			if (bLayerInd_Euroform == true) {
				DGEnableItem (dialogID, LABEL_LAYER_EUROFORM);
				DGEnableItem (dialogID, USERCONTROL_LAYER_EUROFORM);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_EUROFORM);
				DGDisableItem (dialogID, USERCONTROL_LAYER_EUROFORM);
			}

			ucb.itemID	 = USERCONTROL_LAYER_RECTPIPE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, 1);
			if (bLayerInd_RectPipe == true) {
				DGEnableItem (dialogID, LABEL_LAYER_RECTPIPE);
				DGEnableItem (dialogID, USERCONTROL_LAYER_RECTPIPE);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_RECTPIPE);
				DGDisableItem (dialogID, USERCONTROL_LAYER_RECTPIPE);
			}

			ucb.itemID	 = USERCONTROL_LAYER_PINBOLT;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, 1);
			if (bLayerInd_PinBolt == true) {
				DGEnableItem (dialogID, LABEL_LAYER_PINBOLT);
				DGEnableItem (dialogID, USERCONTROL_LAYER_PINBOLT);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_PINBOLT);
				DGDisableItem (dialogID, USERCONTROL_LAYER_PINBOLT);
			}

			ucb.itemID	 = USERCONTROL_LAYER_WALLTIE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, 1);
			if (bLayerInd_WallTie == true) {
				DGEnableItem (dialogID, LABEL_LAYER_WALLTIE);
				DGEnableItem (dialogID, USERCONTROL_LAYER_WALLTIE);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_WALLTIE);
				DGDisableItem (dialogID, USERCONTROL_LAYER_WALLTIE);
			}

			ucb.itemID	 = USERCONTROL_LAYER_JOIN;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, 1);
			if (bLayerInd_Join == true) {
				DGEnableItem (dialogID, LABEL_LAYER_JOIN);
				DGEnableItem (dialogID, USERCONTROL_LAYER_JOIN);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_JOIN);
				DGDisableItem (dialogID, USERCONTROL_LAYER_JOIN);
			}

			ucb.itemID	 = USERCONTROL_LAYER_HEADPIECE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, 1);
			if (bLayerInd_HeadPiece == true) {
				DGEnableItem (dialogID, LABEL_LAYER_HEADPIECE);
				DGEnableItem (dialogID, USERCONTROL_LAYER_HEADPIECE);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_HEADPIECE);
				DGDisableItem (dialogID, USERCONTROL_LAYER_HEADPIECE);
			}

			ucb.itemID	 = USERCONTROL_LAYER_STEELFORM;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, 1);
			if (bLayerInd_Steelform == true) {
				DGEnableItem (dialogID, LABEL_LAYER_STEELFORM);
				DGEnableItem (dialogID, USERCONTROL_LAYER_STEELFORM);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_STEELFORM);
				DGDisableItem (dialogID, USERCONTROL_LAYER_STEELFORM);
			}

			ucb.itemID	 = USERCONTROL_LAYER_PLYWOOD;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, 1);
			if (bLayerInd_Plywood == true) {
				DGEnableItem (dialogID, LABEL_LAYER_PLYWOOD);
				DGEnableItem (dialogID, USERCONTROL_LAYER_PLYWOOD);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_PLYWOOD);
				DGDisableItem (dialogID, USERCONTROL_LAYER_PLYWOOD);
			}

			ucb.itemID	 = USERCONTROL_LAYER_FILLERSP;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, 1);
			if (bLayerInd_Fillersp == true) {
				DGEnableItem (dialogID, LABEL_LAYER_FILLERSP);
				DGEnableItem (dialogID, USERCONTROL_LAYER_FILLERSP);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_FILLERSP);
				DGDisableItem (dialogID, USERCONTROL_LAYER_FILLERSP);
			}

			ucb.itemID	 = USERCONTROL_LAYER_OUTCORNER_ANGLE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, 1);
			if (bLayerInd_OutcornerAngle == true) {
				DGEnableItem (dialogID, LABEL_LAYER_OUTCORNER_ANGLE);
				DGEnableItem (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_OUTCORNER_ANGLE);
				DGDisableItem (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE);
			}

			ucb.itemID	 = USERCONTROL_LAYER_OUTCORNER_PANEL;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, 1);
			if (bLayerInd_OutcornerPanel == true) {
				DGEnableItem (dialogID, LABEL_LAYER_OUTCORNER_PANEL);
				DGEnableItem (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_OUTCORNER_PANEL);
				DGDisableItem (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL);
			}

			ucb.itemID	 = USERCONTROL_LAYER_INCORNER_PANEL;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, 1);
			if (bLayerInd_IncornerPanel == true) {
				DGEnableItem (dialogID, LABEL_LAYER_INCORNER_PANEL);
				DGEnableItem (dialogID, USERCONTROL_LAYER_INCORNER_PANEL);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_INCORNER_PANEL);
				DGDisableItem (dialogID, USERCONTROL_LAYER_INCORNER_PANEL);
			}

			ucb.itemID	 = USERCONTROL_LAYER_RECTPIPE_HANGER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, 1);
			if (bLayerInd_RectpipeHanger == true) {
				DGEnableItem (dialogID, LABEL_LAYER_RECTPIPE_HANGER);
				DGEnableItem (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_RECTPIPE_HANGER);
				DGDisableItem (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER);
			}

			ucb.itemID	 = USERCONTROL_LAYER_EUROFORM_HOOK;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, 1);
			if (bLayerInd_EuroformHook == true) {
				DGEnableItem (dialogID, LABEL_LAYER_EUROFORM_HOOK);
				DGEnableItem (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_EUROFORM_HOOK);
				DGDisableItem (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK);
			}

			ucb.itemID	 = USERCONTROL_LAYER_HIDDEN;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_HIDDEN, 1);
			if (bLayerInd_Hidden == true) {
				DGEnableItem (dialogID, LABEL_LAYER_HIDDEN);
				DGEnableItem (dialogID, USERCONTROL_LAYER_HIDDEN);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_HIDDEN);
				DGDisableItem (dialogID, USERCONTROL_LAYER_HIDDEN);
			}
			break;

		case DG_MSG_CHANGE:
			// ���̾� ���� �ٲ�
			if (DGGetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING) == 1) {
				switch (item) {
					case USERCONTROL_LAYER_SLABTABLEFORM:
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						break;
					case USERCONTROL_LAYER_PROFILE:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						break;
					case USERCONTROL_LAYER_EUROFORM:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						break;
					case USERCONTROL_LAYER_RECTPIPE:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						break;
					case USERCONTROL_LAYER_PINBOLT:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						break;
					case USERCONTROL_LAYER_WALLTIE:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						break;
					case USERCONTROL_LAYER_JOIN:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						break;
					case USERCONTROL_LAYER_HEADPIECE:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						break;
					case USERCONTROL_LAYER_STEELFORM:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						break;
					case USERCONTROL_LAYER_PLYWOOD:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						break;
					case USERCONTROL_LAYER_FILLERSP:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						break;
					case USERCONTROL_LAYER_OUTCORNER_ANGLE:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						break;
					case USERCONTROL_LAYER_OUTCORNER_PANEL:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						break;
					case USERCONTROL_LAYER_INCORNER_PANEL:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						break;
					case USERCONTROL_LAYER_RECTPIPE_HANGER:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						break;
					case USERCONTROL_LAYER_EUROFORM_HOOK:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						break;
				}
			}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// ���̾� ��ȣ ����
					if (bLayerInd_SlabTableform == true)	layerInd_SlabTableform	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM);
					if (bLayerInd_Profile == true)			layerInd_Profile		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE);
					if (bLayerInd_Euroform == true)			layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM);
					if (bLayerInd_RectPipe == true)			layerInd_RectPipe		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE);
					if (bLayerInd_PinBolt == true)			layerInd_PinBolt		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT);
					if (bLayerInd_WallTie == true)			layerInd_WallTie		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE);
					if (bLayerInd_Join == true)				layerInd_Join			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN);
					if (bLayerInd_HeadPiece == true)		layerInd_HeadPiece		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE);
					if (bLayerInd_Steelform == true)		layerInd_Steelform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM);
					if (bLayerInd_Plywood == true)			layerInd_Plywood		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD);
					if (bLayerInd_Fillersp == true)			layerInd_Fillersp		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP);
					if (bLayerInd_OutcornerAngle == true)	layerInd_OutcornerAngle	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE);
					if (bLayerInd_OutcornerPanel == true)	layerInd_OutcornerPanel	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL);
					if (bLayerInd_IncornerPanel == true)	layerInd_IncornerPanel	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL);
					if (bLayerInd_RectpipeHanger == true)	layerInd_RectpipeHanger	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER);
					if (bLayerInd_EuroformHook == true)		layerInd_EuroformHook	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK);
					if (bLayerInd_Hidden == true)			layerInd_Hidden			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_HIDDEN);

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
