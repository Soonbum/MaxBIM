#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "Others.hpp"

using namespace othersDG;

LayerNameSystem	layerInfo;


// ���̾� ���� �ʱ�ȭ
void	initLayerInfo (void)
{
	short xx;

	// ���� ����
	layerInfo.bType_01_S = false;
	layerInfo.bType_02_A = false;
	layerInfo.bType_03_M = false;
	layerInfo.bType_04_E = false;
	layerInfo.bType_05_T = false;
	layerInfo.bType_06_F = false;
	layerInfo.bType_07_Q = false;
	layerInfo.bType_08_L = false;
	layerInfo.bType_09_C = false;
	layerInfo.bType_10_K = false;

	// �� ����
	for (xx = 0 ; xx < 100 ; ++xx) {
		layerInfo.bDong_1xx [xx] = false;
		layerInfo.bDong_2xx [xx] = false;
		layerInfo.bDong_3xx [xx] = false;
		layerInfo.bDong_4xx [xx] = false;
		layerInfo.bDong_5xx [xx] = false;
		layerInfo.bDong_6xx [xx] = false;
		layerInfo.bDong_7xx [xx] = false;
		layerInfo.bDong_8xx [xx] = false;
		layerInfo.bDong_9xx [xx] = false;
		layerInfo.bDong_10xx [xx] = false;
		layerInfo.bDong_11xx [xx] = false;
		layerInfo.bDong_12xx [xx] = false;
		layerInfo.bDong_13xx [xx] = false;
		layerInfo.bDong_14xx [xx] = false;
		layerInfo.bDong_15xx [xx] = false;
		layerInfo.bDong_SHOP = false;
		layerInfo.bDong_SECU = false;
	}

	// �� ����
	for (xx = 0 ; xx < 10 ; ++xx) {
		layerInfo.bFloor_Bxx [xx] = false;
		layerInfo.bFloor_PHxx [xx] = false;
	}
	for (xx = 0 ; xx < 100 ; ++xx) {
		layerInfo.bFloor_Fxx [xx] = false;
	}

	// CJ ����
	for (xx = 0 ; xx < 100 ; ++xx) {
		layerInfo.bCJ [xx] = false;
	}

	// CJ �� �ð�����
	for (xx = 0 ; xx < 100 ; ++xx) {
		layerInfo.bOrderInCJ [xx] = false;
	}

	// ���� �� ��ü ����
	// 01-S ����
	layerInfo.bType_01_S_STAR = false;	// ���
	layerInfo.bType_01_S_COLU = false;	// ���
	layerInfo.bType_01_S_FOOT = false;	// ����
	layerInfo.bType_01_S_WALL = false;	// ��ü
	layerInfo.bType_01_S_BEAM = false;	// ��
	layerInfo.bType_01_S_SLAB = false;	// ������
	layerInfo.bType_01_S_CLST = false;	// ö����
	layerInfo.bType_01_S_BMST = false;	// ö��
	layerInfo.bType_01_S_RAMP = false;	// ����
	layerInfo.bType_01_S_CWAL = false;	// �պ�
	layerInfo.bType_01_S_WTWL = false;	// �����
	layerInfo.bType_01_S_CSTN = false;	// ����
	layerInfo.bType_01_S_MPAD = false;	// ����е�
	layerInfo.bType_01_S_GADN = false;	// ȭ��
	layerInfo.bType_01_S_PARA = false;	// �Ķ���
	layerInfo.bType_01_S_CLPC = false;	// PC���
	layerInfo.bType_01_S_BMPC = false;	// ��PC
	layerInfo.bType_01_S_BMWL = false;	// ����ü
	layerInfo.bType_01_S_STST = false;	// ö����

	// 02-A ���ึ��
	layerInfo.bType_02_A_FURN = false;	// ����
	layerInfo.bType_02_A_INSU = false;	// �ܿ���
	layerInfo.bType_02_A_PAIN = false;	// ����
	layerInfo.bType_02_A_MOLD = false;	// ����
	layerInfo.bType_02_A_MORT = false;	// ��Ż
	layerInfo.bType_02_A_WATE = false;	// ���
	layerInfo.bType_02_A_BRIC = false;	// ����
	layerInfo.bType_02_A_PAPE = false;	// ����
	layerInfo.bType_02_A_BLOC = false;	// ���
	layerInfo.bType_02_A_GYPS = false;	// ������
	layerInfo.bType_02_A_STON = false;	// ����
	layerInfo.bType_02_A_INTE = false;	// ����
	layerInfo.bType_02_A_GLAS = false;	// ����
	layerInfo.bType_02_A_HARD = false;	// ö��
	layerInfo.bType_02_A_TILE = false;	// Ÿ��
	layerInfo.bType_02_A_PANE = false;	// �ǳ�
	layerInfo.bType_02_A_PLYW = false;	// ����
	layerInfo.bType_02_A_PCON = false;	// ������ũ��Ʈ
	
	// 05-T ������
	layerInfo.bType_05_T_TIMB = false;	// ����
	layerInfo.bType_05_T_BIMJ = false;	// �����������
	layerInfo.bType_05_T_BDCM = false;	// ��չ��
	layerInfo.bType_05_T_DMGA = false;	// �ٸ���
	layerInfo.bType_05_T_RIBL = false;	// �����
	layerInfo.bType_05_T_BMSP = false;	// ����ħ�ʷ�
	layerInfo.bType_05_T_BSTA = false;	// �����
	layerInfo.bType_05_T_SPIP = false;	// �簢������
	layerInfo.bType_05_T_SUPT = false;	// ����Ʈ
	layerInfo.bType_05_T_SYSU = false;	// �ý��ۼ���Ʈ
	layerInfo.bType_05_T_OUTA = false;	// �ƿ��ڳʾޱ�
	layerInfo.bType_05_T_OUTP = false;	// �ƿ��ڳ��ǳ�
	layerInfo.bType_05_T_AFOM = false;	// ����
	layerInfo.bType_05_T_CPIP = false;	// ����������
	layerInfo.bType_05_T_UGON = false;	// ��������
	layerInfo.bType_05_T_UFOM = false;	// ������
	layerInfo.bType_05_T_INCO = false;	// ���ڳ��ǳ�
	layerInfo.bType_05_T_JOIB = false;	// ����Ʈ��
	layerInfo.bType_05_T_EFOM = false;	// ���̰�Ǫ��
	layerInfo.bType_05_T_JSUPT = false;	// �輭��Ʈ
	layerInfo.bType_05_T_WTST = false;	// ������
	layerInfo.bType_05_T_CLAM = false;	// Ŭ����
	layerInfo.bType_05_T_LUMB = false;	// �����
	layerInfo.bType_05_T_TRUS = false;	// Ʈ����
	layerInfo.bType_05_T_TBBM = false;	// ������
	layerInfo.bType_05_T_BCWF = false;	// �պ�������
	layerInfo.bType_05_T_PLYW = false;	// ����
	layerInfo.bType_05_T_FISP = false;	// �ٷ������̼�
	layerInfo.bType_05_T_STSE = false;	// ���������
	layerInfo.bType_05_T_SLSE = false;	// ������������
	layerInfo.bType_05_T_RAIL = false;	// ��ɷ���
	
	// 06-F ���ü�
	layerInfo.bType_06_F_STRU = false;	// ����H����
	layerInfo.bType_06_F_HFIL = false;	// ����H����
	layerInfo.bType_06_F_SJAK = false;	// ��ũ����
	layerInfo.bType_06_F_PJAK = false;	// �����ε���
	layerInfo.bType_06_F_BRKT = false;	// �����
	layerInfo.bType_06_F_PBKT = false;	// �ǽ� �����
	layerInfo.bType_06_F_CIP = false;	// �븷�� ��ü
	layerInfo.bType_06_F_LAND = false;	// ����
	layerInfo.bType_06_F_ANGL = false;	// �ޱ�
	layerInfo.bType_06_F_ERAC = false;	// ���ݾ�Ŀ
	layerInfo.bType_06_F_LUMB = false;	// �����
	layerInfo.bType_06_F_BPAN = false;	// ������
	layerInfo.bType_06_F_WALE = false;	// ����
	layerInfo.bType_06_F_PILE = false;	// ����
}

