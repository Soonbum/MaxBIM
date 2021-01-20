#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "Layers.hpp"

using namespace	layersDG;

LayerNameSystem	layerInfo;
LayerNameSystem	selectedInfo;
StatusOfLayerNameSystem	selectedInfoSaved;	// ����� ��ư ���¸� ����� �ε���

short	changedBtnItemIdx;	// ���°� ����� ��ư�� �׸� �ε���
short	clickedBtnItemIdx;	// Ŭ���� ��ư�� �׸� �ε���

short	BUTTON_LOAD;
short	LABEL_CODE;
short	SELECTALL_1_CONTYPE;
short	SELECTALL_2_DONG;
short	SELECTALL_3_FLOOR;
short	SELECTALL_4_CJ;
short	SELECTALL_5_ORDER;


// �޸� �Ҵ�
void		allocateMemory (LayerNameSystem *layerInfo)
{
	short	xx;

	// ���̾� ���� �޸� �Ҵ�
	layerInfo->code_state		= new bool [layerInfo->code_name.size ()];
	layerInfo->dong_state		= new bool [layerInfo->dong_name.size ()];
	layerInfo->floor_state		= new bool [layerInfo->floor_name.size ()];
	layerInfo->CJ_state			= new bool [layerInfo->CJ_name.size ()];
	layerInfo->orderInCJ_state	= new bool [layerInfo->orderInCJ_name.size ()];
	layerInfo->obj_state		= new bool [layerInfo->obj_name.size ()];
	layerInfo->subObj_state		= new bool [layerInfo->subObj_name.size ()];

	layerInfo->code_idx			= new short [layerInfo->code_name.size ()];
	layerInfo->dong_idx			= new short [layerInfo->dong_name.size ()];
	layerInfo->floor_idx		= new short [layerInfo->floor_name.size ()];
	layerInfo->CJ_idx			= new short [layerInfo->CJ_name.size ()];
	layerInfo->orderInCJ_idx	= new short [layerInfo->orderInCJ_name.size ()];
	layerInfo->obj_idx			= new short [layerInfo->obj_name.size ()];
	layerInfo->subObj_idx		= new short [layerInfo->subObj_name.size ()];

	// �迭 �ʱ�ȭ
	for (xx = 0 ; xx < layerInfo->code_name.size () ; ++xx) {
		layerInfo->code_state [xx] = false;
		layerInfo->code_idx [xx] = 0;
	}
	for (xx = 0 ; xx < layerInfo->dong_name.size () ; ++xx) {
		layerInfo->dong_state [xx] = false;
		layerInfo->dong_idx [xx] = 0;
	}
	for (xx = 0 ; xx < layerInfo->floor_name.size () ; ++xx) {
		layerInfo->floor_state [xx] = false;
		layerInfo->floor_idx [xx] = 0;
	}
	for (xx = 0 ; xx < layerInfo->CJ_name.size () ; ++xx) {
		layerInfo->CJ_state [xx] = false;
		layerInfo->CJ_idx [xx] = 0;
	}
	for (xx = 0 ; xx < layerInfo->orderInCJ_name.size () ; ++xx) {
		layerInfo->orderInCJ_state [xx] = false;
		layerInfo->orderInCJ_idx [xx] = 0;
	}
	for (xx = 0 ; xx < layerInfo->obj_name.size () ; ++xx) {
		layerInfo->obj_state [xx] = false;
		layerInfo->obj_idx [xx] = 0;
	}
	for (xx = 0 ; xx < layerInfo->subObj_name.size () ; ++xx) {
		layerInfo->subObj_state [xx] = false;
		layerInfo->subObj_idx [xx] = 0;
	}
}

// �޸� ����
void		deallocateMemory (LayerNameSystem *layerInfo)
{
	// ���̾� ���� �޸� ����
	delete []	layerInfo->code_state;
	delete []	layerInfo->dong_state;
	delete []	layerInfo->floor_state;
	delete []	layerInfo->CJ_state;
	delete []	layerInfo->orderInCJ_state;
	delete []	layerInfo->obj_state;
	delete []	layerInfo->subObj_state;

	delete []	layerInfo->code_idx;
	delete []	layerInfo->dong_idx;
	delete []	layerInfo->floor_idx;
	delete []	layerInfo->CJ_idx;
	delete []	layerInfo->orderInCJ_idx;
	delete []	layerInfo->obj_idx;
	delete []	layerInfo->subObj_idx;
}

