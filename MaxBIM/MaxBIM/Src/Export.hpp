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
	vector<vector<string>>	var1value;			// ����1 �̸��� ���� ��
	vector<API_AddParID>	var1type;			// ����1 �̸��� ���� ���� Ÿ��

	vector<string>	var2name;					// ����2 �̸� (��: nom)
	vector<string>	var2desc;					// ����2 �̸��� ���� ���� (��: �԰�)
	vector<vector<string>>	var2value;			// ����2 �̸��� ���� ��
	vector<API_AddParID>	var2type;			// ����2 �̸��� ���� ���� Ÿ��
	
	vector<string>	var3name;					// ����3 �̸� (��: nom)
	vector<string>	var3desc;					// ����3 �̸��� ���� ���� (��: �԰�)
	vector<vector<string>>	var3value;			// ����3 �̸��� ���� ��
	vector<API_AddParID>	var3type;			// ����3 �̸��� ���� ���� Ÿ��

	vector<string>	var4name;					// ����4 �̸� (��: nom)
	vector<string>	var4desc;					// ����4 �̸��� ���� ���� (��: �԰�)
	vector<vector<string>>	var4value;			// ����4 �̸��� ���� ��
	vector<API_AddParID>	var4type;			// ����4 �̸��� ���� ���� Ÿ��

	vector<string>	var5name;					// ����5 �̸� (��: nom)
	vector<string>	var5desc;					// ����5 �̸��� ���� ���� (��: �԰�)
	vector<vector<string>>	var5value;			// ����5 �̸��� ���� ��
	vector<API_AddParID>	var5type;			// ����5 �̸��� ���� ���� Ÿ��

	vector<string>	var6name;					// ����6 �̸� (��: nom)
	vector<string>	var6desc;					// ����6 �̸��� ���� ���� (��: �԰�)
	vector<vector<string>>	var6value;			// ����6 �̸��� ���� ��
	vector<API_AddParID>	var6type;			// ����6 �̸��� ���� ���� Ÿ��

	vector<string>	var7name;					// ����7 �̸� (��: nom)
	vector<string>	var7desc;					// ����7 �̸��� ���� ���� (��: �԰�)
	vector<vector<string>>	var7value;			// ����7 �̸��� ���� ��
	vector<API_AddParID>	var7type;			// ����7 �̸��� ���� ���� Ÿ��

	vector<string>	var8name;					// ����8 �̸� (��: nom)
	vector<string>	var8desc;					// ����8 �̸��� ���� ���� (��: �԰�)
	vector<vector<string>>	var8value;			// ����8 �̸��� ���� ��
	vector<API_AddParID>	var8type;			// ����8 �̸��� ���� ���� Ÿ��

	vector<string>	var9name;					// ����9 �̸� (��: nom)
	vector<string>	var9desc;					// ����9 �̸��� ���� ���� (��: �԰�)
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

// ������ ������� ��� ����
struct SummaryOfSelectedObjects
{
	// ������
	int uformWidth [100];				// ������ �ʺ�
	int uformHeight [100];				// ������ ����
	int uformCount [100];				// �ش� ������ �ʺ�/���� ���տ� ���� ����
	int sizeOfUformKinds;				// ������ ������ ����

	// ��ƿ����
	int steelformWidth [100];			// ��ƿ�� �ʺ�
	int steelformHeight [100];			// ��ƿ�� ����
	int steelformCount [100];			// �ش� ��ƿ�� �ʺ�/���� ���տ� ���� ����
	int	sizeOfSteelformKinds;			// ��ƿ�� ������ ����

	// ���ڳ��ǳ�
	int incornerPanelHor [100];			// ���ڳ��ǳ� ����
	int incornerPanelVer [100];			// ���ڳ��ǳ� ����
	int incornerPanelHei [100];			// ���ڳ��ǳ� ����
	int incornerPanelCount [100];		// �ش� ���ڳ��ǳ� ����/����/���� ���տ� ���� ����
	int	sizeOfIncornerPanelKinds;		// ���ڳ��ǳ� ������ ����

