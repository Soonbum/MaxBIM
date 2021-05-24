#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "LibraryConvert.hpp"

//const GS::uchar_t*	gsmUFOM = L("������v2.0.gsm");
//const GS::uchar_t*	gsmSPIP = L("���������v1.0.gsm");
//const GS::uchar_t*	gsmPINB = L("�ɺ�Ʈ��Ʈv1.0.gsm");
//const GS::uchar_t*	gsmTIE = L("��ü Ÿ�� v1.0.gsm");
//const GS::uchar_t*	gsmCLAM = L("����Ŭ����v1.0.gsm");
//const GS::uchar_t*	gsmPUSH = L("RS Push-Pull Props ����ǽ� v2.0 (�ξ�� ����).gsm");
//const GS::uchar_t*	gsmJOIN = L("����ö�� (�簢�ͼ�Ȱ��) v1.0.gsm");
//const GS::uchar_t*	gsmPLYW = L("����v1.0.gsm");
//const GS::uchar_t*	gsmTIMB = L("����v1.0.gsm");

using namespace libraryConvertDG;

short	floorInd;		// ���� �������� �� �ε��� ����

// �ش� ���̾ ����ϴ��� ����
static bool		bLayerInd_Euroform;			// ���̾� ��ȣ: ������
static bool		bLayerInd_RectPipe;			// ���̾� ��ȣ: ��� ������
static bool		bLayerInd_PinBolt;			// ���̾� ��ȣ: �ɺ�Ʈ ��Ʈ
static bool		bLayerInd_WallTie;			// ���̾� ��ȣ: ��ü Ÿ��
static bool		bLayerInd_Clamp;			// ���̾� ��ȣ: ���� Ŭ����
static bool		bLayerInd_HeadPiece;		// ���̾� ��ȣ: ����ǽ�
static bool		bLayerInd_Join;				// ���̾� ��ȣ: ����ö��
static bool		bLayerInd_Plywood;			// ���̾� ��ȣ: ����
static bool		bLayerInd_Wood;				// ���̾� ��ȣ: ����

static bool		bLayerInd_SlabTableform;	// ���̾� ��ȣ: ������ ���̺���
//static bool		bLayerInd_Plywood;			// ���̾� ��ȣ: ����
//static bool		bLayerInd_Wood;				// ���̾� ��ȣ: ����
static bool		bLayerInd_Profile;			// ���̾� ��ȣ: KS��������
static bool		bLayerInd_Fittings;			// ���̾� ��ȣ: ����ö��

static bool		bLayerInd_Steelform;		// ���̾� ��ȣ: ��ƿ��
//static bool		bLayerInd_Plywood;			// ���̾� ��ȣ: ����
static bool		bLayerInd_Fillersp;			// ���̾� ��ȣ: �ٷ������̼�
static bool		bLayerInd_OutcornerAngle;	// ���̾� ��ȣ: �ƿ��ڳʾޱ�
static bool		bLayerInd_OutcornerPanel;	// ���̾� ��ȣ: �ƿ��ڳʾޱ�
static bool		bLayerInd_IncornerPanel;	// ���̾� ��ȣ: ���ڳʾޱ�

// �ش� ���̾��� ��ȣ
static short	layerInd_Euroform;			// ���̾� ��ȣ: ������
static short	layerInd_RectPipe;			// ���̾� ��ȣ: ��� ������
static short	layerInd_PinBolt;			// ���̾� ��ȣ: �ɺ�Ʈ ��Ʈ
static short	layerInd_WallTie;			// ���̾� ��ȣ: ��ü Ÿ��
static short	layerInd_Clamp;				// ���̾� ��ȣ: ���� Ŭ����
static short	layerInd_HeadPiece;			// ���̾� ��ȣ: ����ǽ�
static short	layerInd_Join;				// ���̾� ��ȣ: ����ö��
static short	layerInd_Plywood;			// ���̾� ��ȣ: ����
static short	layerInd_Wood;				// ���̾� ��ȣ: ����

static short	layerInd_SlabTableform;		// ���̾� ��ȣ: ������ ���̺���
//static short	layerInd_Plywood;			// ���̾� ��ȣ: ����
//static short	layerInd_Wood;				// ���̾� ��ȣ: ����
static short	layerInd_Profile;			// ���̾� ��ȣ: KS��������
static short	layerInd_Fittings;			// ���̾� ��ȣ: ����ö��

static short	layerInd_Steelform;			// ���̾� ��ȣ: ��ƿ��
//static short	layerInd_Plywood;			// ���̾� ��ȣ: ����
static short	layerInd_Fillersp;			// ���̾� ��ȣ: �ٷ������̼�
static short	layerInd_OutcornerAngle;	// ���̾� ��ȣ: �ƿ��ڳʾޱ�
static short	layerInd_OutcornerPanel;	// ���̾� ��ȣ: �ƿ��ڳʾޱ�
static short	layerInd_IncornerPanel;		// ���̾� ��ȣ: ���ڳʾޱ�

