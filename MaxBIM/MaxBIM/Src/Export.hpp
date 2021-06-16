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

	vector<string>	var10name;					// ����10 �̸� (��: nom)
	vector<string>	var10desc;					// ����10 �̸��� ���� ���� (��: �԰�)
	vector<short>	var10showFlag;				// ����10 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var10value;			// ����10 �̸��� ���� ��
	vector<API_AddParID>	var10type;			// ����10 �̸��� ���� ���� Ÿ��

	vector<string>	var11name;					// ����11 �̸� (��: nom)
	vector<string>	var11desc;					// ����11 �̸��� ���� ���� (��: �԰�)
	vector<short>	var11showFlag;				// ����11 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var11value;			// ����11 �̸��� ���� ��
	vector<API_AddParID>	var11type;			// ����11 �̸��� ���� ���� Ÿ��

	vector<string>	var12name;					// ����12 �̸� (��: nom)
	vector<string>	var12desc;					// ����12 �̸��� ���� ���� (��: �԰�)
	vector<short>	var12showFlag;				// ����12 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var12value;			// ����12 �̸��� ���� ��
	vector<API_AddParID>	var12type;			// ����12 �̸��� ���� ���� Ÿ��

	vector<string>	var13name;					// ����13 �̸� (��: nom)
	vector<string>	var13desc;					// ����13 �̸��� ���� ���� (��: �԰�)
	vector<short>	var13showFlag;				// ����13 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var13value;			// ����13 �̸��� ���� ��
	vector<API_AddParID>	var13type;			// ����13 �̸��� ���� ���� Ÿ��

	vector<string>	var14name;					// ����14 �̸� (��: nom)
	vector<string>	var14desc;					// ����14 �̸��� ���� ���� (��: �԰�)
	vector<short>	var14showFlag;				// ����14 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var14value;			// ����14 �̸��� ���� ��
	vector<API_AddParID>	var14type;			// ����14 �̸��� ���� ���� Ÿ��

	vector<string>	var15name;					// ����15 �̸� (��: nom)
	vector<string>	var15desc;					// ����15 �̸��� ���� ���� (��: �԰�)
	vector<short>	var15showFlag;				// ����15 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var15value;			// ����15 �̸��� ���� ��
	vector<API_AddParID>	var15type;			// ����15 �̸��� ���� ���� Ÿ��

	vector<string>	var16name;					// ����16 �̸� (��: nom)
	vector<string>	var16desc;					// ����16 �̸��� ���� ���� (��: �԰�)
	vector<short>	var16showFlag;				// ����16 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var16value;			// ����16 �̸��� ���� ��
	vector<API_AddParID>	var16type;			// ����16 �̸��� ���� ���� Ÿ��

	vector<string>	var17name;					// ����17 �̸� (��: nom)
	vector<string>	var17desc;					// ����17 �̸��� ���� ���� (��: �԰�)
	vector<short>	var17showFlag;				// ����17 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var17value;			// ����17 �̸��� ���� ��
	vector<API_AddParID>	var17type;			// ����17 �̸��� ���� ���� Ÿ��

	vector<string>	var18name;					// ����18 �̸� (��: nom)
	vector<string>	var18desc;					// ����18 �̸��� ���� ���� (��: �԰�)
	vector<short>	var18showFlag;				// ����18 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var18value;			// ����18 �̸��� ���� ��
	vector<API_AddParID>	var18type;			// ����18 �̸��� ���� ���� Ÿ��

	vector<string>	var19name;					// ����19 �̸� (��: nom)
	vector<string>	var19desc;					// ����19 �̸��� ���� ���� (��: �԰�)
	vector<short>	var19showFlag;				// ����19 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var19value;			// ����19 �̸��� ���� ��
	vector<API_AddParID>	var19type;			// ����19 �̸��� ���� ���� Ÿ��

	vector<string>	var20name;					// ����20 �̸� (��: nom)
	vector<string>	var20desc;					// ����20 �̸��� ���� ���� (��: �԰�)
	vector<short>	var20showFlag;				// ����20 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var20value;			// ����20 �̸��� ���� ��
	vector<API_AddParID>	var20type;			// ����20 �̸��� ���� ���� Ÿ��

	vector<string>	var21name;					// ����21 �̸� (��: nom)
	vector<string>	var21desc;					// ����21 �̸��� ���� ���� (��: �԰�)
	vector<short>	var21showFlag;				// ����21 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var21value;			// ����21 �̸��� ���� ��
	vector<API_AddParID>	var21type;			// ����21 �̸��� ���� ���� Ÿ��

	vector<string>	var22name;					// ����22 �̸� (��: nom)
	vector<string>	var22desc;					// ����22 �̸��� ���� ���� (��: �԰�)
	vector<short>	var22showFlag;				// ����22 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var22value;			// ����22 �̸��� ���� ��
	vector<API_AddParID>	var22type;			// ����22 �̸��� ���� ���� Ÿ��

	vector<string>	var23name;					// ����23 �̸� (��: nom)
	vector<string>	var23desc;					// ����23 �̸��� ���� ���� (��: �԰�)
	vector<short>	var23showFlag;				// ����23 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var23value;			// ����23 �̸��� ���� ��
	vector<API_AddParID>	var23type;			// ����23 �̸��� ���� ���� Ÿ��

	vector<string>	var24name;					// ����24 �̸� (��: nom)
	vector<string>	var24desc;					// ����24 �̸��� ���� ���� (��: �԰�)
	vector<short>	var24showFlag;				// ����24 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var24value;			// ����24 �̸��� ���� ��
	vector<API_AddParID>	var24type;			// ����24 �̸��� ���� ���� Ÿ��

	vector<string>	var25name;					// ����25 �̸� (��: nom)
	vector<string>	var25desc;					// ����25 �̸��� ���� ���� (��: �԰�)
	vector<short>	var25showFlag;				// ����25 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var25value;			// ����25 �̸��� ���� ��
	vector<API_AddParID>	var25type;			// ����25 �̸��� ���� ���� Ÿ��

	vector<string>	var26name;					// ����26 �̸� (��: nom)
	vector<string>	var26desc;					// ����26 �̸��� ���� ���� (��: �԰�)
	vector<short>	var26showFlag;				// ����26 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var26value;			// ����26 �̸��� ���� ��
	vector<API_AddParID>	var26type;			// ����26 �̸��� ���� ���� Ÿ��

	vector<string>	var27name;					// ����27 �̸� (��: nom)
	vector<string>	var27desc;					// ����27 �̸��� ���� ���� (��: �԰�)
	vector<short>	var27showFlag;				// ����27 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var27value;			// ����27 �̸��� ���� ��
	vector<API_AddParID>	var27type;			// ����27 �̸��� ���� ���� Ÿ��

	vector<string>	var28name;					// ����28 �̸� (��: nom)
	vector<string>	var28desc;					// ����28 �̸��� ���� ���� (��: �԰�)
	vector<short>	var28showFlag;				// ����28 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var28value;			// ����28 �̸��� ���� ��
	vector<API_AddParID>	var28type;			// ����28 �̸��� ���� ���� Ÿ��

	vector<string>	var29name;					// ����29 �̸� (��: nom)
	vector<string>	var29desc;					// ����29 �̸��� ���� ���� (��: �԰�)
	vector<short>	var29showFlag;				// ����29 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var29value;			// ����29 �̸��� ���� ��
	vector<API_AddParID>	var29type;			// ����29 �̸��� ���� ���� Ÿ��

	vector<string>	var30name;					// ����30 �̸� (��: nom)
	vector<string>	var30desc;					// ����30 �̸��� ���� ���� (��: �԰�)
	vector<short>	var30showFlag;				// ����30 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var30value;			// ����30 �̸��� ���� ��
	vector<API_AddParID>	var30type;			// ����30 �̸��� ���� ���� Ÿ��

	vector<string>	var31name;					// ����31 �̸� (��: nom)
	vector<string>	var31desc;					// ����31 �̸��� ���� ���� (��: �԰�)
	vector<short>	var31showFlag;				// ����31 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var31value;			// ����31 �̸��� ���� ��
	vector<API_AddParID>	var31type;			// ����31 �̸��� ���� ���� Ÿ��

	vector<string>	var32name;					// ����32 �̸� (��: nom)
	vector<string>	var32desc;					// ����32 �̸��� ���� ���� (��: �԰�)
	vector<short>	var32showFlag;				// ����32 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var32value;			// ����32 �̸��� ���� ��
	vector<API_AddParID>	var32type;			// ����32 �̸��� ���� ���� Ÿ��

	vector<string>	var33name;					// ����33 �̸� (��: nom)
	vector<string>	var33desc;					// ����33 �̸��� ���� ���� (��: �԰�)
	vector<short>	var33showFlag;				// ����33 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var33value;			// ����33 �̸��� ���� ��
	vector<API_AddParID>	var33type;			// ����33 �̸��� ���� ���� Ÿ��

	vector<string>	var34name;					// ����34 �̸� (��: nom)
	vector<string>	var34desc;					// ����34 �̸��� ���� ���� (��: �԰�)
	vector<short>	var34showFlag;				// ����34 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var34value;			// ����34 �̸��� ���� ��
	vector<API_AddParID>	var34type;			// ����34 �̸��� ���� ���� Ÿ��

	vector<string>	var35name;					// ����35 �̸� (��: nom)
	vector<string>	var35desc;					// ����35 �̸��� ���� ���� (��: �԰�)
	vector<short>	var35showFlag;				// ����35 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var35value;			// ����35 �̸��� ���� ��
	vector<API_AddParID>	var35type;			// ����35 �̸��� ���� ���� Ÿ��

	vector<string>	var36name;					// ����36 �̸� (��: nom)
	vector<string>	var36desc;					// ����36 �̸��� ���� ���� (��: �԰�)
	vector<short>	var36showFlag;				// ����36 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var36value;			// ����36 �̸��� ���� ��
	vector<API_AddParID>	var36type;			// ����36 �̸��� ���� ���� Ÿ��

	vector<string>	var37name;					// ����37 �̸� (��: nom)
	vector<string>	var37desc;					// ����37 �̸��� ���� ���� (��: �԰�)
	vector<short>	var37showFlag;				// ����37 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var37value;			// ����37 �̸��� ���� ��
	vector<API_AddParID>	var37type;			// ����37 �̸��� ���� ���� Ÿ��

	vector<string>	var38name;					// ����38 �̸� (��: nom)
	vector<string>	var38desc;					// ����38 �̸��� ���� ���� (��: �԰�)
	vector<short>	var38showFlag;				// ����38 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var38value;			// ����38 �̸��� ���� ��
	vector<API_AddParID>	var38type;			// ����38 �̸��� ���� ���� Ÿ��

	vector<string>	var39name;					// ����39 �̸� (��: nom)
	vector<string>	var39desc;					// ����39 �̸��� ���� ���� (��: �԰�)
	vector<short>	var39showFlag;				// ����39 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var39value;			// ����39 �̸��� ���� ��
	vector<API_AddParID>	var39type;			// ����39 �̸��� ���� ���� Ÿ��

	vector<string>	var40name;					// ����40 �̸� (��: nom)
	vector<string>	var40desc;					// ����40 �̸��� ���� ���� (��: �԰�)
	vector<short>	var40showFlag;				// ����40 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var40value;			// ����40 �̸��� ���� ��
	vector<API_AddParID>	var40type;			// ����40 �̸��� ���� ���� Ÿ��

	vector<string>	var41name;					// ����41 �̸� (��: nom)
	vector<string>	var41desc;					// ����41 �̸��� ���� ���� (��: �԰�)
	vector<short>	var41showFlag;				// ����41 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var41value;			// ����41 �̸��� ���� ��
	vector<API_AddParID>	var41type;			// ����41 �̸��� ���� ���� Ÿ��

	vector<string>	var42name;					// ����42 �̸� (��: nom)
	vector<string>	var42desc;					// ����42 �̸��� ���� ���� (��: �԰�)
	vector<short>	var42showFlag;				// ����42 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var42value;			// ����42 �̸��� ���� ��
	vector<API_AddParID>	var42type;			// ����42 �̸��� ���� ���� Ÿ��

	vector<string>	var43name;					// ����43 �̸� (��: nom)
	vector<string>	var43desc;					// ����43 �̸��� ���� ���� (��: �԰�)
	vector<short>	var43showFlag;				// ����43 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var43value;			// ����43 �̸��� ���� ��
	vector<API_AddParID>	var43type;			// ����43 �̸��� ���� ���� Ÿ��

	vector<string>	var44name;					// ����44 �̸� (��: nom)
	vector<string>	var44desc;					// ����44 �̸��� ���� ���� (��: �԰�)
	vector<short>	var44showFlag;				// ����44 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var44value;			// ����44 �̸��� ���� ��
	vector<API_AddParID>	var44type;			// ����44 �̸��� ���� ���� Ÿ��

	vector<string>	var45name;					// ����45 �̸� (��: nom)
	vector<string>	var45desc;					// ����45 �̸��� ���� ���� (��: �԰�)
	vector<short>	var45showFlag;				// ����45 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var45value;			// ����45 �̸��� ���� ��
	vector<API_AddParID>	var45type;			// ����45 �̸��� ���� ���� Ÿ��

	vector<string>	var46name;					// ����46 �̸� (��: nom)
	vector<string>	var46desc;					// ����46 �̸��� ���� ���� (��: �԰�)
	vector<short>	var46showFlag;				// ����46 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var46value;			// ����46 �̸��� ���� ��
	vector<API_AddParID>	var46type;			// ����46 �̸��� ���� ���� Ÿ��

	vector<string>	var47name;					// ����47 �̸� (��: nom)
	vector<string>	var47desc;					// ����47 �̸��� ���� ���� (��: �԰�)
	vector<short>	var47showFlag;				// ����47 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var47value;			// ����47 �̸��� ���� ��
	vector<API_AddParID>	var47type;			// ����47 �̸��� ���� ���� Ÿ��

	vector<string>	var48name;					// ����48 �̸� (��: nom)
	vector<string>	var48desc;					// ����48 �̸��� ���� ���� (��: �԰�)
	vector<short>	var48showFlag;				// ����48 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var48value;			// ����48 �̸��� ���� ��
	vector<API_AddParID>	var48type;			// ����48 �̸��� ���� ���� Ÿ��

	vector<string>	var49name;					// ����49 �̸� (��: nom)
	vector<string>	var49desc;					// ����49 �̸��� ���� ���� (��: �԰�)
	vector<short>	var49showFlag;				// ����49 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var49value;			// ����49 �̸��� ���� ��
	vector<API_AddParID>	var49type;			// ����49 �̸��� ���� ���� Ÿ��

	vector<string>	var50name;					// ����50 �̸� (��: nom)
	vector<string>	var50desc;					// ����50 �̸��� ���� ���� (��: �԰�)
	vector<short>	var50showFlag;				// ����50 �׸� ǥ�� ���� (0: �״�� ǥ��, n: n�� ������ ����̸� ǥ��, -n: n�� ������ �����̸� ǥ��)
	vector<vector<string>>	var50value;			// ����50 �̸��� ���� ��
	vector<API_AddParID>	var50type;			// ����50 �̸��� ���� ���� Ÿ��

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

	// �˷����� ���� Object Ÿ���� ��ü
	bool	bShow_Unknown;
	long	nUnknownObjects;

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