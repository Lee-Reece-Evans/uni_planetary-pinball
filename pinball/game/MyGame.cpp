#include "stdafx.h"
#include "MyGame.h"
#include <fstream>

CMyGame::CMyGame(void) :
	menu(400, 450, "menu.png", 0),
	coin(600, 100, "coin.bmp", CColor::White(), 0),
	slot(400, 219, "slot.bmp", 0),
	back(400, 450, "pinballnoback.png", 0),
	pinback(400, 450, "pinback.bmp", 0),
	ball(700, 150, "ball.bmp", CColor::White(), 0),
	redPlanet(235, 1030, "redplanet.bmp", CColor::Black(), 0),
	bluePlanet(235, -130, "blueplanet.bmp", CColor::Black(), 0),
	spring(490, 150, "spring.png", 0),
	leftPaddle(190, 133, "paddleleft.bmp", CColor::White(), 0),
	leftTopPaddle(190, 767, "paddletopleft.bmp", CColor::White(), 0),
	rightPaddle(285, 133, "paddleright.bmp", CColor::White(), 0),
	rightTopPaddle(285, 767, "paddletopright.bmp", CColor::White(), 0),
	music(650,875, "music.bmp", 0)
{
	// spring values
	spring.SetPivot(490, 0);
	k = 4; 

	// ballweight
	ball.SetMass(150);

	// paddle pivot points
	leftPaddle.SetPivotFromCenter(-23, 0);
	leftTopPaddle.SetPivotFromCenter(-23, 0);
	rightPaddle.SetPivotFromCenter(+23, 0);
	rightTopPaddle.SetPivotFromCenter(+23, 0);

	// planet angular velocity
	redPlanet.SetOmega(2);
	redPlanet.SetSize(400, 400);
	bluePlanet.SetOmega(2);
	bluePlanet.SetSize(400, 400);
	
	r = 13.0f;			// radius of the ball
	dBounce = 0.8f;	// decrease bounce speed
	pBounce = 1.0f; // perfect bounce
	iBounce = 1.2f; // increase bounce speed
	fBounce = 1.5f; // fast bounce

	// music alternative image
	music.LoadImage("music.bmp", "ismusic");
	music.LoadImage("musicoff.bmp", "nomusic");
}

CMyGame::~CMyGame(void)
{
}

/////////////////////////////////////////////////////
// Per-Frame Callback Funtions (must be implemented!)

void CMyGame::OnUpdate()
{
	if (IsMenuMode() || IsGameOver()) return;

	// ball spin
	if (globGravity < 0) // blue planet
		ball.SetOmega(ball.GetXVelocity());
	else // red planet
		ball.SetOmega(-ball.GetXVelocity());

	// keep speed managable
	if (ball.GetSpeed() > 450) ball.SetSpeed(450);

	// Time
	long t = GetTime();			// current time (in milliseconds)

	redPlanet.Update(t);
	bluePlanet.Update(t);

	for (CSprite* explosion : explosions) explosion->Update(t);
	for (CSprite* bonus : bonusPoints) bonus->Update(t);

	gravityControl();
	springControl();
	paddlesControl();
	leftPaddle.Update(t);
	leftTopPaddle.Update(t);
	rightPaddle.Update(t);
	rightTopPaddle.Update(t);
	wallsControl();
	ballsControl();
	ball.Update(t);
	livesControl();
	bonusPointsControl();
	collectedPointsControl();
}

void CMyGame::gravityControl()
{
	// Time variables:
	long t = GetTime();			// current time (in milliseconds)
	long dt = GetDeltaTime();	// time since the last frame (in milliseconds)

	if (!ball.HitTest(&spring)) // not touching spring - apply gravity
	{
		// calculate net force
		float mass = ball.GetMass();	// mass of the ball
		float rblue = -200 - ball.GetY();	// distance from the centre of the blue planet
		float rred = 1100 - ball.GetY();	// distance from the centre of the red planet
		float gblue = 60000 * mass / (rblue * rblue);	// blue gravitation
		float gred = 54000 * mass / (rred * rred);		// red gravitation
		float force = -gblue + gred; // pull / push force

		// calculate acceleration
		float acc = force / mass;
		// for correct ball spin direction (used in onUpdate)
		globGravity = acc * dt;
		// apply velocity
		ball.SetVelocity(ball.GetXVelocity(), ball.GetYVelocity() + acc * dt);
	}
}

