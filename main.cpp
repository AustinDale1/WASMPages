#include "raylib.h"

#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <cstdlib> 
#include <string>
#include <cmath>
#include <chrono>  
#include <vector>
#include <thread>
#include <iomanip>


#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif




constexpr int SCREEN_WIDTH = 2400;
constexpr int SCREEN_HEIGHT = 1350;
constexpr float GRAVITY = 9.81f;
constexpr int BULLET_INITIAL_SPEED = 823;
constexpr float SCALE_FACTOR = 100.0;
constexpr float TIME_STEP = 1.0f / 60.0f;

static bool gameOver = false;
static bool pause = false;


Texture2D texture;
Texture2D textureFlip;
Texture2D textureFlip2;
Texture2D textureFlip3;


static std::vector<Rectangle> planeImages;
class Plane;
static std::vector<Plane> enemyPlanes;

class Plane {
    public:          
        Vector2 position;
        Vector2 velocity; 
        double planeAngle = 0.0f;
        bool isCrashed = false;
        bool isShooting = false;
        std::chrono::time_point<std::chrono::system_clock> start, end, ct;
        std::chrono::time_point<std::chrono::system_clock> newStart, newEnd;
        std::chrono::duration<double> dur{5.0};
        Rectangle planeImage;
        int index;
        bool him = false;
        int health = 20;
        bool init = false;
        void CreatePlane(Vector2 startPos, int counter)
        {
            index = counter;
            this->position = startPos;
            this->velocity.x = 5;
            this->velocity.y = 0;
            planeImage = {
                    this->position.x, 
                    this->position.y,
                    texture.width * .1f,
                    texture.height * .1f
                };
            planeImages.emplace_back(planeImage);
        }

    Plane() { 
    }
};

class Bullet {
   public:  
        Vector2 position;        
        double angle = 0.0f;
        bool isDone = false;
        Plane shotBy;
        
    Bullet(Vector2 pos, double planeAngle, Plane fired) {
        position = pos;
        angle = planeAngle;
        shotBy = fired;
    } 
};

class SmokeParticle {
    public:
        Vector2 position;
        std::chrono::time_point<std::chrono::system_clock> start;

    SmokeParticle(Vector2 pos, std::chrono::time_point<std::chrono::system_clock> cTime) {
        int randomNum = rand() % 20; 
        position = {pos.x + randomNum - 10, pos.y};
        start = cTime;
    }
};

Plane myPlane;
Plane plane2;
static std::vector<Bullet> bulletsInAir;
static std::vector<SmokeParticle> smokeInAir;
int counter = 0;
int score = 0;
int enemyPlaneCount = 1;
bool isDetecting = false;


static Plane planes[10] = {myPlane};
static void InitGame(void);
static void RestartGame(void);
static void UpdateDrawFrame(); 
static void UpdateGame();  
static void GetMovement(); 
static void UpdatePlane(Plane& plane);
static bool DetectBullets(Plane& plane);
static void UpdateEnemyPlane(Plane& plane);
static void DrawGame();

double DegToRad(double x)
{
    return ((x * 3.14)/180);
}


int main(void)
{
    
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "if you see this, gimme a job");
    texture = LoadTexture("pfiddyone2.png");
    textureFlip = LoadTexture("pfiddyone2flip.png");
    textureFlip2 = LoadTexture("pfiddyone2flip2.png");
    textureFlip3 = LoadTexture("pfiddyone2flip3.png");
    InitGame();
    // Image image = LoadImage("Untitled design.png");     // Loaded in CPU memory (RAM)
    // texture = LoadTextureFromImage(image); 

    
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        UpdateDrawFrame();
    }

    CloseWindow();

    return 0;
}




void InitGame(void)
{
    std::cout << "In init" << '\n';
    myPlane.CreatePlane({SCREEN_WIDTH/2, SCREEN_HEIGHT/2}, counter);
    myPlane.him = true;
    counter++;
    plane2.CreatePlane({SCREEN_WIDTH/2, (SCREEN_HEIGHT/2) - 100}, counter);
    enemyPlanes.emplace_back(plane2);
    

}