// ���̾� ���� �����ϱ�
GSErrCode	showLayersEasily (void)
{
	GSErrCode	err = NoError;

	FILE	*fp;			// ���� ������
	char	line [10240];	// ���Ͽ��� �о�� ���� �ϳ�
	string	insElem;		// ��ū�� string���� ��ȯ�ؼ� vector�� ����
	char	*token;			// �о�� ���ڿ��� ��ū
	short	lineCount;		// �о�� ���� ��
	short	tokCount;		// �о�� ��ū ����

	API_Attribute	attrib;
	short			xx, yy, i;
	short			nLayers;

	bool	success;
	char	tempStr [20];
	char	tok1 [5];
	char	tok2 [5];
	char	tok3 [5];
	char	tok4 [5];
	char	tok5 [5];
	char	tok6 [5];
	char	tok7 [5];
	char	tok8 [5];
	char	constructionCode [5];

	short	result;


	// ���̾� ���� ���� ��������
	fp = fopen ("C:\\layer.csv", "r");

	for (lineCount = 1 ; lineCount <= 14 ; ++lineCount) {
		if (fp != NULL) {
			tokCount = 0;
			fscanf (fp, "%s\n", line);
			token = strtok (line, ",");
			tokCount ++;
			while (token != NULL) {
				if (strlen (token) > 0) {
					if (tokCount > 1) {
						insElem = token;

						if (lineCount == 1)		layerInfo.code_name.push_back (insElem);		// ���籸��
						if (lineCount == 2)		layerInfo.code_desc.push_back (insElem);		// ���籸�� ����
						if (lineCount == 3) {
							// ���� �� ��ȣ ���ڰ� 3�ڸ��� ������ �տ� 0�� ���� ��
							if (strlen (token) == 3) {
								strcpy (tempStr, "0");
								strcat (tempStr, token);
								insElem = tempStr;
							}

							layerInfo.dong_name.push_back (insElem);							// ��
						}
						if (lineCount == 4)		layerInfo.dong_desc.push_back (insElem);		// �� ����
						if (lineCount == 5)		layerInfo.floor_name.push_back (insElem);		// ��
						if (lineCount == 6)		layerInfo.floor_desc.push_back (insElem);		// �� ����
						if (lineCount == 7) {
							// ���� CJ ��ȣ ���ڰ� 1�ڸ��� ������ �տ� 0�� ���� ��
							if (strlen (token) == 1) {
								strcpy (tempStr, "0");
								strcat (tempStr, token);
								insElem = tempStr;
							}

							layerInfo.CJ_name.push_back (insElem);			// CJ
						}
						if (lineCount == 8) {
							// ���� CJ �� �ð����� ��ȣ ���ڰ� 1�ڸ��� ������ �տ� 0�� ���� ��
							if (strlen (token) == 1) {
								strcpy (tempStr, "0");
								strcat (tempStr, token);
								insElem = tempStr;
							}

							layerInfo.orderInCJ_name.push_back (insElem);	// CJ �� �ð�����
						}
						if (lineCount == 9)		layerInfo.obj_name.push_back (insElem);			// ����
						if (lineCount == 10)	layerInfo.obj_desc.push_back (insElem);			// ���� ����
						if (lineCount == 11)	layerInfo.obj_cat.push_back (insElem);			// ���簡 ���� ī�װ�(���籸��)
						if (lineCount == 12)	layerInfo.subObj_name.push_back (insElem);		// ��ü
						if (lineCount == 13)	layerInfo.subObj_desc.push_back (insElem);		// ��ü ����
						if (lineCount == 14)	layerInfo.subObj_cat.push_back (insElem);		// ��ü�� ���� ī�װ�(���籸��)
					}
				}
				token = strtok (NULL, ",");
				tokCount ++;
			}
		} else {
			ACAPI_WriteReport ("layer.csv ������ C:\\�� �����Ͻʽÿ�.", true);
			return	err;
		}
	}

	fclose (fp);

	// ����ü �ʱ�ȭ
	allocateMemory (&layerInfo);
	selectedInfo = layerInfo;	// selectedInfo���� vector�� ��� �����Ƿ� �ʱ�ȭ�� ���� ������ ��
	allocateMemory (&selectedInfo);

	// ������Ʈ �� ���̾� ������ �˾Ƴ�
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
			strcpy (tok8, "");
			i = 1;
			success = false;
			// ���� �ڵ�: 05-T-(0101)-9B1-(01)-(01)-COLU-(UFOM)  ��, ��ȣ ���� ���û���
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
						i=100;
						success = false;
					}
				}
				// 2�� (���� ����) - �ʼ� (1����, ����)
				else if (i == 2) {
					strcpy (tempStr, token);
					if (strlen (tempStr) == 1) {
						strcpy (tok2, tempStr);
						success = true;
					} else {
						i=100;
						success = false;
					}
				}
				// 3�� (�� ����) - ���� (4����)
				else if (i == 3) {
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
						i=100;
						success = false;
					}
				}
				// 4�� (�� ����) - �ʼ� (3����)
				else if (i == 4) {
					strcpy (tempStr, token);
					if (strlen (tempStr) == 3) {
						strcpy (tok4, tempStr);
						success = true;
					} else {
						i=100;
						success = false;
					}
				}
				// 5�� (CJ ����) - ���� (2����, ����)
				else if (i == 5) {
					strcpy (tempStr, token);
					if (strlen (tempStr) == 2) {
						strcpy (tok5, tempStr);
						success = true;
					} else if (strlen (tempStr) >= 3) {
						strcpy (tok7, tempStr);
						i=7;
						success = true;
					} else {
						i=100;
						success = false;
					}
				}
				// 6�� (CJ �� �ð�����) - ���� (2����, ����) - �� CJ ������ ������ �̰͵� ������
				else if (i == 6) {
					strcpy (tempStr, token);
					if (strlen (tempStr) == 2) {
						strcpy (tok6, tempStr);
						success = true;
					} else if (strlen (tempStr) >= 3) {
						strcpy (tok7, tempStr);
						i=7;
						success = true;
					} else {
						i=100;
						success = false;
					}
				}
				// 7�� (���� ����) - �ʼ� (3���� �̻�)
				else if (i == 7) {
					strcpy (tempStr, token);
					if (strlen (tempStr) >= 3) {
						strcpy (tok7, tempStr);
						success = true;
					} else {
						success = false;
					}
				}
				// 8�� (��ü ����) - ���� (3���� �̻�)
				else if (i == 8) {
					strcpy (tempStr, token);
					if (strlen (tempStr) >= 3) {
						strcpy (tok8, tempStr);
						success = true;
					} else {
						success = false;
					}
				}
				++i;
				token = strtok (NULL, "-");
			}

			// 8�ܰ���� ���������� �Ϸ�Ǹ� ����ü�� ����
			if (success == true) {
				// �Ϸ� ��ȣ�� ���� ���� ���ڸ� ���� ��ħ
				sprintf (constructionCode, "%s-%s", tok1, tok2);
				
				// 1,2�ܰ�. ���� ���� Ȯ��
				for (yy = 0 ; yy < layerInfo.code_name.size () ; ++yy) {
					if (strncmp (constructionCode, layerInfo.code_name [yy].c_str (), 4) == 0)
						layerInfo.code_state [yy] = true;
				}

				// 3�ܰ�. �� ����
				for (yy = 0 ; yy < layerInfo.dong_name.size () ; ++yy) {
					if (strncmp (tok3, layerInfo.dong_name [yy].c_str (), 4) == 0) {
						layerInfo.dong_state [yy] = true;
						layerInfo.bDongAllShow = true;
					}
				}

				// 4�ܰ�. �� ����
				for (yy = 0 ; yy < layerInfo.floor_name.size () ; ++yy) {
					if (strncmp (tok4, layerInfo.floor_name [yy].c_str (), 3) == 0) {
						layerInfo.floor_state [yy] = true;
					}
				}

				// 5�ܰ�. CJ ����
				for (yy = 0 ; yy < layerInfo.CJ_name.size () ; ++yy) {
					if (strncmp (tok5, layerInfo.CJ_name [yy].c_str (), 2) == 0) {
						layerInfo.CJ_state [yy] = true;
						layerInfo.bCJAllShow = true;
					}
				}

				// 6�ܰ�. CJ �� �ð�����
				for (yy = 0 ; yy < layerInfo.CJ_name.size () ; ++yy) {
					if (strncmp (tok6, layerInfo.orderInCJ_name [yy].c_str (), 2) == 0) {
						layerInfo.orderInCJ_state [yy] = true;
						layerInfo.bOrderInCJAllShow = true;
					}
				}

				// 7�ܰ�. ���� ����
				for (yy = 0 ; yy < layerInfo.obj_name.size () ; ++yy) {
					if ((strncmp (constructionCode, layerInfo.obj_cat [yy].c_str (), 4) == 0) && (strncmp (tok7, layerInfo.obj_name [yy].c_str (), 5) == 0)) {
						layerInfo.obj_state [yy] = true;
					}
				}

				// 8�ܰ�. ��ü ����
				for (yy = 0 ; yy < layerInfo.subObj_name.size () ; ++yy) {
					if ((strncmp (constructionCode, layerInfo.subObj_cat [yy].c_str (), 4) == 0) && (strncmp (tok8, layerInfo.subObj_name [yy].c_str (), 5) == 0)) {
						layerInfo.subObj_state [yy] = true;
					}
				}
			}
		}
		if (err == APIERR_DELETED)
			err = NoError;
	}

	// [���̾�α� �ڽ�] ���̾� ���� �����ϱ�
	result = DGBlankModalDialog (700, 450, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, layerShowHandler, 0);

	// OK ��ư�� �ƴϸ� �޸� �����ϰ� ����
	if (result != DG_OK) {
		deallocateMemory (&layerInfo);
		deallocateMemory (&selectedInfo);
		return	err;
	}

	// ��� ���̾� �����
	BNZeroMemory (&attrib, sizeof (API_Attribute));
	attrib.header.typeID = API_LayerID;
	
	for (xx = 1; xx <= nLayers ; ++xx) {
		attrib.header.index = xx;
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			//if ((attrib.layer.head.flags & APILay_Hidden) == false) {
				attrib.layer.head.flags |= APILay_Hidden;
				ACAPI_Attribute_Modify (&attrib, NULL);
			//}
		}
	}

	short	z;
	char	code1 [10][5];		// ���� �ڵ�
	short	LenCode1;
	char	code2 [1600][5];	// �� �ڵ�
	short	LenCode2;
	char	code3 [120][5];		// �� �ڵ�
	short	LenCode3;
	char	code4 [100][5];		// CJ �ڵ�
	short	LenCode4;
	char	code5 [100][5];		// CJ �� �ð����� �ڵ�
	short	LenCode5;
	char	code6 [90][5];		// ���� �ڵ�
	short	LenCode6;
	char	code7 [90][5];		// ��ü �ڵ�
	short	LenCode7;

	char	fullLayerName [40];
	short	x1, x2, x3, x4, x5, x6, x7, x8;
	bool	bCode2, bCode4, bCode5, bCode7;

	// 1. ���� �ڵ� ���ڿ� �����
	z = 0;
	for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx) {
		if (selectedInfo.code_state [xx] == true) {
			sprintf (tempStr, "%s", layerInfo.code_name [xx].c_str ());
			strcpy (code1 [z++], tempStr);
		}
	}
	LenCode1 = z;

	// 2. �� �ڵ� ���ڿ� �����
	z = 0;
	strcpy (code2 [z++], "????");
	for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx) {
		if (selectedInfo.dong_state [xx] == true) {
			sprintf (tempStr, "%s", layerInfo.dong_name [xx].c_str ());
			strcpy (code2 [z++], tempStr);
		}
	}
	LenCode2 = z;

	// 3. �� �ڵ� ���ڿ� �����
	z = 0;
	for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx) {
		if (selectedInfo.floor_state [xx] == true) {
			sprintf (tempStr, "%s", layerInfo.floor_name [xx].c_str ());
			strcpy (code3 [z++], tempStr);
		}
	}
	LenCode3 = z;

	// 4. CJ �ڵ� ���ڿ� �����
	z = 0;
	strcpy (code4 [z++], "??");
	for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx) {
		if (selectedInfo.CJ_state [xx] == true) {
			sprintf (tempStr, "%s", layerInfo.CJ_name [xx].c_str ());
			strcpy (code4 [z++], tempStr);
		}
	}
	LenCode4 = z;

	// 5. CJ �� �ð����� ���ڿ� �����
	z = 0;
	strcpy (code5 [z++], "??");
	for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx) {
		if (selectedInfo.orderInCJ_state [xx] == true) {
			sprintf (tempStr, "%s", layerInfo.orderInCJ_name [xx].c_str ());
			strcpy (code4 [z++], tempStr);
		}
	}
	LenCode5 = z;

	// 6. ���� �ڵ� ���ڿ� �����
	z = 0;
	for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
		if (selectedInfo.obj_state [xx] == true) {
			sprintf (tempStr, "%s", layerInfo.obj_name [xx].c_str ());
			strcpy (code6 [z++], tempStr);
		}
	}
	LenCode6 = z;

	// 7. ��ü �ڵ� ���ڿ� ����� (�����翡 ����)
	z = 0;
	strcpy (code7 [z++], "????");
	for (xx = 0 ; xx < layerInfo.subObj_name.size () ; ++xx) {
		if (selectedInfo.subObj_state [xx] == true) {
			sprintf (tempStr, "%s", layerInfo.subObj_name [xx].c_str ());
			strcpy (code7 [z++], tempStr);
		}
	}
	LenCode7 = z;

	bCode2 = false;	bCode4 = false;	bCode5 = false; bCode7 = false;

	// ���̾� �̸� �����ϱ�
	for (x1 = 0 ; x1 < LenCode1 ; ++x1) {
		for (x2 = 0 ; x2 < LenCode2 ; ++x2) {
			for (x3 = 0 ; x3 < LenCode3 ; ++x3) {
				for (x4 = 0 ; x4 < LenCode4 ; ++x4) {
					for (x5 = 0 ; x5 < LenCode5 ; ++x5) {
						for (x6 = 0 ; x6 < LenCode6 ; ++x6) {
							for (x7 = 0 ; x7 < LenCode7 ; ++x7) {
								for (x8 = 1 ; x8 <= 12 ; ++x8) {
									// bCode2 (�� �ڵ� ����), bCode4 (CJ �ڵ� ����), bCode5 (CJ �� �ð����� �ڵ� ����), bCode7 (���� ���� ��ü �ڵ� ����)
									if (x8 == 1) { bCode2 = false;	bCode4 = false;	bCode5 = false;	bCode7 = false;	}
									if (x8 == 2) { bCode2 = false;	bCode4 = true;	bCode5 = false;	bCode7 = false;	}
									if (x8 == 3) { bCode2 = false;	bCode4 = true;	bCode5 = true;	bCode7 = false;	}
									if (x8 == 4) { bCode2 = true;	bCode4 = false;	bCode5 = false;	bCode7 = false;	}
									if (x8 == 5) { bCode2 = true;	bCode4 = true;	bCode5 = false;	bCode7 = false;	}
									if (x8 == 6) { bCode2 = true;	bCode4 = true;	bCode5 = true;	bCode7 = false;	}

									if (x8 == 7) { bCode2 = false;	bCode4 = false;	bCode5 = false;	bCode7 = true;	}
									if (x8 == 8) { bCode2 = false;	bCode4 = true;	bCode5 = false;	bCode7 = true;	}
									if (x8 == 9) { bCode2 = false;	bCode4 = true;	bCode5 = true;	bCode7 = true;	}
									if (x8 == 10) { bCode2 = true;	bCode4 = false;	bCode5 = false;	bCode7 = true;	}
									if (x8 == 11) { bCode2 = true;	bCode4 = true;	bCode5 = false;	bCode7 = true;	}
									if (x8 == 12) { bCode2 = true;	bCode4 = true;	bCode5 = true;	bCode7 = true;	}

									// ���� ����
									strcpy (fullLayerName, "");
									strcpy (fullLayerName, code1 [x1]);

									// �� ����
									if ((LenCode2 > 1) && (bCode2 == true)) {
										strcat (fullLayerName, "-");
										strcat (fullLayerName, code2 [x2]);
									}

									// �� ����
									strcat (fullLayerName, "-");
									strcat (fullLayerName, code3 [x3]);

									// CJ ����
									if (bCode4 == true) {
										if (LenCode4 > 1) {
											strcat (fullLayerName, "-");
											strcat (fullLayerName, code4 [x4]);
										} else {
											strcat (fullLayerName, "-00");
										}
									}

									// CJ �� �ð�����
									if (bCode5 == true) {
										if (LenCode5 > 1) {
											strcat (fullLayerName, "-");
											strcat (fullLayerName, code5 [x5]);
										} else {
											strcat (fullLayerName, "-00");
										}
									}

									// ����
									strcat (fullLayerName, "-");
									strcat (fullLayerName, code6 [x6]);

									// ��ü
									if (bCode7 == true) {
										if (LenCode7 > 1) {
											strcat (fullLayerName, "-");
											strcat (fullLayerName, code7 [x7]);
										} else {
											// ������ ����
										}
									}

									// !!!
									//ACAPI_WriteReport (fullLayerName, true);

									// ������ ���̾� �̸� �˻��ϱ�
									BNZeroMemory (&attrib, sizeof (API_Attribute));
									attrib.header.typeID = API_LayerID;
									CHCopyC (fullLayerName, attrib.header.name);
									err = ACAPI_Attribute_Get (&attrib);

									// �ش� ���̾� �����ֱ�
									if ((attrib.layer.head.flags & APILay_Hidden) == true) {
										attrib.layer.head.flags ^= APILay_Hidden;
										ACAPI_Attribute_Modify (&attrib, NULL);
									}
								}
							}
						}
					}
				}
			}
		}
	}
	
	// ȭ�� ���ΰ�ħ
	ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	bool	regenerate = true;
	ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

	// �޸� ����
	deallocateMemory (&layerInfo);
	deallocateMemory (&selectedInfo);

	return err;
}