	// �ƿ��ڳ��ǳ�
	int outcornerPanelHor [100];		// �ƿ��ڳ��ǳ� ����
	int outcornerPanelVer [100];		// �ƿ��ڳ��ǳ� ����
	int outcornerPanelHei [100];		// �ƿ��ڳ��ǳ� ����
	int outcornerPanelCount [100];		// �ش� �ƿ��ڳ��ǳ� ����/����/���� ���տ� ���� ����
	int	sizeOfOutcornerPanelKinds;		// �ƿ��ڳ��ǳ� ������ ����

	// �ƿ��ڳʾޱ�
	int outcornerAngleLength [100];		// �ƿ��ڳʾޱ� ����
	int outcornerAngleCount [100];		// �ش� �ƿ��ڳʾޱ� ���̿� ���� ����
	int sizeOfOutcornerAngleKinds;		// �ƿ��ڳʾޱ� ������ ����

	// ����
	int woodThk [100];					// ���� �β�
	int woodWidth [100];				// ���� �ʺ�
	int woodLength [100];				// ���� ����
	int woodCount [100];				// �ش� ���� �β�/�ʺ�/���� ���տ� ���� ����
	int sizeOfWoodKinds;				// ���� ������ ����

	// �ٷ������̼�
	int fsThk [100];					// �ٷ������̼� �β�
	int fsLength [100];					// �ٷ������̼� ����
	int fsCount [100];					// �ش� �ٷ������̼� �β�/���� ���տ� ���� ����
	int sizeOfFsKinds;					// �ٷ������̼� ������ ����

	// �簢������ (���������, ���簢������)
	int sqrPipeHor [100];				// �簢������ �ܸ� ����
	int sqrPipeVer [100];				// �簢������ �ܸ� ����
	int sqrPipeLength [100];			// �簢������ ����
	int sqrPipeCount [100];				// �ش� �簢������ ����/����/���� ���տ� ���� ����
	int sizeOfSqrPipeKinds;				// �簢������ ������ ����

	// ����
	int plywoodHor [100];				// ����
	int plywoodVer [100];				// ����
	double plywoodThk [100];			// �β�
	int plywoodCount [100];				// �ش� ���� ����/����/�β� ���տ� ���� ����
	int sizeOfPlywoodKinds;				// ���� ������ ����

	// RS Push-Pull Props ����ǽ� (�ξ�� ����)
	int nHeadpiece;

	// RS Push-Pull Props
	bool bPropsUpperSupp [100];			// ���(����) ������ ����
	char PropsNomUpperSupp [100][30];	// ���(����) ������ �԰�
	bool bPropsLowerSupp [100];			// �Ϻ�(����) ������ ����
	char PropsNomLowerSupp [100][30];	// �Ϻ�(����) ������ �԰�
	bool bPropsBase [100];				// ���̽� �÷���Ʈ ����
	int propsCount [100];				// �ش� Props ������ ����/�԰�, ���̽� �÷���Ʈ ���� ���տ� ���� ����
	int sizeOfPropsKinds;				// Props ������ ����

	// ��ü Ÿ��
	int nTie;

	// ����Ŭ����
	int nClamp;

	// �ɺ�Ʈ��Ʈ
	int pinboltLen [100];				// �ɺ�Ʈ ����
	int pinboltCount [100];				// �ش� ���̿� ���� ����
	int sizeOfPinboltKinds;				// �ɺ�Ʈ ������ ����

	// ����ö�� (�簢�ͼ�Ȱ��)
	int nJoin;

	// �� �ۿ���
	int beamYokeLength [10];			// �� �ۿ��� ����
	int beamYokeCount [10];				// �ش� �� �ۿ��� ���̿� ���� ����
	int sizeOfBeamYokeKinds;			// �� �ۿ��� ������ ����

