#ifndef	__EXPORT__
#define __EXPORT__

#include "MaxBIM.hpp"

using namespace std;

namespace exportDG {
	// ���̾�α� �׸� �ε���
	enum	idxItems_1_exportDG {
		LABEL_DIST_BTW_COLUMN		= 3,
		EDITCONTROL_DIST_BTW_COLUMN
	};

	enum	layerTypeEnum {
		UNDEFINED,	// ���ǵ��� ����
		WALL,	// �� -> ���� �յ޸� ĸ���� ��
		SLAB,	// ������ -> ���� �Ʒ��� ĸ���� ��
		COLU,	// ��� -> ������ ĸ���� ��
		BEAM,	// �� -> ����, �Ϻ� ĸ���� ��
		WLBM	// ���纸 -> ����, �Ϻ� ĸ���� ��
	};

	enum	filterSelectionDG {
		BUTTON_ALL_SEL = 3,
		BUTTON_ALL_UNSEL,
		CHECKBOX_INCLUDE_UNKNOWN_OBJECT
	};
}

// ���� ����� ����
struct ColumnInfo
{
	short	floorInd;	// �� �ε���
	double	posX;		// �߽��� X��ǥ
	double	posY;		// �߽��� Y��ǥ
	double	horLen;		// ���� ����
	double	verLen;		// ���� ����
	double	height;		// ����

	short	iHor;		// ���� ����
	short	iVer;		// ���� ����
};

// ����� ��ġ ����
struct ColumnPos
{
	ColumnInfo	columns [5000];		// ��� ����
	short	nColumns;				// ��� ����

	// �ֿ� ����
	double	node_hor [100][100];	// ���� ���� ���� ��� ��ǥ (X) : dim1 - ��, dim2 - ��� X��ǥ
	short	nNodes_hor [100];		// ���� ���� ���� ��� ����
	double	node_ver [100][100];	// ���� ���� ���� ��� ��ǥ (Y) : dim1 - ��, dim2 - ��� Y��ǥ
	short	nNodes_ver [100];		// ���� ���� ���� ��� ����

	short	nStories;				// �� ���� �ǹ���, = storyInfo.lastStory - storyInfo.firstStory + (storyInfo.skipNullFloor) * 1 (0���� �����ߴٸ� +1)
	short	firstStory;				// ������ �� �ε��� (��: ���� 4���� ���, -4)
	short	lastStory;				// �ֻ��� �� �ε��� (��: ���� 35���� ���, 34)
};

// ������ ������� ��� ���� (new)
class SummaryOfObjectInfo
{
public:
	SummaryOfObjectInfo ();		// ������

public:
	// Ű
	vector<string>	nameKey;		// ��ü�� ������ �� �ִ� ��(���ڿ�)�� ��� �ִ� ���� �̸� (��: sup_type)
	vector<string>	nameVal;		// ��ü�� �̸� (��: KS��������)

	vector<short>	nInfo;			// ǥ���� ���� �ʵ� ����

	// ��ü�� ���� (Object Ÿ��)
	vector<string>	var1name;					// ����1 �̸� (��: nom)
	vector<string>	var1desc;					// ����1 �̸��� ���� ���� (��: �԰�)
	vector<short>	var1showFlag;				// ����1 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var1value;			// ����1 �̸��� ���� ��
	vector<API_AddParID>	var1type;			// ����1 �̸��� ���� ���� Ÿ��

	vector<string>	var2name;					// ����2 �̸� (��: nom)
	vector<string>	var2desc;					// ����2 �̸��� ���� ���� (��: �԰�)
	vector<short>	var2showFlag;				// ����2 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var2value;			// ����2 �̸��� ���� ��
	vector<API_AddParID>	var2type;			// ����2 �̸��� ���� ���� Ÿ��
	
	vector<string>	var3name;					// ����3 �̸� (��: nom)
	vector<string>	var3desc;					// ����3 �̸��� ���� ���� (��: �԰�)
	vector<short>	var3showFlag;				// ����3 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var3value;			// ����3 �̸��� ���� ��
	vector<API_AddParID>	var3type;			// ����3 �̸��� ���� ���� Ÿ��

	vector<string>	var4name;					// ����4 �̸� (��: nom)
	vector<string>	var4desc;					// ����4 �̸��� ���� ���� (��: �԰�)
	vector<short>	var4showFlag;				// ����4 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var4value;			// ����4 �̸��� ���� ��
	vector<API_AddParID>	var4type;			// ����4 �̸��� ���� ���� Ÿ��

	vector<string>	var5name;					// ����5 �̸� (��: nom)
	vector<string>	var5desc;					// ����5 �̸��� ���� ���� (��: �԰�)
	vector<short>	var5showFlag;				// ����5 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var5value;			// ����5 �̸��� ���� ��
	vector<API_AddParID>	var5type;			// ����5 �̸��� ���� ���� Ÿ��

