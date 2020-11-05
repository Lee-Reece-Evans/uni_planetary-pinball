#include "stdafx.h"
#include "MyGame.h"

CMyGame game;
CGameApp app;

int main(int argc, char* argv[])
{
	app.SetFPS(125);
	app.OpenWindow(800, 900, "PLANETARY PINBALL");
	//app.OpenFullScreen(800, 600, 24);
	app.SetClearColor(CColor::Black());
	app.Run(&game);
	return(0);
}
