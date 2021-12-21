#include <raylib.h>
#include "raymath.h"
#include <stdio.h>

typedef enum GameScreen { tela1 = 0, tela2, tela3, menu, infos, gameOver, gameplay, victory} GameScreen;

#define G 400
#define PLAYER_JUMP_SPD 350.0f
#define PLAYER_HOR_SPD 200.0f

typedef struct Player {
    Vector2 position;
    float speed;
    bool canJump;
    int life;
} Player;

typedef struct EnvItem {
    Rectangle rect;
    int blocking;
    Color color;
} EnvItem;

typedef struct Bullet {
    Vector2 position;
    float speedX;
    float speedY;
} Bullet;

typedef struct Enemy {
    Rectangle rect;
    Color color;
    int life;
    int speed;
    float centerPos;
    int dir;
    Texture2D eneTex;
    int texIndex;
    Bullet tiro[2];
} Enemy;

void UpdatePlayer(Player *player, EnvItem *envItems, int envItemsLength, float delta);
void UpdateBullets(Bullet *bullets, int *bulletsCount, Enemy *enemies, Player *player, int enemyCount, float delta);

void CameraCenterInsideMap(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height);

GameScreen currentScreen = menu;
float pontuacao = 0;

int main(void){

    //incia tela
    const int screenWidth = 800;
    const int screenHeight = 450;

    // Propriedades do tiro
    int bulletCD = 1, lastBulletSec = 0;
    int enemyBulletCD = 2, enemyLastBulletSec = 0;

    InitWindow(screenWidth, screenHeight, "Navin's adventures");
    Texture2D gameplayBackground = LoadTexture("resources/BG.png");
    Texture2D backgroundTex = LoadTexture("resources/cenarios/BG.png");
    Texture2D backgroundMenu = LoadTexture("resources/cenarios/menu.png");
    Texture2D backgroundInfo = LoadTexture("resources/cenarios/inducao.png");
    Texture2D gameOverScreen = LoadTexture("resources/cenarios/gameOver.png");
    Texture2D victoryScreen = LoadTexture("resources/cenarios/victoryScreen.png");

    // Player Texture
    Texture2D playerTex = LoadTexture("resources/player.png");
    Rectangle playerRec = { 0.0f, 0.0f, (float)playerTex.width/3, (float)playerTex.height };
    int plCurrentFrame = 0;
    int plFramesCounter = 0;
    int plFramesSpeed = 8;

    // Enemy Texture
    Texture2D enemySprite = LoadTexture("resources/enemy2.png");
    Rectangle frameRec = { 0.0f, 0.0f, (float)enemySprite.width/4, (float)enemySprite.height };
    int currentFrame = 0;
    int eneFramesCounter = 0;
    int framesSpeed = 8;

    // Initializing Scenario Textures //
    int texturePosX = 0;
    int texturePosY = 0;

    // Platforms Textures
    Texture2D grassTex = LoadTexture("resources/Tiles/2.png");
    Texture2D groundTex = LoadTexture("resources/Tiles/5.png");

    // Player Initialization
    Player player = { 0 };
    player.position = (Vector2){ 200, 400 };
    player.speed = 0;
    player.canJump = false;
    player.life = 8;

    // Scenario Initialization
    EnvItem envItems[] = {
        {{ 0, 0, 3000, 400 }, 0, BLANK },
        {{ 500, 125, 100, 10 }, 1, BLANK },
        {{ 1900, 146, 100, 10 }, 1, BLANK },
        {{ 2100, 180, 600, 10 }, 1, BLANK },
        {{ 300, 217, 400, 10 }, 1, BLANK },
        {{ 2500, 338, 200, 10 }, 1, BLANK },
        {{ 650, 300, 250, 10 }, 1, BLANK },
        {{ 860, 339, 100, 10 }, 1, BLANK },
        {{ 1600, 318, 100, 10 }, 1, BLANK },
        {{ 0, 400, 3000, 200 }, 1, BLANK },
    };

    // Balas
    Bullet *bullets = NULL;
    int bulletsCount = 0;

    // Inimigos
    Enemy enemies[] = {
        {{600, 360, 40, 40}, YELLOW , 3, 100, 600, 1, enemySprite, 2, {(Bullet){(Vector2){0, 0}, -300, 0 }, (Bullet){(Vector2){0, 0}, 300, 0 }}},
        {{700, 260, 40, 40}, YELLOW , 3, 100, 700, 1, LoadTexture("enemy1.png"), 1, {(Bullet){(Vector2){0, 0}, -300, 0 }, (Bullet){(Vector2){0, 0}, 300, 0 }}},
        {{1000, 360, 40, 40}, YELLOW , 3, 100, 1000, 1, enemySprite, 2, {(Bullet){(Vector2){0, 0}, -300, 0 }, (Bullet){(Vector2){0, 0}, 300, 0 }}},
        {{2150, 140, 40, 40}, YELLOW , 3, 100, 2150, 1, LoadTexture("enemy1.png"), 1, {(Bullet){(Vector2){0, 0}, -300, 0 }, (Bullet){(Vector2){0, 0}, 300, 0 }}}
    };
    int enemyCount = sizeof(enemies)/sizeof(enemies[0]);

    int envItemsLength = sizeof(envItems)/sizeof(envItems[0]);

    // Camera
    Camera2D camera = { 0 };
    camera.target = player.position;
    camera.offset = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    

    //inicia todas as variaveis aqui
    const char narrador1[] = "\"Seja nas emoções ou na lógica, tudo tem suas maravilhas, até mesmo a\nescuridão e o silêncio. E eu aprendi que em qualquer estado que esteja,\nestarei contente, no último suspiro do universo, na fusão dos meus\nátomos, eu renasço no infinito; estarei pronto para extrair o que há de\nmelhor na minha alma\".";

    const char narrador2[] = "Após sair vitorioso no Exame Nacional do Ensino Médio, Navin, agora em\nsua nova jornada, enfrentará problemas muito mais desafiadores.";

    const char narrador3[] = "Inspirando sua imaginação no grande Kenneth H. Rosen, sonha em como\nsuperar a lógica clássica, para não ir à temida jornada FINAL. Para isso\nterá que ser teletransportado ao Mundo Primitivo da Matemática Discreta\ne derrotar os inimigos (Indução e Grafo).";

    char indu[] = "Grande vilão da primeira lua, com\nseus filhos: tese e hipótese foi\nmotivo de tristeza de grande parte\ndos alunos e estará de volta para\ntentar levar Navin à jornada Final.";

    char grafo[] = "Um pouco mais fraco que a indução,\n mas extenso na sua trajetória de\nvida, com seus diversos vértices e\narestas, essencial para completar\no ciclo da terceira lua.";

    int framesCounter = 0;  //conta frames das letras

    SetTargetFPS(60);

    // Atira uma  bala no começo pra evitar um bug
    bulletsCount += 1;
    bullets = (Bullet*) realloc(bullets, bulletsCount * sizeof(Bullet));
    bullets[bulletsCount-1] = {(Vector2){0, 0}, -400, 0 };

    while (!WindowShouldClose()){
        
        float deltaTime = GetFrameTime();
        double secs = GetTime();
        
        //atualiza variaveis aqui
        switch(currentScreen){

          case tela1:
          {
            DrawTexture(backgroundTex, 0, 0, WHITE);
            if (IsKeyDown(KEY_SPACE))
              framesCounter += 8;
            else
              framesCounter++;

            if (IsKeyPressed(KEY_ENTER)){
                currentScreen = tela2;
                framesCounter = 0;
            }
          } break;

          case tela2:
          {
            DrawTexture(backgroundTex, 0, 0, WHITE);
            if (IsKeyDown(KEY_SPACE))
              framesCounter += 8;
            else
              framesCounter++;

            if (IsKeyPressed(KEY_ENTER))
            {
              currentScreen = tela3;
              framesCounter = 0;
            }
          } break;

          case tela3:
          { 
            DrawTexture(backgroundTex, 0, 0, WHITE);
            if (IsKeyDown(KEY_SPACE))
              framesCounter += 8;
            else
              framesCounter++;
            if (IsKeyPressed(KEY_ENTER))
            {
              currentScreen = gameplay;
                pontuacao = 0;
                player.position = (Vector2){ 200, 400 };
                player.speed = 0;
                player.canJump = false;
                player.life = 8;
                free(bullets);
                bullets = NULL;
                bulletsCount = 0;
                enemies[0] = {{600, 360, 40, 40}, YELLOW , 3, 100, 600, 1, enemySprite, 2, {(Bullet){(Vector2){0, 0}, -300, 0 }, (Bullet){(Vector2){0, 0}, 300, 0 }}};
                enemies[1] = {{700, 260, 40, 40}, YELLOW , 3, 100, 700, 1, LoadTexture("enemy1.png"), 1, {(Bullet){(Vector2){0, 0}, -300, 0 }, (Bullet){(Vector2){0, 0}, 300, 0 }}};
                enemies[2] = {{1000, 360, 40, 40}, YELLOW , 3, 100, 1000, 1, enemySprite, 2, {(Bullet){(Vector2){0, 0}, -300, 0 }, (Bullet){(Vector2){0, 0}, 300, 0 }}};
                enemies[3] = {{2150, 140, 40, 40}, YELLOW , 3, 100, 2150, 1, LoadTexture("enemy1.png"), 1, {(Bullet){(Vector2){0, 0}, -300, 0 }, (Bullet){(Vector2){0, 0}, 300, 0 }}};
                bulletsCount += 1;
                bullets = (Bullet*) realloc(bullets, bulletsCount * sizeof(Bullet));
                bullets[bulletsCount-1] = {(Vector2){0, 0}, -400, 0 };
              framesCounter = 0;
            }
          } break;

          case menu:
          {
            DrawTexture(backgroundMenu, 0, 0, WHITE);
            if (IsKeyPressed(KEY_ENTER))
            {
              currentScreen = tela1;
            }
            if (IsKeyPressed(KEY_SPACE)){
              currentScreen = infos;
              framesCounter = 0;
            }
          } break;

          case infos:
          {
            DrawTexture(backgroundInfo, 0, 0, WHITE);
            if (IsKeyDown(KEY_SPACE))
              framesCounter += 8;
            else
              framesCounter++;
            if (IsKeyPressed(KEY_ENTER))
            {
              framesCounter = 0;
              currentScreen = menu;
              framesCounter = 0;
            }
          } break;

          case gameOver:
          {
            ClearBackground(BLACK);
            DrawTexture(gameOverScreen, 0, 0, WHITE);
            if(IsKeyDown(KEY_SPACE)){
              currentScreen = menu;
            }
          } break;
          
          case victory:
            ClearBackground(BLACK);
            DrawTexture(victoryScreen, 0, 0, WHITE);
          break;

          case gameplay:
            if(IsKeyDown(KEY_E) && lastBulletSec + bulletCD <= secs){
              lastBulletSec = secs;
              bulletsCount += 1;
              bullets = (Bullet*) realloc(bullets, bulletsCount * sizeof(Bullet));
              bullets[bulletsCount-1] = {(Vector2){player.position.x + 50, player.position.y - 20}, 400, 0 };
            }

            // Inimigo Atirar
            if(enemyLastBulletSec + enemyBulletCD <= secs){
              for(int i = 0; i < enemyCount; i++){
                  enemies[i].tiro[0].position = (Vector2){enemies[i].rect.x, enemies[i].rect.y};
                  enemies[i].tiro[1].position = (Vector2){enemies[i].rect.x, enemies[i].rect.y};
              }
              enemyLastBulletSec = secs;
            }

            if (IsKeyDown(KEY_LEFT) && (player.position.x)-20 > 0){
              plFramesCounter++;
              if (plFramesCounter >= (60/plFramesSpeed)){
                  plFramesCounter = 0;
                  plCurrentFrame++;
                  if (plCurrentFrame > 2) plCurrentFrame = 0;
                  playerRec.x = (float)plCurrentFrame*(float)playerTex.width/3;
              }
              player.position.x -= PLAYER_HOR_SPD*deltaTime;
            }
            if (IsKeyDown(KEY_RIGHT) && (player.position.x)+20 < envItems[envItemsLength-1].rect.width){
              plFramesCounter++;
              if (plFramesCounter >= (60/plFramesSpeed)){
                  plFramesCounter = 0;
                  plCurrentFrame++;
                  if (plCurrentFrame > 2) plCurrentFrame = 0;
                      playerRec.x = (float)plCurrentFrame*(float)playerTex.width/3;
              }
              player.position.x += PLAYER_HOR_SPD*deltaTime;
            }
            UpdatePlayer(&player, envItems, envItemsLength, deltaTime);
            UpdateBullets(bullets, &bulletsCount, enemies, &player, enemyCount, deltaTime);

            eneFramesCounter++;
            if (eneFramesCounter >= (60/framesSpeed)){
                eneFramesCounter = 0;
                currentFrame++;
                if (currentFrame > 3) currentFrame = 0;
                frameRec.x = (float)currentFrame*(float)enemySprite.width/4;
            }

            // Call update camera function by its pointer
            CameraCenterInsideMap(&camera, &player, envItems, envItemsLength, deltaTime, screenWidth, screenHeight);
            break;

          default: break;
        }
        
      BeginDrawing();
        //exibicao das variaveis aqui
        switch(currentScreen){
            case tela1:
            {
              //narrador 1
              ClearBackground(BLACK);
              DrawText(TextSubtext(narrador1, 0, framesCounter/4), 15, 300, 20, WHITE);
              DrawText("PRESSIONE [SPACE] PARA ACELERAR !", 10, 10, 18, WHITE);
              DrawText("PRESSIONE [ENTER] PARA PULAR !", 10, 35, 18, WHITE);
            } break;

            case tela2:
            {
              //narrador 2
              ClearBackground(BLACK);
              DrawText(TextSubtext(narrador2, 0, framesCounter/4), 15, 300, 20, WHITE);
              DrawText("PRESSIONE [SPACE] PARA ACELERAR !", 10, 10, 18, WHITE);
              DrawText("PRESSIONE [ENTER] PARA PULAR !", 10, 35, 18, WHITE);
            } break;

            case tela3:
            {
              //narrador 3
              ClearBackground(BLACK);
              DrawText(TextSubtext(narrador3, 0, framesCounter/4), 15, 325, 20, WHITE);
              DrawText("PRESSIONE [SPACE] PARA ACELERAR !", 10, 10, 18, WHITE);
              DrawText("PRESSIONE [ENTER] PARA PULAR !", 10, 35, 18, WHITE);
            } break;

            case menu:
            {
              DrawText("PRESSIONE [ENTER] PARA", 275, 115, 19, WHITE);
              DrawText("JOGAR !", 360, 135, 19, WHITE);

              DrawText("PRESSIONE [SPACE] PARA", 275, 190, 19, WHITE);
              DrawText("MAIS INFORMAÇÕES !", 300, 210, 19, WHITE);
            }break;

            case infos:
            {
              DrawText("PRESSIONE ENTER PARA VOLTAR", 233, 5, 19, WHITE);
              DrawText(TextSubtext(indu, 0, framesCounter/3), 15, 300, 20, WHITE);
              DrawText(TextSubtext(grafo, 0, framesCounter/3), 425, 300, 20, WHITE);
              
            }break;

            case gameOver:
            {
              DrawText("Aperte Espaço para voltar pro Menu!", 10, 30, 18, WHITE);
            } break;

            case gameplay:
              ClearBackground(LIGHTGRAY);

              BeginMode2D(camera);

                for (int setor = 0; texturePosX <= envItems[0].rect.width + gameplayBackground.width; setor++){
                  texturePosX = setor*gameplayBackground.width;
                  DrawTexture(gameplayBackground, texturePosX, 0, WHITE);
                }
                texturePosX = 0;

                for (int i = 1; i < envItemsLength; i++) {
                  for (int setor = 0; texturePosX <= envItems[i].rect.width + grassTex.width; setor++) {
                        texturePosX = envItems[i].rect.x + setor*grassTex.width;
                        texturePosY = envItems[i].rect.y;
                        DrawTexture(grassTex, texturePosX, texturePosY, WHITE);
                  }
                  texturePosX = 0;

                  for (int setor = 0; texturePosX <= envItems[i].rect.width + groundTex.width; setor++) {
                        texturePosX = envItems[i].rect.x + setor*groundTex.width;
                        texturePosY = envItems[i].rect.y + groundTex.width;
                        DrawTexture(groundTex, texturePosX, texturePosY, WHITE);
                  }
                  texturePosX = 0;
                }

                for (int i = 0; i < envItemsLength; i++) DrawRectangleRec(envItems[i].rect, envItems[i].color);

                for (int i = 0; i < enemyCount; i++){
                    Vector2 pos = (Vector2){enemies[i].rect.x, enemies[i].rect.y};
                    Rectangle frameDest = { pos.x+577, pos.y+328, (float)enemies[i].eneTex.width/4, (float)enemies[i].eneTex.height/1 };
                    DrawTextureTiled(enemies[i].eneTex, frameRec, frameDest, (Vector2){600, 360}, 0, 1, WHITE);
                    Rectangle enemyBulletRect = { enemies[i].tiro[0].position.x + 10, enemies[i].tiro[0].position.y + 5, 10, 10 };
                    Rectangle enemyBulletRect2 = { enemies[i].tiro[1].position.x + 10, enemies[i].tiro[1].position.y + 5, 10, 10 };
                    DrawRectangleRec(enemyBulletRect, DARKPURPLE);
                    DrawRectangleRec(enemyBulletRect2, DARKPURPLE);
                } 
                for (int i = 0; i < bulletsCount; i++){
                    Rectangle bulletRect = { bullets[i].position.x - 10, bullets[i].position.y - 5, 10, 10 };
                    DrawRectangleRec(bulletRect, DARKPURPLE);
                }

                Vector2 pos = player.position;
                Rectangle frameDest = { pos.x+570, pos.y+289, (float)playerTex.width*2.5/3, (float)playerTex.height*2.5/1 };
                DrawTextureTiled(playerTex, playerRec, frameDest, (Vector2){600, 360}, 0, 2.5, WHITE);

              EndMode2D();
            break;
          }
      EndDrawing();
    }
    UnloadTexture(backgroundTex);
    UnloadTexture(gameplayBackground);
    UnloadTexture(victoryScreen);
    UnloadTexture(gameOverScreen);
    UnloadTexture(backgroundMenu);
    UnloadTexture(backgroundInfo);
    UnloadTexture(playerTex);
    UnloadTexture(enemySprite);
    UnloadTexture(grassTex);
    UnloadTexture(groundTex);
    CloseWindow();
    free(bullets);
    return 0;
}
void UpdatePlayer(Player *player, EnvItem *envItems, int envItemsLength, float delta)
{
    if (IsKeyDown(KEY_SPACE) && player->canJump)
    {
        player->speed = -PLAYER_JUMP_SPD;
        player->canJump = false;
    }

    int hitObstacle = 0;
    for (int i = 0; i < envItemsLength; i++)
    {
        EnvItem *ei = envItems + i;
        Vector2 *p = &(player->position);
        if (ei->blocking &&
            ei->rect.x <= p->x &&
            ei->rect.x + ei->rect.width >= p->x &&
            ei->rect.y >= p->y &&
            ei->rect.y < p->y + player->speed*delta)
        {
            hitObstacle = 1;
            player->speed = 0.0f;
            p->y = ei->rect.y;
        }
    }

    if (!hitObstacle)
    {
        player->position.y += player->speed*delta;
        player->speed += G*delta;
        player->canJump = false;
    }
    else player->canJump = true;
}
void UpdateBullets(Bullet *bullets, int *bulletsCount, Enemy *enemies, Player *player, int enemyCount, float delta){
    for(int i = 0; i < enemyCount; i++){
        Enemy *ene = enemies + i;
        int mod = ene->dir;
        ene->rect.x += ene->speed*mod*delta;
        if((mod == 1 && ene->rect.x > ene->centerPos + 40) ||
           (mod == -1 && ene->rect.x < ene->centerPos - 40)){
              ene->dir *= -1;
              if(ene->texIndex == 2){
                  if(ene->dir == -1) ene->eneTex = LoadTexture("resources/enemy2Backwards.png");
                  else ene->eneTex = LoadTexture("resources/enemy2.png");
              }
              else{
                if(ene->dir == -1) ene->eneTex = LoadTexture("resources/enemy1Backwards.png");
                else ene->eneTex = LoadTexture("resources/enemy1.png");
              }
           }
        ene->tiro[0].position.x += ene->tiro[0].speedX*delta;
        ene->tiro[1].position.x += ene->tiro[1].speedX*delta;
        Vector2 tiro = ene->tiro[0].position;
        if((tiro.x >= player->position.x - 20 && tiro.x <= player->position.x + 20) &&
           (tiro.y >= player->position.y - 60 && tiro.y <= player->position.y)){
               player->life--;
               ene->tiro[0].position = (Vector2){0, 0};
               if(player->life == 0)
                   currentScreen = gameOver;
           }
        tiro = ene->tiro[1].position;
        if((tiro.x >= player->position.x - 20 && tiro.x <= player->position.x + 20) &&
           (tiro.y >= player->position.y - 60 && tiro.y <= player->position.y)){
               player->life--;
               ene->tiro[0].position = (Vector2){0, 0};
               if(player->life == 0)
                   currentScreen = gameOver;
           }
    }
    for(int i = 0; i < *bulletsCount; i++){
        Bullet *bullet = bullets + i;
        bullet->position.x += bullet->speedX*delta;
        for(int j = 0; j < enemyCount; j++){
            Enemy *ene = enemies + j;
            Vector2 *p = &(bullet->position);
            if (p->x >= ene->rect.x && p->x <= ene->rect.x + ene->rect.width &&
                p->y >= ene->rect.y && p->y <= ene->rect.y + ene->rect.height){
                ene->life--;
                if(ene->life == 0){
                    pontuacao += 1;
                    if(pontuacao == 4){
                        currentScreen = victory;
                    }
                    ene->rect = (Rectangle){-100, -100, 0, 0};
                }
                *bulletsCount -= i;
                if(*bulletsCount == 0){
                    free(bullets);
                }
                else{
                    //for(int k = i; k < *bulletsCount; k++)
                    //    bullets[k].position = bullets[k+1].position;*/
                    bullets = (Bullet*) realloc(bullets+(i+1), *bulletsCount * sizeof(Bullet));
                }
                return;
            }
        }
    }
}

void CameraCenterInsideMap(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height)
{
    camera->target = player->position;
    camera->offset = (Vector2){ width/2.0f, height/2.0f };
    float minX = 1000, minY = 1000, maxX = -1000, maxY = -1000;

    for (int i = 0; i < envItemsLength; i++)
    {
        EnvItem *ei = envItems + i;
        minX = fminf(ei->rect.x, minX);
        maxX = fmaxf(ei->rect.x + ei->rect.width, maxX);
        minY = fminf(ei->rect.y, minY);
        maxY = fmaxf(ei->rect.y + ei->rect.height, maxY);
    }

    Vector2 max = GetWorldToScreen2D((Vector2){ maxX, maxY }, *camera);
    Vector2 min = GetWorldToScreen2D((Vector2){ minX, minY }, *camera);

    if (max.x < width) camera->offset.x = width - (max.x - width/2);
    if (max.y < height) camera->offset.y = height - (max.y - height/2);
    if (min.x > 0) camera->offset.x = width/2 - min.x;
    if (min.y > 0) camera->offset.y = height/2 - min.y;
}