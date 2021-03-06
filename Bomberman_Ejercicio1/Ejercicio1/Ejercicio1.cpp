#include  "pch.h"
#include <iostream>
#include <fstream>
#include <string>

#include "raylib.h"

std::string* textures;
std::string title;
int textures_num = 0;
int levels_num = 0;

Image background;
Texture2D texture;
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

struct Bomb {
	Vector3 bomb_pos = { .0f, .0f, .0f };
	Vector3 bomb_pos_center;

	float time = 15;
	float current_time = 0.0f;

	float size = .5f;
	const float size_boom = 1.0f;
	float size_current = size;

	int expansion_hor = 2;
	int expansion_ver = 2;

	const float time_expansion = 4.0f;
	float time_cur_expansion = 0.0f;

	bool active = false;

	Color color = BLACK;
	Color color_exploding = ORANGE;
	Color color_explosion_final = RED;
};

const int maxBombs = 3;
Bomb bombs[maxBombs];
int bombs_num = -1;

int loadBackground(std::ifstream& file, int level_num, int w, int h) {
	//1; 1; 1; 1; 1; 1; 1; 1; 1; 1; 1; 1;;
	std::string word;
	int value = -1;

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

int w;
int h;
int num_level;

int loadLevels(std::ifstream& file, int num) {

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
		levels[i].width = w;


		std::getline(file, word, ';');
		h = std::stoi(word);
		levels[i].height = h;
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

int ExplosionBomb(Bomb b1)
{
	int corrector_x = b1.bomb_pos.x  + (w / 2);
	int corrector_z = b1.bomb_pos.z + (h / 2);  
	std::string word;

	// horizontal
	for (int i = -b1.expansion_hor; i <= b1.expansion_hor; i++) {
		if ((corrector_x + i) > 0 && (corrector_x + i) < w) {
			if (levels[1].collisions[corrector_x + i][corrector_z] == "w") {
				levels[1].collisions[corrector_x + i][corrector_z] = "x";
				levels[1].foreground[corrector_x + i][corrector_z] = 0;
			}
		}
	}
	// vertical
	for (int i = -b1.expansion_ver; i <= b1.expansion_ver; i++) {
		if ((corrector_z + i) > 0 && (corrector_z + i) < h) {
			if (levels[1].collisions[corrector_x][corrector_z + i] == "w") {
				levels[1].collisions[corrector_x][corrector_z + i] = "x";
				levels[1].foreground[corrector_x][corrector_z + i] = 0;
			}
		}
	}
	return 0;
}

int DeployBomb() 
{
	float t = GetFrameTime();
	for (int i = 0; i < sizeof(bombs) / sizeof(*bombs); i++)
	{
		if (bombs[i].active){
			bombs[i].time_cur_expansion += t;
			Color c = bombs[i].color;
			
			if (bombs[i].time_cur_expansion > bombs[i].time_expansion / 2){
				c = bombs[i].color_exploding;			
			}
			if (bombs[i].time_cur_expansion > bombs[i].time_expansion) {
				bombs[i].active = false;
				bombs[i].time_cur_expansion = 0.0f;
				ExplosionBomb(bombs[i]);
			}
			
			// Horizontal pos correction
			//if ((int)bombs[i].bomb_pos.x == (int)(-levels[0].width / 2))
			if ((int)bombs[i].bomb_pos.x == -6)
				bombs[i].bomb_pos.x = -5;
			else if ((int)bombs[i].bomb_pos.x == (int)levels[0].width)
				bombs[i].bomb_pos.x = w-1;
			
			// Vertical pos correction
			if ((int)bombs[i].bomb_pos.z == (int)(-levels[0].height / 2))
				bombs[i].bomb_pos.y = (-levels[0].height / 2) + 1;
			else if((int)bombs[i].bomb_pos.z == (int)levels[0].height)
				bombs[i].bomb_pos.x = h - 1;

			DrawSphereEx(bombs[i].bomb_pos, bombs[i].size, 8, 8, c);
			DrawSphereWires(bombs[i].bomb_pos, bombs[i].size, 8, 8, WHITE);
		}
	}
	return 0;
}

int doRaylibMagic() {
	// Initialization
	//--------------------------------------------------------------------------------------
	int screenWidth = 800;
	int screenHeight = 450;

	InitWindow(screenWidth, screenHeight, "raylib [models] example - box collisions");

	// Define the camera to look into our 3d world
	Camera camera = { { 0.0f, 10.0f, 10.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 45.0f, 0 };

	Vector3 playerPosition = { -5.0f, 2.0f, -3.0f };
	Vector3 playerPosition_prev = playerPosition;
	Vector3 playerSize = { 0.5f, 1.0f, 0.5f };
	Color playerColor = GREEN;

	//Vector3 enemyBoxPos = { -4.0f, 2.0f, 0.0f };
	Vector3 enemyBoxSize = { 1.0f, 1.0f, 1.0f };

	Vector3 enemySpherePos = { 4.0f, 0.0f, 0.0f };
	float enemySphereSize = 1.5f;

	bool collision = false;


	Image img_list[10];     // Load image in CPU memory (RAM)
	Texture2D textures2d[10];

	for (int i = 1; i < textures->length(); i++) {
		std::string tmp = "../Ejercicio1/" + textures[i];
		img_list[i] = LoadImage(tmp.c_str());
		textures2d[i] = LoadTextureFromImage(img_list[i]);
	}

	Image background = LoadImage("../Ejercicio1/piedra.png");


	Texture2D texture = LoadTextureFromImage(background);


	SetTargetFPS(60);   // Set our game to run at 60 frames-per-second
	//--------------------------------------------------------------------------------------

	// Main game loop
	while (!WindowShouldClose())    // Detect window close button or ESC key
	{ 
		// Update
		//----------------------------------------------------------------------------------
		playerPosition_prev = playerPosition;

		// Move player
		if (IsKeyDown(KEY_RIGHT)) playerPosition.x += 0.1f;
		else if (IsKeyDown(KEY_LEFT)) playerPosition.x -= 0.1f;
		else if (IsKeyDown(KEY_DOWN)) playerPosition.z += 0.1f;
		else if (IsKeyDown(KEY_UP)) playerPosition.z -= 0.1f;

		if (IsKeyPressed(KEY_SPACE))
		{
				bombs_num++;
				if (bombs_num == maxBombs)
					bombs_num = 0;

				if (!bombs[bombs_num].active)
				{
					bombs[bombs_num].bomb_pos = playerPosition;
					bombs[bombs_num].bomb_pos.x = (int)floor(bombs[bombs_num].bomb_pos.x);
					bombs[bombs_num].bomb_pos.z = (int)floor(bombs[bombs_num].bomb_pos.z);

					bombs[bombs_num].active = true;
				}
		}

		collision = false;
		// Check collisions player vs enemy-box
		int cur_level = 1;
		int offset_x = -levels[cur_level].height / 2;
		int offset_z = -levels[cur_level].width / 2;

		for (int i = 0; i < levels[cur_level].height; i++) {
			for (int j = 0; j < levels[cur_level].width; j++) {
				int n = levels[cur_level].background[i][j];

				Vector3 enemyBoxPos = {
					offset_z + j,
					1.0f,
					offset_x + i
				};

				std::string t_s = levels[cur_level].collisions[i][j];
				// PRIMER CASO
				if ((t_s == "y" && CheckCollisionBoxes(
					(BoundingBox({
					(Vector3({
					(float)(playerPosition.x - playerSize.x / 2),
						(float)(playerPosition.y - playerSize.y / 2),
						(float)(playerPosition.z - playerSize.z / 2)
				})),
						(Vector3({
						(float)(playerPosition.x + playerSize.x / 2),
							(float)(playerPosition.y + playerSize.y / 2),
							(float)(playerPosition.z + playerSize.z / 2)
					}))
						})),
						(BoundingBox({
						(Vector3({
						(float)(enemyBoxPos.x - enemyBoxSize.x / 2),
							(float)(enemyBoxPos.y - enemyBoxSize.y / 2),
							(float)(enemyBoxPos.z - enemyBoxSize.z / 2)
					})),
							(Vector3({
							(float)(enemyBoxPos.x + enemyBoxSize.x / 2),
								(float)(enemyBoxPos.y + enemyBoxSize.y / 2),
								(float)(enemyBoxPos.z + enemyBoxSize.z / 2)
						}))
							})))) || 
					// SEGUNDO CASO
							(t_s == "w" && CheckCollisionBoxes(
					(BoundingBox({
					(Vector3({
					(float)(playerPosition.x - playerSize.x / 2),
						(float)(playerPosition.y - playerSize.y / 2),
						(float)(playerPosition.z - playerSize.z / 2)
				})),
						(Vector3({
						(float)(playerPosition.x + playerSize.x / 2),
							(float)(playerPosition.y + playerSize.y / 2),
							(float)(playerPosition.z + playerSize.z / 2)
					}))
						})),
						(BoundingBox({
						(Vector3({
						(float)(enemyBoxPos.x - enemyBoxSize.x / 2),
							(float)(enemyBoxPos.y - enemyBoxSize.y / 2),
							(float)(enemyBoxPos.z - enemyBoxSize.z / 2)
					})),
							(Vector3({
							(float)(enemyBoxPos.x + enemyBoxSize.x / 2),
								(float)(enemyBoxPos.y + enemyBoxSize.y / 2),
								(float)(enemyBoxPos.z + enemyBoxSize.z / 2)
						}))
							}))))
					) {
					collision = true;
				}
			}
		}

		if (collision == true) {
			playerColor = RED;
			playerPosition = playerPosition_prev;
		}
		else {
			playerColor = GREEN;
		}
		//----------------------------------------------------------------------------------

		// Draw
		//----------------------------------------------------------------------------------
		BeginDrawing();

		ClearBackground(RAYWHITE);

		BeginMode3D(camera);
		cur_level = 1;

		offset_x = -levels[cur_level].height / 2;
		offset_z = -levels[cur_level].width / 2;
		/*offset_x = 0;
		offset_z = 0;*/

		// Draw enemy-box
		for (int i = 0; i < levels[cur_level].height; i++) {
			for (int j = 0; j < levels[cur_level].width; j++) {
				int n = levels[cur_level].background[i][j];

				Vector3 enemyBoxPos = {
					offset_z + j,
					1.0f,
					offset_x + i
				};
				if (n != 0) {
					DrawCubeTexture(textures2d[n], enemyBoxPos, enemyBoxSize.x, enemyBoxSize.y, enemyBoxSize.z, GRAY);
					DrawCubeWires(enemyBoxPos, enemyBoxSize.x, enemyBoxSize.y, enemyBoxSize.z, DARKGRAY);
				}
			}
		}
		
		//Foreground
		for (int i = 0; i < levels[cur_level].height; i++) {
			for (int j = 0; j < levels[cur_level].width; j++) {
				int n = levels[cur_level].foreground[i][j];

				Vector3 enemyBoxPos = {
					offset_z + j,
					2.0f,
					offset_x + i
				};
				if (n != 0) {
					DrawCubeTexture(textures2d[n], enemyBoxPos, enemyBoxSize.x, enemyBoxSize.y, enemyBoxSize.z, GRAY);
					DrawCubeWires(enemyBoxPos, enemyBoxSize.x, enemyBoxSize.y, enemyBoxSize.z, DARKGRAY);

				}
			}
		}
		
		// Draw Bombs
		DeployBomb();

		// Draw player
		DrawCubeV(playerPosition, playerSize, playerColor);

		EndMode3D();

		DrawText("Move player with cursors to collide", 220, 40, 20, GRAY);
		DrawText(FormatText("Posición: x: %f, y: %f, z: %f", playerPosition.x, playerPosition.y, playerPosition.z), 100, 200, 20, RED);

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
	/*
	int w = 32, h = 8;
	int** matrix;
	matrix = new int*[h];
	for (int i = 0; i < h; ++i) {
		matrix[i] = new int[w];
	}
	*/

	std::ifstream game_conf("../Ejercicio1/game.csv");

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