void CMyGame::springControl()
{
	if (ball.HitTest(&spring) && !springReleased) // spring not in use
	{
		ball.SetVelocity(0, 0);
		if (IsKeyDown(SDLK_DOWN) && ball.GetY() > 50) // spring being used
		{
			ball.Move(0, -1);
			springEngaged = true;
		}
	}

	if (!ballExited && ball.GetYVelocity() < 0) // ball does not have enough power to leave area
	{
		springReleased = false;
	}
	else if (ballExited && !wallCreated) // ball exited
	{
		// reset spring
		springReleased = false;
		// create wall to stop ball going back in
		wallCreated = true;

		// create wall to restrict ball re-entering
		CSprite* wall = new CSpriteRect(CRectangle(485, 507, 10, 60), CColor::Yellow(), GetTime());
		wall->SetRotation(45);
		wall->SetStatus(6); // this will be deleted and created up to 3 times per game
		theWalls.push_back(wall);
	}

	// animate the spring
	if (ball.HitTest(&spring) && ball.GetY() < 157)
		spring.SetSize(30, ball.GetY() - 7);
	else 
		if (spring.GetHeight() < 149) spring.SetSize(30, spring.GetHeight() + 7);
}

void CMyGame::paddlesControl()
{
	// Time variables:
	long t = GetTime();			// current time (in milliseconds)
	long dt = GetDeltaTime();	// time since the last frame (in milliseconds)

	// left side paddles
	if (IsKeyDown(SDLK_a))
	{
		// bottom left paddle
		if (leftPaddle.GetRotation() > -40) leftPaddle.SetOmega(-500);
		else leftPaddle.SetOmega(0);
		// top left padddle
		if (leftTopPaddle.GetRotation() < 40) leftTopPaddle.SetOmega(500);
		else leftTopPaddle.SetOmega(0);
	}
	else if (!IsKeyDown(SDLK_a))
	{
		// bottom left paddle
		if (leftPaddle.GetRotation() < 20) leftPaddle.SetOmega(500);
		else { leftPaddle.SetOmega(0); leftPaddle.SetRotation(20); }
		// top left paddle
		if (leftTopPaddle.GetRotation() > -20) leftTopPaddle.SetOmega(-500);
		else { leftTopPaddle.SetOmega(0); leftTopPaddle.SetRotation(-20); }
	}

	// right side paddles
	if (IsKeyDown(SDLK_d))
	{
		// bottom right paddle
		if (rightPaddle.GetRotation() < 40) rightPaddle.SetOmega(500);
		else rightPaddle.SetOmega(0);
		// top right paddle
		if (rightTopPaddle.GetRotation() > -40) rightTopPaddle.SetOmega(-500);
		else rightTopPaddle.SetOmega(0);
	}
	else if (!IsKeyDown(SDLK_d))
	{
		// bottom right paddle
		if (rightPaddle.GetRotation() > -20) rightPaddle.SetOmega(-500);
		else { rightPaddle.SetOmega(0); rightPaddle.SetRotation(-20); }
		// top right paddle
		if (rightTopPaddle.GetRotation() < 20) rightTopPaddle.SetOmega(500);
		else { rightTopPaddle.SetOmega(0); rightTopPaddle.SetRotation(20); }
	}

	// paddles hittest
	// left bottom paddle
	if (ball.HitTest(&leftPaddle))
	{
		float a = DEG2RAD(leftPaddle.GetRotation());
		CVector n = CVector(sin(a), cos(a));
		CVector v = ball.GetVelocity() * dt / 1000;

		float w = abs(DEG2RAD(leftPaddle.GetOmega()));
		float r = Length(ball.GetPos() - leftPaddle.GetPos());

		if (Dot(v, n) < 0)
		{
			ball.SetVelocity(dBounce * Reflect(ball.GetVelocity(), n));
			ball.Accelerate((w * r * n) * 1.3f); // multiplied to compensate for higher gravity from blue planet
		}

	}
	// left top paddle
	if (ball.HitTest(&leftTopPaddle))
	{
		float a = DEG2RAD(leftTopPaddle.GetRotation());
		CVector n = CVector(-sin(a), -cos(a));
		CVector v = ball.GetVelocity() * dt / 1000;

		float w = abs(DEG2RAD(leftTopPaddle.GetOmega()));
		float r = Length(ball.GetPos() - leftTopPaddle.GetPos());

		if (Dot(v, n) < 0)
		{
			ball.SetVelocity(dBounce * Reflect(ball.GetVelocity(), n));
			ball.Accelerate(w * r * n);
		}
	}
	// right bottom paddle
	if (ball.HitTest(&rightPaddle))
	{
		float a = DEG2RAD(rightPaddle.GetRotation());
		CVector n = CVector(sin(a), cos(a));
		CVector v = ball.GetVelocity() * dt / 1000;

		float w = abs(DEG2RAD(rightPaddle.GetOmega()));
		float r = Length(ball.GetPos() - rightPaddle.GetPos());

		if (Dot(v, n) < 0)
		{
			ball.SetVelocity(dBounce * Reflect(ball.GetVelocity(), n));
			ball.Accelerate((w * r * n) * 1.3f); // multiplied to compensate for higher gravity from blue planet
		}
	}
	// right top paddle
	if (ball.HitTest(&rightTopPaddle))
	{
		float a = DEG2RAD(rightTopPaddle.GetRotation());
		CVector n = CVector(-sin(a), -cos(a));
		CVector v = ball.GetVelocity() * dt / 1000;

		float w = abs(DEG2RAD(rightTopPaddle.GetOmega()));
		float r = Length(ball.GetPos() - rightTopPaddle.GetPos());

		if (Dot(v, n) < 0)
		{
			ball.SetVelocity(dBounce * Reflect(ball.GetVelocity(), n));
			ball.Accelerate(w * r * n);
		}
	}
}

