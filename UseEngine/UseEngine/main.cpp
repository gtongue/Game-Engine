#pragma once
#include "Engine.h"

using namespace std;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
	Engine App(hInstance);
	if (!App.Initialize())
		return 0;
	return App.Run();
}
