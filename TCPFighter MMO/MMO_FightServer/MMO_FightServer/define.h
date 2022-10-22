#pragma once

//-----------------------------------------------------------------
// 기본 정보
//-----------------------------------------------------------------
#define dfSERVER_NAME			L"FightServer"
#define dfSERVER_PORT			20000
#define dfFRAME					25
#define dfINTERVAL				1000 / dfFRAME
#define dfMAX_USER				10000
#define dfCHARACTER_HP			100

//-----------------------------------------------------------------
// 캐릭터 이동 속도				// 25 fps 기준 이동속도
//-----------------------------------------------------------------
#define dfSPEED_PLAYER_X		6	// 3   50fps
#define dfSPEED_PLAYER_Y		4	// 2   50fps

//-----------------------------------------------------------------
// 30초 이상이 되도록 아무런 메시지 수신도 없는경우 접속 끊음.
//-----------------------------------------------------------------
#define dfNETWORK_PACKET_RECV_TIMEOUT	30000

//-----------------------------------------------------------------
// 화면 이동영역
//-----------------------------------------------------------------
#define dfRANGE_MOVE_TOP		0
#define dfRANGE_MOVE_LEFT		0
#define dfRANGE_MOVE_RIGHT		6400
#define dfRANGE_MOVE_BOTTOM		6400

//-----------------------------------------------------------------
// 이동 오류체크 범위
//-----------------------------------------------------------------
#define dfERROR_RANGE			50

//-----------------------------------------------------------------
// 공격 범위
//-----------------------------------------------------------------
#define dfATTACK1_RANGE_X		80
#define dfATTACK2_RANGE_X		90
#define dfATTACK3_RANGE_X		100
#define dfATTACK1_RANGE_Y		10
#define dfATTACK2_RANGE_Y		10
#define dfATTACK3_RANGE_Y		20

//-----------------------------------------------------------------
// 공격 데미지
//-----------------------------------------------------------------
#define dfATTACK1_DAMAGE		1
#define dfATTACK2_DAMAGE		2
#define dfATTACK3_DAMAGE		3

//-----------------------------------------------------------------
// 월드캡 캐릭터 섹터 범위
//-----------------------------------------------------------------
#define dfSECTOR_SIZE_X			200
#define dfSECTOR_SIZE_Y			200
#define dfSECTOR_MAX_X			(dfRANGE_MOVE_RIGHT / dfSECTOR_SIZE_X) + 1
#define dfSECTOR_MAX_Y			(dfRANGE_MOVE_BOTTOM / dfSECTOR_SIZE_Y) + 1