void CMyGame::wallsControl()
{
	// Time variables:
	long t = GetTime();			// current time (in milliseconds)
	long dt = GetDeltaTime();	// time since the last frame (in milliseconds)

	for (CSprite* wall : theWalls)
	{
		float halfWallWidth = wall->GetWidth() / 2;
		float halfWallHeight = wall->GetHeight() / 2;

		float alpha = DEG2RAD(wall->GetRotation());

		CVector v = ball.GetVelocity() * dt / 1000;
		CVector t = wall->GetPosition() - ball.GetPosition();

		// top side
		CVector n = CVector(sin(alpha), cos(alpha));
		if (Dot(v, n) < 0)
		{
			// perpendicular component (oncoming)
			float vy = Dot(v, n);
			CVector d = t + (halfWallHeight + r) * n;
			float dy = Dot(d, n);
			float f1 = dy / vy;

			// parrallel component (breadth control)
			float vx = Cross(v, n);
			float tx = Cross(t, n);
			float f2 = (tx - vx * f1) / (halfWallWidth + r);

			if (f1 >= 0 && f1 <= 1 && f2 >= -1 && f2 <= 1)
			{
				if (wall->GetStatus() == 1) // ball exit wall
				{
					ball.SetVelocity(pBounce * Reflect(ball.GetVelocity(), n));
					if (!ballExited) ballExited = true;
					stdHit.Play("stdhit.wav");
				}
				else if (wall->GetStatus() == 4) // green triangle wall
				{
					ball.SetVelocity(fBounce * Reflect(ball.GetVelocity(), n));
					speedHit.Play("speedhit.wav");
					score += 175;
				}
				else if (wall->GetStatus() == 5) // blue and redish triangles
				{
					ball.SetVelocity(iBounce * Reflect(ball.GetVelocity(), n));
					triangleHit.Play("trianglehit.wav");
					score += 150;
				}
				else
				{
					ball.SetVelocity(dBounce * Reflect(ball.GetVelocity(), n));
					stdHit.Play("stdhit.wav");
				}
			}
		}
		// bottom side
		n = CVector(-sin(alpha), -cos(alpha));
		if (Dot(v, n) < 0)
		{
			// perpendicular component (oncoming)
			float vy = Dot(v, n);
			CVector d = t + (halfWallHeight + r) * n;
			float dy = Dot(d, n);
			float f1 = dy / vy;

			// parrallel component (breadth control)
			float vx = Cross(v, n);
			float tx = Cross(t, n);
			float f2 = (tx - vx * f1) / (halfWallWidth + r);

			if (f1 >= 0 && f1 <= 1 && f2 >= -1 && f2 <= 1)
			{
				if (wall->GetStatus() == 1) // ball exit wall
				{
					ball.SetVelocity(pBounce * Reflect(ball.GetVelocity(), n));
					if (!ballExited) ballExited = true;
					stdHit.Play("stdhit.wav");
				}
				else if (wall->GetStatus() == 4) // green triangle wall
				{
					ball.SetVelocity(fBounce * Reflect(ball.GetVelocity(), n));
					speedHit.Play("speedhit.wav");
					score += 175;
				}
				else if (wall->GetStatus() == 5) // blue and redish triangles
				{
					ball.SetVelocity(iBounce * Reflect(ball.GetVelocity(), n));
					triangleHit.Play("trianglehit.wav");
					score += 150;
				}
				else
				{
					ball.SetVelocity(dBounce * Reflect(ball.GetVelocity(), n));
					stdHit.Play("stdhit.wav");
				}
			}
		}
		// right side
		n = CVector(cos(alpha), -sin(alpha));
		if (Dot(v, n) < 0)
		{
			// perpendicular component (oncoming)
			float vy = Dot(v, n);
			CVector d = t + ((halfWallWidth + r)  * n);
			float dy = Dot(d, n);
			float f1 = dy / vy;

			// parrallel component (breadth control)
			float vx = Cross(v, n);
			float tx = Cross(t, n);
			float f2 = (tx + vx * f1) / (halfWallHeight + r);

			if (f1 >= 0 && f1 <= 1 && f2 >= -1 && f2 <= 1)
			{
				if (wall->GetStatus() == 1)
				{
					ball.SetVelocity(pBounce * Reflect(ball.GetVelocity(), n));
					if (!ballExited) ballExited = true;
					stdHit.Play("stdhit.wav");
				}
				else if (wall->GetStatus() == 2) // bounce or roll down ramp
				{
					if (ball.GetYVelocity() < -150 || ball.GetYVelocity() > 150) // bounce
					{
						stdHit.Play("stdhit.wav");
						ball.SetVelocity(dBounce * Reflect(ball.GetVelocity(), n));
					}
					else if (ball.GetXVelocity() < -150) // bounce
					{
						stdHit.Play("stdhit.wav");
						ball.SetVelocity(dBounce * Reflect(ball.GetVelocity(), n));
					}
					else // roll
					{
						ball.SetYVelocity(0); // stop gravity pulling through object
						if (ball.GetXVelocity() > 75) ball.SetXVelocity(75); // limit speed to allow gravity to keep ball on ramp
						else if (ball.GetXVelocity() < 0) ball.SetXVelocity(5); // initalise speed in right direction
						else ball.Accelerate(10, 0); // progressively roll
					}
				}
				else if (wall->GetStatus() == 4) // green triangle wall
				{
					ball.SetVelocity(fBounce * Reflect(ball.GetVelocity(), n));
					speedHit.Play("speedhit.wav");
					score += 175;
				}
				else if (wall->GetStatus() == 5) // blue and redish triangles
				{
					ball.SetVelocity(iBounce * Reflect(ball.GetVelocity(), n));
					triangleHit.Play("trianglehit.wav");
					score += 150;
				}
				else
				{
					ball.SetVelocity(dBounce * Reflect(ball.GetVelocity(), n));
					stdHit.Play("stdhit.wav");
				}
			}
		}
		// left side
		n = CVector(-cos(alpha), sin(alpha));
		if (Dot(v, n) < 0)
		{
			// perpendicular component (oncoming)
			float vy = Dot(v, n);
			CVector d = t + ((halfWallWidth + r)  * n);
			float dy = Dot(d, n);
			float f1 = dy / vy;

			// parrallel component (breadth control)
			float vx = Cross(v, n);
			float tx = Cross(t, n);
			float f2 = (tx + vx * f1) / (halfWallHeight + r);

			if (f1 >= 0 && f1 <= 1 && f2 >= -1 && f2 <= 1)
			{
				if (wall->GetStatus() == 1) // ball exit wall
				{
					ball.SetVelocity(pBounce * Reflect(ball.GetVelocity(), n));
					if (!ballExited) ballExited = true;
					stdHit.Play("stdhit.wav");
				}
				else if (wall->GetStatus() == 2) // bounce or roll down ramp
				{ 
					if (ball.GetYVelocity() < -150 || ball.GetYVelocity() > 150) // bounce
					{
						stdHit.Play("stdhit.wav");
						ball.SetVelocity(dBounce * Reflect(ball.GetVelocity(), n));
					}
					else if (ball.GetXVelocity() > 150) // bounce
					{
						stdHit.Play("stdhit.wav");
						ball.SetVelocity(dBounce * Reflect(ball.GetVelocity(), n));
					}
					else // roll
					{ 
						ball.SetYVelocity(0); // stop gravity pulling through object
						if (ball.GetXVelocity() < -75) ball.SetXVelocity(-75); // limit speed to allow gravity to keep ball on ramp
						else if (ball.GetXVelocity() > 0) ball.SetXVelocity(-5);  // initalise speed in right direction
						else ball.Accelerate(-10, 0); // progressively roll
					}
				}
				else if (wall->GetStatus() == 4) // green triangle wall
				{
					ball.SetVelocity(fBounce * Reflect(ball.GetVelocity(), n));
					speedHit.Play("speedhit.wav");
					score += 175;
				}
				else if (wall->GetStatus() == 5) // blue and redish triangles
				{
					ball.SetVelocity(iBounce * Reflect(ball.GetVelocity(), n));
					triangleHit.Play("trianglehit.wav");
					score += 150;
				}
				else
				{
					ball.SetVelocity(dBounce * Reflect(ball.GetVelocity(), n));
					stdHit.Play("stdhit.wav");
				}
		}
		}
	}
}

