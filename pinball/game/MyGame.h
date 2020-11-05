#pragma once

class CMyGame : public CGame
{
public:
	CMyGame(void);
	~CMyGame(void);

	float k; // spring (stiffness)
	float r; // radius of ball
	float dBounce;	// decrease bounce speed
	float pBounce; // perfect bounce
	float iBounce; // increase bounce speed
	float fBounce; // fast bounce
	float globGravity; // storing the value of gravity so it is globally avaiable
	float coinGrabX, coinGrabY;

	int score;

	bool springReleased;
	bool springEngaged;
	bool ballExited;
	bool wallCreated;

	bool coinMove; // used for menu

	// Sprites
	// display menu
	CSprite menu;
	CSprite coin;
	CSprite slot;
	// game mode
	CSprite back;			// background objects
	CSprite pinback;			// background black
	CSprite leftPaddle;
	CSprite leftTopPaddle;
	CSprite rightPaddle;
	CSprite rightTopPaddle;
	CSprite redPlanet;
	CSprite bluePlanet;
	CSprite spring;
	CSprite ball;
	CSprite music;

	// lists
	CSpriteList theWalls;	// wall bounces
	CSpriteList theBalls; // cicle bounces
	CSpriteList lives; // number of balls
	CSpriteList bonusPoints; // bonus points item randomly spawning
	CSpriteList collectPoints; // collecting 500points
	CSpriteList explosions; // when ball collides with planet

	// sounds
	CSoundPlayer speedHit; // green triangle
	CSoundPlayer triangleHit; // triangle object
	CSoundPlayer circleHit; // circle objects
	CSoundPlayer stdHit; // stand surface hits
	CSoundPlayer bonusHit; // bonus points item
	CSoundPlayer pointsHit; // 500 pts sound
	CSoundPlayer flipperSound;
	CSoundPlayer coinSound;
	CSoundPlayer explosionSound;
	CSoundPlayer musicSound;

	void gravityControl();
	void springControl();
	void paddlesControl();
	void wallsControl();
	void ballsControl();
	void livesControl();
	void bonusPointsControl();
	void collectedPointsControl();

	// Per-Frame Callback Funtions (must be implemented!)
	virtual void OnUpdate();
	virtual void OnDraw(CGraphics* g);

	// Game Life Cycle
	virtual void OnInitialize();
	virtual void OnDisplayMenu();
	virtual void OnStartGame();
	virtual void OnGameOver();
	virtual void OnTerminate();

	// Keyboard Event Handlers
	virtual void OnKeyDown(SDLKey sym, SDLMod mod, Uint16 unicode);
	virtual void OnKeyUp(SDLKey sym, SDLMod mod, Uint16 unicode);

	// Mouse Events Handlers
	virtual void OnMouseMove(Uint16 x,Uint16 y,Sint16 relx,Sint16 rely,bool bLeft,bool bRight,bool bMiddle);
	virtual void OnLButtonDown(Uint16 x,Uint16 y);
	virtual void OnLButtonUp(Uint16 x,Uint16 y);
	virtual void OnRButtonDown(Uint16 x,Uint16 y);
	virtual void OnRButtonUp(Uint16 x,Uint16 y);
	virtual void OnMButtonDown(Uint16 x,Uint16 y);
	virtual void OnMButtonUp(Uint16 x,Uint16 y);
};