void RestartGame(void)
{
    myPlane.position = {SCREEN_WIDTH/2, SCREEN_HEIGHT/2};
    myPlane.health = 20;
    myPlane.isShooting = false;
    myPlane.init = false;
    myPlane.isCrashed = false;
    myPlane.planeAngle = 0.0f;

    myPlane.dur = std::chrono::duration<double>(5.0);
    for(Plane& ep : enemyPlanes)
    {
        ep.position = {SCREEN_WIDTH/2, (SCREEN_HEIGHT/2) - 100};
        ep.health = 20;
        ep.isShooting = false;
        ep.init = false;
        ep.isCrashed = false;
        ep.planeAngle = 0.0f;
        ep.dur = std::chrono::duration<double>(5.0);;
    }
    score = 0;
    bulletsInAir.clear();
    smokeInAir.clear();

}

void UpdateDrawFrame()
{
    UpdateGame();
    DrawGame();
}

void UpdateGame()
{
    if (!gameOver)
    {
        if (IsKeyPressed('P')) pause = !pause;

        if (!pause)
        {
            GetMovement();
            UpdatePlane(myPlane);
            for(Plane& ep : enemyPlanes)
            {
                UpdateEnemyPlane(ep);
            }
            // UpdateEnemyPlane(plane2);
            int i = 0;
            for (Bullet& bullet : bulletsInAir) {
                if(!bullet.isDone)
                {
                    if(bullet.position.x > SCREEN_WIDTH || bullet.position.x < 0 || bullet.position.y > SCREEN_HEIGHT || bullet.position.y < 0)
                    {
                        bullet.isDone = true;
                    }
                    bullet.position.x += (cosf(DegToRad(bullet.angle)) * 8);
                    bullet.position.y -= (sinf(DegToRad(bullet.angle)) * 8);
                } else
                {
                    bulletsInAir.erase(bulletsInAir.begin() + i);
                }
                i++;
            }
        }
    }
    else
    {
        if (IsKeyPressed(KEY_ENTER))
        {
            RestartGame();
            std::cout << "restarting game" << '\n';
            gameOver = false;
        }
    }
}

int bulletCount = 0;
bool initialized = false;

void GetMovement()
{
    if(myPlane.planeAngle >= 360)
    {
        myPlane.planeAngle = std::fmod(myPlane.planeAngle, 360);
    }
    if(myPlane.planeAngle < 0)
    {
        myPlane.planeAngle = 360 + myPlane.planeAngle;
    }
    if(!myPlane.isCrashed)
    {
        if (IsKeyDown(KEY_W)) 
        {
            if(myPlane.planeAngle == 90)
                myPlane.planeAngle = myPlane.planeAngle;
            else if(myPlane.planeAngle < 90 || myPlane.planeAngle >= 270)
                myPlane.planeAngle += 3;
            else
            myPlane.planeAngle -= 3; 
        }
        if (IsKeyDown(KEY_A)) 
        {
            if(myPlane.planeAngle == 180)
                myPlane.planeAngle = myPlane.planeAngle;
            else if(myPlane.planeAngle < 180)
                myPlane.planeAngle += 3;
            else
            myPlane.planeAngle -= 3; 
        }
        if (IsKeyDown(KEY_S)) 
        {
            if(myPlane.planeAngle == 270)
                myPlane.planeAngle = myPlane.planeAngle;
            else if(myPlane.planeAngle <= 90 || myPlane.planeAngle > 270)
                myPlane.planeAngle -= 3;
            else
            myPlane.planeAngle += 3; 
        }
        if (IsKeyDown(KEY_D)) 
        {
            if(myPlane.planeAngle == 0)
                myPlane.planeAngle = myPlane.planeAngle;
            else if(myPlane.planeAngle <= 180)
                myPlane.planeAngle -= 3;
            else
            myPlane.planeAngle += 3; 
        }
    }

    if(IsMouseButtonDown(MOUSE_LEFT_BUTTON))
    {
        // std::cout << "oy " << myPlane.dur.count() << '\n';
        if(myPlane.dur > std::chrono::seconds(0))
        {
            if(!myPlane.isShooting)
            {
                myPlane.newStart = std::chrono::system_clock::now();
                bulletsInAir.emplace_back(Bullet(myPlane.position, myPlane.planeAngle, myPlane));
                myPlane.isShooting = true;

            } else
            {
                bulletsInAir.emplace_back(Bullet(myPlane.position, myPlane.planeAngle, myPlane));
                myPlane.newEnd = std::chrono::system_clock::now();
                
                myPlane.dur = myPlane.dur - (std::chrono::system_clock::now() - myPlane.newStart);
            }
        }
    } else {
        if(myPlane.isShooting)
        {
            myPlane.isShooting = false;
            myPlane.newEnd = std::chrono::system_clock::now();
            myPlane.init = true;
        }
        if(myPlane.dur <= std::chrono::seconds(5) && myPlane.init)
        {
            myPlane.dur = myPlane.dur + (std::chrono::system_clock::now() - myPlane.newEnd);
        }  
    }
}