// [���̾�α� �ڽ�] ���̾� ���� �����ϱ�
short DGCALLBACK layerShowHandler (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;
	short	itmPosX, itmPosY;
	short	xx, yy;
	char	tempStr [20];
	short	dialogSizeX, dialogSizeY;

	GSErrCode err = NoError;
	API_ModulData  info;

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, "���̾� ���� �����ϱ�: LayerName Ex) 05-T-(0101)-9B1-(01)-(01)-COLU-(UFOM)");

			// Ȯ�� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 0, 20, 40, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "Ȯ��");
			DGShowItem (dialogID, DG_OK);

			// ��� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 0, 50, 40, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "���");
			DGShowItem (dialogID, DG_CANCEL);

			// �ε� ��ư
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 0, 80, 40, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "�ε�");
			DGShowItem (dialogID, itmIdx);
			BUTTON_LOAD = itmIdx;

			// ��: �ڵ� �����ֱ�
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 0, 120, 50, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "CODE");
			DGShowItem (dialogID, itmIdx);
			LABEL_CODE = itmIdx;

			// ��: ���� ����
			itmPosX = 40;
			itmPosY = 25;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "���� ����");
			DGShowItem (dialogID, itmIdx);

			// üũ�ڽ�: ���� ���� ��ư
			itmPosX = 150;
			itmPosY = 20;
			for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx) {
				if (layerInfo.code_state [xx] == true) {
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					sprintf (tempStr, "%s %s", layerInfo.code_name [xx].c_str (), layerInfo.code_desc [xx].c_str ());
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.code_idx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			// ��� ����
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
			DGSetItemText (dialogID, itmIdx, "��� ����");
			DGShowItem (dialogID, itmIdx);
			SELECTALL_1_CONTYPE = itmIdx;
			itmPosX += 100;
			if (itmPosX >= 600) {
				itmPosX = 150;
				itmPosY += 30;
			}

			itmPosY += 30;

			// ��: �� ����
			itmPosX = 40;
			itmPosY += 10;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "�� ����");
			DGShowItem (dialogID, itmIdx);

			// üũ�ڽ�: �� ���� ��ư
			itmPosX = 150;
			itmPosY -= 5;
			for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx) {
				if (layerInfo.dong_state [xx] == true) {
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, layerInfo.dong_desc [xx].c_str ());
					DGShowItem (dialogID, itmIdx);
					layerInfo.dong_idx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			// ��� ����
			if (layerInfo.bDongAllShow) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, "��� ����");
				DGShowItem (dialogID, itmIdx);
				SELECTALL_2_DONG = itmIdx;
				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}

			itmPosY += 30;

			// ��: �� ����
			itmPosX = 40;
			itmPosY += 10;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "�� ����");
			DGShowItem (dialogID, itmIdx);

			// üũ�ڽ�: �� ���� ��ư
			itmPosX = 150;
			itmPosY -= 5;
			for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx) {
				if (layerInfo.floor_state [xx] == true) {
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, layerInfo.floor_desc [xx].c_str ());
					DGShowItem (dialogID, itmIdx);
					layerInfo.floor_idx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			// ��� ����
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
			DGSetItemText (dialogID, itmIdx, "��� ����");
			DGShowItem (dialogID, itmIdx);
			SELECTALL_3_FLOOR = itmIdx;
			itmPosX += 100;
			if (itmPosX >= 600) {
				itmPosX = 150;
				itmPosY += 30;
			}

			itmPosY += 30;

			// ��: CJ ����
			itmPosX = 40;
			itmPosY += 10;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "CJ ����");
			DGShowItem (dialogID, itmIdx);

			// üũ�ڽ�: CJ ���� ��ư
			itmPosX = 150;
			itmPosY -= 5;
			for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx) {
				if (layerInfo.CJ_state [xx] == true) {
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, layerInfo.CJ_name [xx].c_str ());
					DGShowItem (dialogID, itmIdx);
					layerInfo.CJ_idx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			// ��� ����
			if (layerInfo.bCJAllShow) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, "��� ����");
				DGShowItem (dialogID, itmIdx);
				SELECTALL_4_CJ = itmIdx;
				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}

			itmPosY += 30;

			// ��: CJ �� �ð�����
			itmPosX = 40;
			itmPosY += 10;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "�ð�����");
			DGShowItem (dialogID, itmIdx);

			// üũ�ڽ�: CJ �� �ð����� ��ư
			itmPosX = 150;
			itmPosY -= 5;
			for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx) {
				if (layerInfo.orderInCJ_state [xx] == true) {
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, layerInfo.orderInCJ_name [xx].c_str ());
					DGShowItem (dialogID, itmIdx);
					layerInfo.orderInCJ_idx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			// ��� ����
			if (layerInfo.bOrderInCJAllShow) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, "��� ����");
				DGShowItem (dialogID, itmIdx);
				SELECTALL_5_ORDER = itmIdx;
				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}

			itmPosY += 30;

			// ��: ����(����)
			itmPosX = 40;
			itmPosY += 10;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "*����");
			DGShowItem (dialogID, itmIdx);

			// üũ�ڽ�: ����(����)
			itmPosX = 150;
			itmPosY -= 5;
			for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
				if (strncmp (layerInfo.obj_cat [xx].c_str (), "01-S", 4) == 0) {
					if (layerInfo.obj_state [xx] == true) {
						itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
						DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
						DGSetItemText (dialogID, itmIdx, layerInfo.obj_desc [xx].c_str ());
						DGShowItem (dialogID, itmIdx);
						layerInfo.obj_idx [xx] = itmIdx;

						itmPosX += 100;
						if (itmPosX >= 600) {
							itmPosX = 150;
							itmPosY += 30;
						}
					}
				}
			}

			itmPosY += 30;

			// ��: ����(���ึ��)
			itmPosX = 40;
			itmPosY += 10;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "*���ึ��");
			DGShowItem (dialogID, itmIdx);

			// üũ�ڽ�: ����(���ึ��)
			itmPosX = 150;
			itmPosY -= 5;
			for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
				if (strncmp (layerInfo.obj_cat [xx].c_str (), "02-A", 4) == 0) {
					if (layerInfo.obj_state [xx] == true) {
						itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
						DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
						DGSetItemText (dialogID, itmIdx, layerInfo.obj_desc [xx].c_str ());
						DGShowItem (dialogID, itmIdx);
						layerInfo.obj_idx [xx] = itmIdx;

						itmPosX += 100;
						if (itmPosX >= 600) {
							itmPosX = 150;
							itmPosY += 30;
						}
					}
				}
			}

			itmPosY += 30;

			// ��: ����(������)
			itmPosX = 40;
			itmPosY += 10;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "*������");
			DGShowItem (dialogID, itmIdx);

			// üũ�ڽ�: ����(������)
			itmPosX = 150;
			itmPosY -= 5;
			for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
				if (strncmp (layerInfo.obj_cat [xx].c_str (), "05-T", 4) == 0) {
					if (layerInfo.obj_state [xx] == true) {
						itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
						DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
						DGSetItemText (dialogID, itmIdx, layerInfo.obj_desc [xx].c_str ());
						DGShowItem (dialogID, itmIdx);
						layerInfo.obj_idx [xx] = itmIdx;

						itmPosX += 100;
						if (itmPosX >= 600) {
							itmPosX = 150;
							itmPosY += 30;
						}
					}
				}
			}

			itmPosY += 30;

			// ��: ����(���ü�)
			itmPosX = 40;
			itmPosY += 10;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "*���ü�");
			DGShowItem (dialogID, itmIdx);

			// üũ�ڽ�: ����(���ü�)
			itmPosX = 150;
			itmPosY -= 5;
			for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
				if (strncmp (layerInfo.obj_cat [xx].c_str (), "06-F", 4) == 0) {
					if (layerInfo.obj_state [xx] == true) {
						itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
						DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
						DGSetItemText (dialogID, itmIdx, layerInfo.obj_desc [xx].c_str ());
						DGShowItem (dialogID, itmIdx);
						layerInfo.obj_idx [xx] = itmIdx;

						itmPosX += 100;
						if (itmPosX >= 600) {
							itmPosX = 150;
							itmPosY += 30;
						}
					}
				}
			}

			itmPosY += 30;

			// ��: ��ü(������)
			itmPosX = 40;
			itmPosY += 10;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "**������(����)");
			DGShowItem (dialogID, itmIdx);

			// üũ�ڽ�: ��ü(������)
			itmPosX = 150;
			itmPosY -= 5;
			for (xx = 0 ; xx < layerInfo.subObj_name.size () ; ++xx) {
				for (yy = 0 ; yy < layerInfo.code_name.size () ; ++yy) {
					if ((strncmp (layerInfo.subObj_cat [xx].c_str (), "05-T", 4) == 0) && (strncmp (layerInfo.code_name [yy].c_str (), layerInfo.subObj_cat [xx].c_str (), 4) == 0) && (layerInfo.code_state [yy] == true)) {
						if (layerInfo.obj_state [xx] == true) {
							itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
							DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
							DGSetItemText (dialogID, itmIdx, layerInfo.subObj_desc [xx].c_str ());
							DGShowItem (dialogID, itmIdx);
							layerInfo.subObj_idx [xx] = itmIdx;

							itmPosX += 100;
							if (itmPosX >= 600) {
								itmPosX = 150;
								itmPosY += 30;
							}
						}
					}
				}
			}

			dialogSizeX = 700;
			dialogSizeY = itmPosY + 150;
			DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);

			break;
		
		case DG_MSG_CHANGE:
			changedBtnItemIdx = item;

			if (changedBtnItemIdx == SELECTALL_1_CONTYPE) {
				if (DGGetItemValLong (dialogID, SELECTALL_1_CONTYPE) == TRUE) {
					// ��� ����
					for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx)
						DGSetItemValLong (dialogID, layerInfo.code_idx [xx], TRUE);
				} else {
					// ��� ����
					for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx)
						DGSetItemValLong (dialogID, layerInfo.code_idx [xx], FALSE);
				}
			}
			if (changedBtnItemIdx == SELECTALL_2_DONG) {
				if (DGGetItemValLong (dialogID, SELECTALL_2_DONG) == TRUE) {
					// ��� ����
					for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx)
						DGSetItemValLong (dialogID, layerInfo.dong_idx [xx], TRUE);
				} else {
					// ��� ����
					for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx)
						DGSetItemValLong (dialogID, layerInfo.dong_idx [xx], FALSE);
				}
			}
			if (changedBtnItemIdx == SELECTALL_3_FLOOR) {
				if (DGGetItemValLong (dialogID, SELECTALL_3_FLOOR) == TRUE) {
					// ��� ����
					for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx)
						DGSetItemValLong (dialogID, layerInfo.floor_idx [xx], TRUE);
				} else {
					// ��� ����
					for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx)
						DGSetItemValLong (dialogID, layerInfo.floor_idx [xx], FALSE);
				}
			}
			if (changedBtnItemIdx == SELECTALL_4_CJ) {
				if (DGGetItemValLong (dialogID, SELECTALL_4_CJ) == TRUE) {
					// ��� ����
					for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx)
						DGSetItemValLong (dialogID, layerInfo.CJ_idx [xx], TRUE);
				} else {
					// ��� ����
					for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx)
						DGSetItemValLong (dialogID, layerInfo.CJ_idx [xx], FALSE);
				}
			}
			if (changedBtnItemIdx == SELECTALL_5_ORDER) {
				if (DGGetItemValLong (dialogID, SELECTALL_5_ORDER) == TRUE) {
					// ��� ����
					for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx)
						DGSetItemValLong (dialogID, layerInfo.orderInCJ_idx [xx], TRUE);
				} else {
					// ��� ����
					for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx)
						DGSetItemValLong (dialogID, layerInfo.orderInCJ_idx [xx], FALSE);
				}
			}

			// ���縦 �����ϸ� ���� �ڵ带 �ڵ� ����
			for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
				if (DGGetItemValLong (dialogID, layerInfo.obj_idx [xx]) == TRUE) {
					for (yy = 0 ; yy < layerInfo.code_name.size () ; ++yy) {
						if (strncmp (layerInfo.obj_cat [xx].c_str (), layerInfo.code_name [yy].c_str (), 4) == 0) {
							DGSetItemValLong (dialogID, layerInfo.code_idx [yy], TRUE);
						}
					}
				}
			}

			// ��ü�� �����ϸ� ���� �ڵ带 �ڵ� ����
			for (xx = 0 ; xx < layerInfo.subObj_name.size () ; ++xx) {
				if (DGGetItemValLong (dialogID, layerInfo.subObj_idx [xx]) == TRUE) {
					for (yy = 0 ; yy < layerInfo.code_name.size () ; ++yy) {
						if (strncmp (layerInfo.subObj_cat [xx].c_str (), layerInfo.code_name [yy].c_str (), 4) == 0) {
							DGSetItemValLong (dialogID, layerInfo.code_idx [yy], TRUE);
						}
					}
				}
			}

			// ��ư�� �̸� �����ֱ�
			for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx) {
				if ((layerInfo.code_idx [xx] == changedBtnItemIdx) && (layerInfo.code_state [xx] == true)) {
					sprintf (tempStr, "%s", layerInfo.code_name [xx].c_str ());
					DGSetItemText (dialogID, LABEL_CODE, tempStr);
				}
			}
			for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx) {
				if ((layerInfo.dong_idx [xx] == changedBtnItemIdx) && (layerInfo.dong_state [xx] == true)) {
					sprintf (tempStr, "%s", layerInfo.dong_name [xx].c_str ());
					DGSetItemText (dialogID, LABEL_CODE, tempStr);
				}
			}
			for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx) {
				if ((layerInfo.floor_idx [xx] == changedBtnItemIdx) && (layerInfo.floor_state [xx] == true)) {
					sprintf (tempStr, "%s", layerInfo.floor_name [xx].c_str ());
					DGSetItemText (dialogID, LABEL_CODE, tempStr);
				}
			}
			for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx) {
				if ((layerInfo.CJ_idx [xx] == changedBtnItemIdx) && (layerInfo.CJ_state [xx] == true)) {
					sprintf (tempStr, "%s", layerInfo.CJ_name [xx].c_str ());
					DGSetItemText (dialogID, LABEL_CODE, tempStr);
				}
			}
			for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx) {
				if ((layerInfo.orderInCJ_idx [xx] == changedBtnItemIdx) && (layerInfo.orderInCJ_state [xx] == true)) {
					sprintf (tempStr, "%s", layerInfo.orderInCJ_name [xx].c_str ());
					DGSetItemText (dialogID, LABEL_CODE, tempStr);
				}
			}
			for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
				if ((layerInfo.obj_idx [xx] == changedBtnItemIdx) && (layerInfo.obj_state [xx] == true)) {
					sprintf (tempStr, "%s", layerInfo.obj_name [xx].c_str ());
					DGSetItemText (dialogID, LABEL_CODE, tempStr);
				}
			}
			for (xx = 0 ; xx < layerInfo.subObj_name.size () ; ++xx) {
				if ((layerInfo.subObj_idx [xx] == changedBtnItemIdx) && (layerInfo.subObj_state [xx] == true)) {
					sprintf (tempStr, "%s", layerInfo.subObj_name [xx].c_str ());
					DGSetItemText (dialogID, LABEL_CODE, tempStr);
				}
			}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// ���� ����
					for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx)
						(DGGetItemValLong (dialogID, layerInfo.code_idx [xx]) == TRUE) ? selectedInfo.code_state [xx] = true : selectedInfo.code_state [xx] = false;

					// �� ����
					for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx)
						(DGGetItemValLong (dialogID, layerInfo.dong_idx [xx]) == TRUE) ? selectedInfo.dong_state [xx] = true : selectedInfo.dong_state [xx] = false;

					// �� ����
					for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx)
						(DGGetItemValLong (dialogID, layerInfo.floor_idx [xx]) == TRUE) ? selectedInfo.floor_state [xx] = true : selectedInfo.floor_state [xx] = false;

					// CJ
					for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx)
						(DGGetItemValLong (dialogID, layerInfo.CJ_idx [xx]) == TRUE) ? selectedInfo.CJ_state [xx] = true : selectedInfo.CJ_state [xx] = false;

					// CJ �� �ð�����
					for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx)
						(DGGetItemValLong (dialogID, layerInfo.orderInCJ_idx [xx]) == TRUE) ? selectedInfo.orderInCJ_state [xx] = true : selectedInfo.orderInCJ_state [xx] = false;

					// ����
					for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx)
						(DGGetItemValLong (dialogID, layerInfo.obj_idx [xx]) == TRUE) ? selectedInfo.obj_state [xx] = true : selectedInfo.obj_state [xx] = false;

					// ��ü
					for (xx = 0 ; xx < layerInfo.subObj_name.size () ; ++xx)
						(DGGetItemValLong (dialogID, layerInfo.subObj_idx [xx]) == TRUE) ? selectedInfo.subObj_state [xx] = true : selectedInfo.subObj_state [xx] = false;

					// ��ư ���� ����
					saveButtonStatus ();

					break;

				case DG_CANCEL:
					break;

				default:
					clickedBtnItemIdx = item;
					item = 0;

					// ����� ��ư ���¸� �ҷ���
					if (clickedBtnItemIdx == BUTTON_LOAD) {
						BNZeroMemory (&info, sizeof (API_ModulData));
						err = ACAPI_ModulData_Get (&info, "ButtonStatus");

						if (err == NoError && info.dataVersion == 1) {
							selectedInfoSaved = *(reinterpret_cast<StatusOfLayerNameSystem*> (*info.dataHdl));
							
							for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx) {
								if (selectedInfoSaved.code_state [xx] == true)
									DGSetItemValLong (dialogID, layerInfo.code_idx [xx], TRUE);
							}
							for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx) {
								if (selectedInfoSaved.dong_state [xx] == true)
									DGSetItemValLong (dialogID, layerInfo.dong_idx [xx], TRUE);
							}
							for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx) {
								if (selectedInfoSaved.floor_state [xx] == true)
									DGSetItemValLong (dialogID, layerInfo.floor_idx [xx], TRUE);
							}
							for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx) {
								if (selectedInfoSaved.CJ_state [xx] == true)
									DGSetItemValLong (dialogID, layerInfo.CJ_idx [xx], TRUE);
							}
							for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx) {
								if (selectedInfoSaved.orderInCJ_state [xx] == true)
									DGSetItemValLong (dialogID, layerInfo.orderInCJ_idx [xx], TRUE);
							}
							for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
								if (selectedInfoSaved.obj_state [xx] == true)
									DGSetItemValLong (dialogID, layerInfo.obj_idx [xx], TRUE);
							}
							for (xx = 0 ; xx < layerInfo.subObj_name.size () ; ++xx) {
								if (selectedInfoSaved.subObj_state [xx] == true)
									DGSetItemValLong (dialogID, layerInfo.subObj_idx [xx], TRUE);
							}
						}

						BMKillHandle (&info.dataHdl);
					}

					break;
			}
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