// ���̾� ���� �����ϱ�
GSErrCode	showLayersEasily (void)
{
	GSErrCode	err = NoError;
	
	API_Attribute	attrib;
	short			xx, i;
	short			nLayers;

	bool	success;
	char	*token;
	char	tempStr [5];
	char	tok1 [5];
	char	tok2 [5];
	char	tok3 [5];
	char	tok4 [5];
	char	tok5 [5];
	char	tok6 [5];
	char	tok7 [5];
	char	constructionCode [5];
	int		DongNumber;
	int		CJNumber;
	int		OrderInCJNumber;

	short	result;

	
	// ���̾� ���� �ʱ�ȭ
	initLayerInfo ();

	// ������Ʈ �� ���̾� �̸��� ���� �о��
	BNZeroMemory (&attrib, sizeof (API_Attribute));
	attrib.header.typeID = API_LayerID;
	err = ACAPI_Attribute_GetNum (API_LayerID, &nLayers);

	for (xx = 1; xx <= nLayers && err == NoError ; ++xx) {
		attrib.header.index = xx;
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			strcpy (tok1, "");
			strcpy (tok2, "");
			strcpy (tok3, "");
			strcpy (tok4, "");
			strcpy (tok5, "");
			strcpy (tok6, "");
			strcpy (tok7, "");
			i = 1;
			success = 0;
			// ���� �ڵ�: 01-S-(0101)-9B1-(01)-(01)-WALL  ��, ��ȣ ���� ���û���
			// ���̾� �̸��� "-" ���� �������� �ɰ���
			token = strtok (attrib.layer.head.name, "-");
			while (token != NULL) {
				// ���� �� ���� Ȯ��
				// 1�� (�Ϸù�ȣ) - �ʼ� (2����, ����)
				if (i == 1) {
					strcpy (tempStr, token);
					if (strlen (tempStr) == 2) {
						strcpy (tok1, tempStr);
						success = true;
					} else {
						i=8;
						success = false;
					}
				}
				// 2�� (���� ����) - �ʼ� (1����, ����)
				if (i == 2) {
					strcpy (tempStr, token);
					if (strlen (tempStr) == 1) {
						strcpy (tok2, tempStr);
						success = true;
					} else {
						i=8;
						success = false;
					}
				}
				// 3�� (�� ����) - ���� (4����)
				if (i == 3) {
					strcpy (tempStr, token);
					// �� ������ ���,
					if (strlen (tempStr) == 4) {
						strcpy (tok3, tempStr);
						success = true;
					// �� ������ ���
					} else if (strlen (tempStr) == 3) {
						strcpy (tok4, tempStr);
						i=4;
						success = true;
					} else {
						i=8;
						success = false;
					}
				}
				// 4�� (�� ����) - �ʼ� (3����)
				if (i == 4) {
					strcpy (tempStr, token);
					if (strlen (tempStr) == 3) {
						strcpy (tok4, tempStr);
						success = true;
					} else {
						success = false;
					}
				}
				// 5�� (CJ ����) - ���� (2����, ����)
				if (i == 5) {
					strcpy (tempStr, token);
					if (strlen (tempStr) == 2) {
						strcpy (tok5, tempStr);
						success = true;
					} else if (strlen (tempStr) > 3) {
						strcpy (tok7, tempStr);
						i=8;
						success = true;
					} else {
						i=6;
						success = true;
					}
				}
				// 6�� (CJ �� �ð�����) - ���� (2����, ����)
				if (i == 6) {
					strcpy (tempStr, token);
					if (strlen (tempStr) == 2) {
						strcpy (tok6, tempStr);
						success = true;
					} else if (strlen (tempStr) >= 3) {
						strcpy (tok7, tempStr);
						i=8;
						success = true;
					} else {
						success = false;
					}
				}
				// 7�� (���� �� ��ü ����) - �ʼ� (3���� �̻�)
				if (i == 7) {
					strcpy (tempStr, token);
					if (strlen (tempStr) >= 3) {
						strcpy (tok7, tempStr);
						success = true;
					} else {
						success = false;
					}
				}
				i++;
				token = strtok (NULL, "-");
			}

			// 7�ܰ���� ���������� �Ϸ�Ǹ� ����ü�� ����
			if (success == true) {
				// �Ϸ� ��ȣ�� ���� ���� ���ڸ� ���� ��ħ
				sprintf (constructionCode, "%s-%s", tok1, tok2);
				
				// 1,2�ܰ�. ���� ���� Ȯ��
				if (strncmp (constructionCode, "01-S", 4) == 0)	layerInfo.bType_01_S = true;
				if (strncmp (constructionCode, "02-A", 4) == 0)	layerInfo.bType_02_A = true;
				if (strncmp (constructionCode, "03-M", 4) == 0)	layerInfo.bType_03_M = true;
				if (strncmp (constructionCode, "04-E", 4) == 0)	layerInfo.bType_04_E = true;
				if (strncmp (constructionCode, "05-T", 4) == 0)	layerInfo.bType_05_T = true;
				if (strncmp (constructionCode, "06-F", 4) == 0)	layerInfo.bType_06_F = true;
				if (strncmp (constructionCode, "07-Q", 4) == 0)	layerInfo.bType_07_Q = true;
				if (strncmp (constructionCode, "08-L", 4) == 0)	layerInfo.bType_08_L = true;
				if (strncmp (constructionCode, "09-C", 4) == 0)	layerInfo.bType_09_C = true;
				if (strncmp (constructionCode, "10-K", 4) == 0)	layerInfo.bType_10_K = true;

				// 3�ܰ�. �� ����
				DongNumber = atoi (tok3);
				if (DongNumber == 0) {
					if (strncmp (tok4, "SHOP", 4) == 0)	layerInfo.bDong_SHOP = true;
					if (strncmp (tok4, "SECU", 4) == 0)	layerInfo.bDong_SECU = true;
				} else {
					if ((DongNumber >= 100) && (DongNumber <= 199))
						layerInfo.bDong_1xx [DongNumber - 100] = true;
					if ((DongNumber >= 200) && (DongNumber <= 299))
						layerInfo.bDong_2xx [DongNumber - 200] = true;
					if ((DongNumber >= 300) && (DongNumber <= 399))
						layerInfo.bDong_3xx [DongNumber - 300] = true;
					if ((DongNumber >= 400) && (DongNumber <= 499))
						layerInfo.bDong_4xx [DongNumber - 400] = true;
					if ((DongNumber >= 500) && (DongNumber <= 599))
						layerInfo.bDong_5xx [DongNumber - 500] = true;
					if ((DongNumber >= 600) && (DongNumber <= 699))
						layerInfo.bDong_6xx [DongNumber - 600] = true;
					if ((DongNumber >= 700) && (DongNumber <= 799))
						layerInfo.bDong_7xx [DongNumber - 700] = true;
					if ((DongNumber >= 800) && (DongNumber <= 899))
						layerInfo.bDong_8xx [DongNumber - 800] = true;
					if ((DongNumber >= 900) && (DongNumber <= 999))
						layerInfo.bDong_9xx [DongNumber - 900] = true;
					if ((DongNumber >= 1000) && (DongNumber <= 1099))
						layerInfo.bDong_10xx [DongNumber - 1000] = true;
					if ((DongNumber >= 1100) && (DongNumber <= 1199))
						layerInfo.bDong_11xx [DongNumber - 1100] = true;
					if ((DongNumber >= 1200) && (DongNumber <= 1299))
						layerInfo.bDong_12xx [DongNumber - 1200] = true;
					if ((DongNumber >= 1300) && (DongNumber <= 1399))
						layerInfo.bDong_13xx [DongNumber - 1300] = true;
					if ((DongNumber >= 1400) && (DongNumber <= 1499))
						layerInfo.bDong_14xx [DongNumber - 1400] = true;
					if ((DongNumber >= 1500) && (DongNumber <= 1599))
						layerInfo.bDong_15xx [DongNumber - 1500] = true;
				}

				// 4�ܰ�. �� ����
				if (strncmp (tok4, "9B1", 3) == 0)	layerInfo.bFloor_Bxx [1] = true;
				if (strncmp (tok4, "8B2", 3) == 0)	layerInfo.bFloor_Bxx [2] = true;
				if (strncmp (tok4, "7B3", 3) == 0)	layerInfo.bFloor_Bxx [3] = true;
				if (strncmp (tok4, "6B4", 3) == 0)	layerInfo.bFloor_Bxx [4] = true;
				if (strncmp (tok4, "5B5", 3) == 0)	layerInfo.bFloor_Bxx [5] = true;
				if (strncmp (tok4, "4B6", 3) == 0)	layerInfo.bFloor_Bxx [6] = true;
				if (strncmp (tok4, "3B7", 3) == 0)	layerInfo.bFloor_Bxx [7] = true;
				if (strncmp (tok4, "2B8", 3) == 0)	layerInfo.bFloor_Bxx [8] = true;
				if (strncmp (tok4, "1B9", 3) == 0)	layerInfo.bFloor_Bxx [9] = true;

				if (strncmp (tok4, "F01", 3) == 0)	layerInfo.bFloor_Fxx [1] = true;
				if (strncmp (tok4, "F02", 3) == 0)	layerInfo.bFloor_Fxx [2] = true;
				if (strncmp (tok4, "F03", 3) == 0)	layerInfo.bFloor_Fxx [3] = true;
				if (strncmp (tok4, "F04", 3) == 0)	layerInfo.bFloor_Fxx [4] = true;
				if (strncmp (tok4, "F05", 3) == 0)	layerInfo.bFloor_Fxx [5] = true;
				if (strncmp (tok4, "F06", 3) == 0)	layerInfo.bFloor_Fxx [6] = true;
				if (strncmp (tok4, "F07", 3) == 0)	layerInfo.bFloor_Fxx [7] = true;
				if (strncmp (tok4, "F08", 3) == 0)	layerInfo.bFloor_Fxx [8] = true;
				if (strncmp (tok4, "F09", 3) == 0)	layerInfo.bFloor_Fxx [9] = true;
				if (strncmp (tok4, "F10", 3) == 0)	layerInfo.bFloor_Fxx [10] = true;
				if (strncmp (tok4, "F11", 3) == 0)	layerInfo.bFloor_Fxx [11] = true;
				if (strncmp (tok4, "F12", 3) == 0)	layerInfo.bFloor_Fxx [12] = true;
				if (strncmp (tok4, "F13", 3) == 0)	layerInfo.bFloor_Fxx [13] = true;
				if (strncmp (tok4, "F14", 3) == 0)	layerInfo.bFloor_Fxx [14] = true;
				if (strncmp (tok4, "F15", 3) == 0)	layerInfo.bFloor_Fxx [15] = true;
				if (strncmp (tok4, "F16", 3) == 0)	layerInfo.bFloor_Fxx [16] = true;
				if (strncmp (tok4, "F17", 3) == 0)	layerInfo.bFloor_Fxx [17] = true;
				if (strncmp (tok4, "F18", 3) == 0)	layerInfo.bFloor_Fxx [18] = true;
				if (strncmp (tok4, "F19", 3) == 0)	layerInfo.bFloor_Fxx [19] = true;
				if (strncmp (tok4, "F20", 3) == 0)	layerInfo.bFloor_Fxx [20] = true;
				if (strncmp (tok4, "F21", 3) == 0)	layerInfo.bFloor_Fxx [21] = true;
				if (strncmp (tok4, "F22", 3) == 0)	layerInfo.bFloor_Fxx [22] = true;
				if (strncmp (tok4, "F23", 3) == 0)	layerInfo.bFloor_Fxx [23] = true;
				if (strncmp (tok4, "F24", 3) == 0)	layerInfo.bFloor_Fxx [24] = true;
				if (strncmp (tok4, "F25", 3) == 0)	layerInfo.bFloor_Fxx [25] = true;
				if (strncmp (tok4, "F26", 3) == 0)	layerInfo.bFloor_Fxx [26] = true;
				if (strncmp (tok4, "F27", 3) == 0)	layerInfo.bFloor_Fxx [27] = true;
				if (strncmp (tok4, "F28", 3) == 0)	layerInfo.bFloor_Fxx [28] = true;
				if (strncmp (tok4, "F29", 3) == 0)	layerInfo.bFloor_Fxx [29] = true;
				if (strncmp (tok4, "F30", 3) == 0)	layerInfo.bFloor_Fxx [30] = true;
				if (strncmp (tok4, "F31", 3) == 0)	layerInfo.bFloor_Fxx [31] = true;
				if (strncmp (tok4, "F32", 3) == 0)	layerInfo.bFloor_Fxx [32] = true;
				if (strncmp (tok4, "F33", 3) == 0)	layerInfo.bFloor_Fxx [33] = true;
				if (strncmp (tok4, "F34", 3) == 0)	layerInfo.bFloor_Fxx [34] = true;
				if (strncmp (tok4, "F35", 3) == 0)	layerInfo.bFloor_Fxx [35] = true;
				if (strncmp (tok4, "F36", 3) == 0)	layerInfo.bFloor_Fxx [36] = true;
				if (strncmp (tok4, "F37", 3) == 0)	layerInfo.bFloor_Fxx [37] = true;
				if (strncmp (tok4, "F38", 3) == 0)	layerInfo.bFloor_Fxx [38] = true;
				if (strncmp (tok4, "F39", 3) == 0)	layerInfo.bFloor_Fxx [39] = true;
				if (strncmp (tok4, "F40", 3) == 0)	layerInfo.bFloor_Fxx [40] = true;
				if (strncmp (tok4, "F41", 3) == 0)	layerInfo.bFloor_Fxx [41] = true;
				if (strncmp (tok4, "F42", 3) == 0)	layerInfo.bFloor_Fxx [42] = true;
				if (strncmp (tok4, "F43", 3) == 0)	layerInfo.bFloor_Fxx [43] = true;
				if (strncmp (tok4, "F44", 3) == 0)	layerInfo.bFloor_Fxx [44] = true;
				if (strncmp (tok4, "F45", 3) == 0)	layerInfo.bFloor_Fxx [45] = true;
				if (strncmp (tok4, "F46", 3) == 0)	layerInfo.bFloor_Fxx [46] = true;
				if (strncmp (tok4, "F47", 3) == 0)	layerInfo.bFloor_Fxx [47] = true;
				if (strncmp (tok4, "F48", 3) == 0)	layerInfo.bFloor_Fxx [48] = true;
				if (strncmp (tok4, "F49", 3) == 0)	layerInfo.bFloor_Fxx [49] = true;
				if (strncmp (tok4, "F50", 3) == 0)	layerInfo.bFloor_Fxx [50] = true;
				if (strncmp (tok4, "F51", 3) == 0)	layerInfo.bFloor_Fxx [51] = true;
				if (strncmp (tok4, "F52", 3) == 0)	layerInfo.bFloor_Fxx [52] = true;
				if (strncmp (tok4, "F53", 3) == 0)	layerInfo.bFloor_Fxx [53] = true;
				if (strncmp (tok4, "F54", 3) == 0)	layerInfo.bFloor_Fxx [54] = true;
				if (strncmp (tok4, "F55", 3) == 0)	layerInfo.bFloor_Fxx [55] = true;
				if (strncmp (tok4, "F56", 3) == 0)	layerInfo.bFloor_Fxx [56] = true;
				if (strncmp (tok4, "F57", 3) == 0)	layerInfo.bFloor_Fxx [57] = true;
				if (strncmp (tok4, "F58", 3) == 0)	layerInfo.bFloor_Fxx [58] = true;
				if (strncmp (tok4, "F59", 3) == 0)	layerInfo.bFloor_Fxx [59] = true;
				if (strncmp (tok4, "F60", 3) == 0)	layerInfo.bFloor_Fxx [60] = true;
				if (strncmp (tok4, "F61", 3) == 0)	layerInfo.bFloor_Fxx [61] = true;
				if (strncmp (tok4, "F62", 3) == 0)	layerInfo.bFloor_Fxx [62] = true;
				if (strncmp (tok4, "F63", 3) == 0)	layerInfo.bFloor_Fxx [63] = true;
				if (strncmp (tok4, "F64", 3) == 0)	layerInfo.bFloor_Fxx [64] = true;
				if (strncmp (tok4, "F65", 3) == 0)	layerInfo.bFloor_Fxx [65] = true;
				if (strncmp (tok4, "F66", 3) == 0)	layerInfo.bFloor_Fxx [66] = true;
				if (strncmp (tok4, "F67", 3) == 0)	layerInfo.bFloor_Fxx [67] = true;
				if (strncmp (tok4, "F68", 3) == 0)	layerInfo.bFloor_Fxx [68] = true;
				if (strncmp (tok4, "F69", 3) == 0)	layerInfo.bFloor_Fxx [69] = true;
				if (strncmp (tok4, "F70", 3) == 0)	layerInfo.bFloor_Fxx [70] = true;
				if (strncmp (tok4, "F71", 3) == 0)	layerInfo.bFloor_Fxx [71] = true;
				if (strncmp (tok4, "F72", 3) == 0)	layerInfo.bFloor_Fxx [72] = true;
				if (strncmp (tok4, "F73", 3) == 0)	layerInfo.bFloor_Fxx [73] = true;
				if (strncmp (tok4, "F74", 3) == 0)	layerInfo.bFloor_Fxx [74] = true;
				if (strncmp (tok4, "F75", 3) == 0)	layerInfo.bFloor_Fxx [75] = true;
				if (strncmp (tok4, "F76", 3) == 0)	layerInfo.bFloor_Fxx [76] = true;
				if (strncmp (tok4, "F77", 3) == 0)	layerInfo.bFloor_Fxx [77] = true;
				if (strncmp (tok4, "F78", 3) == 0)	layerInfo.bFloor_Fxx [78] = true;
				if (strncmp (tok4, "F79", 3) == 0)	layerInfo.bFloor_Fxx [79] = true;
				if (strncmp (tok4, "F80", 3) == 0)	layerInfo.bFloor_Fxx [80] = true;
				if (strncmp (tok4, "F81", 3) == 0)	layerInfo.bFloor_Fxx [81] = true;
				if (strncmp (tok4, "F82", 3) == 0)	layerInfo.bFloor_Fxx [82] = true;
				if (strncmp (tok4, "F83", 3) == 0)	layerInfo.bFloor_Fxx [83] = true;
				if (strncmp (tok4, "F84", 3) == 0)	layerInfo.bFloor_Fxx [84] = true;
				if (strncmp (tok4, "F85", 3) == 0)	layerInfo.bFloor_Fxx [85] = true;
				if (strncmp (tok4, "F86", 3) == 0)	layerInfo.bFloor_Fxx [86] = true;
				if (strncmp (tok4, "F87", 3) == 0)	layerInfo.bFloor_Fxx [87] = true;
				if (strncmp (tok4, "F88", 3) == 0)	layerInfo.bFloor_Fxx [88] = true;
				if (strncmp (tok4, "F89", 3) == 0)	layerInfo.bFloor_Fxx [89] = true;
				if (strncmp (tok4, "F90", 3) == 0)	layerInfo.bFloor_Fxx [90] = true;
				if (strncmp (tok4, "F91", 3) == 0)	layerInfo.bFloor_Fxx [91] = true;
				if (strncmp (tok4, "F92", 3) == 0)	layerInfo.bFloor_Fxx [92] = true;
				if (strncmp (tok4, "F93", 3) == 0)	layerInfo.bFloor_Fxx [93] = true;
				if (strncmp (tok4, "F94", 3) == 0)	layerInfo.bFloor_Fxx [94] = true;
				if (strncmp (tok4, "F95", 3) == 0)	layerInfo.bFloor_Fxx [95] = true;
				if (strncmp (tok4, "F96", 3) == 0)	layerInfo.bFloor_Fxx [96] = true;
				if (strncmp (tok4, "F97", 3) == 0)	layerInfo.bFloor_Fxx [97] = true;
				if (strncmp (tok4, "F98", 3) == 0)	layerInfo.bFloor_Fxx [98] = true;
				if (strncmp (tok4, "F99", 3) == 0)	layerInfo.bFloor_Fxx [99] = true;

				if (strncmp (tok4, "PH1", 3) == 0)	layerInfo.bFloor_PHxx [1] = true;
				if (strncmp (tok4, "PH2", 3) == 0)	layerInfo.bFloor_PHxx [2] = true;
				if (strncmp (tok4, "PH3", 3) == 0)	layerInfo.bFloor_PHxx [3] = true;
				if (strncmp (tok4, "PH4", 3) == 0)	layerInfo.bFloor_PHxx [4] = true;
				if (strncmp (tok4, "PH5", 3) == 0)	layerInfo.bFloor_PHxx [5] = true;
				if (strncmp (tok4, "PH6", 3) == 0)	layerInfo.bFloor_PHxx [6] = true;
				if (strncmp (tok4, "PH7", 3) == 0)	layerInfo.bFloor_PHxx [7] = true;
				if (strncmp (tok4, "PH8", 3) == 0)	layerInfo.bFloor_PHxx [8] = true;
				if (strncmp (tok4, "PH9", 3) == 0)	layerInfo.bFloor_PHxx [9] = true;

				// 5�ܰ�. CJ ����
				CJNumber = atoi (tok5);
				if (CJNumber != 0)	layerInfo.bCJ [CJNumber] = true;

				// 6�ܰ�. CJ �� �ð�����
				OrderInCJNumber = atoi (tok6);
				if (OrderInCJNumber != 0)	layerInfo.bOrderInCJ [OrderInCJNumber] = true;

				// 7�ܰ�. ���� �� ��ü ����
				// 01-S ����
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "STAR", 4) == 0))	layerInfo.bType_01_S_STAR = true;	// ���
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "COLU", 4) == 0))	layerInfo.bType_01_S_COLU = true;	// ���
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "FOOT", 4) == 0))	layerInfo.bType_01_S_FOOT = true;	// ����
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "WALL", 4) == 0))	layerInfo.bType_01_S_WALL = true;	// ��ü
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "BEAM", 4) == 0))	layerInfo.bType_01_S_BEAM = true;	// ��
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "SLAB", 4) == 0))	layerInfo.bType_01_S_SLAB = true;	// ������
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "CLST", 4) == 0))	layerInfo.bType_01_S_CLST = true;	// ö����
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "BMST", 4) == 0))	layerInfo.bType_01_S_BMST = true;	// ö��
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "RAMP", 4) == 0))	layerInfo.bType_01_S_RAMP = true;	// ����
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "CWAL", 4) == 0))	layerInfo.bType_01_S_CWAL = true;	// �պ�
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "WTWL", 4) == 0))	layerInfo.bType_01_S_WTWL = true;	// �����
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "CSTN", 4) == 0))	layerInfo.bType_01_S_CSTN = true;	// ����
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "MPAD", 4) == 0))	layerInfo.bType_01_S_MPAD = true;	// ����е�
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "GADN", 4) == 0))	layerInfo.bType_01_S_GADN = true;	// ȭ��
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "PARA", 4) == 0))	layerInfo.bType_01_S_PARA = true;	// �Ķ���
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "CLPC", 4) == 0))	layerInfo.bType_01_S_CLPC = true;	// PC���
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "BMPC", 4) == 0))	layerInfo.bType_01_S_BMPC = true;	// ��PC
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "BMWL", 4) == 0))	layerInfo.bType_01_S_BMWL = true;	// ����ü
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "STST", 4) == 0))	layerInfo.bType_01_S_STST = true;	// ö����

				// 02-A ���ึ��
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "FURN", 4) == 0))	layerInfo.bType_02_A_FURN = true;	// ����
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "INSU", 4) == 0))	layerInfo.bType_02_A_INSU = true;	// �ܿ���
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "PAIN", 4) == 0))	layerInfo.bType_02_A_PAIN = true;	// ����
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "MOLD", 4) == 0))	layerInfo.bType_02_A_MOLD = true;	// ����
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "MORT", 4) == 0))	layerInfo.bType_02_A_MORT = true;	// ��Ż
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "WATE", 4) == 0))	layerInfo.bType_02_A_WATE = true;	// ���
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "BRIC", 4) == 0))	layerInfo.bType_02_A_BRIC = true;	// ����
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "PAPE", 4) == 0))	layerInfo.bType_02_A_PAPE = true;	// ����
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "BLOC", 4) == 0))	layerInfo.bType_02_A_BLOC = true;	// ���
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "GYPS", 4) == 0))	layerInfo.bType_02_A_GYPS = true;	// ������
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "STON", 4) == 0))	layerInfo.bType_02_A_STON = true;	// ����
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "INTE", 4) == 0))	layerInfo.bType_02_A_INTE = true;	// ����
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "GLAS", 4) == 0))	layerInfo.bType_02_A_GLAS = true;	// ����
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "HARD", 4) == 0))	layerInfo.bType_02_A_HARD = true;	// ö��
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "TILE", 4) == 0))	layerInfo.bType_02_A_TILE = true;	// Ÿ��
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "PANE", 4) == 0))	layerInfo.bType_02_A_PANE = true;	// �ǳ�
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "PLYW", 4) == 0))	layerInfo.bType_02_A_PLYW = true;	// ����
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "PCON", 4) == 0))	layerInfo.bType_02_A_PCON = true;	// ������ũ��Ʈ

				// 05-T ������
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "TUNB", 4) == 0))	layerInfo.bType_05_T_TIMB = true;	// ����
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "BIMJ", 4) == 0))	layerInfo.bType_05_T_BIMJ = true;	// �����������
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "BDCM", 4) == 0))	layerInfo.bType_05_T_BDCM = true;	// ��չ��
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "DMGA", 4) == 0))	layerInfo.bType_05_T_DMGA = true;	// �ٸ���
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "RIBL", 4) == 0))	layerInfo.bType_05_T_RIBL = true;	// �����
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "BMSP", 4) == 0))	layerInfo.bType_05_T_BMSP = true;	// ����ħ�ʷ�
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "BSTA", 4) == 0))	layerInfo.bType_05_T_BSTA = true;	// �����
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "SPIP", 4) == 0))	layerInfo.bType_05_T_SPIP = true;	// �簢������
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "SUPT", 4) == 0))	layerInfo.bType_05_T_SUPT = true;	// ����Ʈ
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "SYSU", 4) == 0))	layerInfo.bType_05_T_SYSU = true;	// �ý��ۼ���Ʈ
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "OUTA", 4) == 0))	layerInfo.bType_05_T_OUTA = true;	// �ƿ��ڳʾޱ�
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "OUTP", 4) == 0))	layerInfo.bType_05_T_OUTP = true;	// �ƿ��ڳ��ǳ�
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "AFOM", 4) == 0))	layerInfo.bType_05_T_AFOM = true;	// ����
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "CPIP", 4) == 0))	layerInfo.bType_05_T_CPIP = true;	// ����������
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "UGON", 4) == 0))	layerInfo.bType_05_T_UGON = true;	// ��������
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "UFOM", 4) == 0))	layerInfo.bType_05_T_UFOM = true;	// ������
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "INCO", 4) == 0))	layerInfo.bType_05_T_INCO = true;	// ���ڳ��ǳ�
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "JOIB", 4) == 0))	layerInfo.bType_05_T_JOIB = true;	// ����Ʈ��
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "EFOM", 4) == 0))	layerInfo.bType_05_T_EFOM = true;	// ���̰�Ǫ��
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "JSUPT", 4) == 0))	layerInfo.bType_05_T_JSUPT = true;	// �輭��Ʈ
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "WTST", 4) == 0))	layerInfo.bType_05_T_WTST = true;	// ������
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "CLAM", 4) == 0))	layerInfo.bType_05_T_CLAM = true;	// Ŭ����
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "LUMB", 4) == 0))	layerInfo.bType_05_T_LUMB = true;	// �����
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "TRUS", 4) == 0))	layerInfo.bType_05_T_TRUS = true;	// Ʈ����
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "TBBM", 4) == 0))	layerInfo.bType_05_T_TBBM = true;	// ������
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "BCWF", 4) == 0))	layerInfo.bType_05_T_BCWF = true;	// �պ�������
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "PLYW", 4) == 0))	layerInfo.bType_05_T_PLYW = true;	// ����
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "FISP", 4) == 0))	layerInfo.bType_05_T_FISP = true;	// �ٷ������̼�
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "STSE", 4) == 0))	layerInfo.bType_05_T_STSE = true;	// ���������
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "SLSE", 4) == 0))	layerInfo.bType_05_T_SLSE = true;	// ������������
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "RAIL", 4) == 0))	layerInfo.bType_05_T_RAIL = true;	// ��ɷ���

				// 06-F ���ü�
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "STRU", 4) == 0))	layerInfo.bType_06_F_STRU = true;	// ����H����
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "HFIL", 4) == 0))	layerInfo.bType_06_F_HFIL = true;	// ����H����
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "SJAK", 4) == 0))	layerInfo.bType_06_F_SJAK = true;	// ��ũ����
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "PJAK", 4) == 0))	layerInfo.bType_06_F_PJAK = true;	// �����ε���
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "BRKT", 4) == 0))	layerInfo.bType_06_F_BRKT = true;	// �����
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "PBKT", 4) == 0))	layerInfo.bType_06_F_PBKT = true;	// �ǽ� �����
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "CIP", 4) == 0))	layerInfo.bType_06_F_CIP = true;	// �븷�� ��ü
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "LAND", 4) == 0))	layerInfo.bType_06_F_LAND = true;	// ����
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "ANGL", 4) == 0))	layerInfo.bType_06_F_ANGL = true;	// �ޱ�
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "ERAC", 4) == 0))	layerInfo.bType_06_F_ERAC = true;	// ���ݾ�Ŀ
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "LUMB", 4) == 0))	layerInfo.bType_06_F_LUMB = true;	// ������
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "BPAN", 4) == 0))	layerInfo.bType_06_F_BPAN = true;	// ������
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "WALE", 4) == 0))	layerInfo.bType_06_F_WALE = true;	// ����
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "PILE", 4) == 0))	layerInfo.bType_06_F_PILE = true;	// ����
			}
		}
		if (err == APIERR_DELETED)
			err = NoError;
	}

	// [���̾�α� �ڽ�] ���̾� �����ֱ�
	result = DGBlankModalDialog (700, 300, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, layerShowHandler, 0);

	return err;
}

