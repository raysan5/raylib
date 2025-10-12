/*******************************************************************************************
*
*   raylib [shapes] example - simple particles
*
*   Example complexity rating: [★☆☆☆] 1/4
*
*   Example originally created with raylib 5.6, last time updated with raylib 5.6
*
*   Example contributed by Aanjishnu Bhattacharyya (@nimcompoo-04)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 Aanjishnu Bhattacharyya (@nimcompoo-04)
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"        // For Vector2Scale and Vector2Add

#include <stdio.h>
#include <math.h>

#define MAX_PARTICLES 512

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct Particle {
    Vector2 position;       // Position of the particle on screen
    Vector2 velocity;       // Velocity of the particle
    float radius;           // Radius of the circular particle
    Color color;            // Color of the particle
    float life_time;        // Total time particle will be alive
    float elapsed_time;     // Time the particle has been alive
} Particle;


//----------------------------------------------------------------------------------
// Program main entry point
//----------------------------------------------------------------------------------
int main(void)
{
    //----------------------------------------------------------------------------------
    // Initialization

    const int screenWidth = 800;
    const int screenHeight = 450;

    const float gravity = 150;                 // How fast the particles fall
    const float emission_rate = 80;            // Number of particles created each second

    float elapsed_time = 0;                    // time passed from last particle emission
    float emission_time = 1. / emission_rate;  // time passed between each particle emitted

    Particle particles[MAX_PARTICLES] = {0};   // Particles that emitter has emitted
    int particle_count = 0;                    // Number of particles emmitted yet

    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - simple particles");

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //----------------------------------------------------------------------------------

    //--------------------------------------------------------------------------------------
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        float dt = GetFrameTime();

        //--------------------------------------------------------------------------------------
        // Update

        // Updating states of all the particles
        for(int i = 0; i < particle_count; i++)
        {
            // Updating time that the particle has been alive for
            particles[i].elapsed_time += dt;

            // Checking if the time they are alive is greater than their supposed lifetime
            if(particles[i].elapsed_time > particles[i].life_time)
            {
                // Swapping out the last particle with the particle to be removed
                Particle temp = particles[i];
                particles[i] = particles[particle_count - 1];
                particles[particle_count - 1] = temp;

                // Removing the last particle which is now the
                particle_count--;
            }

            // If the particle is alive we update the particle
            else
            {
                // Creating a copy for clarity
                Particle *this = &particles[i];

                // Adding a small vertical velocity
                this->velocity.y += gravity * dt;
                this->position = Vector2Add(this->position, Vector2Scale(this->velocity, dt));

                // We also reduce the opacity of the color each frame
                this->color.a = 255 * (1 - this->elapsed_time / this->life_time);
            }
        }

        // We check if enough time has passed, if so we add new particle
        if(elapsed_time >= emission_time && particle_count < MAX_PARTICLES)
        {
            float degrees = GetRandomValue(180, 360);   // a random angle towards the top
            float angle = degrees / 180 * PI;           // converting to radians
            float vel = GetRandomValue(0, 1000)/10.;    // a random magnitude for the velocity, value between [0, 2]

            Particle new = {
                .position = GetMousePosition(),     // Generating new particles at the location of the mouse
                .velocity = {vel * cos(angle),
                             vel * sin(angle)},     // Throwing the particle at a random direction
                .radius = 6,                       // Radius of the particle generated
                .color = WHITE,
                .life_time = 4,                     // the particle will be alive for 3 seconds
                .elapsed_time = 0                   // the particle just spawned
            };

            particles[particle_count] = new;      // adding the particle to the list
            particle_count++;

            elapsed_time = 0;       // Resetting the timer
        }
        elapsed_time += dt;         // incrementing the timer
        //----------------------------------------------------------------------------------

        //----------------------------------------------------------------------------------
        // Draw
        BeginDrawing();
            ClearBackground(BLACK);

            // Drawing all the particles
            for(int i = 0; i < particle_count; i++)
            {
                DrawCircleV(particles[i].position, particles[i].radius, particles[i].color);
            }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