// �ֱ� ��ư ���� �����ϱ�
GSErrCode	saveButtonStatus (void)
{
	GSErrCode err = NoError;

	short	xx;
	API_ModulData	info;
	BNZeroMemory (&info, sizeof (API_ModulData));
	info.dataVersion = 1;
	info.platformSign = GS::Act_Platform_Sign;
	info.dataHdl = BMAllocateHandle (sizeof (StatusOfLayerNameSystem), 0, 0);
	if (info.dataHdl != NULL) {

		for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx)
			selectedInfoSaved.code_state [xx] = selectedInfo.code_state [xx];
		for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx)
			selectedInfoSaved.dong_state [xx] = selectedInfo.dong_state [xx];
		for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx)
			selectedInfoSaved.floor_state [xx] = selectedInfo.floor_state [xx];
		for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx)
			selectedInfoSaved.CJ_state [xx] = selectedInfo.CJ_state [xx];
		for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx)
			selectedInfoSaved.orderInCJ_state [xx] = selectedInfo.orderInCJ_state [xx];
		for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx)
			selectedInfoSaved.obj_state [xx] = selectedInfo.obj_state [xx];
		for (xx = 0 ; xx < layerInfo.subObj_name.size () ; ++xx)
			selectedInfoSaved.subObj_state [xx] = selectedInfo.subObj_state [xx];

		*(reinterpret_cast<StatusOfLayerNameSystem*> (*info.dataHdl)) = selectedInfoSaved;
		err = ACAPI_ModulData_Store (&info, "ButtonStatus");
		BMKillHandle (&info.dataHdl);
	} else {
		err = APIERR_MEMFULL;
	}

	return	err;
}