void CMyGame::ballsControl()
{
	for (CSprite* circle : theBalls)
	{
		CVector v1 = ball.GetVelocity();
		CVector v2 = circle->GetVelocity();

		CVector t = circle->GetPos() - ball.GetPos();
		if (circle->HitTest(&ball))
		{
			if (Dot(v2 - v1, t) < 0)
			{
				CVector n = Normalise(t);
				if (circle->GetStatus() == 1) // green circles
				{
					ball.SetVelocity(iBounce * Reflect(v1, n));
					circleHit.Play("circlehit.wav");
					score += 200;
				}
				else // round edge collisions
				{
					ball.SetVelocity(dBounce * Reflect(v1, n));
					stdHit.Play("stdhit.wav");
				}
			}
		}
	}
}

void CMyGame::livesControl()
{
	for (CSprite* life : lives)
	{
		if (ball.HitTest(&redPlanet) || ball.GetY() > 915 || ball.HitTest(&bluePlanet) || ball.GetY() < -20)
		{
			life->Delete();

			// play explosion at player pos
			CSprite* pExplosion = new CSprite(ball.GetX(), ball.GetY(), 0, 0, GetTime());
			pExplosion->AddImage("explosion.bmp", "explode", 5, 5, 0, 0, 4, 4, CColor::Black());
			pExplosion->SetAnimation("explode", 36);
			explosionSound.Play("explosion.wav");
			pExplosion->Die(700);
			explosions.push_back(pExplosion);
			
			// remove ball exit wall to allow ball to pass
			for (CSprite* wall : theWalls)
			{
				if (wall->GetStatus() == 6) wall->Delete();
			}
			theWalls.remove_if(deleted);

			wallCreated = false;
			ballExited = false;

			//reset position
			ball.SetPos(490, 168);
			ball.SetVelocity(0, 0);
		}
	}
	explosions.remove_if(deleted);
	lives.remove_if(deleted);

	// game over condition
	if (lives.size() == 0)
	{
		ball.SetPos(-100, 168);
		GameOver();
	}
}