void UpdatePlane(Plane& plane)
{
    if(plane.isCrashed && plane.position.y > SCREEN_HEIGHT)
    {
        gameOver = true;
    }
    if(myPlane.planeAngle >= 360)
    {
        myPlane.planeAngle = std::fmod(myPlane.planeAngle, 360);
    }
    if(myPlane.planeAngle < 0)
    {
        myPlane.planeAngle = 360 + myPlane.planeAngle;
    }
    if(!plane.isCrashed)
    {
        plane.position.x += (cosf(DegToRad(myPlane.planeAngle)) * plane.velocity.x);
        plane.position.y -= (sinf(DegToRad(myPlane.planeAngle)) * plane.velocity.x);
    } else {
        if(plane.planeAngle >= 90 && plane.planeAngle <= 270)
        {
            if(plane.planeAngle != 270)
            {
                plane.planeAngle += .5;
            }
        } else
        {
            plane.planeAngle -= .5;
        }
        plane.position.x += (cosf(DegToRad(plane.planeAngle)) * plane.velocity.x);
        plane.position.y -= (sinf(DegToRad(plane.planeAngle)) * (plane.velocity.x));

    }
}

double angle = 0;
double x = 1;
int bulCount = 0;
// bool DetectBullets(Plane& plane) {
//         //     Vector2 position;        
//         // double angle = 0.0f;
//         // bool isDone = false;
//         // Plane shotBy;

//     int dangerCount = 0;
//     for(Bullet& bullet : bulletsInAir) {
//         // if((bullet.position.x - plane.position.x < 200 || bullet.position.x - plane.position.x > -200) && (bullet.position.y - plane.position.y < 200 || bullet.position.y - plane.position.y > -200)){
//         //     continue;
//         // }
//         // if((bullet.position.x - plane.position.x > 1200 || bullet.position.x - plane.position.x < -1200) && (bullet.position.y - plane.position.y > 1200 || bullet.position.y - plane.position.y < -1200)){
//         //     continue;
//         // }
//         // if(bullet.angle > 90 && bullet.angle < 270 && bullet.position.x < plane.position.x) {
//         //     continue;
//         // }
//         // if(bullet.angle < 90 && bullet.angle > 270 && bullet.position.x > plane.position.x) {
//         //     continue;
//         // }
        
//         float bulletAngle = bullet.angle;  // your bullet's direction
//         float dx = plane.position.x - bullet.position.x;
//         float dy = bullet.position.y - plane.position.y;
//         float angleToTarget = atan2(dy, dx) * 180.0f / PI;

//         // Normalize both angles and adjust relative to bullet's direction
//         float relativeAngle = angleToTarget - bulletAngle;
//         if (relativeAngle < 0) relativeAngle += 360.0f;
//         if(relativeAngle < 6 && relativeAngle > -6)
//                 std::cout << "here " << relativeAngle << '\n';

