#include <sstream>
#include <Windows.h>
#include "../Rainmeter/RainmeterAPI.h"
#include "./InternalSimpleCmd.h"

enum PlayerType {
	PTYPE_TITLE,
	PTYPE_DURATION,
	PTYPE_POSITION,
	PTYPE_PROGRESS,
	PTYPE_STATE,
	PTYPE_STATUS,
	PTYPE_VOLUME,
	PTYPE_UNKNOWN
};

struct Measure {
	PlayerType playerType;

	Measure() :
		playerType(PTYPE_UNKNOWN) {}
};

DWORD lastCheckTime = 0;
HWND playerHwnd = NULL;

int state = 0;
double duration = 0, position = 0;

bool checkPlayer() {
	DWORD time = GetTickCount64();

	if (time - lastCheckTime < 5000) {
		return playerHwnd != NULL;
	}

	playerHwnd = FindWindow(L"PotPlayer", NULL);
	if (!playerHwnd) {
		playerHwnd = FindWindow(L"PotPlayer64", NULL);
	}
	lastCheckTime = time;

	return playerHwnd != NULL;
}

void setPlayerType(Measure* measure, LPCWSTR type) {
	if (_wcsicmp(type, L"Title") == 0) {
		measure->playerType = PTYPE_TITLE;
	}
	else if (_wcsicmp(type, L"Duration") == 0) {
		measure->playerType = PTYPE_DURATION;
	}
	else if (_wcsicmp(type, L"Position") == 0) {
		measure->playerType = PTYPE_POSITION;
	}
	else if (_wcsicmp(type, L"Progress") == 0) {
		measure->playerType = PTYPE_PROGRESS;
	}
	else if (_wcsicmp(type, L"State") == 0) {
		measure->playerType = PTYPE_STATE;
	}
	else if (_wcsicmp(type, L"Status") == 0) {
		measure->playerType = PTYPE_STATUS;
	}
	else if (_wcsicmp(type, L"Volume") == 0) {
		measure->playerType = PTYPE_VOLUME;
	}
	else {
		measure->playerType = PTYPE_UNKNOWN;
	}
}

LPCWSTR formatTime(int ms) {
	int durationMin = ms / 60;
	int durationSec = ms % 60;

	WCHAR str[16];
	std::swprintf(str, sizeof(str) / sizeof(*str), L"%d:%02d", durationMin, durationSec);

	return str;
}

PLUGIN_EXPORT void Initialize(void** data, void* rm) {
	Measure* measure = new Measure;
	*data = measure;

	setPlayerType(measure, RmReadString(rm, L"PlayerType", L"State"));
}

PLUGIN_EXPORT void Reload(void* data, void* rm, double* maxValue) {
	Measure* measure = (Measure*)data;
	
	setPlayerType(measure, RmReadString(rm, L"PlayerType", L"State"));

	switch (measure->playerType) {
		case PTYPE_PROGRESS: *maxValue = 100.0;
	}
}

PLUGIN_EXPORT double Update(void* data) {
	Measure* measure = (Measure*)data;
	if (!checkPlayer()) return 0;

	switch (measure->playerType) {
		case PTYPE_DURATION:
		{
			duration = SendMessage(playerHwnd, POT_COMMAND, POT_GET_TOTAL_TIME, 0);
			return duration / 1000;
		}
			
		case PTYPE_POSITION:
		{
			position = SendMessage(playerHwnd, POT_COMMAND, POT_GET_CURRENT_TIME, 0);
			return position / 1000;
		}

		case PTYPE_PROGRESS:
		{
			duration = SendMessage(playerHwnd, POT_COMMAND, POT_GET_TOTAL_TIME, 0);
			position = SendMessage(playerHwnd, POT_COMMAND, POT_GET_CURRENT_TIME, 0);

			if (duration == 0)
				return 0;

			return position / duration * 100;
		}

		case PTYPE_STATE:
		{
			int stateTemp = SendMessage(playerHwnd, POT_COMMAND, POT_GET_PLAY_STATUS, 0);
			state = 0;

			switch (stateTemp) {
				case 0: 
					state = 0;
					break;

				case 1:
					state = 2;
					break;

				case 2:
					state = 1;
					break;
			}

			return state;
		}
		
		case PTYPE_VOLUME:
			return SendMessage(playerHwnd, POT_COMMAND, POT_GET_VOLUME, 0);

		case PTYPE_STATUS:
			// If it is turned off, always returns 0
			return 1;
	}

	return 0;
}

