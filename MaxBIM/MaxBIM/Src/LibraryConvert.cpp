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

//const GS::uchar_t*	gsmSFOM = L("������ ���̺��� (���ǳ�) v1.0.gsm");
//const GS::uchar_t*	gsmFISP = L("�ٷ������̼�v1.0.gsm");
//const GS::uchar_t*	gsmOUTA = L("�ƿ��ڳʾޱ�v1.0.gsm");
//const GS::uchar_t*	gsmOUTP = L("�ƿ��ڳ��ǳ�v1.0.gsm");
//const GS::uchar_t*	gsmINCO = L("���ڳ��ǳ�v1.0.gsm");

// ��� ���� ������(TCO: Temporary Construction Object)�� ���� ������� ��ȯ��
GSErrCode	convertVirtualTCO (void)
{
	GSErrCode	err = NoError;

	API_Element				elem;
	API_ElementMemo			memo;

	short	xx;
	long	nSel;
	char	buffer [256];

	const char*		productName;	// ���� ������
	const char*		objType;		// ���̺���(��), ���̺���(������), ������, ��ƿ��, ����, �ٷ������̼�, �ƿ��ڳʾޱ�, �ƿ��ڳ��ǳ�, ���ڳ��ǳ�
	const char*		dir;			// �������, �ٴڱ��, �ٴڵ���
	const char*		coverSide;		// ���, �ܸ�
	double			oppSideOffset;	// �ݴ������� �Ÿ�
	bool			leftSide;		// ����(1), ������(0)
	bool			bRegularSize;	// ���� ũ��(1)
	double unit_A, unit_B, unit_ZZYZX;	// ���� ����
	int num_A, num_B, num_ZZYZX;		// ���� ����


	// ������ ��Ұ� ������ ����
	API_SelectionInfo		selectionInfo;
	API_Neig				**selNeigs;
	API_Element				tElem;
	GS::Array<API_Guid>&	objects = GS::Array<API_Guid> ();
	long					nObjects = 0;

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

			if (tElem.header.typeID == API_ObjectID)	// ��ü Ÿ�� ����ΰ�?
				objects.Push (tElem.header.guid);
		}
	}
	BMKillHandle ((GSHandle *) &selNeigs);
	nObjects = objects.GetSize ();

	for (xx = 0 ; xx < nObjects ; ++xx) {
		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		elem.header.guid = objects.Pop ();
		err = ACAPI_Element_Get (&elem);
		err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

		productName = getParameterStringByName (&memo, "productName");

		if (strncmp (productName, "���� ������", strlen ("���� ������")) == 0) {

			// ���� �������� �Ķ���� �� �ҷ�����
			objType = getParameterStringByName (&memo, "objType");
			dir = getParameterStringByName (&memo, "dir");						// �������, �ٴڱ��, �ٴڵ���
			coverSide = getParameterStringByName (&memo, "coverSide");			// ���, �ܸ�
			oppSideOffset = getParameterValueByName (&memo, "oppSideOffset");	// �ݴ������� �Ÿ�
			leftSide = getParameterValueByName (&memo, "leftSide");				// ����(1), ������(0)
			bRegularSize = getParameterValueByName (&memo, "bRegularSize");		// ���� ũ��(1)
			unit_A = getParameterValueByName (&memo, "unit_A");					// ���� ���� (A)
			unit_B = getParameterValueByName (&memo, "unit_B");					// ���� ���� (B)
			unit_ZZYZX = getParameterValueByName (&memo, "unit_ZZYZX");			// ���� ���� (ZZYZX)
			num_A = getParameterValueByName (&memo, "num_A");					// ���� ���� (A)
			num_B = getParameterValueByName (&memo, "num_B");					// ���� ���� (B)
			num_ZZYZX = getParameterValueByName (&memo, "num_ZZYZX");			// ���� ���� (ZZYZX)
			
			// ���� ������ ����
			API_Elem_Head* headList = new API_Elem_Head [1];
			headList [0] = elem.header;
			err = ACAPI_Element_Delete (&headList, 1);
			delete headList;

			//GSErrCode	placeTableformOnWall (WallTableform params);		// ���̺���(��) ��ġ
			//GSErrCode	placeTableformOnSlabBottom (SlabTableform params);	// ���̺���(������) ��ġ
			//GSErrCode	placeEuroform (Euroform params);					// ������/��ƿ�� ��ġ
			//GSErrCode	placePlywood (Plywood params);						// ���� ��ġ
			//GSErrCode	placeFillersp (FillerSpacer params);				// �ٷ������̼� ��ġ
			//GSErrCode	placeOutcornerAngle (OutcornerAngle params);		// �ƿ��ڳʾޱ� ��ġ
			//GSErrCode	placeOutcornerPanel (OutcornerPanel params);		// �ƿ��ڳ��ǳ� ��ġ
			//GSErrCode	placeIncornerPanel (IncornerPanel params);			// ���ڳ��ǳ� ��ġ

			// ... �Ʒ� ���ǹ����� ��ü ����ü ������ fill�� �� ȣ��

			// ���� �����縦 ��ġ��
			if (strncmp (objType, "���̺���(��)", strlen ("���̺���(��)")) == 0) {
				// ...
			} else if (strncmp (objType, "���̺���(������)", strlen ("���̺���(������)")) == 0) {
				// ...
			} else if (strncmp (objType, "������", strlen ("������")) == 0) {
				// ...
			} else if (strncmp (objType, "��ƿ��", strlen ("��ƿ��")) == 0) {
				// ...
			} else if (strncmp (objType, "����", strlen ("����")) == 0) {
				// ...
			} else if (strncmp (objType, "�ٷ������̼�", strlen ("�ٷ������̼�")) == 0) {
				// ...
			} else if (strncmp (objType, "�ƿ��ڳʾޱ�", strlen ("�ƿ��ڳʾޱ�")) == 0) {
				// ...
			} else if (strncmp (objType, "�ƿ��ڳ��ǳ�", strlen ("�ƿ��ڳ��ǳ�")) == 0) {
				// ...
			} else if (strncmp (objType, "���ڳ��ǳ�", strlen ("���ڳ��ǳ�")) == 0) {
				// ...
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

	return	err;
}

// ������/��ƿ�� ��ġ
GSErrCode	placeEuroform (Euroform params)
{
	GSErrCode	err = NoError;

	return	err;
}

// ���� ��ġ
GSErrCode	placePlywood (Plywood params)
{
	GSErrCode	err = NoError;

	return	err;
}

// �ٷ������̼� ��ġ
GSErrCode	placeFillersp (FillerSpacer params)
{
	GSErrCode	err = NoError;

	return	err;
}

// �ƿ��ڳʾޱ� ��ġ
GSErrCode	placeOutcornerAngle (OutcornerAngle params)
{
	GSErrCode	err = NoError;

	return	err;
}

// �ƿ��ڳ��ǳ� ��ġ
GSErrCode	placeOutcornerPanel (OutcornerPanel params)
{
	GSErrCode	err = NoError;

	return	err;
}

// ���ڳ��ǳ� ��ġ
GSErrCode	placeIncornerPanel (IncornerPanel params)
{
	GSErrCode	err = NoError;

	return	err;
}

