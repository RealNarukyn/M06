#include "pch.h"
#include <iostream>
#include <fstream>
#include <string>
#include <array>

#include "raylib.h"

std::string* textures;
std::string title;
int textures_num = 0;
int levels_num = 0;



std::array<Texture2D, 4> misTexturas;

//LEVEL; 1; Prado del MAL; 12; 8;;

struct level {
	std::string title;
	int number;
	int width;
	int height;
	int** background;
	std::string** collisions;
	int** foreground;
};
level* levels;



struct bomb {
	Vector3 pos = { 0.f, 0.f, 0.f };
	float time_limit = 10.f;
	float cur_time = 0.f;

	float expansion_time = 2.f;
	float expansion_time_cur = 0.f;

	float size = 1.f;
	float size_max = 2.f;


	Color color = BLACK;
	Color color_hot = PINK;
	Color color_explosion = RED;
};



int loadBackground(std::ifstream& file, int level_num, int w, int h) {
	//1; 1; 1; 1; 1; 1; 1; 1; 1; 1; 1; 1;;
	std::string word;
	int value = -1;

	levels[level_num].width = w;
	levels[level_num].height = h;

	for (int i = 0; i < h; ++i) {
		for (int j = 0; j < w; ++j) {
			std::getline(file, word, ';');
			value = std::stoi(word);

			levels[level_num].background[i][j] = value;

			std::cout << value;
		}
		std::cout << '\n';

		std::getline(file, word, '\n');
	}

	return 0;
}


int loadForeground(std::ifstream& file, int level_num, int w, int h) {
	//1; 1; 1; 1; 1; 1; 1; 1; 1; 1; 1; 1;;
	std::string word;
	int value = -1;

	for (int i = 0; i < h; ++i) {
		for (int j = 0; j < w; ++j) {
			std::getline(file, word, ';');
			value = std::stoi(word);

			levels[level_num].foreground[i][j] = value;

			std::cout << value;
		}
		std::cout << '\n';

		std::getline(file, word, '\n');
	}

	return 0;
}


int loadCollisions(std::ifstream& file, int level_num, int w, int h) {
	std::string word;

	for (int i = 0; i < h; ++i) {
		for (int j = 0; j < w; ++j) {
			std::getline(file, word, ';');
			levels[level_num].collisions[i][j] = word;

			std::cout << word;
		}
		std::cout << '\n';

		std::getline(file, word, '\n');
	}

	return 0;
}


int loadLevels(std::ifstream& file, int num) {
	int w;
	int h;
	int num_level;
	std::string title;

	std::string word;

	levels = new level[num + 1];

	levels[0].title = "none";
	for (int i = 1; i < num + 1; ++i) {
		std::getline(file, word, ';');
		if (word.compare("LEVEL") != 0)
			return 1;

		std::getline(file, word, ';');
		num_level = std::stoi(word);

		std::getline(file, word, ';');
		title = word;

		std::getline(file, word, ';');
		w = std::stoi(word);

		std::getline(file, word, ';');
		h = std::stoi(word);

		std::getline(file, word, '\n');

		levels[i].background = new int*[h];
		levels[i].collisions = new std::string*[h];
		levels[i].foreground = new int*[h];
		for (int j = 0; j < h; ++j) {
			levels[i].background[j] = new int[w];
			levels[i].collisions[j] = new std::string[w];
			levels[i].foreground[j] = new int[w];
		}

		for (int k = 0; k < 3; ++k) {
			std::getline(file, word, ';');
			std::string block = word;
			std::getline(file, word, '\n');

			std::cout << block << '\n';

			if (block.compare("BACKGROUND") == 0)
				loadBackground(file, i, w, h);
			else if (block.compare("FOREGROUND") == 0)
				loadForeground(file, i, w, h);
			else if (block.compare("COLLISION") == 0)
				loadCollisions(file, i, w, h);
		}

	}

	return 0;
}

int loadTextures(std::ifstream& file, int num) {

	std::string word;

	textures = new std::string[num + 1];
	textures[0] = "none";

	int counter = 1;
	while (counter < num + 1) {
		//1; suelo_verde.png;;
		std::getline(file, word, ';');
		std::getline(file, word, ';');
		textures[counter] = word;
		std::cout << textures[counter] << '\n';
		std::getline(file, word, '\n');

		counter++;
	}

	return 0;
}

int n = 1;
int cur_level = 1;