// ���̾� ���� �����
GSErrCode	makeLayersEasily (void)
{
	GSErrCode	err = NoError;

	FILE	*fp;			// ���� ������
	char	line [10240];	// ���Ͽ��� �о�� ���� �ϳ�
	string	insElem;		// ��ū�� string���� ��ȯ�ؼ� vector�� ����
	char	*token;			// �о�� ���ڿ��� ��ū
	short	lineCount;		// �о�� ���� ��
	short	tokCount;		// �о�� ��ū ����

	API_Attribute	attrib;
	short			xx, yy, i;
	short			nLayers;

	bool	success;
	char	tempStr [20];
	char	tok1 [5];
	char	tok2 [5];
	char	tok3 [5];
	char	tok4 [5];
	char	tok5 [5];
	char	tok6 [5];
	char	tok7 [5];
	char	tok8 [5];
	char	constructionCode [5];

	short	result;


	// ���̾� ���� ���� ��������
	fp = fopen ("C:\\layer.csv", "r");

	for (lineCount = 1 ; lineCount <= 14 ; ++lineCount) {
		if (fp != NULL) {
			tokCount = 0;
			fscanf (fp, "%s\n", line);
			token = strtok (line, ",");
			tokCount ++;
			while (token != NULL) {
				if (strlen (token) > 0) {
					if (tokCount > 1) {
						insElem = token;

						if (lineCount == 1)		layerInfo.code_name.push_back (insElem);		// ���籸��
						if (lineCount == 2)		layerInfo.code_desc.push_back (insElem);		// ���籸�� ����
						if (lineCount == 3) {
							// ���� �� ��ȣ ���ڰ� 3�ڸ��� ������ �տ� 0�� ���� ��
							if (strlen (token) == 3) {
								strcpy (tempStr, "0");
								strcat (tempStr, token);
								insElem = tempStr;
							}

							layerInfo.dong_name.push_back (insElem);							// ��
						}
						if (lineCount == 4)		layerInfo.dong_desc.push_back (insElem);		// �� ����
						if (lineCount == 5)		layerInfo.floor_name.push_back (insElem);		// ��
						if (lineCount == 6)		layerInfo.floor_desc.push_back (insElem);		// �� ����
						if (lineCount == 7) {
							// ���� CJ ��ȣ ���ڰ� 1�ڸ��� ������ �տ� 0�� ���� ��
							if (strlen (token) == 1) {
								strcpy (tempStr, "0");
								strcat (tempStr, token);
								insElem = tempStr;
							}

							layerInfo.CJ_name.push_back (insElem);			// CJ
						}
						if (lineCount == 8) {
							// ���� CJ �� �ð����� ��ȣ ���ڰ� 1�ڸ��� ������ �տ� 0�� ���� ��
							if (strlen (token) == 1) {
								strcpy (tempStr, "0");
								strcat (tempStr, token);
								insElem = tempStr;
							}

							layerInfo.orderInCJ_name.push_back (insElem);	// CJ �� �ð�����
						}
						if (lineCount == 9)		layerInfo.obj_name.push_back (insElem);			// ����
						if (lineCount == 10)	layerInfo.obj_desc.push_back (insElem);			// ���� ����
						if (lineCount == 11)	layerInfo.obj_cat.push_back (insElem);			// ���簡 ���� ī�װ�(���籸��)
						if (lineCount == 12)	layerInfo.subObj_name.push_back (insElem);		// ��ü
						if (lineCount == 13)	layerInfo.subObj_desc.push_back (insElem);		// ��ü ����
						if (lineCount == 14)	layerInfo.subObj_cat.push_back (insElem);		// ��ü�� ���� ī�װ�(���籸��)
					}
				}
				token = strtok (NULL, ",");
				tokCount ++;
			}
		} else {
			ACAPI_WriteReport ("layer.csv ������ C:\\�� �����Ͻʽÿ�.", true);
			return	err;
		}
	}

	fclose (fp);

	// ����ü �ʱ�ȭ
	allocateMemory (&layerInfo);

	/*
	// ���̾�α׿� ����Ʈ �ڽ��� �߰��Ͽ� ����ü ���� ������ ǥ���� (���� ���� ����)

	// DG_OK ��ư�� ������ ����Ʈ �ڽ��� ���� ���¸� state ������ �����ϰ�
	// ���̾�α� �ڽ� ���� �Ŀ� state ���¿� ���� ���̾� �̸��� ������ �� �װ��� �����.
	
	// �����ϸ� ������ ���̾� ������ �����ְ�, �����ص� �޽��� ����� ��
	*/

	// [���̾�α� �ڽ�] ���̾� ���� �����
	result = DGBlankModalDialog (900, 120, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, layerMakeHandler, 0);

	// OK ��ư�� �ƴϸ� �޸� �����ϰ� ����
	if (result != DG_OK) {
		deallocateMemory (&layerInfo);
		return	err;
	}

	//// ��� ���̾� �����
	//BNZeroMemory (&attrib, sizeof (API_Attribute));
	//attrib.header.typeID = API_LayerID;
	//
	//for (xx = 1; xx <= nLayers ; ++xx) {
	//	attrib.header.index = xx;
	//	err = ACAPI_Attribute_Get (&attrib);
	//	if (err == NoError) {
	//		//if ((attrib.layer.head.flags & APILay_Hidden) == false) {
	//			attrib.layer.head.flags |= APILay_Hidden;
	//			ACAPI_Attribute_Modify (&attrib, NULL);
	//		//}
	//	}
	//}

	//short	z;
	//char	code1 [10][5];		// ���� �ڵ�
	//short	LenCode1;
	//char	code2 [1600][5];	// �� �ڵ�
	//short	LenCode2;
	//char	code3 [120][5];		// �� �ڵ�
	//short	LenCode3;
	//char	code4 [100][5];		// CJ �ڵ�
	//short	LenCode4;
	//char	code5 [100][5];		// CJ �� �ð����� �ڵ�
	//short	LenCode5;
	//char	code6 [90][5];		// ���� �ڵ�
	//short	LenCode6;
	//char	code7 [90][5];		// ��ü �ڵ�
	//short	LenCode7;

	//char	fullLayerName [40];
	//short	x1, x2, x3, x4, x5, x6, x7, x8;
	//bool	bCode2, bCode4, bCode5, bCode7;

	//// 1. ���� �ڵ� ���ڿ� �����
	//z = 0;
	//for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx) {
	//	if (selectedInfo.code_state [xx] == true) {
	//		sprintf (tempStr, "%s", layerInfo.code_name [xx].c_str ());
	//		strcpy (code1 [z++], tempStr);
	//	}
	//}
	//LenCode1 = z;

	//// 2. �� �ڵ� ���ڿ� �����
	//z = 0;
	//strcpy (code2 [z++], "????");
	//for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx) {
	//	if (selectedInfo.dong_state [xx] == true) {
	//		sprintf (tempStr, "%s", layerInfo.dong_name [xx].c_str ());
	//		strcpy (code2 [z++], tempStr);
	//	}
	//}
	//LenCode2 = z;

	//// 3. �� �ڵ� ���ڿ� �����
	//z = 0;
	//for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx) {
	//	if (selectedInfo.floor_state [xx] == true) {
	//		sprintf (tempStr, "%s", layerInfo.floor_name [xx].c_str ());
	//		strcpy (code3 [z++], tempStr);
	//	}
	//}
	//LenCode3 = z;

	//// 4. CJ �ڵ� ���ڿ� �����
	//z = 0;
	//strcpy (code4 [z++], "??");
	//for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx) {
	//	if (selectedInfo.CJ_state [xx] == true) {
	//		sprintf (tempStr, "%s", layerInfo.CJ_name [xx].c_str ());
	//		strcpy (code4 [z++], tempStr);
	//	}
	//}
	//LenCode4 = z;

	//// 5. CJ �� �ð����� ���ڿ� �����
	//z = 0;
	//strcpy (code5 [z++], "??");
	//for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx) {
	//	if (selectedInfo.orderInCJ_state [xx] == true) {
	//		sprintf (tempStr, "%s", layerInfo.orderInCJ_name [xx].c_str ());
	//		strcpy (code4 [z++], tempStr);
	//	}
	//}
	//LenCode5 = z;

	//// 6. ���� �ڵ� ���ڿ� �����
	//z = 0;
	//for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
	//	if (selectedInfo.obj_state [xx] == true) {
	//		sprintf (tempStr, "%s", layerInfo.obj_name [xx].c_str ());
	//		strcpy (code6 [z++], tempStr);
	//	}
	//}
	//LenCode6 = z;

	//// 7. ��ü �ڵ� ���ڿ� ����� (�����翡 ����)
	//z = 0;
	//strcpy (code7 [z++], "????");
	//for (xx = 0 ; xx < layerInfo.subObj_name.size () ; ++xx) {
	//	if (selectedInfo.subObj_state [xx] == true) {
	//		sprintf (tempStr, "%s", layerInfo.subObj_name [xx].c_str ());
	//		strcpy (code7 [z++], tempStr);
	//	}
	//}
	//LenCode7 = z;

	//bCode2 = false;	bCode4 = false;	bCode5 = false; bCode7 = false;

	//// ���̾� �̸� �����ϱ�
	//for (x1 = 0 ; x1 < LenCode1 ; ++x1) {
	//	for (x2 = 0 ; x2 < LenCode2 ; ++x2) {
	//		for (x3 = 0 ; x3 < LenCode3 ; ++x3) {
	//			for (x4 = 0 ; x4 < LenCode4 ; ++x4) {
	//				for (x5 = 0 ; x5 < LenCode5 ; ++x5) {
	//					for (x6 = 0 ; x6 < LenCode6 ; ++x6) {
	//						for (x7 = 0 ; x7 < LenCode7 ; ++x7) {
	//							for (x8 = 1 ; x8 <= 12 ; ++x8) {
	//								// bCode2 (�� �ڵ� ����), bCode4 (CJ �ڵ� ����), bCode5 (CJ �� �ð����� �ڵ� ����), bCode7 (���� ���� ��ü �ڵ� ����)
	//								if (x8 == 1) { bCode2 = false;	bCode4 = false;	bCode5 = false;	bCode7 = false;	}
	//								if (x8 == 2) { bCode2 = false;	bCode4 = true;	bCode5 = false;	bCode7 = false;	}
	//								if (x8 == 3) { bCode2 = false;	bCode4 = true;	bCode5 = true;	bCode7 = false;	}
	//								if (x8 == 4) { bCode2 = true;	bCode4 = false;	bCode5 = false;	bCode7 = false;	}
	//								if (x8 == 5) { bCode2 = true;	bCode4 = true;	bCode5 = false;	bCode7 = false;	}
	//								if (x8 == 6) { bCode2 = true;	bCode4 = true;	bCode5 = true;	bCode7 = false;	}

	//								if (x8 == 7) { bCode2 = false;	bCode4 = false;	bCode5 = false;	bCode7 = true;	}
	//								if (x8 == 8) { bCode2 = false;	bCode4 = true;	bCode5 = false;	bCode7 = true;	}
	//								if (x8 == 9) { bCode2 = false;	bCode4 = true;	bCode5 = true;	bCode7 = true;	}
	//								if (x8 == 10) { bCode2 = true;	bCode4 = false;	bCode5 = false;	bCode7 = true;	}
	//								if (x8 == 11) { bCode2 = true;	bCode4 = true;	bCode5 = false;	bCode7 = true;	}
	//								if (x8 == 12) { bCode2 = true;	bCode4 = true;	bCode5 = true;	bCode7 = true;	}

	//								// ���� ����
	//								strcpy (fullLayerName, "");
	//								strcpy (fullLayerName, code1 [x1]);

	//								// �� ����
	//								if ((LenCode2 > 1) && (bCode2 == true)) {
	//									strcat (fullLayerName, "-");
	//									strcat (fullLayerName, code2 [x2]);
	//								}

	//								// �� ����
	//								strcat (fullLayerName, "-");
	//								strcat (fullLayerName, code3 [x3]);

	//								// CJ ����
	//								if (bCode4 == true) {
	//									if (LenCode4 > 1) {
	//										strcat (fullLayerName, "-");
	//										strcat (fullLayerName, code4 [x4]);
	//									} else {
	//										strcat (fullLayerName, "-00");
	//									}
	//								}

	//								// CJ �� �ð�����
	//								if (bCode5 == true) {
	//									if (LenCode5 > 1) {
	//										strcat (fullLayerName, "-");
	//										strcat (fullLayerName, code5 [x5]);
	//									} else {
	//										strcat (fullLayerName, "-00");
	//									}
	//								}

	//								// ����
	//								strcat (fullLayerName, "-");
	//								strcat (fullLayerName, code6 [x6]);

	//								// ��ü
	//								if (bCode7 == true) {
	//									if (LenCode7 > 1) {
	//										strcat (fullLayerName, "-");
	//										strcat (fullLayerName, code7 [x7]);
	//									} else {
	//										// ������ ����
	//									}
	//								}

	//								// !!!
	//								//ACAPI_WriteReport (fullLayerName, true);

	//								// ������ ���̾� �̸� �˻��ϱ�
	//								BNZeroMemory (&attrib, sizeof (API_Attribute));
	//								attrib.header.typeID = API_LayerID;
	//								CHCopyC (fullLayerName, attrib.header.name);
	//								err = ACAPI_Attribute_Get (&attrib);

	//								// �ش� ���̾� �����ֱ�
	//								if ((attrib.layer.head.flags & APILay_Hidden) == true) {
	//									attrib.layer.head.flags ^= APILay_Hidden;
	//									ACAPI_Attribute_Modify (&attrib, NULL);
	//								}
	//							}
	//						}
	//					}
	//				}
	//			}
	//		}
	//	}
	//}
	//
	//// ȭ�� ���ΰ�ħ
	//ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	//bool	regenerate = true;
	//ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

	//// �޸� ����
	//deallocateMemory (&layerInfo);
	//deallocateMemory (&selectedInfo);

	return	err;
}

