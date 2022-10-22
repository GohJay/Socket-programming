#pragma once

//-----------------------------------------------------------------
// �⺻ ����
//-----------------------------------------------------------------
#define dfSERVER_NAME			L"FightServer"
#define dfSERVER_PORT			20000
#define dfFRAME					25
#define dfINTERVAL				1000 / dfFRAME
#define dfMAX_USER				10000
#define dfCHARACTER_HP			100

//-----------------------------------------------------------------
// ĳ���� �̵� �ӵ�				// 25 fps ���� �̵��ӵ�
//-----------------------------------------------------------------
#define dfSPEED_PLAYER_X		6	// 3   50fps
#define dfSPEED_PLAYER_Y		4	// 2   50fps

//-----------------------------------------------------------------
// 30�� �̻��� �ǵ��� �ƹ��� �޽��� ���ŵ� ���°�� ���� ����.
//-----------------------------------------------------------------
#define dfNETWORK_PACKET_RECV_TIMEOUT	30000

//-----------------------------------------------------------------
// ȭ�� �̵�����
//-----------------------------------------------------------------
#define dfRANGE_MOVE_TOP		0
#define dfRANGE_MOVE_LEFT		0
#define dfRANGE_MOVE_RIGHT		6400
#define dfRANGE_MOVE_BOTTOM		6400

//-----------------------------------------------------------------
// �̵� ����üũ ����
//-----------------------------------------------------------------
#define dfERROR_RANGE			50

//-----------------------------------------------------------------
// ���� ����
//-----------------------------------------------------------------
#define dfATTACK1_RANGE_X		80
#define dfATTACK2_RANGE_X		90
#define dfATTACK3_RANGE_X		100
#define dfATTACK1_RANGE_Y		10
#define dfATTACK2_RANGE_Y		10
#define dfATTACK3_RANGE_Y		20

//-----------------------------------------------------------------
// ���� ������
//-----------------------------------------------------------------
#define dfATTACK1_DAMAGE		1
#define dfATTACK2_DAMAGE		2
#define dfATTACK3_DAMAGE		3

//-----------------------------------------------------------------
// ����ĸ ĳ���� ���� ����
//-----------------------------------------------------------------
#define dfSECTOR_SIZE_X			200
#define dfSECTOR_SIZE_Y			200
#define dfSECTOR_MAX_X			(dfRANGE_MOVE_RIGHT / dfSECTOR_SIZE_X) + 1
#define dfSECTOR_MAX_Y			(dfRANGE_MOVE_BOTTOM / dfSECTOR_SIZE_Y) + 1