//         // Now check if it's within ±5 degrees of the bullet's path
//         bool inCone = (relativeAngle >= 355 || relativeAngle <= 5);  // -5 and +5 relative to bullet
//         if(!inCone) {
//             continue;
//         }

//         dangerCount++;
//         if(dangerCount >= 5) {
//             return true;
//         }
//     }
//     return false;
// }

void UpdateEnemyPlane(Plane& plane)
{
    // if(DetectBullets(plane)){
    //     std::cout << "AHHHHHHHHHH";
    //     plane.planeAngle -= 90;
    // }
    if(plane.planeAngle >= 360)
    {
        plane.planeAngle = std::fmod(plane.planeAngle, 360);
    }
    if(plane.planeAngle < 0)
    {
        plane.planeAngle = 360 + plane.planeAngle;
    }
    if(plane.isCrashed && plane.position.y > SCREEN_HEIGHT)
    {
        plane.isCrashed = false;
        plane.health = 20;
        plane.position = {-200, 100};
    }
    if(!plane.isCrashed) {
        angle = atanf((myPlane.position.x - plane.position.x)/(-myPlane.position.y + plane.position.y));
        angle = (angle * 180)/3.14;
        if(myPlane.position.x - plane.position.x > 0)
        {
            if(-myPlane.position.y + plane.position.y >= 0)
            {
                angle = 90 - angle;
            } else if(-myPlane.position.y + plane.position.y < 0)
            {
                angle = -(-270 + angle);
            } 
        } else if(myPlane.position.x - plane.position.x < 0)
        {
            if(-myPlane.position.y + plane.position.y > 0)
            {
                angle = -(-90 + angle);
            } else if(-myPlane.position.y + plane.position.y < 0)
            {
                angle = 270 - angle;
            } else{
                
            }
        } else{
            if(-myPlane.position.y + plane.position.y > 0)
            {
                angle = 90;
            } else
            {
                angle = 270;
            }
        }
        // if(DetectBullets(plane)) {
        //     isDetecting = true;
        // }
        // isDetecting = DetectBullets(plane);
        // if(DetectBullets(plane)) {
        //     plane.planeAngle -= 2;
        // } else 
        if(angle > plane.planeAngle)
        {
            plane.planeAngle += 2;
        } else if(angle < plane.planeAngle)
        {
            plane.planeAngle -= 2;
        }
        plane.position.x += (cosf(DegToRad(plane.planeAngle)) * plane.velocity.x);
        plane.position.y -= (sinf(DegToRad(plane.planeAngle)) * plane.velocity.x);
    }
    else {
        
        if(plane.planeAngle >= 90 && plane.planeAngle <= 270)
        {
            if(plane.planeAngle != 270)
            {
                plane.planeAngle++;
            }
        } else
        {
            plane.planeAngle --;
        }
        plane.position.x += (cosf(DegToRad(plane.planeAngle)) * plane.velocity.x);
        plane.position.y -= (sinf(DegToRad(plane.planeAngle)) * (plane.velocity.x));
        x = x + .01;

    }
    if(angle - plane.planeAngle <= 5 && angle - plane.planeAngle >= 0)
    {
        if(plane.dur > std::chrono::seconds(0))
        {
            if(!plane.isShooting)
            {
                plane.newStart = std::chrono::system_clock::now();
                // std::cout << "Started" << '\n';
                bulletsInAir.emplace_back(Bullet(plane.position, plane.planeAngle, plane));
                plane.isShooting = true;
            } else
            {
                bulletsInAir.emplace_back(Bullet(plane.position, plane.planeAngle, plane));
                plane.newEnd = std::chrono::system_clock::now();
                plane.dur = plane.dur - (std::chrono::system_clock::now() - plane.newStart);
                bulletCount++;

            }
        }
    } else 
    {
        if(plane.isShooting)
        {
            if(std::chrono::system_clock::now() - plane.newStart > std::chrono::seconds(1)) {
                // std::cout << "Do you stop shooting? " << '\n';
                plane.isShooting = false;
                plane.newEnd = std::chrono::system_clock::now();
                plane.init = true;
            } else {
                // std::cout << "Do you go here often" << '\n';
            }
        }
        if(plane.dur <= std::chrono::seconds(5) && plane.init)//This used to be if(plane.dur <= std::chrono::seconds(5) && initialized)??  dunno what that's about
        {
            plane.dur = plane.dur + (std::chrono::system_clock::now() - plane.newEnd);
        }  
    }

}