PLUGIN_EXPORT LPCWSTR GetString(void* data) {
	Measure* measure = (Measure*) data;
	checkPlayer();

	switch (measure->playerType) {
		case PTYPE_TITLE :
		{
			if (!playerHwnd) return L"";

			WCHAR buffer[512];
			GetWindowTextW(playerHwnd, buffer, 512);

			// 제목 뒤에 붙는 " - 팟플레이어" 제거하기
			std::wstring titleStr(buffer);
			std::wstring::size_type delimPos = titleStr.find_last_of(L"-");
			
			if (delimPos == std::wstring::npos)
				return titleStr.c_str();

			return titleStr.substr(0, delimPos - 1).c_str();
		}

		case PTYPE_STATE:
			WCHAR buffer[4];
			_itow_s(state, buffer, 4);

			return buffer;
		
		case PTYPE_DURATION :
			if (!playerHwnd) return L"0:00";

			return formatTime(duration / 1000);
		
		case PTYPE_POSITION:
			if (!playerHwnd) return L"0:00";

			return formatTime(position / 1000);
	}

	return L"";
}

PLUGIN_EXPORT void ExecuteBang(void* data, LPCWSTR args)
{
	if (!checkPlayer()) return;

	std::wstring argsStr(args);
	std::wstring::size_type delimPos = argsStr.find_first_of(L" ");

	LPCWSTR bangName;
	int leftArgs = 0;
	bool isRelative = false;
	
	if (delimPos != std::wstring::npos) {
		bangName = argsStr.substr(0, delimPos).c_str();
		std::wstring leftArgsStr = argsStr.substr(delimPos + 1);
		isRelative = leftArgsStr[0] == L'+' || leftArgsStr[1] == L'-';
		leftArgs = std::stoi(leftArgsStr);
	}
	else {
		bangName = args;
	}

	if (_wcsicmp(bangName, L"Previous") == 0) {
		PostMessage(playerHwnd, POT_COMMAND, POT_SET_PLAY_ORDER, 0);
	}
	else if (_wcsicmp(bangName, L"PlayPause") == 0) {
		int status = SendMessage(playerHwnd, POT_COMMAND, POT_GET_PLAY_STATUS, 0);

		switch (status) {
			case 0:
				PostMessage(playerHwnd, POT_COMMAND, POT_SET_PLAY_STATUS, 2);
				break;

			case 1:
				PostMessage(playerHwnd, POT_COMMAND, POT_SET_PLAY_STATUS, 2);
				break;

			case 2:
				PostMessage(playerHwnd, POT_COMMAND, POT_SET_PLAY_STATUS, 1);
				break;
		}
	}
	else if (_wcsicmp(bangName, L"Play") == 0) {
		PostMessage(playerHwnd, POT_COMMAND, POT_SET_PLAY_STATUS, 2);
	}
	else if (_wcsicmp(bangName, L"Pause") == 0) {
		PostMessage(playerHwnd, POT_COMMAND, POT_SET_PLAY_STATUS, 1);
	}
	else if (_wcsicmp(bangName, L"Stop") == 0) {
		PostMessage(playerHwnd, POT_COMMAND, POT_SET_PLAY_STATUS, 0);
	}
	else if (_wcsicmp(bangName, L"Next") == 0) {
		PostMessage(playerHwnd, POT_COMMAND, POT_SET_PLAY_ORDER, 1);
	}
	else if (_wcsicmp(bangName, L"SetPosition") == 0) {
		int total = SendMessage(playerHwnd, POT_COMMAND, POT_GET_TOTAL_TIME, 0);
		int current = SendMessage(playerHwnd, POT_COMMAND, POT_GET_CURRENT_TIME, 0);
		double percentage = current / total;

		if (isRelative) {
			percentage += ((double) leftArgs) / 100;
		}
		else {
			percentage = ((double) leftArgs) / 100;
		}

		percentage = min(0, max(percentage, 1));
		PostMessage(playerHwnd, POT_COMMAND, POT_SET_CURRENT_TIME, percentage * total);
	}
	else if (_wcsicmp(bangName, L"SetVolume") == 0) {
		int volume = SendMessage(playerHwnd, POT_COMMAND, POT_GET_VOLUME, 0);

		if (isRelative) {
			volume += leftArgs;
		}
		else {
			volume = leftArgs;
		}

		volume = min(0, max(volume, 100));
		PostMessage(playerHwnd, POT_COMMAND, POT_SET_VOLUME, volume);
	}
}

PLUGIN_EXPORT void Finalize(void* data) {
	Measure* measure = (Measure*)data;
	delete measure;
}