// [���̾�α� �ڽ�] ���̾� �����ֱ�
short DGCALLBACK layerShowHandler (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;
	short	itmPosX, itmPosY;
	short	xx;
	char	tempStr [20];
	bool	bNextLine;
	short	dialogSizeX, dialogSizeY;

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, "���̾� ���� �����ϱ�");

			// Ȯ�� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 20, 40, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "Ȯ��");
			DGShowItem (dialogID, DG_OK);

			// ��� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 50, 40, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "���");
			DGShowItem (dialogID, DG_CANCEL);

			// ��: ���� ����
			bNextLine = false;
			itmPosX = 50;
			itmPosY = 25;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 75, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "���� ����");
			DGShowItem (dialogID, itmIdx);

			// üũ�ڽ�: ���� ���� ��ư
			itmPosX = 150;
			itmPosY = 20;
			if (layerInfo.bType_01_S) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "01-S ����");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_02_A) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "02-A ���ึ��");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_03_M) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "03-M ��輳��");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_04_E) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "04-E ���⼳��");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_05_T) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "05-T ������");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_06_F) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "06-F ���ü�");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_07_Q) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "07-Q ��������");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_08_L) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "08-L ����");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_09_C) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "09-C ���");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_10_K) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "10-K �Ǽ����");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}

			if (!bNextLine)
				itmPosY += 30;

			// ��: �� ����
			bNextLine = false;
			itmPosX = 50;
			itmPosY += 10;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 75, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "�� ����");
			DGShowItem (dialogID, itmIdx);

			// üũ�ڽ�: �� ���� ��ư
			itmPosX = 150;
			itmPosY -= 5;
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_1xx [xx]) {
					sprintf (tempStr, "%d��", 100 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
						bNextLine = true;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_2xx [xx]) {
					sprintf (tempStr, "%d��", 200 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
						bNextLine = true;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_3xx [xx]) {
					sprintf (tempStr, "%d��", 300 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
						bNextLine = true;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_4xx [xx]) {
					sprintf (tempStr, "%d��", 400 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
						bNextLine = true;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_5xx [xx]) {
					sprintf (tempStr, "%d��", 500 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
						bNextLine = true;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_6xx [xx]) {
					sprintf (tempStr, "%d��", 600 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
						bNextLine = true;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_7xx [xx]) {
					sprintf (tempStr, "%d��", 700 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
						bNextLine = true;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_8xx [xx]) {
					sprintf (tempStr, "%d��", 800 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
						bNextLine = true;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_9xx [xx]) {
					sprintf (tempStr, "%d��", 900 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
						bNextLine = true;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_10xx [xx]) {
					sprintf (tempStr, "%d��", 1000 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
						bNextLine = true;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_11xx [xx]) {
					sprintf (tempStr, "%d��", 1100 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
						bNextLine = true;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_12xx [xx]) {
					sprintf (tempStr, "%d��", 1200 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
						bNextLine = true;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_13xx [xx]) {
					sprintf (tempStr, "%d��", 1300 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
						bNextLine = true;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_14xx [xx]) {
					sprintf (tempStr, "%d��", 1400 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
						bNextLine = true;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_15xx [xx]) {
					sprintf (tempStr, "%d��", 1500 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
						bNextLine = true;
					}
				}
			}
			if (layerInfo.bDong_SHOP) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�ٸ�����");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bDong_SECU) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}

			if (!bNextLine)
				itmPosY += 30;

			// ��: �� ����
			bNextLine = false;
			itmPosX = 50;
			itmPosY += 10;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 75, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "�� ����");
			DGShowItem (dialogID, itmIdx);

			// üũ�ڽ�: �� ���� ��ư
			itmPosX = 150;
			itmPosY -= 5;
			for (xx = 9 ; xx >= 1 ; --xx) {
				if (layerInfo.bFloor_Bxx [xx]) {
					sprintf (tempStr, "����%d��", xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
						bNextLine = true;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bFloor_Fxx [xx]) {
					sprintf (tempStr, "����%d��", xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
						bNextLine = true;
					}
				}
			}
			for (xx = 1 ; xx < 10 ; ++xx) {
				if (layerInfo.bFloor_PHxx [xx]) {
					sprintf (tempStr, "PH%d��", xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
						bNextLine = true;
					}
				}
			}

			if (!bNextLine)
				itmPosY += 30;

			// ��: CJ ����
			bNextLine = false;
			itmPosX = 50;
			itmPosY += 10;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 75, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "CJ ����");
			DGShowItem (dialogID, itmIdx);

			// üũ�ڽ�: CJ ���� ��ư
			itmPosX = 150;
			itmPosY -= 5;
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bCJ [xx]) {
					sprintf (tempStr, "%2d", xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
						bNextLine = true;
					}
				}
			}

			if (!bNextLine)
				itmPosY += 30;

			// ��: CJ �� �ð�����
			bNextLine = false;
			itmPosX = 50;
			itmPosY += 10;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 75, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "�ð�����");
			DGShowItem (dialogID, itmIdx);

			// üũ�ڽ�: CJ �� �ð����� ��ư
			itmPosX = 150;
			itmPosY -= 5;
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bOrderInCJ [xx]) {
					sprintf (tempStr, "%2d", xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
						bNextLine = true;
					}
				}
			}

			if (!bNextLine)
				itmPosY += 30;

			// ��: ��ü(����)
			bNextLine = false;
			itmPosX = 50;
			itmPosY += 10;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 75, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "��ü(����)");
			DGShowItem (dialogID, itmIdx);

			// üũ�ڽ�: ��ü(����)
			itmPosX = 150;
			itmPosY -= 5;
			if (layerInfo.bType_01_S_STAR) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "���");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_01_S_COLU) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "���");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_01_S_FOOT) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_01_S_WALL) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��ü");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_01_S_BEAM) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_01_S_SLAB) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "������");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_01_S_CLST) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "ö����");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_01_S_BMST) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "ö��");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_01_S_RAMP) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_01_S_CWAL) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�պ�");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_01_S_WTWL) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�����");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_01_S_CSTN) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_01_S_MPAD) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����е�");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_01_S_GADN) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "ȭ��");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_01_S_PARA) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�Ķ���");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_01_S_CLPC) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "PC���");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_01_S_BMPC) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��PC");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_01_S_BMWL) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����ü");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_01_S_STST) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "ö����");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}

			if (!bNextLine)
				itmPosY += 30;

			// ��: ��ü(���ึ��)
			bNextLine = false;
			itmPosX = 50;
			itmPosY += 10;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 75, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "��ü(���ึ��)");
			DGShowItem (dialogID, itmIdx);

			// üũ�ڽ�: ��ü(���ึ��)
			itmPosX = 150;
			itmPosY -= 5;
			if (layerInfo.bType_02_A_FURN) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_02_A_INSU) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�ܿ���");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_02_A_PAIN) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_02_A_MOLD) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_02_A_MORT) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��Ż");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_02_A_WATE) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "���");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_02_A_BRIC) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_02_A_PAPE) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_02_A_BLOC) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "���");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_02_A_GYPS) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "������");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_02_A_STON) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_02_A_INTE) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_02_A_GLAS) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_02_A_HARD) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "ö��");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_02_A_TILE) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "Ÿ��");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_02_A_PANE) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�ǳ�");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_02_A_PLYW) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_02_A_PCON) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "������ũ��Ʈ");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}

			if (!bNextLine)
				itmPosY += 30;

			// ��: ��ü(������)
			bNextLine = false;
			itmPosX = 50;
			itmPosY += 10;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 75, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "��ü(������)");
			DGShowItem (dialogID, itmIdx);

			// üũ�ڽ�: ��ü(������)
			itmPosX = 150;
			itmPosY -= 5;
			if (layerInfo.bType_05_T_TIMB) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_05_T_BIMJ) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�����������");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_05_T_BDCM) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��չ��");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_05_T_DMGA) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�ٸ���");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_05_T_RIBL) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�����");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_05_T_BMSP) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����ħ�ʷ�");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_05_T_BSTA) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�����");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_05_T_SPIP) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�簢������");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_05_T_SUPT) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����Ʈ");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_05_T_SYSU) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�ý��ۼ���Ʈ");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_05_T_OUTA) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�ƿ��ڳʾޱ�");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_05_T_OUTP) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�ƿ��ڳ��ǳ�");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_05_T_AFOM) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_05_T_CPIP) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����������");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_05_T_UGON) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��������");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_05_T_UFOM) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "������");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_05_T_INCO) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "���ڳ��ǳ�");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_05_T_JOIB) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����Ʈ��");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_05_T_EFOM) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "���̰�Ǫ��");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_05_T_JSUPT) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�輭��Ʈ");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_05_T_WTST) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "������");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_05_T_CLAM) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "Ŭ����");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_05_T_LUMB) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�����");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_05_T_TRUS) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "Ʈ����");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_05_T_TBBM) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "������");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_05_T_BCWF) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�պ�������");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_05_T_PLYW) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_05_T_FISP) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�ٷ������̼�");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_05_T_STSE) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "���������");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_05_T_SLSE) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "������������");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_05_T_RAIL) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��ɷ���");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}

			if (!bNextLine)
				itmPosY += 30;

			// ��: ��ü(���ü�)
			bNextLine = false;
			itmPosX = 50;
			itmPosY += 10;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 75, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "��ü(���ü�)");
			DGShowItem (dialogID, itmIdx);

			// üũ�ڽ�: ��ü(���ü�)
			itmPosX = 150;
			itmPosY -= 5;
			if (layerInfo.bType_06_F_STRU) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����H����");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_06_F_HFIL) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����H����");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_06_F_SJAK) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��ũ����");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_06_F_PJAK) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�����ε���");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_06_F_BRKT) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�����");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_06_F_PBKT) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�ǽ� �����");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_06_F_CIP) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�븷�� ��ü");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_06_F_LAND) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_06_F_ANGL) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�ޱ�");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_06_F_ERAC) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "���ݾ�Ŀ");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_06_F_LUMB) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�����");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_06_F_BPAN) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "������");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_06_F_WALE) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}
			if (layerInfo.bType_06_F_PILE) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
					bNextLine = true;
				}
			}

			dialogSizeX = 700;
			dialogSizeY = itmPosY + 150;
			DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);

			break;
		
		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:

					// �ϴ� ��� ���̾ ����ٰ�,

					// ���ڿ� ���� �˰���
					/*
						1. ���� �ڵ� flag�� �о�� (�����̸� ��� �о��)
						2. �� �ڵ带 �о�� (���� ����, �ڵ� ���� ���� �� ����)
						3. �� �ڵ带 �о�� (���� ����)
						4. CJ ���� �ڵ带 �о�� (���� ����, �ڵ� ���� ���� �� ����)
						5. CJ �� �ð����� �ڵ带 �о�� (���� ����, �ڵ� ���� ���� �� ����)
						6. ���� �� ��ü ���� �ڵ带 �о�� (���� ����)
					*/

					// ������ �ڵ�
					// ���̾� ���̱�/���� ���� (APILay_Hidden �� ������ ����, ������ ����)
					// 1. Ư�� �̸����� ���̾� �Ӽ��� ã�� ���� (ACAPI_Attribute_Search)
					// 2. ���̾�α��� �Է� ���� ���� ����/���� �ɼ��� �ο��Ͽ� �Ӽ��� ���� (ACAPI_Attribute_Modify)
			
					// ���� ǥ���ϱ�
					//if ((attrib.layer.head.flags & APILay_Hidden) == true) {
					//	attrib.layer.head.flags ^= APILay_Hidden;
					//	ACAPI_Attribute_Modify (&attrib, NULL);
					//}

					// ���� �����
					//if ((attrib.layer.head.flags & APILay_Hidden) == false) {
					//	attrib.layer.head.flags |= APILay_Hidden;
					//	ACAPI_Attribute_Modify (&attrib, NULL);
					//}

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