int counter2 = 0;



Vector2 RotateCircle(Vector2 coords, Vector2 origin, float angle) {
    // Vector2 originPos = {
    //     origin.x,
    //     origin.y
    // };
    float angle2 = (angle * 3.14)/180;
    
    double s = sinf(angle2);
    double c = cosf(angle2);

    // translate point back to origin
    Vector2 rotated = {
    rotated.x = coords.x - origin.x,
    rotated.y = coords.y - origin.y,
    };

    // Vector2 rotated = {
    // rotated.x = coords.x,
    // rotated.y = coords.y,
    // };

    // rotate point
    double x_new = rotated.x * c - rotated.y * s;
    double y_new = rotated.x * s + rotated.y * c;

    // translate point back
    rotated.x = x_new + origin.x;
    rotated.y = y_new + origin.y;

    return rotated;
}

static int counterr = 0;

void DrawGame()
{
    BeginDrawing();

        ClearBackground(RAYWHITE);

        Rectangle r = {
            200, 
            200,
            20,
            20
            };
            counterr++;

        Rectangle source = {
            0.0f,
            0.0f,
            (float)texture.width,
            (float)texture.height
        };

        // Define where to draw it on screen and how big
        Rectangle dest = {
            myPlane.position.x, 
            myPlane.position.y,
            texture.width * .1f,
            texture.height * .1f
        };


        Vector2 origin = {
            dest.width/2,
            dest.height/2
        };

        if (!gameOver)
        {
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, {135, 206, 255, 255});
            Rectangle grad = {
                0,
                0,
                SCREEN_WIDTH,
                SCREEN_HEIGHT
            };
            Color semiTransparentRed = {255, 0, 0, 128};

            Color skyBlue = {135, 206, 255, 255};
                
                // Edges
            if(myPlane.isCrashed) {
                DrawRectangleGradientV(50, 0, SCREEN_WIDTH-100, 50, RED, skyBlue);
                DrawRectangleGradientV(50, SCREEN_HEIGHT - 50, SCREEN_WIDTH-100, 50, skyBlue, RED);
                DrawRectangleGradientH(0, 50, 50, SCREEN_HEIGHT-100, RED, skyBlue);
                DrawRectangleGradientH(SCREEN_WIDTH - 50, 50, 50, SCREEN_HEIGHT-100, skyBlue, RED);
                
                // Corners
                DrawRectangleGradientEx({0, 0, 50, 50}, RED, RED, skyBlue, RED);
                DrawRectangleGradientEx({0, SCREEN_HEIGHT-50, 50, 50}, RED, RED, RED, skyBlue);
                DrawRectangleGradientEx({SCREEN_WIDTH-50, 0, 50, 50}, RED, skyBlue, RED, RED);
                DrawRectangleGradientEx({SCREEN_WIDTH-50, SCREEN_HEIGHT-50, 50, 50}, skyBlue, RED, RED, RED);
            }

            std::string text = "Score: " + std::to_string(score);

            DrawText(text.c_str(), 20, 20, 20, BLACK); 
            DrawFPS(20, 60);

            
            text = "Health: " + std::to_string(myPlane.health);
            DrawText(text.c_str(), 1800, 20, 20, BLACK); 
            text = "Bullets: " + std::to_string(myPlane.dur.count());
            DrawText(text.c_str(), 1800, 120, 20, BLACK);

            for(Plane loopEP : enemyPlanes) {
                if(loopEP.health > 0) {
                    text = "EnemyHealth: " + std::to_string(loopEP.health);
                    DrawText(text.c_str(), 2000, 20, 20, BLACK); 
                    text = "EnemyBullets: " + std::to_string(loopEP.dur.count());
                    DrawText(text.c_str(), 2000, 120, 20, BLACK); 

                }
            }
  


            text = "shoot pew pew";
            // DrawRectangleRec((Rectangle){dest.x, dest.y, dest.width, dest.height}, RED);
            // DrawRectangleLines(dest.x - (dest.width/2), dest.y-(dest.height/2), dest.width, dest.height, RED);
            // DrawRectanglePro(dest, origin, -myPlane.planeAngle, GREEN);
            if(isDetecting)
                DrawRectangle(200, 200, 1000, 200, RED);
            if(myPlane.planeAngle <= 90 || myPlane.planeAngle > 270)
                DrawTexturePro(texture, source, dest, origin, -myPlane.planeAngle, WHITE);  // Draw a Texture2D with extended parameters
            else
            {           
                DrawTexturePro(textureFlip3, source, dest, origin, -myPlane.planeAngle, WHITE);
            }
            if(myPlane.isCrashed) {
                smokeInAir.emplace_back(SmokeParticle(myPlane.position, std::chrono::system_clock::now()));
            }
            int x = 0;

            for(Plane& ep : enemyPlanes)
            {
                // DrawRectangleRec(planeImages[x], RED);
                // DrawRectangleLines(planeImages[x].x - (planeImages[x].width/2), planeImages[x].y -(planeImages[x].height/2), planeImages[x].width, planeImages[x].height, RED);
                // DrawRectanglePro(planeImages[x], origin, -ep.planeAngle, GREEN);
                planeImages[x].x = ep.position.x;
                planeImages[x].y = ep.position.y;
                if(ep.planeAngle <= 90 || ep.planeAngle > 270) {
                    DrawTexturePro(texture, source, planeImages[x], origin, -ep.planeAngle, PINK);  // Draw a Texture2D with extended parameters
                }
                else
                {           
                    DrawTexturePro(textureFlip3, source, planeImages[x], origin, -ep.planeAngle, PINK);
                }
                if(ep.isCrashed) {
                    smokeInAir.emplace_back(SmokeParticle(ep.position, std::chrono::system_clock::now()));
                }
                x++;
                text = std::to_string(static_cast<int>(ep.dur.count()));
            }


            DrawText(text.c_str(), 200, 20, 20, BLACK); 
            if (pause) DrawText("GAME PAUSED", SCREEN_WIDTH/2 - MeasureText("GAME PAUSED", 40)/2, SCREEN_HEIGHT/2 - 40, 40, GRAY);
            text = std::to_string(angle);
            DrawText(text.c_str(), 400, 20, 20, BLACK); 
            text = std::to_string(myPlane.planeAngle);
            DrawText(text.c_str(), 600, 20, 20, BLACK); 
            for (Bullet bullet : bulletsInAir) {
                if(!bullet.isDone)
                {
                    int y = 0;
                    for(Plane& ep : enemyPlanes)
                    {
                        if(CheckCollisionCircles(myPlane.position, myPlane.planeImage.height, ep.position, ep.planeImage.height)) {
                            ep.isCrashed = true;
                            myPlane.isCrashed = true;
                            score++;
                        }
                        if(CheckCollisionCircleRec(RotateCircle(bullet.position, ep.position, myPlane.planeAngle), 5.0, planeImages[y]) && (bullet.shotBy.index != ep.index))
                        {
                            ep.health--;
                            if(bullet.shotBy.him && !ep.isCrashed && ep.health <= 0)
                            {
                                if(!myPlane.isCrashed) {
                                    if(myPlane.health >=15){
                                        myPlane.health = 20;
                                    } else {
                                        myPlane.health += 5;
                                    }
                                }
                                score++;
                                ep.isCrashed = true;
                            }
                            text = "HITTHETARGET";
                            DrawText(text.c_str(), 200, 200, 20, BLACK);
                            bullet.isDone = true;

                            text = std::to_string(planes->planeAngle);
                        }
                        y++;
                    }
                    Vector2 adjusted = RotateCircle(bullet.position, myPlane.position, myPlane.planeAngle);
                    // if(CheckCollisionCircleRec(adjusted, 5.0, dest)) {
                    //     std::cout << dest.x << " " << dest.y << '\n';
                    //     std::cout << adjusted.x << " " << adjusted.y << '\n';
                    //     std::cout << bullet.position.x << " " << bullet.position.y << '\n';
                    //     gameOver = true;
                    //     break;
                    // }
                    if(CheckCollisionCircleRec(RotateCircle(bullet.position, myPlane.position, myPlane.planeAngle), 5.0, dest) && (bullet.shotBy.index != myPlane.index))
                    {
                        counter2++;
                        //std::cout << "ok " << bullet.shotBy.name << " " << myPlane.name << '\n';
                        myPlane.health--;
                        if(bullet.isDone)
                        {
                            gameOver = true;
                        }

                        if(myPlane.health <= 0)
                        {
                            myPlane.isCrashed = true;
                        }
                        // myPlane.isCrashed = true;
                        bullet.isDone = true;
                    }
                    DrawCircleV(bullet.position, 3.0f, GRAY);
                    // DrawCircleV(RotateCircle(bullet.position, myPlane.position, myPlane.planeAngle), 3.0f, PINK);
                }
            }
            
            int spCount = 0;
            for(SmokeParticle sp : smokeInAir) {
                if(std::chrono::system_clock::now() - sp.start >= std::chrono::seconds(5)) {
                    // std::time_t now_time_t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                    // std::tm now_tm = *std::localtime(&now_time_t);
                    // std::cout << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S") << std::endl;
                    // std::cout << "It worked tho" << '\n';
                    // std::time_t now_time_t2 = std::chrono::system_clock::to_time_t(sp.start);
                    // std::tm now_tm2 = *std::localtime(&now_time_t2);
                    // std::cout << std::put_time(&now_tm2, "%Y-%m-%d %H:%M:%S") << std::endl;
                    smokeInAir.erase(smokeInAir.begin() + spCount);
                } else {
                    // if(spCount != 0) {
                    //     break;
                    // }
                    // std::time_t now_time_t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                    // std::tm now_tm = *std::localtime(&now_time_t);
                    // std::cout << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S") << std::endl;
                    // std::cout << "Break" << '\n';
                    // std::time_t now_time_t2 = std::chrono::system_clock::to_time_t(sp.start);
                    // std::tm now_tm2 = *std::localtime(&now_time_t2);
                    // std::cout << std::put_time(&now_tm2, "%Y-%m-%d %H:%M:%S") << std::endl;

                }
                if(std::chrono::system_clock::now() - sp.start >= std::chrono::seconds(5)) {
                    // std::cout << "bigger than five ";
                }

                std::chrono::duration<float> duration = std::chrono::system_clock::now() - sp.start;
                // float ouch = (std::chrono::system_clock::now() - sp.start).count();
                float ouch = duration.count();
                unsigned char transparent = 128.0f - (128.0f * (ouch / 5));
                Color tempt = {28, 28, 28, transparent};
                if(spCount == 0) {
                    // std::cout << ouch << '\n';
                }
                DrawCircle(sp.position.x, sp.position.y, 5, tempt);
                spCount++;
            }
            //text = myPlane.dur;
            //text = "maybe";
            text = std::to_string(myPlane.health);
            DrawText(text.c_str(), 800, 20, 20, BLACK); 
        }
        else 
        {
            DrawText("PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth()/2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20)/2, GetScreenHeight()/2 - 50, 20, GRAY);
        }
    EndDrawing();
}



          