void CMyGame::bonusPointsControl()
{
	// randomly spawn bonus points
	if (rand() % 2000 == 0 && bonusPoints.size() == 0)
	{
		int randnum = rand() % 7; // rand number between 0 and 6
		int x = 0, y = 0; // for bonus random spawn location

		if (randnum == 0) { x = 40; y = 347; }
		else if (randnum == 1) { x = 351; y = 531; }
		else if (randnum == 2) { x = 465; y = 571; }
		else if (randnum == 3) { x = 227; y = 612; }
		else if (randnum == 4) { x = 235; y = 314; }
		else if (randnum == 5) { x = 418; y = 341; }
		else { x = 317; y = 719; }

		// bonus item
		CSprite* bonus = new CSprite(x, y, "bonuspoints.bmp", CColor::White(), GetTime());
		bonusPoints.push_back(bonus);

		// bonus item fx
		CSprite* bonusfx = new CSprite(x, y, 0, 0, GetTime());
		bonusfx->AddImage("bonusfx.bmp", "bonusfxflash", 6, 1, 5, 0, 0, 0, CColor::Black());
		bonusfx->SetAnimation("bonusfxflash");
		bonusfx->SetStatus(1);
		explosions.push_back(bonusfx);
	}

	// remove bonus item
	for (CSprite* bonus : bonusPoints)
	{
		if (ball.HitTest(bonus))
		{
			bonusHit.Play("bonushit.wav");
			bonus->Delete();
			score += 5000;
		}
	}
	bonusPoints.remove_if(deleted);

	// remove bonus item fx
	for (CSprite* bonusfx : explosions)
	{
		if (bonusPoints.size() == 0 && bonusfx->GetStatus() == 1)
		{
			bonusfx->Delete();
		}
	}
	explosions.remove_if(deleted);
}