int doRaylibMagic() {
	float offset_x = levels[cur_level].width / 2;
	float offset_z = levels[cur_level].height / 2;

	// Initialization
	//--------------------------------------------------------------------------------------
	int screenWidth = 1000;
	int screenHeight = 650;

	InitWindow(screenWidth, screenHeight, "raylib [models] example - box collisions");

	// Define the camera to look into our 3d world
	Camera camera = { { 0.0f, 15.0f, 15.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 45.0f, 0 };

	Vector3 playerPosition = { 1.0f, 2.0f, -2.0f };
	Vector3 playerPositionPrev;
	Vector3 playerSize = { 1.0f, 2.0f, 1.0f };
	Color playerColor = GREEN;

	Vector3 enemyBoxPos = { -4.0f, 1.0f, 0.0f };
	Vector3 enemyBoxSize = { 1.0f, 1.0f, 1.0f };

	Vector3 enemySpherePos = { 4.0f, 0.0f, 0.0f };
	float enemySphereSize = 1.5f;

	bool collision = false;

	for (int i = 1; i < misTexturas.size(); i++)
	{
		std::string texture = "../Debug/" + textures[i];
		Image background = LoadImage(texture.c_str());     // Load image in CPU memory (RAM)
		misTexturas[i] = LoadTextureFromImage(background);
	}

	SetTargetFPS(60);   // Set our game to run at 60 frames-per-second
	//--------------------------------------------------------------------------------------

	// Main game loop
	while (!WindowShouldClose())    // Detect window close button or ESC key
	{
		// Update
		//----------------------------------------------------------------------------------
		playerPositionPrev = playerPosition;
		// Move player
		if (IsKeyDown(KEY_RIGHT)) playerPosition.x += 0.2f;
		else if (IsKeyDown(KEY_LEFT)) playerPosition.x -= 0.2f;
		else if (IsKeyDown(KEY_DOWN)) playerPosition.z += 0.2f;
		else if (IsKeyDown(KEY_UP)) playerPosition.z -= 0.2f;

		collision = false;

		// Check collisions player vs enemy-box

		for (int i = 0; i < levels[cur_level].width; i++)
		{
			for (int j = 0; j < levels[cur_level].height; j++)
			{
				Vector3 enemyPos = {
					-offset_x + j,
					1.f,
					-offset_z + i
				};

				if (levels[cur_level].collisions[i][j] == "y")
				{
					/*	if (CheckCollisionBoxes(
							(BoundingBox({ (Vector3({
														   (float)(playerPosition.x - playerSize.x / 2),
														   (float)(playerPosition.y - playerSize.y / 2),
														   (float)(playerPosition.z - playerSize.z / 2),})),
											  (Vector3({
														   (float)(playerPosition.x + playerSize.x / 2),
														   (float)(playerPosition.y + playerSize.y / 2),
														   (float)(playerPosition.z + playerSize.z / 2),}))
								})),
								(BoundingBox({ (Vector3({
															   (float)(enemyPos.x - enemyBoxSize.x / 2),
															   (float)(enemyPos.y - enemyBoxSize.y / 2),
															   (float)(enemyPos.z - enemyBoxSize.z / 2),})),
												(Vector3({
															   (float)(enemyPos.x + enemyBoxSize.x / 2),
															   (float)(enemyPos.y + enemyBoxSize.y / 2),
															   (float)(enemyPos.z + enemyBoxSize.z / 2) }))
									})
									))
							) {
							collision = true;
						}

						if (collision)
						{
							playerColor = RED;
							playerPosition = playerPositionPrev;
						}
						else
							playerColor = GREEN; */
				}
			}
		}

		//----------------------------------------------------------------------------------

		// Draw
		//----------------------------------------------------------------------------------
		BeginDrawing();

		ClearBackground(RAYWHITE);

		BeginMode3D(camera);

		// Draw enemy-box


		for (int i = 0; i < levels[cur_level].height; i++)
		{
			for (int j = 0; j < levels[cur_level].width; j++)
			{
				Vector3 enemyPos = {
				   (float)-levels[1].height / 2 + j + 1,
				   1.f,
				   (float)-levels[1].width / 2 + i + 1
				};
				n = levels[cur_level].background[i][j];
				DrawCubeTexture(misTexturas[1], enemyPos, enemyBoxSize.x, enemyBoxSize.y, enemyBoxSize.z, GRAY);
			}
		}

		// <-- FOREGROUND --> 
		for (int i = 0; i < levels[cur_level].height; i++)
		{
			for (int j = 0; j < levels[cur_level].width; j++)
			{
				n = levels[cur_level].foreground[i][j];
				Vector3 enemyPos = {
					(float)-levels[1].height / 2 + j + 1,
					2.f,
					(float)-levels[1].width / 2 + i + 1
				};

				if (n != 0)
				{
					DrawCubeTexture(misTexturas[n], enemyPos, enemyBoxSize.x, enemyBoxSize.y, enemyBoxSize.z, GRAY);
				}
			}
		}

		// Draw player
		DrawCubeV(playerPosition, playerSize, playerColor);

		//	DrawGrid(10, 1.0f);        // Draw a grid

		EndMode3D();

		DrawText("Move player with cursors to collide", 220, 40, 20, GRAY);

		DrawFPS(10, 10);

		EndDrawing();
		//----------------------------------------------------------------------------------
	}

	// De-Initialization
	//--------------------------------------------------------------------------------------
	CloseWindow();

	return 0;
}


int main(int argc, char* argv[]) {
	//Matrix snipet, guardadlo
	int w = 32, h = 8;
	int** matrix;
	matrix = new int*[h];
	for (int i = 0; i < h; ++i) {
		matrix[i] = new int[w];
	}

	std::ifstream game_conf("../Debug/game.csv");

	if (!game_conf.is_open()) {
		std::cout << "ERROR: Error al abrir el archivo" << '\n';
		return 1;
	}

	std::string word;

	while (game_conf.good()) {
		std::getline(game_conf, word, ';');

		if (word.compare("TEXTURES") == 0) {
			std::getline(game_conf, word, ';');
			textures_num = std::stoi(word);
			std::getline(game_conf, word, '\n');

			loadTextures(game_conf, textures_num);
		}
		else if (word.compare("TITLE") == 0) {
			std::getline(game_conf, word, ';');
			title = word;
			std::cout << title << std::endl;
			std::getline(game_conf, word, '\n');
		}
		else if (word.compare("LEVELS") == 0) {
			std::getline(game_conf, word, ';');
			levels_num = std::stoi(word);
			std::getline(game_conf, word, '\n');
			loadLevels(game_conf, levels_num);
		}
	}

	game_conf.close();

	doRaylibMagic();

	return 0;
}