	vector<string>	var6name;					// ����6 �̸� (��: nom)
	vector<string>	var6desc;					// ����6 �̸��� ���� ���� (��: �԰�)
	vector<short>	var6showFlag;				// ����6 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var6value;			// ����6 �̸��� ���� ��
	vector<API_AddParID>	var6type;			// ����6 �̸��� ���� ���� Ÿ��

	vector<string>	var7name;					// ����7 �̸� (��: nom)
	vector<string>	var7desc;					// ����7 �̸��� ���� ���� (��: �԰�)
	vector<short>	var7showFlag;				// ����7 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var7value;			// ����7 �̸��� ���� ��
	vector<API_AddParID>	var7type;			// ����7 �̸��� ���� ���� Ÿ��

	vector<string>	var8name;					// ����8 �̸� (��: nom)
	vector<string>	var8desc;					// ����8 �̸��� ���� ���� (��: �԰�)
	vector<short>	var8showFlag;				// ����8 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var8value;			// ����8 �̸��� ���� ��
	vector<API_AddParID>	var8type;			// ����8 �̸��� ���� ���� Ÿ��

	vector<string>	var9name;					// ����9 �̸� (��: nom)
	vector<string>	var9desc;					// ����9 �̸��� ���� ���� (��: �԰�)
	vector<short>	var9showFlag;				// ����9 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var9value;			// ����9 �̸��� ���� ��
	vector<API_AddParID>	var9type;			// ����9 �̸��� ���� ���� Ÿ��

	vector<vector<short>>	combinationCount;	// ���� �������� ���տ� ���� ����

	vector<short>	nCounts;					// ���� �ٸ� ���� ���� ����
	short nKnownObjects;						// ������ ��ü�� ����

	// ��ü�� ���� (Beam Ÿ��)
	vector<int>		beamLength;					// �� ����
	vector<short>	beamCount;					// �ش� �� ���̿� ���� ����
	short nCountsBeam;							// �� ������ ����

	// �� �� ���� ��ü
	short nUnknownObjects;						// �������� ���� ��ü�� ����
};

// ���̴� ���̾� ���� ��ü�� ��Ī, ���� ����, ���̱� ����
struct VisibleObjectInfo
{
	// Object Ÿ��
	short	nKinds;				// ��ü ���� ����
	char	varName [100][50];	// 1��: ������
	char	objName [100][128];	// 2��: ��ü��
	bool	bExist [100];		// ���� ����
	bool	bShow [100];		// ǥ�� ����
	short	itmIdx [100];		// ���̾�α� �� �ε���

	// ������ Ÿ��
	bool	bExist_Walls;
	bool	bShow_Walls;
	short	itmIdx_Walls;

	bool	bExist_Columns;
	bool	bShow_Columns;
	short	itmIdx_Columns;

	bool	bExist_Beams;
	bool	bShow_Beams;
	short	itmIdx_Beams;

	bool	bExist_Slabs;
	bool	bShow_Slabs;
	short	itmIdx_Slabs;

	bool	bExist_Roofs;
	bool	bShow_Roofs;
	short	itmIdx_Roofs;

	bool	bExist_Meshes;
	bool	bShow_Meshes;
	short	itmIdx_Meshes;

	bool	bExist_Morphs;
	bool	bShow_Morphs;
	short	itmIdx_Morphs;

	bool	bExist_Shells;
	bool	bShow_Shells;
	short	itmIdx_Shells;

	// �����ϴ� �׸� ��ü ����
	short	nItems;
};

void		initArray (double arr [], short arrSize);											// �迭 �ʱ�ȭ �Լ�
int			compare (const void* first, const void* second);									// ������������ ������ �� ����ϴ� ���Լ� (����Ʈ)
ColumnInfo	findColumn (ColumnPos* columnPos, short iHor, short iVer, short floorInd);			// �����ֿ�, �����ֿ�, �� ������ �̿��Ͽ� ��� ã��
GSErrCode	exportGridElementInfo (void);														// ����(���,��,������)���� ������ �����ϰ� �����ؼ� ���� ���Ϸ� ��������
short		DGCALLBACK inputThresholdHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// [���̾�α�] ��� �� �ּ� ���� �Ÿ��� ����ڿ��� �Է� ���� (�⺻��: 2000 mm)
GSErrCode	exportSelectedElementInfo (void);													// ������ ���� ���� �������� (Single ���)
GSErrCode	exportElementInfoOnVisibleLayers (void);											// ������ ���� ���� �������� (Multi ���)
GSErrCode	filterSelection (void);																// ���纰 ���� �� �����ֱ�
short		DGCALLBACK filterSelectionHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// [���̾�α�] ���̾�α׿��� ���̴� ���̾� �� �ִ� ��ü���� ������ �����ְ�, üũ�� ������ ��ü�鸸 ���� �� ������

#endif