void CMyGame::collectedPointsControl()
{
	for (CSprite* points : collectPoints)
	{
		if (points->HitTest(&ball) && points->GetStatus() != 1) // activate
		{
			points->SetImage("glowingpoints");
			pointsHit.Play("500pts.wav");
			score += 500;
			points->SetStatus(1);
		}
		else if (!points->HitTest(&ball) && points->GetStatus() == 1) // deactivate
		{
			points->SetImage("500pts.bmp");
			points->SetStatus(0);
		}
	}
}

void CMyGame::OnDraw(CGraphics* g)
{
	if (IsMenuMode())
	{
		slot.Draw(g);
		menu.Draw(g);
		coin.Draw(g);
		return;
	}

	pinback.Draw(g);
	back.Draw(g);
	for (CSprite* points : collectPoints) points->Draw(g);
	ball.Draw(g);
	for (CSprite* life : lives) life->Draw(g);
	//for (CSprite* wall : theWalls) wall->Draw(g); // wall collision
	//for (CSprite* ball : theBalls) ball->Draw(g); // circle collision
	redPlanet.Draw(g);
	bluePlanet.Draw(g);
	leftPaddle.Draw(g);
	leftTopPaddle.Draw(g);
	rightPaddle.Draw(g);
	rightTopPaddle.Draw(g);
	for (CSprite* explosion : explosions) explosion->Draw(g);
	for (CSprite* bonus : bonusPoints) bonus->Draw(g);
	spring.Draw(g);
	music.Draw(g);

	*g << font(26) << color(CColor::LightGreen()) << xy(560, 423) << score;

	*g << font(12) << color(CColor::White()) << xy(600, 50) << "G-Force: " << globGravity;

	if (!IsGameOver())
		*g << font(26) << color(CColor::White()) << xy(560, 283) << GetTime() / 1000 << "s";
	else 
		*g << font(26) << color(CColor::White()) << xy(560, 283) << GetTimeGameOver() / 1000 << "s";

	if (IsGameOver())
		*g << font(36) << color(CColor::Red()) << xy(150, 425) << "GAME OVER!" << endl;
}

/////////////////////////////////////////////////////
// Game Life Cycle

// one time initialisation
void CMyGame::OnInitialize()
{
}

// called at the start of a new game - display menu here
void CMyGame::OnDisplayMenu()
{
	// clear lists
	theWalls.clear();
	theBalls.clear();
	lives.clear();
	bonusPoints.clear();
	collectPoints.clear();
	explosions.clear();

	// menu reset
	musicSound.Stop();
	coinMove = false;
	coin.SetPos(600, 100);
}