// [���̾�α� �ڽ�] ���̾� ���� �����
short DGCALLBACK layerMakeHandler (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;
	short	itmPosX, itmPosY;
	short	dialogSizeX, dialogSizeY;
	short	xx;
	short	anyTrue;

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, "���̾� ���� �����: LayerName Ex) 05-T-(0101)-9B1-(01)-(01)-COLU-(UFOM)");

			// Ȯ�� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 820, 20, 60, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "����");
			DGShowItem (dialogID, DG_OK);

			// ��� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 820, 55, 60, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "���");
			DGShowItem (dialogID, DG_CANCEL);

			// ��ư: ���� ����
			itmPosX = 30;
			itmPosY = 50;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
			DGSetItemFont (dialogID, BUTTON_CODE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_CODE, "���� ����");
			DGShowItem (dialogID, BUTTON_CODE);

			// ��ư: �� ����
			itmPosX += 110;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
			DGSetItemFont (dialogID, BUTTON_DONG, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_DONG, "�� ����");
			DGShowItem (dialogID, BUTTON_DONG);

			// ��ư: �� ����
			itmPosX += 110;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
			DGSetItemFont (dialogID, BUTTON_FLOOR, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_FLOOR, "�� ����");
			DGShowItem (dialogID, BUTTON_FLOOR);

			// ��ư: CJ
			itmPosX += 110;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
			DGSetItemFont (dialogID, BUTTON_CJ, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_CJ, "CJ");
			DGShowItem (dialogID, BUTTON_CJ);

			// ��ư: �ð�����
			itmPosX += 110;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
			DGSetItemFont (dialogID, BUTTON_ORDER, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_ORDER, "�ð�����");
			DGShowItem (dialogID, BUTTON_ORDER);

			// ��ư: ����
			itmPosX += 110;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
			DGSetItemFont (dialogID, BUTTON_OBJ, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_OBJ, "����");
			DGShowItem (dialogID, BUTTON_OBJ);

			// ��ư: ��ü
			itmPosX += 110;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
			DGSetItemFont (dialogID, BUTTON_SUBOBJ, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_SUBOBJ, "��ü");
			DGShowItem (dialogID, BUTTON_SUBOBJ);

			// ������
			itmPosX = 115;
			itmPosY = 60;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 20, 1);
			DGShowItem (dialogID, SEPARATOR_1);

			itmPosX += 110;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 20, 1);
			DGShowItem (dialogID, SEPARATOR_2);

			itmPosX += 110;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 20, 1);
			DGShowItem (dialogID, SEPARATOR_3);

			itmPosX += 110;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 20, 1);
			DGShowItem (dialogID, SEPARATOR_4);

			itmPosX += 110;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 20, 1);
			DGShowItem (dialogID, SEPARATOR_5);

			itmPosX += 110;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 20, 1);
			DGShowItem (dialogID, SEPARATOR_6);

			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 800, 5, 1, 110);
			DGShowItem (dialogID, SEPARATOR_7);

			// üũ�ڽ� ǥ��
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 160, 25, 40, 20);
			DGSetItemFont (dialogID, CHECKBOX_DONG, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, CHECKBOX_DONG, "����");
			DGShowItem (dialogID, CHECKBOX_DONG);
			DGSetItemValLong (dialogID, CHECKBOX_DONG, TRUE);

			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 380, 25, 40, 20);
			DGSetItemFont (dialogID, CHECKBOX_CJ, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, CHECKBOX_CJ, "����");
			DGShowItem (dialogID, CHECKBOX_CJ);
			DGSetItemValLong (dialogID, CHECKBOX_CJ, TRUE);

			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 490, 25, 40, 20);
			DGSetItemFont (dialogID, CHECKBOX_ORDER, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, CHECKBOX_ORDER, "����");
			DGShowItem (dialogID, CHECKBOX_ORDER);
			DGSetItemValLong (dialogID, CHECKBOX_ORDER, TRUE);

			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 710, 25, 40, 20);
			DGSetItemFont (dialogID, CHECKBOX_SUBOBJ, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, CHECKBOX_SUBOBJ, "����");
			DGShowItem (dialogID, CHECKBOX_SUBOBJ);
			DGSetItemValLong (dialogID, CHECKBOX_SUBOBJ, TRUE);

			break;
		
		case DG_MSG_CHANGE:
			switch (item) {
				case CHECKBOX_DONG:
				case CHECKBOX_CJ:
				case CHECKBOX_ORDER:
				case CHECKBOX_SUBOBJ:

					if (DGGetItemValLong (dialogID, CHECKBOX_DONG) == TRUE) {
						DGEnableItem (dialogID, BUTTON_DONG);
					} else {
						DGDisableItem (dialogID, BUTTON_DONG);
					}

					if (DGGetItemValLong (dialogID, CHECKBOX_CJ) == TRUE) {
						DGEnableItem (dialogID, BUTTON_CJ);
						DGEnableItem (dialogID, CHECKBOX_ORDER);
					} else {
						DGDisableItem (dialogID, BUTTON_CJ);
						DGDisableItem (dialogID, CHECKBOX_ORDER);
					}

					if ((DGGetItemValLong (dialogID, CHECKBOX_CJ) == TRUE) && (DGGetItemValLong (dialogID, CHECKBOX_ORDER) == TRUE)) {
						DGEnableItem (dialogID, BUTTON_ORDER);
					} else {
						DGDisableItem (dialogID, BUTTON_ORDER);
					}

					if (DGGetItemValLong (dialogID, CHECKBOX_SUBOBJ) == TRUE) {
						DGEnableItem (dialogID, BUTTON_SUBOBJ);
					} else {
						DGDisableItem (dialogID, BUTTON_SUBOBJ);
					}

					break;
			}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					break;

				case DG_CANCEL:
					break;

				default:
					clickedBtnItemIdx = item;
					item = 0;	// �ٸ� ��ư�� ������ �� ���̾�αװ� ������ �ʰ� ��

					dialogSizeX = 600;
					dialogSizeY = 500;

					// ���� â ũ��
					if (clickedBtnItemIdx == BUTTON_CODE) {
						dialogSizeX = 600;
						dialogSizeY = 90;
					} else if (clickedBtnItemIdx == BUTTON_DONG) {
					} else if (clickedBtnItemIdx == BUTTON_FLOOR) {
						dialogSizeX = 1000;
						dialogSizeY = 550;
					} else if (clickedBtnItemIdx == BUTTON_CJ) {
						dialogSizeX = 500;
						dialogSizeY = 350;
					} else if (clickedBtnItemIdx == BUTTON_ORDER) {
						dialogSizeX = 500;
						dialogSizeY = 350;
					} else if (clickedBtnItemIdx == BUTTON_OBJ) {
						// ...
					} else if (clickedBtnItemIdx == BUTTON_SUBOBJ) {
						// ...
					}

					// [���̾�α� �ڽ�] ���̾� ���� ����� 2��
					result = DGBlankModalDialog (dialogSizeX, dialogSizeY, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, layerMakeHandler_2, 0);

					// ��ư�� �۲� ���� (���� ����)
					anyTrue = 0;
					for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx)
						if (layerInfo.code_state [xx] == true)	anyTrue++;
					if (anyTrue > 0)
						DGSetItemFont (dialogID, BUTTON_CODE, DG_IS_LARGE | DG_IS_BOLD);
					else
						DGSetItemFont (dialogID, BUTTON_CODE, DG_IS_LARGE | DG_IS_PLAIN);

					// ��ư�� �۲� ���� (�� ����)
					// ...

					// ��ư�� �۲� ���� (�� ����)
					anyTrue = 0;
					for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx)
						if (layerInfo.floor_state [xx] == true)	anyTrue++;
					if (anyTrue > 0)
						DGSetItemFont (dialogID, BUTTON_FLOOR, DG_IS_LARGE | DG_IS_BOLD);
					else
						DGSetItemFont (dialogID, BUTTON_FLOOR, DG_IS_LARGE | DG_IS_PLAIN);

					// ��ư�� �۲� ���� (CJ)
					anyTrue = 0;
					for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx)
						if (layerInfo.CJ_state [xx] == true)	anyTrue++;
					if (anyTrue > 0)
						DGSetItemFont (dialogID, BUTTON_CJ, DG_IS_LARGE | DG_IS_BOLD);
					else
						DGSetItemFont (dialogID, BUTTON_CJ, DG_IS_LARGE | DG_IS_PLAIN);

					// ��ư�� �۲� ���� (�ð�����)
					anyTrue = 0;
					for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx)
						if (layerInfo.orderInCJ_state [xx] == true)	anyTrue++;
					if (anyTrue > 0)
						DGSetItemFont (dialogID, BUTTON_ORDER, DG_IS_LARGE | DG_IS_BOLD);
					else
						DGSetItemFont (dialogID, BUTTON_ORDER, DG_IS_LARGE | DG_IS_PLAIN);

					// ��ư�� �۲� ���� (����)
					// ...

					// ��ư�� �۲� ���� (��ü)
					// ...

					break;
			}
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