	// KS��������
	char KSProfileType [200][10];		// KS�������� Ÿ�� (���, ��)
	char KSProfileShape [200][30];		// KS�������� ����
	char KSProfileNom [200][50];		// KS�������� �԰�
	double KSProfileLen [200];			// KS�������� ����
	int KSProfileCount [200];			// �ش� KS�������� ����/�԰�/���� ���տ� ���� ����
	int sizeOfKSProfileKinds;			// KS�������� ������ ����

	// PERI���ٸ� ������
	char PERI_suppVerPostNom [100][10];	// PERI���ٸ� ������ �԰�
	int PERI_suppVerPostLen [100];		// PERI���ٸ� ������ ���� ����
	int PERI_suppVerPostCount [100];	// �ش� PERI���ٸ� ������ �԰�/���� ���� ���տ� ���� ����
	int sizeOfPERI_suppVerPostKinds;	// PERI���ٸ� ������ ������ ����

	// PERI���ٸ� ������
	char PERI_suppHorPostNom [20][10];	// PERI���ٸ� ������ �԰�
	int PERI_suppHorPostCount [20];		// �ش� PERI���ٸ� ������ �԰� ���տ� ���� ����
	int sizeOfPERI_suppHorPostKinds;	// PERI���ٸ� ������ ������ ����

	// GT24 �Ŵ�
	char GT24GirderNom [30][10];		// GT24 �Ŵ� �԰�
	int GT24GirderCount [30];			// �ش� GT24 �Ŵ� �԰ݿ� ���� ����
	int sizeOfGT24Girder;				// GT24 �Ŵ� ������ ����

	// ������
	int MagicBarLen [100];				// ������ ����
	int MagicBarCount [100];			// �ش� ������ ���̿� ���� ����
	int sizeOfMagicBar;					// ������ ������ ����

	// �����ƿ��ڳ�
	char MagicOutcornerType [100][10];	// �����ƿ��ڳ� �԰�
	int MagicOutcornerLen [100];		// �����ƿ��ڳ� ����
	int MagicOutcornerCount [100];		// �ش� �����ƿ��ڳ� �԰�/���� ���տ� ���� ����
	int sizeOfMagicOutcorner;			// �����ƿ��ڳ� ������ ����

	// �������ڳ�
	char MagicIncornerType [100][10];	// �������ڳ� �԰�
	int MagicIncornerLen [100];			// �������ڳ� ����
	int MagicIncornerCount [100];		// �ش� �������ڳ� ���̿� ���� ����
	int sizeOfMagicIncorner;			// �������ڳ� ������ ����

	// �Ϲ� ���
	// ��
	int beamLength [100];				// �� ����
	int beamCount [100];				// �ش� �� ���̿� ���� ����
	int sizeOfBeamKinds;				// �� ������ ����

	// �� �� ���� ��ü
	int nUnknownObjects;				// �������� ���� ��ü�� ����
};

void		initArray (double arr [], short arrSize);											// �迭 �ʱ�ȭ �Լ�
int			compare (const void* first, const void* second);									// ������������ ������ �� ����ϴ� ���Լ� (����Ʈ)
ColumnInfo	findColumn (ColumnPos* columnPos, short iHor, short iVer, short floorInd);			// �����ֿ�, �����ֿ�, �� ������ �̿��Ͽ� ��� ã��
GSErrCode	exportGridElementInfo (void);														// ����(���,��,������)���� ������ �����ϰ� �����ؼ� ���� ���Ϸ� ��������
short		DGCALLBACK inputThresholdHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// [���̾�α�] ��� �� �ּ� ���� �Ÿ��� ����ڿ��� �Է� ���� (�⺻��: 2000 mm)
GSErrCode	exportSelectedElementInfo (void);													// ������ ���� ���� ��������

#endif