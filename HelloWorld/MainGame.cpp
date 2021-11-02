#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

int DISPLAY_WIDTH = 360 * 2;
int DISPLAY_HEIGHT = 480 * 2;
int DISPLAY_SCALE = 1;

enum GameObjectType
{
	Player,
	Asteroid,
	Enemy,
	Beam
};

int playerObjectId = -1;
const int maxBeams = 10;
int beamIds[maxBeams] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

float shipSpeed = 10.0f;

void SpawnBeam()
{
	for (int i = 0; i < maxBeams; i++)
	{
		if (beamIds[i] == -1)
		{
			GameObject& playerObject = Play::GetGameObject(playerObjectId);
			beamIds[i] = Play::CreateGameObject(GameObjectType::Beam, playerObject.pos, 16, "effect_purple");
			GameObject& beam = Play::GetGameObject(beamIds[i]);
			beam.velocity = { 0, -10 };
			break;
		}
	}
}

void HandlePlayerInput(GameObject& playerObject)
{
	if (Play::KeyDown(VK_LEFT))
	{
		playerObject.pos.x -= shipSpeed;
	}
	if (Play::KeyDown(VK_RIGHT))
	{
		playerObject.pos.x += shipSpeed;
	}
	if (Play::KeyDown(VK_UP))
	{
		playerObject.pos.y -= shipSpeed;
	}
	if (Play::KeyDown(VK_DOWN))
	{
		playerObject.pos.y += shipSpeed;
	}
	if (Play::KeyPressed(VK_SPACE))
	{
		SpawnBeam();
	}
}

Vector2D GetRandomPosition()
{
	int xPosition = Play::RandomRoll(DISPLAY_WIDTH);
	return { xPosition, -32 - Play::RandomRoll(400) };
}

void SpawnAsteroid()
{
	int id = Play::CreateGameObject(GameObjectType::Asteroid, GetRandomPosition(), 22, "meteor_L01");
	GameObject& asteroid = Play::GetGameObject(id);
	float speed = 3.0f + Play::RandomRoll(7);
	asteroid.velocity = Vector2D(0, speed);
	asteroid.rotSpeed = Play::RandomRollRange(-25, 25);
}

void SpawnEnemy()
{
	int id = Play::CreateGameObject(GameObjectType::Enemy, GetRandomPosition(), 22, "enemy");
	GameObject& enemy = Play::GetGameObject(id);
	float speed = 3.0f + Play::RandomRoll(7);
	enemy.velocity = Vector2D(0, speed);
}

void ResetGame()
{
	std::vector<int> gameIds = Play::CollectAllGameObjectIDs();
	for (int i = 0; i < gameIds.size(); i++)
	{
		Play::DestroyGameObject(gameIds[i]);
	}
	for (int i = 0; i < maxBeams; i++)
	{
		beamIds[i] = -1;
	}

	playerObjectId = Play::CreateGameObject(GameObjectType::Player, { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 50 }, 26, "player");

	for (int i = 0; i < 8; i++)
	{
		SpawnAsteroid();
	}

	for (int i = 0; i < 8; i++)
	{
		SpawnEnemy();
	}
}

// The entry point for a PlayBuffer program
void MainGameEntry( PLAY_IGNORE_COMMAND_LINE )
{
	Play::CreateManager( DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE );
	ResetGame();
}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate( float elapsedTime )
{
	Play::ClearDrawingBuffer( Play::cBlack );
	GameObject& playerObject = Play::GetGameObject(playerObjectId);

	Play::CentreAllSpriteOrigins();

	HandlePlayerInput(playerObject);

	Play::DrawObject(playerObject);

	std::vector<int> asteroidIds = Play::CollectGameObjectIDsByType(GameObjectType::Asteroid);
	for (int i = 0; i < asteroidIds.size(); i++)
	{
		int asteroidId = asteroidIds[i];
		GameObject& asteroid = Play::GetGameObject(asteroidId);
		Play::UpdateGameObject(asteroid);
		Play::DrawObjectRotated(asteroid);

		if (asteroid.pos.y > DISPLAY_HEIGHT + 32)
		{
			asteroid.pos = GetRandomPosition();
		}

		if (Play::IsColliding(playerObject, asteroid))
		{
			ResetGame();
		}
	}

	for (int i = 0; i < maxBeams; i++)
	{
		if (beamIds[i] == -1)
			continue;

		GameObject& beam = Play::GetGameObject(beamIds[i]);
		Play::UpdateGameObject(beam);
		Play::DrawObject(beam);

		if (beam.pos.y < -32)
		{
			Play::DestroyGameObject(beamIds[i]);
			beamIds[i] = -1;
		}
	}

	std::vector<int> enemyIds = Play::CollectGameObjectIDsByType(GameObjectType::Enemy);
	for (int i = 0; i < enemyIds.size(); i++)
	{
		GameObject& enemy = Play::GetGameObject(enemyIds[i]);
		Play::UpdateGameObject(enemy);
		Play::DrawObject(enemy);

		if (Play::IsColliding(playerObject, enemy))
		{
			ResetGame();
		}

		for (int k = 0; k < maxBeams; k++)
		{
			if (beamIds[k] != -1)
			{
				GameObject& beam = Play::GetGameObject(beamIds[k]);
				if (Play::IsColliding(beam, enemy))
				{
					Play::DestroyGameObject(beamIds[k]);
					beamIds[k] = -1;
					enemy.pos = GetRandomPosition();
				}
			}
		}
	}

	Play::PresentDrawingBuffer();
	return Play::KeyDown( VK_ESCAPE );
}

// Gets called once when the player quits the game 
int MainGameExit( void )
{
	Play::DestroyManager();
	return PLAY_OK;
}