// [���̾�α� �ڽ�] ���̾� ���� ����� 2��
short DGCALLBACK layerMakeHandler_2 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;
	short	itmPosX, itmPosY;
	short	xx;
	char	tempStr [30];

	switch (message) {
		case DG_MSG_INIT:

			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, "���� ����");

			// Ȯ�� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 10, 60, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "Ȯ��");
			DGShowItem (dialogID, DG_OK);

			// ��� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 45, 60, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "���");
			DGShowItem (dialogID, DG_CANCEL);

			if (clickedBtnItemIdx == BUTTON_CODE) {
				itmPosX = 90;
				itmPosY = 10;
				for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx) {
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					sprintf (tempStr, "%s %s", layerInfo.code_name [xx].c_str (), layerInfo.code_desc [xx].c_str ());
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					(layerInfo.code_state [xx] == true) ? DGSetItemValLong (dialogID, itmIdx, TRUE) : DGSetItemValLong (dialogID, itmIdx, FALSE);

					itmPosX += 100;
					if (itmPosX >= 500) {
						itmPosX = 90;
						itmPosY += 30;
					}
				}
			} else if (clickedBtnItemIdx == BUTTON_DONG) {
				// ...
			} else if (clickedBtnItemIdx == BUTTON_FLOOR) {
				itmPosX = 90;
				itmPosY = 10;
				for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx) {
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 35);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					sprintf (tempStr, "%s\n%s", layerInfo.floor_name [xx].c_str (), layerInfo.floor_desc [xx].c_str ());
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					(layerInfo.floor_state [xx] == true) ? DGSetItemValLong (dialogID, itmIdx, TRUE) : DGSetItemValLong (dialogID, itmIdx, FALSE);

					itmPosX += 100;
					if (itmPosX >= 950) {
						itmPosX = 90;
						itmPosY += 40;
					}
				}
			} else if (clickedBtnItemIdx == BUTTON_CJ) {
				itmPosX = 90;
				itmPosY = 10;
				for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx) {
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 35, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					sprintf (tempStr, "%s", layerInfo.CJ_name [xx].c_str ());
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					(layerInfo.CJ_state [xx] == true) ? DGSetItemValLong (dialogID, itmIdx, TRUE) : DGSetItemValLong (dialogID, itmIdx, FALSE);

					itmPosX += 40;
					if (itmPosX >= 480) {
						itmPosX = 90;
						itmPosY += 30;
					}
				}
			} else if (clickedBtnItemIdx == BUTTON_ORDER) {
				itmPosX = 90;
				itmPosY = 10;
				for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx) {
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 35, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					sprintf (tempStr, "%s", layerInfo.orderInCJ_name [xx].c_str ());
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					(layerInfo.orderInCJ_state [xx] == true) ? DGSetItemValLong (dialogID, itmIdx, TRUE) : DGSetItemValLong (dialogID, itmIdx, FALSE);

					itmPosX += 40;
					if (itmPosX >= 480) {
						itmPosX = 90;
						itmPosY += 30;
					}
				}
			} else if (clickedBtnItemIdx == BUTTON_OBJ) {
				// ...
			} else if (clickedBtnItemIdx == BUTTON_SUBOBJ) {
				// ...
			}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:

					if (clickedBtnItemIdx == BUTTON_CODE) {
						for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx) {
							if (DGGetItemValLong (dialogID, 3 + xx) == TRUE)
								layerInfo.code_state [xx] = true;
							else
								layerInfo.code_state [xx] = false;
						}
					} else if (clickedBtnItemIdx == BUTTON_DONG) {
						// ...
					} else if (clickedBtnItemIdx == BUTTON_FLOOR) {
						for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx) {
							if (DGGetItemValLong (dialogID, 3 + xx) == TRUE)
								layerInfo.floor_state [xx] = true;
							else
								layerInfo.floor_state [xx] = false;
						}
					} else if (clickedBtnItemIdx == BUTTON_CJ) {
						for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx) {
							if (DGGetItemValLong (dialogID, 3 + xx) == TRUE)
								layerInfo.CJ_state [xx] = true;
							else
								layerInfo.CJ_state [xx] = false;
						}
					} else if (clickedBtnItemIdx == BUTTON_ORDER) {
						for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx) {
							if (DGGetItemValLong (dialogID, 3 + xx) == TRUE)
								layerInfo.orderInCJ_state [xx] = true;
							else
								layerInfo.orderInCJ_state [xx] = false;
						}
					} else if (clickedBtnItemIdx == BUTTON_OBJ) {
						// ...
					} else if (clickedBtnItemIdx == BUTTON_SUBOBJ) {
						// ...
					}

					break;

				case DG_CANCEL:
					break;
			}
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

// ���̾� ���� �����ϱ�
GSErrCode	assignLayerEasily (void)
{
	GSErrCode	err = NoError;

	// ������ ��ü�� �ִ��� üũ (������ ��� â�� ���� return)

	// layer.csv ���� ������ ����ü�� �ε� (state, idx ������ ���� �Ҵ����� ����)
	// ���̾�α׿� ����Ʈ �ڽ��� �߰��Ͽ� ����ü ���� ������ ǥ���� (���� ���� �Ұ�)

	// DG_OK ��ư�� ������ ������ ���̾ �ִ��� Ȯ���� �� �����ϸ� ���̾� �Ӽ� ���� �� ���� �޽��� ���, �����ص� �޽��� ����� ��

	return	err;
}
