#pragma once

// 플레이어 명령
//
// int Volume = Send(Post)Message(hWnd, POT_COMMAND, POT_GET_VOLUME, 0);
//
// Send(Post)Message(hWnd, POT_COMMAND, POT_SET_VOLUME, Volume);

#define POT_COMMAND				WM_USER
#define POT_GET_VOLUME			0x5000 // 0 ~ 100
#define POT_SET_VOLUME			0x5001 // 0 ~ 100
#define POT_GET_TOTAL_TIME		0x5002 // ms unit
#define POT_GET_PROGRESS_TIME	0x5003 // ms unit
#define POT_GET_CURRENT_TIME	0x5004 // ms unit
#define POT_SET_CURRENT_TIME	0x5005 // ms unit
#define POT_GET_PLAY_STATUS		0x5006 // 0:Stopped, 1:Paused, 2:Running
#define POT_SET_PLAY_STATUS		0x5007 // 0:Toggle, 1:Paused, 2:Running
#define POT_SET_PLAY_ORDER		0x5008 // 0:Prev, 1:Next
#define POT_SET_PLAY_CLOSE		0x5009
#define POT_SEND_VIRTUAL_KEY	0x5010 // Virtual Key(VK_UP, VK_DOWN....)

#define POT_GET_AVISYNTH_USE		0x6000
#define POT_SET_AVISYNTH_USE		0x6001 // 0: off, 1:on
#define POT_GET_VAPOURSYNTH_USE		0x6010
#define POT_SET_VAPOURSYNTH_USE		0x6011 // 0: off, 1:on
#define POT_GET_VIDEO_WIDTH			0x6030 
#define POT_GET_VIDEO_HEIGHT		0x6031
#define POT_GET_VIDEO_FPS			0x6032 // scale by 1000

// String getting
// Send(Post)Message(hWnd, POT_COMMAND, POT_GET_XXXXX, (WPARAM)ReceiveHWND);
// then PotPlayer call SendMessage(ReceiveHWND, WM_COPY_DATA, string(utf8) data...
// COPYDATASTRUCT::dwData is POT_GET_XXXXX
#define POT_GET_AVISYNTH_SCRIPT		0x6002
#define POT_GET_VAPOURSYNTH_SCRIPT	0x6012
#define POT_GET_PLAYFILE_NAME		0x6020

// String setting... Using WM_COPYDATA
// COPYDATASTRUCT cds = { 0, };
// cds.dwData = POT_SET_xxxxxxxx;
// cds.cbData = urf8.GetLength();
// cds.lpData = (void *)(LPCSTR)urf8;
// SendMessage(hWnd, WM_COPYDATA, hwnd, (WPARAM)&cds); 
#define POT_SET_AVISYNTH_SCRIPT		0x6003
#define POT_SET_VAPOURSYNTH_SCRIPT	0x6013
#define POT_SET_SHOW_MESSAGE		0x6040
