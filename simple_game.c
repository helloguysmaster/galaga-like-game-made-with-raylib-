#include "raylib.h"

typedef enum GameMode
{
    PLAYING,
    GAME_OVER
} GameMode;

GameMode gameMode;

typedef struct Entity
{
    Vector2 pos;
    int health;
    int flashTime;
    bool active;
} Entity;

Entity player;
float velocity;
int lives;
int level;
int progress;
Entity bullets[20];
Entity enemies[200];
int enemiesLeftToSpawn;

int timeUntilNextEnemySpawns;

void InitGame()
{
    player.pos = (Vector2){400, 400};
    player.active = true;
    velocity = 0;
    lives = 1;
    level = 1;
    progress = 0;
    
    for (int i = 0; i < 20; i++)
    {
        bullets[i].pos = (Vector2){0};
        bullets[i].active = false;
    }
    
    for (int i = 0; i < 200; i++)
    {
        Entity *e = &enemies[i];
        e->pos = (Vector2){0};
        #define ENEMY_HEALTH 3
        e->health = ENEMY_HEALTH;
        e->flashTime = 0;
        e->active = false;
        
    }
    
    timeUntilNextEnemySpawns = 0;
    enemiesLeftToSpawn = 1;
    
    gameMode = PLAYING;
}

#define PLAYER_ACCELERATION 0.5
#define PLAYER_DECELERATION 0.5
#define PLAYER_MAX_SPEED 5

int main()
{
    const int screenWidth = 800;
    const int screenHeight = 450;
    
    InitWindow(screenWidth, screenHeight, "Galaga Like Game");
    SetTargetFPS(60);
    Texture2D textureMap = LoadTexture("texturemap.png");
    InitAudioDevice();
    Sound snd_shoot = LoadSound("shoot.wav");
    SetSoundVolume(snd_shoot, 0.2f);
    
    InitGame();
    
    while (!WindowShouldClose())
    {   
        if (IsKeyPressed(KEY_R))  InitGame();
        
        if (gameMode == PLAYING)
        {
            // Update player
            int input = IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT);
            if (input != 0)  velocity += input*PLAYER_ACCELERATION;
            else if (velocity > 0)  
            {
                velocity -= PLAYER_DECELERATION;
                if (velocity < 0)  velocity = 0;
            }
            else if (velocity < 0)  
            {
                velocity += PLAYER_DECELERATION;
                if (velocity > 0) velocity = 0;
            }
            if (velocity > PLAYER_MAX_SPEED)  velocity = PLAYER_MAX_SPEED;
            else if (velocity < -PLAYER_MAX_SPEED)  velocity = -PLAYER_MAX_SPEED;
            if (player.pos.x < 0)  player.pos.x = screenWidth;
            else if (player.pos.x > screenWidth)  player.pos.x = 0;
            player.pos.x += velocity;
            
            // Shoot bullet
            if (IsKeyPressed(KEY_SPACE))
            {
                PlaySound(snd_shoot);
                int bulletNumber = 0;
                for (int i = 0; i < 20; i++) 
                {
                    if (bullets[i].active == false) bulletNumber = i;
                }
                bullets[bulletNumber].active = true;
                bullets[bulletNumber].pos = player.pos;
                bullets[bulletNumber].pos.x += 28;
                bullets[bulletNumber].pos.y -= 25;
            }
            
            // Update bullets
            for (int i = 0; i < 20; i++)
            {
                Entity *b = &bullets[i];
                if (b->active)
                {
                    b->pos.y -= 10;
                    
                    // Check for collision with all enemies
                    bool touchingEnemy = false;
                    Entity *hitEnemy;
                    for (int i = 0; i < 200; i++)
                    {
                        Entity *e = &enemies[i];
                        if (e->active && CheckCollisionRecs((Rectangle){b->pos.x, b->pos.y, 58, 14}, 
                                               (Rectangle){e->pos.x, e->pos.y, 15*2, 25*2}))
                        {
                            touchingEnemy = true;
                            hitEnemy = e;
                            break;
                        }                           
                    }
                    
                    if (touchingEnemy)
                    {
                        b->active = false;
                        hitEnemy->health -= 1;
                        hitEnemy->flashTime = 10;
                    }
                    
                    if (b->pos.y <= 0)
                    {
                        b->active = false;
                    }
                }
            }
            
            // Update enemies
            timeUntilNextEnemySpawns -= 1;
            for (int i = 0; i < 200; i++)
            {
                Entity *e = &enemies[i];
                if (e->active)
                {
                    e->pos.y += 1;
                    
                    if (e->flashTime > 0) e->flashTime -= 1;
                    
                    if ((e->health <= 0 && e->flashTime <= 0) || (e->pos.y > screenHeight))  
                    {
                        e->active = false;
                        e->flashTime = 0;
                        e->health = ENEMY_HEALTH;
                        
                        if (e->pos.y > screenHeight) 
                        {
                            lives -= 1;
                            if (lives <= 0)  gameMode = GAME_OVER;
                        }
                        else 
                        {
                            progress += 1;
                            if (progress >= 3*level)
                            {
                                progress = 0;
                                level += 1;
                                lives += 1;
                            }
                        }
                    }
                }
                else if (timeUntilNextEnemySpawns <= 0)
                {
                    e->active = true;
                    e->pos = (Vector2){GetRandomValue(10, screenWidth-10), 10};
                    enemiesLeftToSpawn -= 1;
                    if (enemiesLeftToSpawn <= 0)
                    {
                        timeUntilNextEnemySpawns = GetRandomValue(0, GetRandomValue(300, 600)-(level*10));
                        if (timeUntilNextEnemySpawns < 10)  timeUntilNextEnemySpawns = 10;
                        enemiesLeftToSpawn = GetRandomValue(1, level);
                    }
                }
            }
        }
        
        BeginDrawing();
            ClearBackground(BLACK);
            
            if (gameMode == PLAYING)
            {
                // Draw player
                DrawTextureRec(textureMap, (Rectangle){16, 8, 48, 39}, player.pos, WHITE);
                
                // Draw bullets
                for (int i = 0; i < 20; i++)
                {
                    if (bullets[i].active)
                    {
                        DrawTexturePro(textureMap, (Rectangle){16, 0, 29, 7}, (Rectangle){bullets[i].pos.x, bullets[i].pos.y, 29, 7}, (Vector2){0}, 90.0f, WHITE);
                    }
                }
                
                // Draw enemies
                for (int i = 0; i < 20; i++)
                {
                    Entity *e = &enemies[i];
                    if (e->active)
                    {
                        Color c;
                        if (e->flashTime > 0)  c = RED; // Makes enemy red when they have just taken damage.
                        else c = WHITE;
                        DrawTextureRec(textureMap, (Rectangle){0.0f, 0.0f, 15.0f, 25.0f}, e->pos, c);
                        if (e->health < ENEMY_HEALTH)  DrawRectangle(e->pos.x-10, e->pos.y-10, e->health*15, 3, GREEN); // TODO: Figure out how to center the healthbar.
                    }
                }
                DrawText(TextFormat("Lives: %d", lives), 700, 40, 20, WHITE);
                DrawText(TextFormat("Level: %d", level), 700, 60, 20, WHITE);
                DrawText(TextFormat("Progress: %d out of %d", progress, level*3), 575, 80, 20, WHITE);
            }
            
            DrawText(TextFormat("FPS: %d", GetFPS()), 700, 20, 20, WHITE);
        EndDrawing();
    }
    
    UnloadSound(snd_shoot);
    CloseAudioDevice();
    UnloadTexture(textureMap);
    CloseWindow();
    return 0;
}
