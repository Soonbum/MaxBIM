#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "ColumnEuroformPlacer.hpp"

using namespace columnPlacerDG;

// 5�� �޴�: ��տ� �������� ��ġ�ϴ� ���� ��ƾ
GSErrCode	placeEuroformOnColumn (void)
{
	GSErrCode		err = NoError;
	long			nSel;
	short			xx, yy;
	double			dx, dy, ang;
	short			result;

	// Selection Manager ���� ����
	API_SelectionInfo		selectionInfo;
	API_Element				tElem;
	API_Neig				**selNeigs;
	GS::Array<API_Guid>&	morphs = GS::Array<API_Guid> ();
	GS::Array<API_Guid>&	columns = GS::Array<API_Guid> ();
	GS::Array<API_Guid>&	beams = GS::Array<API_Guid> ();
	long					nMorphs = 0;
	long					nColumns = 0;
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
	GS::Array<API_Coord3D>&	coords = GS::Array<API_Coord3D> ();

	// ���� ��ü ����
	//InfoMorphForColumn		infoMorph;

	// �۾� �� ����
	API_StoryInfo			storyInfo;
	double					workLevel_column;


	// ������ ��� ��������
	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err == APIERR_NOPLAN) {
		ACAPI_WriteReport ("���� ������Ʈ â�� �����ϴ�.", true);
		return err;
	}
	if (err == APIERR_NOSEL) {
		ACAPI_WriteReport ("�ƹ� �͵� �������� �ʾҽ��ϴ�.\n�ʼ� ����: ��� (1��), ��� ������ ���� ���� (1��)\n�ɼ� ����: ��հ� �´�� �� (�ټ�)", true);
		return err;
	}
	if (err != NoError) {
		BMKillHandle ((GSHandle *) &selNeigs);
		return err;
	}

	// ���� ��� 1�� �����ؾ� ��
	if (selectionInfo.typeID != API_SelEmpty) {
		nSel = BMGetHandleSize ((GSHandle) selNeigs) / sizeof (API_Neig);
		for (xx = 0 ; xx < nSel && err == NoError ; ++xx) {
			tElem.header.typeID = Neig_To_ElemID ((*selNeigs)[xx].neigID);

			tElem.header.guid = (*selNeigs)[xx].guid;
			if (ACAPI_Element_Get (&tElem) != NoError)	// ������ �� �ִ� ����ΰ�?
				continue;

			if (tElem.header.typeID == API_ColumnID)	// ����ΰ�?
				morphs.Push (tElem.header.guid);

			if (tElem.header.typeID == API_MorphID)		// �����ΰ�?
				morphs.Push (tElem.header.guid);

			if (tElem.header.typeID == API_BeamID)		// ���ΰ�?
				beams.Push (tElem.header.guid);
		}
	}
	BMKillHandle ((GSHandle *) &selNeigs);
	nColumns = columns.GetSize ();
	nMorphs = morphs.GetSize ();
	nBeams = beams.GetSize ();

	// ����� 1���ΰ�?
	if (nColumns != 1) {
		ACAPI_WriteReport ("����� 1�� �����ؾ� �մϴ�.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// ������ 1���ΰ�?
	if (nMorphs != 1) {
		ACAPI_WriteReport ("��� ������ ���� ������ 1�� �����ϼž� �մϴ�.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// ��� ���� ����: ����, �ʺ�A/B, ����
	// ���� ���� ����
	// �� ���� ����: ����, �Ϻ� ��, ��ǥ(��հ� ������� ��ġ�� ���� ��/��/��/���� ã���� ��)...

	// ���� ������ ���� ���� ���� ���� ������

	/*
	��ġ�� ����: ������, �ƿ��ڳ��ǳ�, ���ڳ��ǳ�, ������, �������ڳ�, ����
	������ �ֻ�� ���ΰ� �� �ϴ� ���� ���� ������ 80mm �̻� (UI���� ��ȿ���� ���� ������ ������ �ؽ�Ʈ�� ���!)

	- ���� ��հ� ������� ���踦 �ľ�
		- ��� ���� ���� ã�´�.
		- �� �� ���� ���� ���� ���� ã�ƾ� ��
	- 1�� DG (�� �ܸ�)
		- ���� �Ÿ��� ��Ȳ���� �ƴ����� �׸����� ǥ���� �� (����. ��� �ʺ�: 600, �� �β�: 200)
			- (CASE.1) �ܵ� ��
			- (CASE.2) �Ÿ�X, ����� ��ո��� �´���
			- (CASE.3) ����� �Ϻΰ� ���� �Ÿ���: ��� 500�� �� ������ Ƣ���
			- (CASE.4) �Ÿ�O, ����� ��ո� ��ġ: ��� 400�� �� ������ Ƣ���
			- (CASE.5) �� ����: �� �������� 200�� ����� �����
			- ���� ġ��(��� �ʺ�, �� �β�, �Ÿ�/���� ����)�� �����ְ�... ���̽��� �� ���׸�Ʈ �ʺ� ����ڰ� ������ ��!
		- ��� �ڳ�: �ƿ��ڳ��ǳ� (��� �ڳʰ� �� �ӿ� ���� ����)
		- ���� �´�� �κ�: ���ڳ��ǳ� (���� ����� ��쿡��)
		- ������: ��� ���, �׸��� ���� (��� ������ ���ӿ� �Ÿ��� ���)
	- 2�� DG (�� ����)
		- ���� ���� ���� ������ ���� ������ ǥ���ؾ� �� (80mm �̻��̾�� ��)
		- ��� ������ ������ ������ �������� �����
	*/

	return	err;
}