// called when Game Mode entered
void CMyGame::OnStartGame()
{
	// music
	musicSound.Play("music.wav", -1);
	music.SetImage("ismusic");
	
	// set up the ball
	spring.SetSize(30, 149);
	ball.SetPosition(490, 168);
	ball.SetVelocity(0, 0);

	// paddle
	leftPaddle.SetRotation(20);
	leftTopPaddle.SetRotation(-20);
	rightPaddle.SetRotation(-20);
	rightTopPaddle.SetRotation(20);

	// reset score
	score = 0;

	// lives
	for (int i = 489; i >= 409; i = i - 40)
	{
		CSprite* newLife = new CSprite(33, i, "ball2.png", 0);
		lives.push_back(newLife);
	}

	// --- level details ---
	springReleased = false;
	springEngaged = false;
	wallCreated = false;
	ballExited = false;

	// collison structures
	fstream myfile;
	myfile.open("level1.txt", ios_base::in);

	int type, x, y, w, h, rot, stat;
	do
	{
		myfile >> type >> x >> y >> w >> h >> rot >> stat;

		if (type == 1) // walls
		{
			CSprite* wall = new CSpriteRect(CRectangle(x, y, w, h), CColor::Yellow(), GetTime());
			wall->SetRotation((float)rot);
			wall->SetStatus(stat);
			theWalls.push_back(wall);
		}
		if (type == 2) // circles
		{
			CSprite* circle = new CSpriteOval((float)x, (float)y, (float)w, CColor::Red(), GetTime());
			circle->SetRotation((float)rot);
			circle->SetStatus(stat);
			theBalls.push_back(circle);
		}
		if (type == 3) // 500 points
		{
			CSprite* points = new CSprite((float)x, (float)y, "500pts.bmp", 0);
			points->SetRotation((float)rot);
			points->SetStatus(stat);
			points->LoadImage("500ptsglow.bmp", "glowingpoints");
			collectPoints.push_back(points);
		}

	} while (!myfile.eof());

	myfile.close();
}

// called when Game is Over
void CMyGame::OnGameOver()
{
}

// one time termination code
void CMyGame::OnTerminate()
{
}

/////////////////////////////////////////////////////
// Keyboard Event Handlers

void CMyGame::OnKeyDown(SDLKey sym, SDLMod mod, Uint16 unicode)
{
	if (sym == SDLK_F4 && (mod & (KMOD_LALT | KMOD_RALT)))
		StopGame();
	if (sym == SDLK_SPACE)
		PauseGame();
	if (sym == SDLK_F2)
		NewGame();

	if (IsGameMode())
	{
		if (sym == SDLK_a || sym == SDLK_d) flipperSound.Play("flipperup.wav");
	}
}

void CMyGame::OnKeyUp(SDLKey sym, SDLMod mod, Uint16 unicode)
{
	if (IsGameMode())
	{
		if (sym == SDLK_DOWN && springEngaged)
		{
			springReleased = true;
			springEngaged = false;

			// Implementation of the Hooke's Law
			float y = ball.GetY() - 168;
			ball.Accelerate(0, -k * y);
			ball.SetSpeed(ball.GetSpeed());
		}
		if (sym == SDLK_a || sym == SDLK_d) flipperSound.Play("flipperdown.wav");
	}
}


/////////////////////////////////////////////////////
// Mouse Events Handlers


void CMyGame::OnMouseMove(Uint16 x,Uint16 y,Sint16 relx,Sint16 rely,bool bLeft,bool bRight,bool bMiddle)
{
	if (IsMenuMode())
	{
		x -= coinGrabX; y -= coinGrabY;
		if (coinMove) coin.SetPos(x, y);
	}
}

void CMyGame::OnLButtonDown(Uint16 x,Uint16 y)
{
	if (IsMenuMode())
	{
		if (coin.HitTest(x, y))
		{
			coinMove = true;
			coinGrabX = x - coin.GetX();
			coinGrabY = y - coin.GetY();
		}
	}
	
	if (IsGameMode())
	{
		if (music.HitTest(x, y))
		{
			if (!musicSound.IsPaused())
			{
				musicSound.Pause();
				music.SetImage("nomusic");
			}
			else
			{
				musicSound.Resume();
				music.SetImage("ismusic");
			}
		}
	}
}

void CMyGame::OnLButtonUp(Uint16 x, Uint16 y)
{
	if (IsMenuMode())
	{
		if (coin.HitTest(&slot))
		{
			coinMove = false;
			coinSound.Play("coin.wav");
			StartGame();
		}
		else coinMove = false;
	}
}

void CMyGame::OnRButtonDown(Uint16 x,Uint16 y)
{
}

void CMyGame::OnRButtonUp(Uint16 x,Uint16 y)
{
}

void CMyGame::OnMButtonDown(Uint16 x,Uint16 y)
{
}

void CMyGame::OnMButtonUp(Uint16 x,Uint16 y)
{
}