static GS::Array<API_Guid>	elemList;		// �׷�ȭ�� ���� ������ ��������� GUID�� ���� ������


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
	char			objType [32];		// ���̺���(��), ���̺���(������), ������, ��ƿ��, ����, �ٷ������̼�, �ƿ��ڳʾޱ�, �ƿ��ڳ��ǳ�, ���ڳ��ǳ�
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
	GS::Array<API_Guid>&	objects = GS::Array<API_Guid> ();
	GS::Array<API_Guid>&	objectsRetry = GS::Array<API_Guid> ();


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
	bLayerInd_Clamp = false;
	bLayerInd_HeadPiece = false;
	bLayerInd_Join = false;
	bLayerInd_Plywood = false;
	bLayerInd_Wood = false;

	bLayerInd_SlabTableform = false;
	bLayerInd_Profile = false;
	bLayerInd_Fittings = false;

	bLayerInd_Steelform = false;
	bLayerInd_Fillersp = false;
	bLayerInd_OutcornerAngle = false;
	bLayerInd_OutcornerPanel = false;
	bLayerInd_IncornerPanel = false;

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

		if (strncmp (productName, "���� ������", strlen ("���� ������")) == 0) {
			
			// ���� �������� �Ķ���� �� �ҷ�����
			tempStr = getParameterStringByName (&memo, "objType");
			strncpy (objType, tempStr, strlen (tempStr));
			objType [strlen (tempStr)] = '\0';

			if (strncmp (objType, "���̺���(��)", strlen ("���̺���(��)")) == 0) {
				bLayerInd_Euroform = true;
				bLayerInd_RectPipe = true;
				bLayerInd_PinBolt = true;
				bLayerInd_WallTie = true;
				bLayerInd_Clamp = true;
				bLayerInd_HeadPiece = true;
				bLayerInd_Join = true;
				bLayerInd_Plywood = true;
				bLayerInd_Wood = true;

			} else if (strncmp (objType, "���̺���(������)", strlen ("���̺���(������)")) == 0) {
				bLayerInd_SlabTableform = true;
				bLayerInd_Plywood = true;
				bLayerInd_Wood = true;
				bLayerInd_Profile = true;
				bLayerInd_Fittings = true;

			} else if (strncmp (objType, "������", strlen ("������")) == 0) {
				bLayerInd_Euroform = true;

			} else if (strncmp (objType, "��ƿ��", strlen ("��ƿ��")) == 0) {
				bLayerInd_Steelform = true;

			} else if (strncmp (objType, "����", strlen ("����")) == 0) {
				bLayerInd_Plywood = true;

			} else if (strncmp (objType, "�ٷ������̼�", strlen ("�ٷ������̼�")) == 0) {
				bLayerInd_Fillersp = true;

			} else if (strncmp (objType, "�ƿ��ڳʾޱ�", strlen ("�ƿ��ڳʾޱ�")) == 0) {
				bLayerInd_OutcornerAngle = true;

			} else if (strncmp (objType, "�ƿ��ڳ��ǳ�", strlen ("�ƿ��ڳ��ǳ�")) == 0) {
				bLayerInd_OutcornerPanel = true;

			} else if (strncmp (objType, "���ڳ��ǳ�", strlen ("���ڳ��ǳ�")) == 0) {
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

		if (strncmp (productName, "���� ������", strlen ("���� ������")) == 0) {

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
			leftSide = getParameterValueByName (&memo, "leftSide");						// ����(1), ������(0)
			bRegularSize = getParameterValueByName (&memo, "bRegularSize");				// ���� ũ��(1)
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
				// ...
				//GSErrCode	placeTableformOnWall (WallTableform params);
			} else if (strncmp (objType, "���̺���(������)", strlen ("���̺���(������)")) == 0) {

				//const char* typeStr = getParameterStringByName (&memo, "type");
				//strncpy (slabtableform.type, typeStr, strlen (typeStr));
				//slabtableform.type [strlen (typeStr)] = '\0';
				//ACAPI_WriteReport (slabtableform.type, true);

				//if (strncmp (dir, "�ٴڱ��", strlen ("�ٴڱ��")) == 0) {

				//	placeTableformOnSlabBottom (slabtableform);

				//} else if (strncmp (dir, "�ٴڵ���", strlen ("�ٴڵ���")) == 0) {

				//	placeTableformOnSlabBottom (slabtableform);
				//}

			} else if (strncmp (objType, "������", strlen ("������")) == 0) {

				euroform.ang = elem.object.angle;
				euroform.eu_stan_onoff = bRegularSize;
				if (bRegularSize == true) {
					euroform.eu_wid = unit_A;
					euroform.eu_hei = unit_ZZYZX;
				} else {
					euroform.eu_wid2 = unit_A;
					euroform.eu_hei2 = unit_ZZYZX;
				}

				if (strncmp (dir, "�������", strlen ("�������")) == 0) {
					euroform.leftBottomX = elem.object.pos.x;
					euroform.leftBottomY = elem.object.pos.y;
					euroform.leftBottomZ = elem.object.level;
					euroform.u_ins_wall = true;					// true: �������, false: ��������
					euroform.ang_x = DegreeToRad (90.0);		// ��(90)

					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							placeEuroform (euroform);
							moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('z', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					}

					// ���
					if (strncmp (coverSide, "���", strlen ("���")) == 0) {
						euroform.leftBottomX = elem.object.pos.x;
						euroform.leftBottomY = elem.object.pos.y;
						euroform.leftBottomZ = elem.object.level;
						euroform.ang = elem.object.angle + DegreeToRad (180.0);
						moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('y', elem.object.angle, oppSideOffset, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						
						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							for (yy = 0 ; yy < num_A ; ++yy) {
								placeEuroform (euroform);
								moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
							}
							moveIn3D ('x', elem.object.angle, -unit_A * num_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
					}
				} else if (strncmp (dir, "��������", strlen ("��������")) == 0) {
					euroform.leftBottomX = elem.object.pos.x;
					euroform.leftBottomY = elem.object.pos.y;
					euroform.leftBottomZ = elem.object.level;
					moveIn3D ('x', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					euroform.u_ins_wall = false;				// true: �������, false: ��������
					euroform.ang_x = DegreeToRad (90.0);		// ��(90)

					for (xx = 0 ; xx < num_A ; ++xx) {
						for (yy = 0 ; yy < num_ZZYZX ; ++yy) {
							placeEuroform (euroform);
							moveIn3D ('x', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_ZZYZX * num_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('z', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					}

					// ���
					if (strncmp (coverSide, "���", strlen ("���")) == 0) {
						euroform.leftBottomX = elem.object.pos.x;
						euroform.leftBottomY = elem.object.pos.y;
						euroform.leftBottomZ = elem.object.level;
						euroform.ang = elem.object.angle + DegreeToRad (180.0);
						moveIn3D ('y', elem.object.angle, oppSideOffset, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						
						for (xx = 0 ; xx < num_A ; ++xx) {
							for (yy = 0 ; yy < num_ZZYZX ; ++yy) {
								placeEuroform (euroform);
								moveIn3D ('x', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
							}
							moveIn3D ('x', elem.object.angle, -unit_ZZYZX * num_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
							moveIn3D ('z', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
					}
				} else if (strncmp (dir, "�ٴڱ��", strlen ("�ٴڱ��")) == 0) {
					euroform.leftBottomX = elem.object.pos.x;
					euroform.leftBottomY = elem.object.pos.y;
					euroform.leftBottomZ = elem.object.level;
					moveIn3D ('y', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					euroform.u_ins_wall = true;					// true: �������, false: ��������
					euroform.ang_x = DegreeToRad (0.0);			// õ��(0)
					
					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							placeEuroform (euroform);
							moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('y', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					}
				} else if (strncmp (dir, "�ٴڵ���", strlen ("�ٴڵ���")) == 0) {
					euroform.leftBottomX = elem.object.pos.x;
					euroform.leftBottomY = elem.object.pos.y;
					euroform.leftBottomZ = elem.object.level;
					euroform.u_ins_wall = true;					// true: �������, false: ��������
					euroform.ang_x = DegreeToRad (180.0);		// �ٴ�(180)

					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							placeEuroform (euroform);
							moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('y', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					}
				}
			} else if (strncmp (objType, "��ƿ��", strlen ("��ƿ��")) == 0) {

				euroform.ang = elem.object.angle;
				euroform.eu_stan_onoff = bRegularSize;
				if (bRegularSize == true) {
					euroform.eu_wid = unit_A;
					euroform.eu_hei = unit_ZZYZX;
				} else {
					euroform.eu_wid2 = unit_A;
					euroform.eu_hei2 = unit_ZZYZX;
				}

				if (strncmp (dir, "�������", strlen ("�������")) == 0) {
					euroform.leftBottomX = elem.object.pos.x;
					euroform.leftBottomY = elem.object.pos.y;
					euroform.leftBottomZ = elem.object.level;
					euroform.u_ins_wall = true;					// true: �������, false: ��������
					euroform.ang_x = DegreeToRad (90.0);		// ��(90)

					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							placeSteelform (euroform);
							moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('z', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					}

					// ���
					if (strncmp (coverSide, "���", strlen ("���")) == 0) {
						euroform.leftBottomX = elem.object.pos.x;
						euroform.leftBottomY = elem.object.pos.y;
						euroform.leftBottomZ = elem.object.level;
						euroform.ang = elem.object.angle + DegreeToRad (180.0);
						moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('y', elem.object.angle, oppSideOffset, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						
						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							for (yy = 0 ; yy < num_A ; ++yy) {
								placeSteelform (euroform);
								moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
							}
							moveIn3D ('x', elem.object.angle, -unit_A * num_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
					}
				} else if (strncmp (dir, "��������", strlen ("��������")) == 0) {
					euroform.leftBottomX = elem.object.pos.x;
					euroform.leftBottomY = elem.object.pos.y;
					euroform.leftBottomZ = elem.object.level;
					moveIn3D ('x', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					euroform.u_ins_wall = false;				// true: �������, false: ��������
					euroform.ang_x = DegreeToRad (90.0);		// ��(90)

					for (xx = 0 ; xx < num_A ; ++xx) {
						for (yy = 0 ; yy < num_ZZYZX ; ++yy) {
							placeSteelform (euroform);
							moveIn3D ('x', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_ZZYZX * num_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('z', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					}

					// ���
					if (strncmp (coverSide, "���", strlen ("���")) == 0) {
						euroform.leftBottomX = elem.object.pos.x;
						euroform.leftBottomY = elem.object.pos.y;
						euroform.leftBottomZ = elem.object.level;
						euroform.ang = elem.object.angle + DegreeToRad (180.0);
						moveIn3D ('y', elem.object.angle, oppSideOffset, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						
						for (xx = 0 ; xx < num_A ; ++xx) {
							for (yy = 0 ; yy < num_ZZYZX ; ++yy) {
								placeSteelform (euroform);
								moveIn3D ('x', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
							}
							moveIn3D ('x', elem.object.angle, -unit_ZZYZX * num_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
							moveIn3D ('z', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
					}
				} else if (strncmp (dir, "�ٴڱ��", strlen ("�ٴڱ��")) == 0) {
					euroform.leftBottomX = elem.object.pos.x;
					euroform.leftBottomY = elem.object.pos.y;
					euroform.leftBottomZ = elem.object.level;
					moveIn3D ('y', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					euroform.u_ins_wall = true;					// true: �������, false: ��������
					euroform.ang_x = DegreeToRad (0.0);			// õ��(0)
					
					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							placeSteelform (euroform);
							moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('y', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					}
				} else if (strncmp (dir, "�ٴڵ���", strlen ("�ٴڵ���")) == 0) {
					euroform.leftBottomX = elem.object.pos.x;
					euroform.leftBottomY = elem.object.pos.y;
					euroform.leftBottomZ = elem.object.level;
					euroform.u_ins_wall = true;					// true: �������, false: ��������
					euroform.ang_x = DegreeToRad (180.0);		// �ٴ�(180)

					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							placeSteelform (euroform);
							moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('y', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					}
				}
			} else if (strncmp (objType, "����", strlen ("����")) == 0) {

				plywood.ang = elem.object.angle;
				plywood.leftBottomX = elem.object.pos.x;
				plywood.leftBottomY = elem.object.pos.y;
				plywood.leftBottomZ = elem.object.level;
				plywood.p_wid = unit_A;
				plywood.p_leng = unit_ZZYZX;

				if (strncmp (dir, "�������", strlen ("�������")) == 0) {
					plywood.w_dir = 1;

					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							placePlywood (plywood);
							moveIn3D ('x', elem.object.angle, unit_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						moveIn3D ('z', elem.object.angle, unit_ZZYZX, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
					}

					// ���
					if (strncmp (coverSide, "���", strlen ("���")) == 0) {
						plywood.leftBottomX = elem.object.pos.x;
						plywood.leftBottomY = elem.object.pos.y;
						plywood.leftBottomZ = elem.object.level;
						plywood.ang = elem.object.angle + DegreeToRad (180.0);
						moveIn3D ('x', elem.object.angle, unit_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						moveIn3D ('y', elem.object.angle, oppSideOffset, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							for (yy = 0 ; yy < num_A ; ++yy) {
								placePlywood (plywood);
								moveIn3D ('x', elem.object.angle, unit_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
							}
							moveIn3D ('x', elem.object.angle, -unit_A * num_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						}
					}
				} else if (strncmp (dir, "��������", strlen ("��������")) == 0) {
					plywood.w_dir = 2;

					for (xx = 0 ; xx < num_A ; ++xx) {
						for (yy = 0 ; yy < num_ZZYZX ; ++yy) {
							placePlywood (plywood);
							moveIn3D ('x', elem.object.angle, unit_ZZYZX, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_ZZYZX * num_ZZYZX, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						moveIn3D ('z', elem.object.angle, unit_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
					}

					// ���
					if (strncmp (coverSide, "���", strlen ("���")) == 0) {
						plywood.leftBottomX = elem.object.pos.x;
						plywood.leftBottomY = elem.object.pos.y;
						plywood.leftBottomZ = elem.object.level;
						plywood.ang = elem.object.angle + DegreeToRad (180.0);
						moveIn3D ('x', elem.object.angle, unit_ZZYZX, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						moveIn3D ('y', elem.object.angle, oppSideOffset, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);

						for (xx = 0 ; xx < num_A ; ++xx) {
							for (yy = 0 ; yy < num_ZZYZX ; ++yy) {
								placePlywood (plywood);
								moveIn3D ('x', elem.object.angle, unit_ZZYZX, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
							}
							moveIn3D ('x', elem.object.angle, -unit_ZZYZX * num_ZZYZX, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
							moveIn3D ('z', elem.object.angle, unit_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						}
					}
				} else if (strncmp (dir, "�ٴڱ��", strlen ("�ٴڱ��")) == 0) {
					plywood.w_dir = 3;

					moveIn3D ('x', elem.object.angle, unit_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
					plywood.ang = elem.object.angle + DegreeToRad (90.0);

					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							placePlywood (plywood);
							moveIn3D ('x', elem.object.angle, unit_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						moveIn3D ('y', elem.object.angle, unit_ZZYZX, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
					}
				} else if (strncmp (dir, "�ٴڵ���", strlen ("�ٴڵ���")) == 0) {
					plywood.w_dir = 4;

					moveIn3D ('x', elem.object.angle, unit_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
					plywood.ang = elem.object.angle + DegreeToRad (90.0);

					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							placePlywood (plywood);
							moveIn3D ('x', elem.object.angle, unit_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						moveIn3D ('y', elem.object.angle, unit_ZZYZX, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
					}
				}
			} else if (strncmp (objType, "�ٷ������̼�", strlen ("�ٷ������̼�")) == 0) {

				fillersp.ang = elem.object.angle;
				fillersp.leftBottomX = elem.object.pos.x;
				fillersp.leftBottomY = elem.object.pos.y;
				fillersp.leftBottomZ = elem.object.level;
				fillersp.f_thk = unit_A;
				fillersp.f_leng = unit_ZZYZX;

				if (strncmp (dir, "�������", strlen ("�������")) == 0) {
					fillersp.f_ang = DegreeToRad (90.0);
					fillersp.f_rota = DegreeToRad (0.0);
					moveIn3D ('x', elem.object.angle, unit_A, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);

					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							placeFillersp (fillersp);
							moveIn3D ('x', elem.object.angle, unit_A, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
						moveIn3D ('z', elem.object.angle, unit_ZZYZX, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
					}

					// ���
					if (strncmp (coverSide, "���", strlen ("���")) == 0) {
						fillersp.leftBottomX = elem.object.pos.x;
						fillersp.leftBottomY = elem.object.pos.y;
						fillersp.leftBottomZ = elem.object.level;
						fillersp.ang = elem.object.angle + DegreeToRad (180.0);
						moveIn3D ('y', elem.object.angle, oppSideOffset, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							for (yy = 0 ; yy < num_A ; ++yy) {
								placeFillersp (fillersp);
								moveIn3D ('x', elem.object.angle, unit_A, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
							}
							moveIn3D ('x', elem.object.angle, -unit_A * num_A, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
						}
					}
				} else if (strncmp (dir, "��������", strlen ("��������")) == 0) {
					fillersp.f_ang = DegreeToRad (0.0);
					fillersp.f_rota = DegreeToRad (0.0);

					for (xx = 0 ; xx < num_A ; ++xx) {
						for (yy = 0 ; yy < num_ZZYZX ; ++yy) {
							placeFillersp (fillersp);
							moveIn3D ('x', elem.object.angle, unit_ZZYZX, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_ZZYZX * num_ZZYZX, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
						moveIn3D ('z', elem.object.angle, unit_A, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
					}

					// ���
					if (strncmp (coverSide, "���", strlen ("���")) == 0) {
						fillersp.leftBottomX = elem.object.pos.x;
						fillersp.leftBottomY = elem.object.pos.y;
						fillersp.leftBottomZ = elem.object.level;
						fillersp.ang = elem.object.angle + DegreeToRad (180.0);
						moveIn3D ('x', elem.object.angle, unit_ZZYZX, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
						moveIn3D ('y', elem.object.angle, oppSideOffset, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);

						for (xx = 0 ; xx < num_A ; ++xx) {
							for (yy = 0 ; yy < num_ZZYZX ; ++yy) {
								placeFillersp (fillersp);
								moveIn3D ('x', elem.object.angle, unit_ZZYZX, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
							}
							moveIn3D ('x', elem.object.angle, -unit_ZZYZX * num_ZZYZX, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
							moveIn3D ('z', elem.object.angle, unit_A, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
						}
					}
				}
			} else if (strncmp (objType, "�ƿ��ڳʾޱ�", strlen ("�ƿ��ڳʾޱ�")) == 0) {

				outcornerAngle.ang = elem.object.angle;
				outcornerAngle.leftBottomX = elem.object.pos.x;
				outcornerAngle.leftBottomY = elem.object.pos.y;
				outcornerAngle.leftBottomZ = elem.object.level;
				outcornerAngle.a_leng = unit_ZZYZX;
				
				if (leftSide == true) {
					if (strncmp (dir, "�������", strlen ("�������")) == 0) {
						moveIn3D ('x', outcornerAngle.ang, unit_A, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						outcornerAngle.a_ang = DegreeToRad (90.0);

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							outcornerAngle.ang = elem.object.angle + DegreeToRad (180.0);
							placeOutcornerAngle (outcornerAngle);
							outcornerAngle.ang = elem.object.angle;
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						}

						// ���
						if (strncmp (coverSide, "���", strlen ("���")) == 0) {
							outcornerAngle.leftBottomX = elem.object.pos.x;
							outcornerAngle.leftBottomY = elem.object.pos.y;
							outcornerAngle.leftBottomZ = elem.object.level;
							outcornerAngle.a_ang = DegreeToRad (90.0);
							moveIn3D ('x', outcornerAngle.ang, unit_A, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
							moveIn3D ('y', elem.object.angle, oppSideOffset, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);

							for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
								outcornerAngle.ang = elem.object.angle + DegreeToRad (90.0);
								placeOutcornerAngle (outcornerAngle);
								outcornerAngle.ang = elem.object.angle;
								moveIn3D ('z', elem.object.angle, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
							}
						}
					} else if (strncmp (dir, "�ٴڱ��", strlen ("�ٴڱ��")) == 0) {
						moveIn3D ('x', outcornerAngle.ang, unit_A, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						outcornerAngle.a_ang = DegreeToRad (0.0);

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							outcornerAngle.ang = elem.object.angle + DegreeToRad (90.0);
							placeOutcornerAngle (outcornerAngle);
							outcornerAngle.ang = elem.object.angle;
							moveIn3D ('y', elem.object.angle, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						}
					} else if (strncmp (dir, "�ٴڵ���", strlen ("�ٴڵ���")) == 0) {
						moveIn3D ('x', elem.object.angle, unit_A, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						moveIn3D ('y', elem.object.angle, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						outcornerAngle.a_ang = DegreeToRad (180.0);

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							outcornerAngle.ang = elem.object.angle + DegreeToRad (90.0);
							placeOutcornerAngle (outcornerAngle);
							outcornerAngle.ang = elem.object.angle;
							moveIn3D ('y', elem.object.angle, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						}
					}
				} else {
					if (strncmp (dir, "�������", strlen ("�������")) == 0) {
						outcornerAngle.a_ang = DegreeToRad (90.0);

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							outcornerAngle.ang = elem.object.angle + DegreeToRad (270.0);
							placeOutcornerAngle (outcornerAngle);
							outcornerAngle.ang = elem.object.angle;
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						}

						// ���
						if (strncmp (coverSide, "���", strlen ("���")) == 0) {
							outcornerAngle.leftBottomX = elem.object.pos.x;
							outcornerAngle.leftBottomY = elem.object.pos.y;
							outcornerAngle.leftBottomZ = elem.object.level;
							outcornerAngle.a_ang = DegreeToRad (90.0);
							moveIn3D ('y', elem.object.angle, oppSideOffset, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);

							for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
								outcornerAngle.ang = elem.object.angle;
								placeOutcornerAngle (outcornerAngle);
								moveIn3D ('z', elem.object.angle, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
							}
						}
					} else if (strncmp (dir, "�ٴڱ��", strlen ("�ٴڱ��")) == 0) {
						moveIn3D ('y', outcornerAngle.ang, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						outcornerAngle.a_ang = DegreeToRad (0.0);

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							outcornerAngle.ang = elem.object.angle + DegreeToRad (270.0);
							placeOutcornerAngle (outcornerAngle);
							outcornerAngle.ang = elem.object.angle;
							moveIn3D ('y', elem.object.angle, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						}
					} else if (strncmp (dir, "�ٴڵ���", strlen ("�ٴڵ���")) == 0) {
						outcornerAngle.a_ang = DegreeToRad (180.0);

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							outcornerAngle.ang = elem.object.angle + DegreeToRad (270.0);
							placeOutcornerAngle (outcornerAngle);
							outcornerAngle.ang = elem.object.angle;
							moveIn3D ('y', elem.object.angle, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						}
					}
				}
			} else if (strncmp (objType, "�ƿ��ڳ��ǳ�", strlen ("�ƿ��ڳ��ǳ�")) == 0) {

				outcornerPanel.ang = elem.object.angle;
				outcornerPanel.leftBottomX = elem.object.pos.x;
				outcornerPanel.leftBottomY = elem.object.pos.y;
				outcornerPanel.leftBottomZ = elem.object.level;

				if (leftSide == true) {
					if (strncmp (dir, "�������", strlen ("�������")) == 0) {
						outcornerPanel.wid_s = unit_A;
						outcornerPanel.leng_s = unit_B;
						outcornerPanel.hei_s = unit_ZZYZX;

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							placeOutcornerPanel (outcornerPanel);
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &outcornerPanel.leftBottomX, &outcornerPanel.leftBottomY, &outcornerPanel.leftBottomZ);
						}

						// ���
						if (strncmp (coverSide, "���", strlen ("���")) == 0) {
							outcornerPanel.leftBottomX = elem.object.pos.x;
							outcornerPanel.leftBottomY = elem.object.pos.y;
							outcornerPanel.leftBottomZ = elem.object.level;

							outcornerPanel.wid_s = unit_B;
							outcornerPanel.leng_s = unit_A;
							outcornerPanel.hei_s = unit_ZZYZX;

							moveIn3D ('y', elem.object.angle, oppSideOffset, &outcornerPanel.leftBottomX, &outcornerPanel.leftBottomY, &outcornerPanel.leftBottomZ);

							for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
								outcornerPanel.ang = elem.object.angle - DegreeToRad (90.0);
								placeOutcornerPanel (outcornerPanel);
								outcornerPanel.ang = elem.object.angle;
								moveIn3D ('z', elem.object.angle, unit_ZZYZX, &outcornerPanel.leftBottomX, &outcornerPanel.leftBottomY, &outcornerPanel.leftBottomZ);
							}
						}
					}
				} else {
					if (strncmp (dir, "�������", strlen ("�������")) == 0) {
						outcornerPanel.wid_s = unit_B;
						outcornerPanel.leng_s = unit_A;
						outcornerPanel.hei_s = unit_ZZYZX;

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							outcornerPanel.ang = elem.object.angle + DegreeToRad (90.0);
							placeOutcornerPanel (outcornerPanel);
							outcornerPanel.ang = elem.object.angle;
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &outcornerPanel.leftBottomX, &outcornerPanel.leftBottomY, &outcornerPanel.leftBottomZ);
						}

						// ���
						if (strncmp (coverSide, "���", strlen ("���")) == 0) {
							outcornerPanel.leftBottomX = elem.object.pos.x;
							outcornerPanel.leftBottomY = elem.object.pos.y;
							outcornerPanel.leftBottomZ = elem.object.level;

							outcornerPanel.wid_s = unit_A;
							outcornerPanel.leng_s = unit_B;
							outcornerPanel.hei_s = unit_ZZYZX;

							moveIn3D ('y', elem.object.angle, oppSideOffset, &outcornerPanel.leftBottomX, &outcornerPanel.leftBottomY, &outcornerPanel.leftBottomZ);

							for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
								outcornerPanel.ang = elem.object.angle + DegreeToRad (180.0);
								placeOutcornerPanel (outcornerPanel);
								outcornerPanel.ang = elem.object.angle;
								moveIn3D ('z', elem.object.angle, unit_ZZYZX, &outcornerPanel.leftBottomX, &outcornerPanel.leftBottomY, &outcornerPanel.leftBottomZ);
							}
						}
					}
				}
			} else if (strncmp (objType, "���ڳ��ǳ�", strlen ("���ڳ��ǳ�")) == 0) {

				incornerPanel.ang = elem.object.angle;
				incornerPanel.leftBottomX = elem.object.pos.x;
				incornerPanel.leftBottomY = elem.object.pos.y;
				incornerPanel.leftBottomZ = elem.object.level;

				if (leftSide == true) {
					if (strncmp (dir, "�������", strlen ("�������")) == 0) {
						incornerPanel.wid_s = unit_B;
						incornerPanel.leng_s = unit_A;
						incornerPanel.hei_s = unit_ZZYZX;

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							incornerPanel.ang = elem.object.angle + DegreeToRad (270.0);
							placeIncornerPanel (incornerPanel);
							incornerPanel.ang = elem.object.angle;
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &incornerPanel.leftBottomX, &incornerPanel.leftBottomY, &incornerPanel.leftBottomZ);
						}

						// ���
						if (strncmp (coverSide, "���", strlen ("���")) == 0) {
							incornerPanel.leftBottomX = elem.object.pos.x;
							incornerPanel.leftBottomY = elem.object.pos.y;
							incornerPanel.leftBottomZ = elem.object.level;

							incornerPanel.wid_s = unit_A;
							incornerPanel.leng_s = unit_B;
							incornerPanel.hei_s = unit_ZZYZX;

							moveIn3D ('y', elem.object.angle, oppSideOffset, &incornerPanel.leftBottomX, &incornerPanel.leftBottomY, &incornerPanel.leftBottomZ);

							for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
								incornerPanel.ang = elem.object.angle;
								placeIncornerPanel (incornerPanel);
								moveIn3D ('z', elem.object.angle, unit_ZZYZX, &incornerPanel.leftBottomX, &incornerPanel.leftBottomY, &incornerPanel.leftBottomZ);
							}
						}
					}
				} else {
					if (strncmp (dir, "�������", strlen ("�������")) == 0) {
						incornerPanel.wid_s = unit_A;
						incornerPanel.leng_s = unit_B;
						incornerPanel.hei_s = unit_ZZYZX;

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							incornerPanel.ang = elem.object.angle + DegreeToRad (180.0);
							placeIncornerPanel (incornerPanel);
							incornerPanel.ang = elem.object.angle;
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &incornerPanel.leftBottomX, &incornerPanel.leftBottomY, &incornerPanel.leftBottomZ);
						}

						// ���
						if (strncmp (coverSide, "���", strlen ("���")) == 0) {
							incornerPanel.leftBottomX = elem.object.pos.x;
							incornerPanel.leftBottomY = elem.object.pos.y;
							incornerPanel.leftBottomZ = elem.object.level;

							incornerPanel.wid_s = unit_B;
							incornerPanel.leng_s = unit_A;
							incornerPanel.hei_s = unit_ZZYZX;

							moveIn3D ('y', elem.object.angle, oppSideOffset, &incornerPanel.leftBottomX, &incornerPanel.leftBottomY, &incornerPanel.leftBottomZ);

							for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
								incornerPanel.ang = elem.object.angle + DegreeToRad (90.0);
								placeIncornerPanel (incornerPanel);
								incornerPanel.ang = elem.object.angle;
								moveIn3D ('z', elem.object.angle, unit_ZZYZX, &incornerPanel.leftBottomX, &incornerPanel.leftBottomY, &incornerPanel.leftBottomZ);
							}
						}
					}
				}
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

// ���̺���(��) ��ġ
GSErrCode	placeTableformOnWall (WallTableform params)
{
	GSErrCode	err = NoError;

	return	err;
}

// ���̺���(������) ��ġ
GSErrCode	placeTableformOnSlabBottom (SlabTableform params)
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
		return err;
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
	element.header.layer = layerInd_SlabTableform;

	// Ÿ��
	setParameterByName (&memo, "type", params.type);

	// ��ü ��ġ
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	err;
}

// ������ ��ġ
GSErrCode	placeEuroform (Euroform params)
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
		return err;
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
	}
	setParameterByName (&memo, "u_ins", tempString);
	setParameterByName (&memo, "ang_x", params.ang_x);	// ȸ��X

	// ��ü ��ġ
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	err;
}

// ��ƿ�� ��ġ
GSErrCode	placeSteelform (Euroform params)
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
		return err;
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

	return	err;
}

// ���� ��ġ
GSErrCode	placePlywood (Plywood params)
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
		return err;
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
	setParameterByName (&memo, "sogak", TRUE);
	setParameterByName (&memo, "prof", "�Ұ�");

	// ��ü ��ġ
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	err;
}

// �ٷ������̼� ��ġ
GSErrCode	placeFillersp (FillerSpacer params)
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
		return err;
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

	return	err;
}

// �ƿ��ڳʾޱ� ��ġ
GSErrCode	placeOutcornerAngle (OutcornerAngle params)
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
		return err;
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

	return	err;
}

// �ƿ��ڳ��ǳ� ��ġ
GSErrCode	placeOutcornerPanel (OutcornerPanel params)
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
		return err;
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

	return	err;
}

// ���ڳ��ǳ� ��ġ
GSErrCode	placeIncornerPanel (IncornerPanel params)
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
		return err;
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

	return	err;
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

			// ���̾� ���� ��
			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, "���纰 ���̾� ����");
			DGSetItemText (dialogID, LABEL_LAYER_SLABTABLEFORM, "������ ���̺���");
			DGSetItemText (dialogID, LABEL_LAYER_PROFILE, "C����");
			DGSetItemText (dialogID, LABEL_LAYER_FITTINGS, "����ö��");
			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM, "������");
			DGSetItemText (dialogID, LABEL_LAYER_RECTPIPE, "��� ������");
			DGSetItemText (dialogID, LABEL_LAYER_PINBOLT, "�ɺ�Ʈ ��Ʈ");
			DGSetItemText (dialogID, LABEL_LAYER_WALLTIE, "��ü Ÿ��");
			DGSetItemText (dialogID, LABEL_LAYER_JOIN, "����ö��");
			DGSetItemText (dialogID, LABEL_LAYER_HEADPIECE, "����ǽ�");
			DGSetItemText (dialogID, LABEL_LAYER_STEELFORM, "��ƿ��");
			DGSetItemText (dialogID, LABEL_LAYER_PLYWOOD, "����");
			DGSetItemText (dialogID, LABEL_LAYER_WOOD, "����");
			DGSetItemText (dialogID, LABEL_LAYER_FILLERSP, "�ٷ������̼�");
			DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER_ANGLE, "�ƿ��ڳʾޱ�");
			DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER_PANEL, "�ƿ��ڳ��ǳ�");
			DGSetItemText (dialogID, LABEL_LAYER_INCORNER_PANEL, "���ڳ��ǳ�");

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

			ucb.itemID	 = USERCONTROL_LAYER_FITTINGS;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_FITTINGS, 1);
			if (bLayerInd_Fittings == true) {
				DGEnableItem (dialogID, LABEL_LAYER_FITTINGS);
				DGEnableItem (dialogID, USERCONTROL_LAYER_FITTINGS);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_FITTINGS);
				DGDisableItem (dialogID, USERCONTROL_LAYER_FITTINGS);
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

			ucb.itemID	 = USERCONTROL_LAYER_WOOD;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, 1);
			if (bLayerInd_Wood == true) {
				DGEnableItem (dialogID, LABEL_LAYER_WOOD);
				DGEnableItem (dialogID, USERCONTROL_LAYER_WOOD);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_WOOD);
				DGDisableItem (dialogID, USERCONTROL_LAYER_WOOD);
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

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// ���̾� ��ȣ ����
					if (bLayerInd_SlabTableform == true)	layerInd_SlabTableform	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM);
					if (bLayerInd_Profile == true)			layerInd_Profile		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE);
					if (bLayerInd_Fittings == true)			layerInd_Fittings		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_FITTINGS);
					if (bLayerInd_Euroform == true)			layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM);
					if (bLayerInd_RectPipe == true)			layerInd_RectPipe		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE);
					if (bLayerInd_PinBolt == true)			layerInd_PinBolt		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT);
					if (bLayerInd_WallTie == true)			layerInd_WallTie		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE);
					if (bLayerInd_Join == true)				layerInd_Join			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN);
					if (bLayerInd_HeadPiece == true)		layerInd_HeadPiece		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE);
					if (bLayerInd_Steelform == true)		layerInd_Steelform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM);
					if (bLayerInd_Plywood == true)			layerInd_Plywood		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD);
					if (bLayerInd_Wood == true)				layerInd_Wood			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD);
					if (bLayerInd_Fillersp == true)			layerInd_Fillersp		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP);
					if (bLayerInd_OutcornerAngle == true)	layerInd_OutcornerAngle	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE);
					if (bLayerInd_OutcornerPanel == true)	layerInd_OutcornerPanel	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL);
					if (bLayerInd_IncornerPanel == true)	layerInd_IncornerPanel	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL);

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
