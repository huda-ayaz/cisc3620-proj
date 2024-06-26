// CISC 3620 Midterm: The Shape of Love
// Authors: Huda Ayaz & Royta Iftakher

#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/sdl_mixer.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "vector.h"
#include "mesh.h"
#include "triangle.h"
#include "matrix.h"

SDL_Renderer *renderer;
SDL_Window *window;
SDL_Texture *texture;
uint32_t *color_buffer;
bool is_running = false;
int window_width = 800;
int window_height = 600;
int originX = 400;
int originY = 300;
void draw_pixel(int x, int y, uint32_t color);
void draw_line(int x0, int y0, int x1, int y1, uint32_t color);
void draw_rectangle(int start_x, int start_y, int rectangle_width, int rectangle_height, uint32_t color);
void draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color);
void draw_filled_triangle(int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color);
bool initialize_windowing_system();
void clean_up_windowing_system();
void project_cube();
void project_pyramid();

int scaling_factor = 500;
int previous_frame_time = 0;

#define FPS 30
#define frame_target_time (scaling_factor / FPS)

vec3_t camera_position = {0, 0, -5};
triangle_t triangles_to_render[1000];
triangle_t p_to_render[1000];

void process_keyboard_input();
void run_render_pipeline();
void set_up_memory_buffers();
void clear_color_buffer(uint32_t color);

void build_cube_model();
void scale_shape();

vec2_t perspective_project_point(vec3_t point_3d);

int t_cnt = 0;

// matrix incorporation
vec3_t cube_scale = {.x = 1, .y = 1, .z = 1};
vec3_t cube_rotation = {.x = 0, .y = 0, .z = 0};
vec3_t cube_translate = {.x = 1, .y = 1, .z = 7};

// pyramid incorporation
vec3_t p_scale = {.x = 1, .y = 1, .z = 1};
vec3_t p_rotation = {.x = 0, .y = 0, .z = 0};
vec3_t p_translate = {.x = 1, .y = 1, .z = 7};

mat4_t scale_matrix;
mat4_t rotation_matrix_x; 
mat4_t rotation_matrix_y;
mat4_t rotation_matrix_z;
mat4_t translate_matrix;

// to keep track of animation time
Uint32 animation_start_time = 0; // SDL time when animation starts
float animation_duration = 5.0f; // Total duration of the animation in seconds
float elapsed_time = 0.0f;

// setting up the parameters for main triangle character
vec2_t triangle_a = {.x = 300, .y = 400}; // side a
vec2_t triangle_b = {.x = 400, .y = 200}; // side b
vec2_t triangle_c = {.x = 500, .y = 400}; // side c
// creating a vector 3 that can hold values of vec2_t's
vec3_t_of_vec2_t lead_triangle = {.a = {.x = 0, .y = 0}, .b = {.x = 0, .y = 0}, .c = {.x = 0, .y = 0}};

// Specific Act Movement Cues
int act2_initial_travel = 0;
int act2_vertical_movement = 0;
int act2_horizontal_movement = 0;
int act2_final_vertical_movement = 0;
int act2_final_horizontal_movement = 0;
int act2_triangle_leaves = 0;

void update_state();

// vvvvvvvvvvvv BASIC SETUP FILES vvvvvvvvvvvv//

void run_render_pipeline()
{
    // SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_UpdateTexture(texture, NULL, color_buffer, window_width * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void process_keyboard_input()
{
    SDL_Event event;
    SDL_PollEvent(&event);
    switch (event.type)
    {
    case SDL_QUIT:
        is_running = false;
        break;
    case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_ESCAPE)
            is_running = false;
        break;
    }
}

bool initialize_windowing_system()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        fprintf(stderr, "SDL_Init failed");
        return false;
    }

    window = SDL_CreateWindow(NULL, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, SDL_WINDOW_SHOWN);
    if (!window)
    {
        fprintf(stderr, "SDL_CreateWindow failed");
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer)
    {
        fprintf(stderr, "SDL_CreateRenderer failed");
        return false;
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, window_width, window_height);
    if (!texture)
    {
        fprintf(stderr, "SDL_CreateTexture failed");
        return false;
    }

    if (Mix_Init(0) != 0)
    {
        fprintf(stderr, "Mix_Init() Failed\n");
    }
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 128);
    Mix_Music *music = Mix_LoadMUS("audio/djo.wav");
    if (!music)
    {
        fprintf(stderr, "MUSIC NOT PLAYING!\n");
    }
    Mix_PlayMusic(music, 0);

    return true;
}

void clean_up_windowing_system()
{
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_Quit();
    SDL_Quit();
}

void set_up_memory_buffers()
{
    color_buffer = (uint32_t *)malloc(window_width * window_height * sizeof(uint32_t));
}

void clear_color_buffer(uint32_t color)
{
    for (int y = 0; y < window_height; y++)
    {
        for (int x = 0; x < window_width; x++)
        {
            color_buffer[(y * window_width) + x] = color;
        }
    }
}

//^^^^^^^^^^^^ BASIC SETUP FILES ^^^^^^^^^^^^//

void draw_rectangle(int start_x, int start_y, int rectangle_width, int rectangle_height, uint32_t color)
{
    for (int y = start_y; y < start_y + rectangle_height; y++)
    {
        for (int x = start_x; x < start_x + rectangle_width; x++)
        {
            draw_pixel(x, y, color);
        }
    }
}

void draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color)
{
    draw_line(x1, y1, x2, y2, color);
    draw_line(x2, y3, x3, y3, color);
    draw_line(x3, y3, x1, y1, color);
}

// Note: Only works for triangles where the base is perpendicular to the height.
void draw_filled_triangle(int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color)
{
    int base = x3 - x1;
    for (int i = 0; i < base; i++)
    {
        draw_triangle(x1 + i, y1, x2, y2, x3 - i, y3, color);
    }
}

vec2_t perspective_project_point(vec3_t point_3d)
{
    point_3d.x -= camera_position.x;
    point_3d.y -= camera_position.y;
    point_3d.z -= camera_position.z;

    vec2_t projected_point = {
        .x = scaling_factor * (point_3d.x / point_3d.z),
        .y = scaling_factor * (point_3d.y / point_3d.z)};
    return projected_point;
}

// vvvvvvvvvvvvvvvvvvvvvv 3D MESH vvvvvvvvvvvvvvvvvvvvvv//

void draw_pixel(int x, int y, uint32_t color)
{
    // confirm pixel is in the visible window space
    if (x < window_width && y < window_height)
        color_buffer[(y * window_width) + x] = color;
}

void draw_line(int x0, int y0, int x1, int y1, uint32_t color)
{
    int delta_x = x1 - x0;
    int delta_y = y1 - y0;

    int steps = abs(delta_x) > abs(delta_y) ? abs(delta_x) : abs(delta_y);

    float x_inc = delta_x / (float)steps;
    float y_inc = delta_y / (float)steps;

    float x = x0;
    float y = y0;

    for (int i = 0; i <= steps; i++)
    {
        draw_pixel(x, y, color);
        x += x_inc;
        y += y_inc;
    }
}

// vvvvvvvvvvvvvvvvvvvvvv DISPLAY vvvvvvvvvvvvvvvvvvvvvv//

void project_cube()
{

    // CUBE PROJECT MODEL
    for (int i = 0; i < N_MESH_FACES; i++)
    { // for each triangle
        face_t mesh_face = mesh_faces[i];
        vec3_t face_vertices[3];                           // create an array of vertices
        face_vertices[0] = mesh_vertices[mesh_face.a - 1]; // place the correct vertices into array
        face_vertices[1] = mesh_vertices[mesh_face.b - 1];
        face_vertices[2] = mesh_vertices[mesh_face.c - 1];

        vec3_t transformed_vertices[3];

        for (int j = 0; j < 3; j++)
        { // loop through vertices, apply transformations and projections
            vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);

            transformed_vertex = mat4_mul_vec4(scale_matrix, transformed_vertex);
            transformed_vertex = mat4_mul_vec4(rotation_matrix_x, transformed_vertex);
            transformed_vertex = mat4_mul_vec4(rotation_matrix_y, transformed_vertex);
            transformed_vertex = mat4_mul_vec4(rotation_matrix_z, transformed_vertex);
            transformed_vertex = mat4_mul_vec4(translate_matrix, transformed_vertex);

            transformed_vertices[j] = vec3_from_vec4(transformed_vertex);
        }

        /* BACKFACE CULLING  */
        // vec3_t vertex_a = transformed_vertices[0]; /*    A    */
        // vec3_t vertex_b = transformed_vertices[1]; /*   / \   */
        // vec3_t vertex_c = transformed_vertices[2]; /*  C---B  */

        // // Get the vector subtractiono of B-A and C-A
        // vec3_t vector_ab = vec3_subtract(vertex_b, vertex_a);
        // vec3_t vector_ac = vec3_subtract(vertex_c, vertex_a);

        // // Compute the face normal (using corss product to find perpendiculiar vector)
        // vec3_t normal = vec3_cross(vector_ab, vector_ac);

        // // Find the vector between a point in the triangle and the camera origin.
        // vec3_t camera_ray = vec3_subtract(camera_position, vertex_a);

        // // Calculate how aligned the camera ray is with the face normal (using the dot product)
        // float dot_normal_camera = vec3_dot(camera_ray, normal);

        // // Bypass triangles that are looking away from the camera by continuing to next face in main loop
        // if (dot_normal_camera < 0)
        // {
        //     continue;
        // }

        /* PROJECTION */

        triangle_t projected_triangle;

        // Sub Loop, project the vertices of curent face
        for (int j = 0; j < 3; j++)
        {
            vec2_t projected_point = perspective_project_point(transformed_vertices[j]);

            projected_triangle.points[j] = projected_point;
        }
        // save the projected triangle
        triangles_to_render[t_cnt++] = projected_triangle;
    }
}

void project_pyramid()
{
    // PYRAMID PROJECT MODEL
    for (int i = 0; i < P_MESH_FACES; i++)
    { // for each triangle
        face_t p_mesh_face = p_mesh_faces[i];
        vec3_t p_face_vertices[3];                               // create an array of vertices
        p_face_vertices[0] = p_mesh_vertices[p_mesh_face.a - 1]; // place the correct vertices into array
        p_face_vertices[1] = p_mesh_vertices[p_mesh_face.b - 1];
        p_face_vertices[2] = p_mesh_vertices[p_mesh_face.c - 1];

        vec3_t transformed_vertices[3];

        for (int j = 0; j < 3; j++)
        { // loop through vertices, apply transformations and projections
            vec4_t transformed_vertex = vec4_from_vec3(p_face_vertices[j]);

            transformed_vertex = mat4_mul_vec4(scale_matrix, transformed_vertex);
            transformed_vertex = mat4_mul_vec4(rotation_matrix_x, transformed_vertex);
            transformed_vertex = mat4_mul_vec4(rotation_matrix_y, transformed_vertex);
            transformed_vertex = mat4_mul_vec4(rotation_matrix_z, transformed_vertex);
            transformed_vertex = mat4_mul_vec4(translate_matrix, transformed_vertex);

            transformed_vertices[j] = vec3_from_vec4(transformed_vertex);
        }

        /* BACKFACE CULLING  */
        // vec3_t vertex_a = transformed_vertices[0];
        // vec3_t vertex_b = transformed_vertices[1];
        // vec3_t vertex_c = transformed_vertices[2];

        // // Get the vector subtractiono of B-A and C-A
        // vec3_t vector_ab = vec3_subtract(vertex_b, vertex_a);
        // vec3_t vector_ac = vec3_subtract(vertex_c, vertex_a);

        // // Compute the face normal (using corss product to find perpendiculiar vector)
        // vec3_t normal = vec3_cross(vector_ab, vector_ac);

        // // Find the vector between a point in the triangle and the camera origin.
        // vec3_t camera_ray = vec3_subtract(camera_position, vertex_a);

        // // Calculate how aligned the camera ray is with the face normal (using the dot product)
        // float dot_normal_camera = vec3_dot(camera_ray, normal);

        // // Bypass triangles that are looking away from the camera by continuing to next face in main loop
        // if (dot_normal_camera < 0)
        // {
        //     continue;
        // }

        /* PROJECTION */

        triangle_t projected_triangle;

        // Sub Loop, project the vertices of curent face
        for (int j = 0; j < 3; j++)
        {
            vec2_t projected_point = perspective_project_point(transformed_vertices[j]);
            projected_triangle.points[j] = projected_point;
        }
        // save the projected triangle
        triangles_to_render[t_cnt++] = projected_triangle;
    }
}

//^^^^^^^^^^^^ 3D MESH ^^^^^^^^^^^^//

void update_state()
{
    clear_color_buffer(0xC3B1E1); // light purple

    Uint32 current_time = SDL_GetTicks();
    printf("Current SDL Ticks: %u\n", current_time);

    elapsed_time = (float)(current_time - animation_start_time) / 1000.0f; // Convert to seconds

    // ---------------------------------ACT I: SCENE 1 - Introducing Triangle--------------------------------- //

    if (elapsed_time >= 0.0f && elapsed_time <= 1.0f)
    {
        draw_filled_triangle(lead_triangle.a.x - 200, lead_triangle.a.y, lead_triangle.b.x - 200, lead_triangle.b.y, lead_triangle.c.x - 200, lead_triangle.c.y, 0xffea00); // triangle on the left of the screen
        draw_filled_triangle(lead_triangle.a.x + 200, lead_triangle.a.y, lead_triangle.b.x + 200, lead_triangle.b.y, lead_triangle.c.x + 200, lead_triangle.c.y, 0xffea00); // triangle on the right of the screen
        draw_filled_triangle(lead_triangle.a.x, lead_triangle.a.y - 180, lead_triangle.b.x, lead_triangle.b.y - 180, lead_triangle.c.x, lead_triangle.c.y - 180, 0xffea00); // triangle on the top of the screen
        draw_filled_triangle(lead_triangle.a.x, lead_triangle.a.y + 180, lead_triangle.b.x, lead_triangle.b.y + 180, lead_triangle.c.x, lead_triangle.c.y + 180, 0xffea00); // triangle on the bottom of the screen
    }
    if (elapsed_time >= 1.0f && elapsed_time <= 6.0f)
    {
        // Yellow triangle pops up on the first beat

        if ((elapsed_time >= 1.0f && elapsed_time <= 1.5f) || (elapsed_time >= 4.0f && elapsed_time <= 4.5f))
        {
            draw_filled_triangle(lead_triangle.a.x, lead_triangle.a.y, lead_triangle.b.x, lead_triangle.b.y, lead_triangle.c.x, lead_triangle.c.y, 0xffea00);
        }
        else if ((elapsed_time >= 1.5f && elapsed_time <= 2.0f) || (elapsed_time >= 4.5f && elapsed_time <= 5.0f))
        {
            draw_filled_triangle(lead_triangle.a.x, lead_triangle.a.y, lead_triangle.b.x, lead_triangle.b.y, lead_triangle.c.x, lead_triangle.c.y, 0xffd500);
        }
        else if ((elapsed_time >= 2.0f && elapsed_time <= 2.5f) || (elapsed_time >= 5.0f && elapsed_time <= 5.5f))
        {
            draw_filled_triangle(lead_triangle.a.x, lead_triangle.a.y, lead_triangle.b.x, lead_triangle.b.y, lead_triangle.c.x, lead_triangle.c.y, 0xffc000);
        }
        else if ((elapsed_time >= 2.5f && elapsed_time <= 3.0f) || (elapsed_time >= 5.5f && elapsed_time <= 6.0f))
        {
            draw_filled_triangle(lead_triangle.a.x, lead_triangle.a.y, lead_triangle.b.x, lead_triangle.b.y, lead_triangle.c.x, lead_triangle.c.y, 0xffd500);
        }
        else if ((elapsed_time >= 3.0f && elapsed_time <= 3.5f) || (elapsed_time >= 6.5f && elapsed_time <= 7.0f))
        {
            draw_filled_triangle(lead_triangle.a.x, lead_triangle.a.y, lead_triangle.b.x, lead_triangle.b.y, lead_triangle.c.x, lead_triangle.c.y, 0xffc000);
        }
        else if ((elapsed_time >= 3.5f && elapsed_time <= 4.0f) || (elapsed_time >= 7.5f && elapsed_time <= 8.0f))
        {
            draw_filled_triangle(lead_triangle.a.x, lead_triangle.a.y, lead_triangle.b.x, lead_triangle.b.y, lead_triangle.c.x, lead_triangle.c.y, 0xffd500);
        }
        else if ((elapsed_time >= 8.0f && elapsed_time <= 10.0f))
        {
            draw_filled_triangle(lead_triangle.a.x, lead_triangle.a.y, lead_triangle.b.x, lead_triangle.b.y, lead_triangle.c.x, lead_triangle.c.y, 0xffea00);
        }
    }

    // ---------------------------------ACT I: SCENE 2 - Introducing Square--------------------------------- //

    // Pop in grid of blue squares
    if (elapsed_time >= 6.0f && elapsed_time <= 10.0f)
    {
        uint32_t color = 0x0071b6; // pastel blue

        int rows = 4;
        int columns = 6;

        int square_size = window_width / (columns + 1);
        int spacing = square_size / 5;

        int total_width = columns * (square_size + spacing);
        int total_height = rows * (square_size + spacing) + 20;

        int start_x = (window_width - total_width) + 20;
        int start_y = (window_height - total_height);

        float pop_wait = 4.0f / (rows * columns);

        int current_square = (int)((elapsed_time - 6.0f) / pop_wait);

        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < columns; j++)
            {
                if ((i * columns + j) <= current_square)
                {
                    int x = start_x + j * (square_size + spacing);
                    int y = start_y + i * (square_size + spacing);
                    draw_rectangle(x, y, square_size, square_size, color);
                }
            }
        }
    }

    if (elapsed_time >= 10.0f && elapsed_time < 10.5f)
    {
        draw_rectangle((window_width - 300) / 2, (window_height - 300) / 2, 300, 300, 0x0071b6);
    }

    if (elapsed_time >= 10.5f && elapsed_time < 11.0f)
    {
        draw_rectangle((window_width - 300) / 2, (window_height - 300) / 2, 300, 300, 0x0081d0);
    }

    if (elapsed_time >= 11.0f && elapsed_time < 11.5f)
    {
        draw_rectangle((window_width - 300) / 2, (window_height - 300) / 2, 300, 300, 0x0071b6);
    }

    if (elapsed_time >= 11.5f && elapsed_time < 12.0f)
    {
        draw_rectangle((window_width - 300) / 2, (window_height - 300) / 2, 300, 300, 0x00619d);
    }

    if (elapsed_time >= 12.0f && elapsed_time < 12.5f)
    {
        draw_rectangle((window_width - 300) / 2, (window_height - 300) / 2, 300, 300, 0x0071b6);
    }

    if (elapsed_time >= 12.5f && elapsed_time < 13.0f)
    {
        draw_rectangle((window_width - 300) / 2, (window_height - 300) / 2, 300, 300, 0x0081d0);
    }

    if (elapsed_time >= 13.0f && elapsed_time < 13.5f)
    {
        draw_rectangle((window_width - 300) / 2, (window_height - 300) / 2, 300, 300, 0x0091e9);
    }

    if (elapsed_time >= 13.5f && elapsed_time < 15.0f)
    {
        draw_rectangle((window_width - 300) / 2, (window_height - 300) / 2, 300, 300, 0x0071b6);
    }

    // ---------------------------------ACT I: SCENE 3 - Introducing Cubes--------------------------------- //

    if (elapsed_time >= 15.0f && elapsed_time < 20.0f)
    {
        // makes sure progress is at 100
        // float progress = elapsed_time / animation_duration;

        // matrix incorporation
        scale_matrix = mat4_make_scale(cube_scale.x, cube_scale.y, cube_scale.z);
        rotation_matrix_x = mat4_make_rotation_x(cube_rotation.x); // pass the angle as float
        rotation_matrix_y = mat4_make_rotation_y(cube_rotation.y);
        rotation_matrix_z = mat4_make_rotation_z(cube_rotation.z);
        translate_matrix = mat4_make_translate(cube_translate.x, cube_translate.y, cube_translate.z);

        cube_rotation.x += .01;
        cube_rotation.y += .01;

        if (elapsed_time >= 15.0f && elapsed_time <= 17.5f)
        {
            cube_scale.x += .01;
            cube_scale.y += .01;
        }
        else if (elapsed_time >= 17.5f && elapsed_time <= 20.0f)
        {
            cube_scale.x -= .01;
            cube_scale.y -= .01;
        }
        project_cube();

        for (int i = 0; i < t_cnt; i++)
        {
            triangle_t triangle = triangles_to_render[i];
            for (int j = 0; j < 3; j++)
            {
                // Cube middle
                draw_line(triangle.points[0].x + originX - 25, triangle.points[0].y + originY, triangle.points[1].x + originX - 25, triangle.points[1].y + originY, 0xFFFFFF);
                draw_line(triangle.points[1].x + originX - 25, triangle.points[1].y + originY, triangle.points[2].x + originX - 25, triangle.points[2].y + originY, 0xFFFFFF);
                draw_line(triangle.points[2].x + originX - 25, triangle.points[2].y + originY, triangle.points[0].x + originX - 25, triangle.points[0].y + originY, 0xFFFFFF);

                // Cube bottom right
                draw_line(triangle.points[0].x + originX + 200, triangle.points[0].y + originY + 150, triangle.points[1].x + originX + 200, triangle.points[1].y + originY + 150, 0xFFFFFF);
                draw_line(triangle.points[1].x + originX + 200, triangle.points[1].y + originY + 150, triangle.points[2].x + originX + 200, triangle.points[2].y + originY + 150, 0xFFFFFF);
                draw_line(triangle.points[2].x + originX + 200, triangle.points[2].y + originY + 150, triangle.points[0].x + originX + 200, triangle.points[0].y + originY + 150, 0xFFFFFF);

                // Cube bottom left
                draw_line(triangle.points[0].x + originX - 250, triangle.points[0].y + originY + 150, triangle.points[1].x + originX - 250, triangle.points[1].y + originY + 150, 0xFFFFFF);
                draw_line(triangle.points[1].x + originX - 250, triangle.points[1].y + originY + 150, triangle.points[2].x + originX - 250, triangle.points[2].y + originY + 150, 0xFFFFFF);
                draw_line(triangle.points[2].x + originX - 250, triangle.points[2].y + originY + 150, triangle.points[0].x + originX - 250, triangle.points[0].y + originY + 150, 0xFFFFFF);

                // Cube top left
                draw_line(triangle.points[0].x + originX - 250, triangle.points[0].y + originY - 200, triangle.points[1].x + originX - 250, triangle.points[1].y + originY - 200, 0xFFFFFF);
                draw_line(triangle.points[1].x + originX - 250, triangle.points[1].y + originY - 200, triangle.points[2].x + originX - 250, triangle.points[2].y + originY - 200, 0xFFFFFF);
                draw_line(triangle.points[2].x + originX - 250, triangle.points[2].y + originY - 200, triangle.points[0].x + originX - 250, triangle.points[0].y + originY - 200, 0xFFFFFF);

                // Cube top right
                draw_line(triangle.points[0].x + originX + 200, triangle.points[0].y + originY - 200, triangle.points[1].x + originX + 200, triangle.points[1].y + originY - 200, 0xFFFFFF);
                draw_line(triangle.points[1].x + originX + 200, triangle.points[1].y + originY - 200, triangle.points[2].x + originX + 200, triangle.points[2].y + originY - 200, 0xFFFFFF);
                draw_line(triangle.points[2].x + originX + 200, triangle.points[2].y + originY - 200, triangle.points[0].x + originX + 200, triangle.points[0].y + originY - 200, 0xFFFFFF);
            }
        }
        t_cnt = 0;
    }

    if (elapsed_time >= 20.0f && elapsed_time <= 21.5f)
    {
        clear_color_buffer(0x000000);
        // makes sure progress is at 100
        // float progress = elapsed_time / animation_duration;

        // matrix incorporation
        scale_matrix = mat4_make_scale(cube_scale.x, cube_scale.y, cube_scale.z);
        rotation_matrix_x = mat4_make_rotation_x(cube_rotation.x); // pass the angle as float
        rotation_matrix_y = mat4_make_rotation_y(cube_rotation.y);
        rotation_matrix_z = mat4_make_rotation_z(cube_rotation.z);
        translate_matrix = mat4_make_translate(cube_translate.x, cube_translate.y, cube_translate.z);

        cube_rotation.x += .01;
        cube_rotation.y += .01;

        if (elapsed_time >= 15.0f && elapsed_time <= 17.5f)
        {
            cube_scale.x += .01;
            cube_scale.y += .01;
        }
        else if (elapsed_time >= 17.5f && elapsed_time <= 20.0f)
        {
            cube_scale.x -= .01;
            cube_scale.y -= .01;
        }

        project_cube();

        for (int i = 0; i < t_cnt; i++)
        {
            triangle_t triangle = triangles_to_render[i];
            for (int j = 0; j < 3; j++)
            {
                // Cube middle
                draw_line(triangle.points[0].x + originX - 25, triangle.points[0].y + originY, triangle.points[1].x + originX - 25, triangle.points[1].y + originY, 0xFFFFFF);
                draw_line(triangle.points[1].x + originX - 25, triangle.points[1].y + originY, triangle.points[2].x + originX - 25, triangle.points[2].y + originY, 0xFFFFFF);
                draw_line(triangle.points[2].x + originX - 25, triangle.points[2].y + originY, triangle.points[0].x + originX - 25, triangle.points[0].y + originY, 0xFFFFFF);

                // Cube bottom right
                draw_line(triangle.points[0].x + originX + 200, triangle.points[0].y + originY + 180, triangle.points[1].x + originX + 200, triangle.points[1].y + originY + 180, 0xFFFFFF);
                draw_line(triangle.points[1].x + originX + 200, triangle.points[1].y + originY + 180, triangle.points[2].x + originX + 200, triangle.points[2].y + originY + 180, 0xFFFFFF);
                draw_line(triangle.points[2].x + originX + 200, triangle.points[2].y + originY + 180, triangle.points[0].x + originX + 200, triangle.points[0].y + originY + 180, 0xFFFFFF);

                // Cube bottom left
                draw_line(triangle.points[0].x + originX - 250, triangle.points[0].y + originY + 180, triangle.points[1].x + originX - 250, triangle.points[1].y + originY + 180, 0xFFFFFF);
                draw_line(triangle.points[1].x + originX - 250, triangle.points[1].y + originY + 180, triangle.points[2].x + originX - 250, triangle.points[2].y + originY + 180, 0xFFFFFF);
                draw_line(triangle.points[2].x + originX - 250, triangle.points[2].y + originY + 180, triangle.points[0].x + originX - 250, triangle.points[0].y + originY + 180, 0xFFFFFF);

                // Cube top left
                draw_line(triangle.points[0].x + originX - 250, triangle.points[0].y + originY - 180, triangle.points[1].x + originX - 250, triangle.points[1].y + originY - 180, 0xFFFFFF);
                draw_line(triangle.points[1].x + originX - 250, triangle.points[1].y + originY - 180, triangle.points[2].x + originX - 250, triangle.points[2].y + originY - 180, 0xFFFFFF);
                draw_line(triangle.points[2].x + originX - 250, triangle.points[2].y + originY - 180, triangle.points[0].x + originX - 250, triangle.points[0].y + originY - 180, 0xFFFFFF);

                // Cube top right
                draw_line(triangle.points[0].x + originX + 200, triangle.points[0].y + originY - 180, triangle.points[1].x + originX + 200, triangle.points[1].y + originY - 180, 0xFFFFFF);
                draw_line(triangle.points[1].x + originX + 200, triangle.points[1].y + originY - 180, triangle.points[2].x + originX + 200, triangle.points[2].y + originY - 180, 0xFFFFFF);
                draw_line(triangle.points[2].x + originX + 200, triangle.points[2].y + originY - 180, triangle.points[0].x + originX + 200, triangle.points[0].y + originY - 180, 0xFFFFFF);
            }
        }
        t_cnt = 0;
    }
    if (elapsed_time >= 21.5f && elapsed_time <= 24.0f)
    {
        clear_color_buffer(0xFFFFFF);

        // matrix incorporation
        scale_matrix = mat4_make_scale(cube_scale.x, cube_scale.y, cube_scale.z);
        rotation_matrix_x = mat4_make_rotation_x(cube_rotation.x); // pass the angle as float
        rotation_matrix_y = mat4_make_rotation_y(cube_rotation.y);
        rotation_matrix_z = mat4_make_rotation_z(cube_rotation.z);
        translate_matrix = mat4_make_translate(cube_translate.x, cube_translate.y, cube_translate.z);

        cube_rotation.x += .01;
        cube_rotation.y += .01;

        if (elapsed_time >= 15.0f && elapsed_time <= 17.5f)
        {
            cube_scale.x += .01;
            cube_scale.y += .01;
        }
        else if (elapsed_time >= 17.5f && elapsed_time <= 20.0f)
        {
            cube_scale.x -= .01;
            cube_scale.y -= .01;
        }

        project_cube();

        for (int i = 0; i < t_cnt; i++)
        {
            triangle_t triangle = triangles_to_render[i];
            for (int j = 0; j < 3; j++)
            {
                // Cube middle
                draw_line(triangle.points[0].x + originX - 25, triangle.points[0].y + originY, triangle.points[1].x + originX - 25, triangle.points[1].y + originY, 0x000000);
                draw_line(triangle.points[1].x + originX - 25, triangle.points[1].y + originY, triangle.points[2].x + originX - 25, triangle.points[2].y + originY, 0x000000);
                draw_line(triangle.points[2].x + originX - 25, triangle.points[2].y + originY, triangle.points[0].x + originX - 25, triangle.points[0].y + originY, 0x000000);

                // Cube bottom right
                draw_line(triangle.points[0].x + originX + 200, triangle.points[0].y + originY + 180, triangle.points[1].x + originX + 200, triangle.points[1].y + originY + 180, 0x000000);
                draw_line(triangle.points[1].x + originX + 200, triangle.points[1].y + originY + 180, triangle.points[2].x + originX + 200, triangle.points[2].y + originY + 180, 0x000000);
                draw_line(triangle.points[2].x + originX + 200, triangle.points[2].y + originY + 180, triangle.points[0].x + originX + 200, triangle.points[0].y + originY + 180, 0x000000);

                // Cube bottom left
                draw_line(triangle.points[0].x + originX - 250, triangle.points[0].y + originY + 180, triangle.points[1].x + originX - 250, triangle.points[1].y + originY + 180, 0x000000);
                draw_line(triangle.points[1].x + originX - 250, triangle.points[1].y + originY + 180, triangle.points[2].x + originX - 250, triangle.points[2].y + originY + 180, 0x000000);
                draw_line(triangle.points[2].x + originX - 250, triangle.points[2].y + originY + 180, triangle.points[0].x + originX - 250, triangle.points[0].y + originY + 180, 0x000000);

                // Cube top left
                draw_line(triangle.points[0].x + originX - 250, triangle.points[0].y + originY - 180, triangle.points[1].x + originX - 250, triangle.points[1].y + originY - 180, 0x000000);
                draw_line(triangle.points[1].x + originX - 250, triangle.points[1].y + originY - 180, triangle.points[2].x + originX - 250, triangle.points[2].y + originY - 180, 0x000000);
                draw_line(triangle.points[2].x + originX - 250, triangle.points[2].y + originY - 180, triangle.points[0].x + originX - 250, triangle.points[0].y + originY - 180, 0x000000);

                // Cube top right
                draw_line(triangle.points[0].x + originX + 200, triangle.points[0].y + originY - 180, triangle.points[1].x + originX + 200, triangle.points[1].y + originY - 180, 0x000000);
                draw_line(triangle.points[1].x + originX + 200, triangle.points[1].y + originY - 180, triangle.points[2].x + originX + 200, triangle.points[2].y + originY - 180, 0x000000);
                draw_line(triangle.points[2].x + originX + 200, triangle.points[2].y + originY - 180, triangle.points[0].x + originX + 200, triangle.points[0].y + originY - 180, 0x000000);
            }
        }
        t_cnt = 0;
    }

    if (elapsed_time >= 25.0f && elapsed_time <= 26.0f)
    {
        // matrix incorporation
        scale_matrix = mat4_make_scale(p_scale.x, p_scale.y, p_scale.z);
        rotation_matrix_x = mat4_make_rotation_x(p_rotation.x); // pass the angle as float
        rotation_matrix_y = mat4_make_rotation_y(p_rotation.y);
        rotation_matrix_z = mat4_make_rotation_z(p_rotation.z);
        translate_matrix = mat4_make_translate(p_translate.x, p_translate.y, p_translate.z);

        p_rotation.x += .01;

        p_scale.x += .08;
        p_scale.y += .08;
        p_scale.z += .08;
        p_translate.y += .0036;
        p_translate.x += .06;

        project_pyramid();
        for (int i = 0; i < t_cnt; i++)
        {
            triangle_t triangle = triangles_to_render[i];
            for (int j = 0; j < 3; j++)
            {
                // loop through every triangle then draw every vertex of every triangle
                draw_line(triangle.points[0].x, triangle.points[0].y + window_height / 2, triangle.points[1].x, triangle.points[1].y + window_height / 2, 0xFFFFFF);
                draw_line(triangle.points[1].x, triangle.points[1].y + window_height / 2, triangle.points[2].x, triangle.points[2].y + window_height / 2, 0xFFFFFF);
                draw_line(triangle.points[2].x, triangle.points[2].y + window_height / 2, triangle.points[0].x, triangle.points[0].y + window_height / 2, 0xFFFFFF);
            }
        }
        t_cnt = 0;
    }

    if (elapsed_time >= 26.0f && elapsed_time <= 28.0f)
    {
        // matrix incorporation
        scale_matrix = mat4_make_scale(p_scale.x, p_scale.y, p_scale.z);
        rotation_matrix_x = mat4_make_rotation_x(p_rotation.x); // pass the angle as float
        rotation_matrix_y = mat4_make_rotation_y(p_rotation.y);
        rotation_matrix_z = mat4_make_rotation_z(p_rotation.z);
        translate_matrix = mat4_make_translate(p_translate.x, p_translate.y, p_translate.z);

        p_rotation.x -= .01;

        p_scale.x -= .01;
        p_scale.y -= .01;
        p_scale.z -= .01;
        p_translate.y -= .009;
        p_translate.x -= .03;

        project_pyramid();
        for (int i = 0; i < t_cnt; i++)
        {
            triangle_t triangle = triangles_to_render[i];
            for (int j = 0; j < 3; j++)
            {
                // loop through every triangle then draw every vertex of every triangle
                draw_line(triangle.points[0].x, triangle.points[0].y + window_height / 2, triangle.points[1].x, triangle.points[1].y + window_height / 2, 0xFFFFFF);
                draw_line(triangle.points[1].x, triangle.points[1].y + window_height / 2, triangle.points[2].x, triangle.points[2].y + window_height / 2, 0xFFFFFF);
                draw_line(triangle.points[2].x, triangle.points[2].y + window_height / 2, triangle.points[0].x, triangle.points[0].y + window_height / 2, 0xFFFFFF);
            }
        }
        t_cnt = 0;
        
        
    }

    // ---------------------------------ACT II: SCENE 1 - Square and Triangle Meet--------------------------------- //

    if (elapsed_time >= 28.0f && elapsed_time <= 60.0f)
    {
        // triangle and square meet eachother in the middle from the left and right, respectively
        if (act2_initial_travel <= 200)
        {
            act2_initial_travel++;
        }
        else
        {
            clear_color_buffer(0xffc0cb); // light pink
        }
        draw_filled_triangle(lead_triangle.a.x - 300 + (act2_initial_travel), lead_triangle.a.y, lead_triangle.b.x - 300 + (act2_initial_travel), lead_triangle.b.y, lead_triangle.c.x - 300 + (act2_initial_travel), lead_triangle.c.y, 0xffea00);
        draw_rectangle(((window_width - 300) / 4) + 500 - (act2_initial_travel), ((window_height - 300) / 4) + 175, 150, 150, 0x0071b6);

        if ((elapsed_time >= 33.0f && elapsed_time <= 33.5f) || (elapsed_time >= 35.0f && elapsed_time <= 35.5f))
        {
            draw_filled_triangle(lead_triangle.a.x - 300 + (act2_initial_travel), lead_triangle.a.y, lead_triangle.b.x - 300 + (act2_initial_travel), lead_triangle.b.y, lead_triangle.c.x - 300 + (act2_initial_travel), lead_triangle.c.y, 0xFFd500);
            draw_rectangle(((window_width - 300) / 4) + 500 - (act2_initial_travel), ((window_height - 300) / 4) + 175, 150, 150, 0x03a0ff);
        }
        else if ((elapsed_time >= 33.5f && elapsed_time <= 34.0f) || (elapsed_time >= 35.5f && elapsed_time <= 36.0f))
        {
            draw_filled_triangle(lead_triangle.a.x - 300 + (act2_initial_travel), lead_triangle.a.y, lead_triangle.b.x - 300 + (act2_initial_travel), lead_triangle.b.y, lead_triangle.c.x - 300 + (act2_initial_travel), lead_triangle.c.y, 0xFFC000);
            draw_rectangle(((window_width - 300) / 4) + 500 - (act2_initial_travel), ((window_height - 300) / 4) + 175, 150, 150, 0x50bdff);
        }
        else if ((elapsed_time >= 34.0f && elapsed_time <= 34.5f) || (elapsed_time >= 36.0f && elapsed_time <= 36.5f))
        {
            draw_filled_triangle(lead_triangle.a.x - 300 + (act2_initial_travel), lead_triangle.a.y, lead_triangle.b.x - 300 + (act2_initial_travel), lead_triangle.b.y, lead_triangle.c.x - 300 + (act2_initial_travel), lead_triangle.c.y, 0xffea00);
            draw_rectangle(((window_width - 300) / 4) + 500 - (act2_initial_travel), ((window_height - 300) / 4) + 175, 150, 150, 0x03a0ff);
        }
        else if ((elapsed_time >= 34.5f && elapsed_time <= 35.0f) || (elapsed_time >= 36.5f && elapsed_time <= 37.0f))
        {
            draw_filled_triangle(lead_triangle.a.x - 300 + (act2_initial_travel), lead_triangle.a.y, lead_triangle.b.x - 300 + (act2_initial_travel), lead_triangle.b.y, lead_triangle.c.x - 300 + (act2_initial_travel), lead_triangle.c.y, 0xffea00);
            draw_rectangle(((window_width - 300) / 4) + 500 - (act2_initial_travel), ((window_height - 300) / 4) + 175, 150, 150, 0x0071b6);
        }

        // triangle moves down, square moves up
        if (elapsed_time >= 37.0f && elapsed_time <= 40.0f)
        {
            clear_color_buffer(0xffc0cb);
            if (act2_vertical_movement <= 150)
            {
                act2_vertical_movement++;
            }

            draw_filled_triangle(lead_triangle.a.x - 300 + (act2_initial_travel), lead_triangle.a.y + (act2_vertical_movement), lead_triangle.b.x - 300 + (act2_initial_travel), lead_triangle.b.y + (act2_vertical_movement), lead_triangle.c.x - 300 + (act2_initial_travel), lead_triangle.c.y + (act2_vertical_movement), 0xffea00);
            draw_rectangle(((window_width - 300) / 4) + 500 - (act2_initial_travel), ((window_height - 300) / 4) + 175 - (act2_vertical_movement), 150, 150, 0x0071b6);
        }

        // triangle moves left, square moves right
        if (elapsed_time >= 40.0f && elapsed_time <= 43.0f)
        {
            clear_color_buffer(0xffc0cb);
            if (act2_horizontal_movement <= 200)
            {
                act2_horizontal_movement++;
            }

            draw_filled_triangle(lead_triangle.a.x - 300 + (act2_initial_travel) - (act2_horizontal_movement), lead_triangle.a.y + (act2_vertical_movement), lead_triangle.b.x - 300 + (act2_initial_travel) - (act2_horizontal_movement), lead_triangle.b.y + (act2_vertical_movement), lead_triangle.c.x - 300 + (act2_initial_travel) - (act2_horizontal_movement), lead_triangle.c.y + (act2_vertical_movement), 0xffea00);
            draw_rectangle(((window_width - 300) / 4) + 500 - (act2_initial_travel) + (act2_horizontal_movement), ((window_height - 300) / 4) + 175 - (act2_vertical_movement), 150, 150, 0x0071b6);
        }

        // triangle moves up, square moves down
        if (elapsed_time >= 43.0f && elapsed_time <= 46.0f)
        {
            clear_color_buffer(0xffc0cb);
            if (act2_final_vertical_movement <= 150)
            {
                act2_final_vertical_movement++;
            }

            draw_filled_triangle(lead_triangle.a.x - 300 + (act2_initial_travel) - (act2_horizontal_movement), lead_triangle.a.y + (act2_vertical_movement) - (act2_final_vertical_movement), lead_triangle.b.x - 300 + (act2_initial_travel) - (act2_horizontal_movement), lead_triangle.b.y + (act2_vertical_movement) - (act2_final_vertical_movement), lead_triangle.c.x - 300 + (act2_initial_travel) - (act2_horizontal_movement), lead_triangle.c.y + (act2_vertical_movement) - (act2_final_vertical_movement), 0xffea00);
            draw_rectangle(((window_width - 300) / 4) + 500 - (act2_initial_travel) + (act2_horizontal_movement), ((window_height - 300) / 4) + 175 - (act2_vertical_movement) + (act2_final_vertical_movement), 150, 150, 0x0071b6);
        }

        // triangle moves right, square moves left, they meet in the middle
        if (elapsed_time >= 46.0f && elapsed_time <= 49.0f)
        {
            clear_color_buffer(0xffc0cb);
            if (act2_final_horizontal_movement <= 300)
            {
                act2_final_horizontal_movement++;
            }

            draw_filled_triangle(lead_triangle.a.x - 300 + (act2_initial_travel) - (act2_horizontal_movement) + (act2_final_horizontal_movement), lead_triangle.a.y + (act2_vertical_movement) - (act2_final_vertical_movement), lead_triangle.b.x - 300 + (act2_initial_travel) - (act2_horizontal_movement) + (act2_final_horizontal_movement), lead_triangle.b.y + (act2_vertical_movement) - (act2_final_vertical_movement), lead_triangle.c.x - 300 + (act2_initial_travel) - (act2_horizontal_movement) + (act2_final_horizontal_movement), lead_triangle.c.y + (act2_vertical_movement) - (act2_final_vertical_movement), 0xffea00);
            draw_rectangle(((window_width - 300) / 4) + 500 - (act2_initial_travel) + (act2_horizontal_movement) - (act2_final_horizontal_movement), ((window_height - 300) / 4) + 175 - (act2_vertical_movement) + (act2_final_vertical_movement), 150, 150, 0x0071b6);
        }

        // triangle leaves
        // act2_triangle_leaves
        if (elapsed_time >= 49.0f && elapsed_time <= 52.0f)
        {
            clear_color_buffer(0xffc0cb);
            if (act2_triangle_leaves <= 200)
            {
                act2_triangle_leaves++;
            }

            draw_filled_triangle(lead_triangle.a.x - 300 + (act2_initial_travel) - (act2_horizontal_movement) + (act2_final_horizontal_movement) - (act2_triangle_leaves), lead_triangle.a.y + (act2_vertical_movement) - (act2_final_vertical_movement), lead_triangle.b.x - 300 + (act2_initial_travel) - (act2_horizontal_movement) + (act2_final_horizontal_movement) - (act2_triangle_leaves), lead_triangle.b.y + (act2_vertical_movement) - (act2_final_vertical_movement), lead_triangle.c.x - 300 + (act2_initial_travel) - (act2_horizontal_movement) + (act2_final_horizontal_movement) - (act2_triangle_leaves), lead_triangle.c.y + (act2_vertical_movement) - (act2_final_vertical_movement), 0xffea00);
            draw_rectangle(((window_width - 300) / 4) + 500 - (act2_initial_travel) + (act2_horizontal_movement) - (act2_final_horizontal_movement), ((window_height - 300) / 4) + 175 - (act2_vertical_movement) + (act2_final_vertical_movement), 150, 150, 0x0071b6);
        }
        if (elapsed_time >= 52.0f && elapsed_time <= 60.0f)
        {
            clear_color_buffer(0x000067);
            if (elapsed_time >= 54.0f && elapsed_time <= 56.0f)
            {
                clear_color_buffer(0x000080);
                if (elapsed_time >= 55.0f && elapsed_time <= 56.0f)
                {
                    draw_rectangle(((window_width - 300) / 4) + 500 - (act2_initial_travel) + (act2_horizontal_movement) - (act2_final_horizontal_movement), ((window_height - 300) / 4) + 175 - (act2_vertical_movement) + (act2_final_vertical_movement), 150, 150, 0x0071b6);
                }
            }
            else if (elapsed_time >= 56.0f && elapsed_time <= 57.0f)
            {
                clear_color_buffer(0x00009a);
                if (elapsed_time >= 56.5f && elapsed_time <= 57.0f)
                {
                    draw_rectangle(((window_width - 300) / 4) + 500 - (act2_initial_travel) + (act2_horizontal_movement) - (act2_final_horizontal_movement), ((window_height - 300) / 4) + 175 - (act2_vertical_movement) + (act2_final_vertical_movement), 150, 150, 0x0081cf);
                }
            }
            else if (elapsed_time >= 57.0f && elapsed_time <= 58.0f)
            {
                clear_color_buffer(0x0000b3);
                if (elapsed_time >= 57.5f && elapsed_time <= 58.0f)
                {
                    draw_rectangle(((window_width - 300) / 4) + 500 - (act2_initial_travel) + (act2_horizontal_movement) - (act2_final_horizontal_movement), ((window_height - 300) / 4) + 175 - (act2_vertical_movement) + (act2_final_vertical_movement), 150, 150, 0x0091e9);
                }
            }
            else if (elapsed_time >= 58.0f && elapsed_time <= 59.0f)
            {
                clear_color_buffer(0x0000b3);
                if (elapsed_time >= 58.5f && elapsed_time <= 59.0f)
                {
                    draw_rectangle(((window_width - 300) / 4) + 500 - (act2_initial_travel) + (act2_horizontal_movement) - (act2_final_horizontal_movement), ((window_height - 300) / 4) + 175 - (act2_vertical_movement) + (act2_final_vertical_movement), 150, 150, 0x03a0ff);
                }
            }
            else if (elapsed_time >= 59.0f && elapsed_time <= 60.0f)
            {
                clear_color_buffer(0x0000cd);
                if (elapsed_time >= 59.5f && elapsed_time <= 60.0f)
                {
                    draw_rectangle(((window_width - 300) / 4) + 500 - (act2_initial_travel) + (act2_horizontal_movement) - (act2_final_horizontal_movement), ((window_height - 300) / 4) + 175 - (act2_vertical_movement) + (act2_final_vertical_movement), 150, 150, 0x1daaff);
                }
            }
            draw_rectangle(((window_width - 300) / 4) + 500 - (act2_initial_travel) + (act2_horizontal_movement) - (act2_final_horizontal_movement), ((window_height - 300) / 4) + 175 - (act2_vertical_movement) + (act2_final_vertical_movement), 150, 150, 0x0071b6);
        }
    }

    // ---------------------------------ACT II: SCENE 3 - Cubes Disapprove--------------------------------- //

    if (elapsed_time >= 60.0f && elapsed_time < 75.0f)
    {
        clear_color_buffer(0x7c0200);
        // Scared triangle :(
        if ((elapsed_time >= 60.0f && elapsed_time < 63.0f) || (elapsed_time >= 66.0f && elapsed_time < 69.0f) || (elapsed_time >= 72.0f && elapsed_time < 75.0f))
        {
            draw_filled_triangle(((lead_triangle.a.x) / 2) + 220, ((lead_triangle.a.y) / 2) + 120, ((lead_triangle.b.x) / 2) + 220, ((lead_triangle.b.y) / 2) + 120, ((lead_triangle.c.x) / 2) + 220, ((lead_triangle.c.y) / 2) + 120, 0xffea00);
        }
        else if ((elapsed_time >= 63.0f && elapsed_time < 66.0f) || (elapsed_time >= 69.0f && elapsed_time < 72.0f))
        {
            draw_filled_triangle(((lead_triangle.a.x) / 2) + 220, ((lead_triangle.a.y) / 2) + 120, ((lead_triangle.b.x) / 2) + 220, ((lead_triangle.b.y) / 2) + 120, ((lead_triangle.c.x) / 2) + 220, ((lead_triangle.c.y) / 2) + 120, 0xffc000);
        }

        // LEFT CUBES

        // matrix incorporation
        scale_matrix = mat4_make_scale(cube_scale.x, cube_scale.y, cube_scale.z);
        rotation_matrix_x = mat4_make_rotation_x(cube_rotation.x); // pass the angle as float
        rotation_matrix_y = mat4_make_rotation_y(cube_rotation.y);
        rotation_matrix_z = mat4_make_rotation_z(cube_rotation.z);
        translate_matrix = mat4_make_translate(cube_translate.x, cube_translate.y, cube_translate.z);

        cube_rotation.x += .01;
        cube_rotation.y += .01;

        if ((elapsed_time >= 60.0f && elapsed_time < 63.0f) || (elapsed_time >= 66.0f && elapsed_time < 69.0f) || (elapsed_time >= 72.0f && elapsed_time < 75.0f))
        {
            cube_scale.x += .006;
            cube_scale.y += .006;
            cube_scale.z += .006;
        }
        else if ((elapsed_time >= 63.0f && elapsed_time < 66.0f) || (elapsed_time >= 69.0f && elapsed_time < 72.0f))
        {
            cube_scale.x -= .006;
            cube_scale.y -= .006;
            cube_scale.z -= .006;
        }
        cube_translate.x += .005;
        project_cube();

        for (int i = 0; i < t_cnt; i++)
        {
            triangle_t triangle = triangles_to_render[i];
            for (int j = 0; j < 3; j++)
            {
                // Cube top
                draw_line(triangle.points[0].x + originX - 220, triangle.points[0].y + originY - 200, triangle.points[1].x + originX - 220, triangle.points[1].y + originY - 200, 0x000000);
                draw_line(triangle.points[1].x + originX - 220, triangle.points[1].y + originY - 200, triangle.points[2].x + originX - 220, triangle.points[2].y + originY - 200, 0x000000);
                draw_line(triangle.points[2].x + originX - 220, triangle.points[2].y + originY - 200, triangle.points[0].x + originX - 220, triangle.points[0].y + originY - 200, 0x000000);

                // Cube bottom
                draw_line(triangle.points[0].x + originX - 220, triangle.points[0].y + originY + 110, triangle.points[1].x + originX - 220, triangle.points[1].y + originY + 110, 0x000000);
                draw_line(triangle.points[1].x + originX - 220, triangle.points[1].y + originY + 110, triangle.points[2].x + originX - 220, triangle.points[2].y + originY + 110, 0x000000);
                draw_line(triangle.points[2].x + originX - 220, triangle.points[2].y + originY + 110, triangle.points[0].x + originX - 220, triangle.points[0].y + originY + 110, 0x000000);
            }
        }
        t_cnt = 0;
        // RIGHT CUBES

        // matrix incorporation
        scale_matrix = mat4_make_scale(cube_scale.x, cube_scale.y, cube_scale.z);
        rotation_matrix_x = mat4_make_rotation_x(cube_rotation.x); // pass the angle as float
        rotation_matrix_y = mat4_make_rotation_y(cube_rotation.y);
        rotation_matrix_z = mat4_make_rotation_z(cube_rotation.z);
        translate_matrix = mat4_make_translate(cube_translate.x, cube_translate.y, cube_translate.z);

        cube_rotation.x += .01;
        cube_rotation.y += .01;

        if ((elapsed_time >= 60.0f && elapsed_time < 63.0f) || (elapsed_time >= 66.0f && elapsed_time < 69.0f) || (elapsed_time >= 72.0f && elapsed_time < 75.0f))
        {
            cube_scale.x += .006;
            cube_scale.y += .006;
            cube_scale.z += .006;
        }
        else if ((elapsed_time >= 63.0f && elapsed_time < 66.0f) || (elapsed_time >= 69.0f && elapsed_time < 72.0f))
        {
            cube_scale.x -= .006;
            cube_scale.y -= .006;
            cube_scale.z -= .006;
        }
        cube_translate.x -= .005;
        project_cube();

        for (int i = 0; i < t_cnt; i++)
        {
            triangle_t triangle = triangles_to_render[i];
            for (int j = 0; j < 3; j++)
            {
                // Cube top
                draw_line(triangle.points[0].x + originX + 180, triangle.points[0].y + originY - 200, triangle.points[1].x + originX + 180, triangle.points[1].y + originY - 200, 0x000000);
                draw_line(triangle.points[1].x + originX + 180, triangle.points[1].y + originY - 200, triangle.points[2].x + originX + 180, triangle.points[2].y + originY - 200, 0x000000);
                draw_line(triangle.points[2].x + originX + 180, triangle.points[2].y + originY - 200, triangle.points[0].x + originX + 180, triangle.points[0].y + originY - 200, 0x000000);

                // Cube bottom
                draw_line(triangle.points[0].x + originX + 180, triangle.points[0].y + originY + 110, triangle.points[1].x + originX + 180, triangle.points[1].y + originY + 110, 0x000000);
                draw_line(triangle.points[1].x + originX + 180, triangle.points[1].y + originY + 110, triangle.points[2].x + originX + 180, triangle.points[2].y + originY + 110, 0x000000);
                draw_line(triangle.points[2].x + originX + 180, triangle.points[2].y + originY + 110, triangle.points[0].x + originX + 180, triangle.points[0].y + originY + 110, 0x000000);
            }
        }
        t_cnt = 0;
    }

    // ---------------------------------ACT III: SCENE 1 - Triangle Gives In--------------------------------- //

    if (elapsed_time >= 75.0f && elapsed_time <= 95.0f)
    {
        clear_color_buffer(0xC3B1E1);

        if (elapsed_time >= 75.0f && elapsed_time <= 80)
        {
            if (elapsed_time >= 75.0f && elapsed_time <= 75.5f)
            {
                // spawn
                draw_filled_triangle(lead_triangle.a.x - 400, lead_triangle.a.y + 150, lead_triangle.b.x - 400, lead_triangle.b.y + 150, lead_triangle.c.x - 400, lead_triangle.c.y + 150, 0xffea00); // triangle on the bottom left of the screen
            }
            if (elapsed_time >= 75.5f && elapsed_time <= 76.0f)
            {
                // spawn
                draw_filled_triangle(lead_triangle.a.x, lead_triangle.a.y + 150, lead_triangle.b.x, lead_triangle.b.y + 150, lead_triangle.c.x, lead_triangle.c.y + 150, 0xffea00); // triangle on the bottom middle of the screen
            }
            if (elapsed_time >= 76.0f && elapsed_time <= 76.5f)
            {
                // spawn
                draw_filled_triangle(lead_triangle.a.x + 400, lead_triangle.a.y + 150, lead_triangle.b.x + 400, lead_triangle.b.y + 150, lead_triangle.c.x + 400, lead_triangle.c.y + 150, 0xffea00); // triangle on the bottom middle of the screen
            }
            if (elapsed_time >= 76.5f && elapsed_time <= 77.0f)
            {
                // spawn
                draw_filled_triangle(lead_triangle.a.x + 400, lead_triangle.a.y - 150, lead_triangle.b.x + 400, lead_triangle.b.y - 150, lead_triangle.c.x + 400, lead_triangle.c.y - 150, 0xffea00); // triangle on the bottom middle of the screen
            }
            if (elapsed_time >= 77.0f && elapsed_time <= 77.5f)
            {
                // spawn
                draw_filled_triangle(lead_triangle.a.x, lead_triangle.a.y - 150, lead_triangle.b.x, lead_triangle.b.y - 150, lead_triangle.c.x, lead_triangle.c.y - 150, 0xffea00); // triangle on the bottom middle of the screen
            }
            if (elapsed_time >= 77.5f && elapsed_time <= 78.0f)
            {
                // spawn
                draw_filled_triangle(lead_triangle.a.x - 400, lead_triangle.a.y - 150, lead_triangle.b.x - 400, lead_triangle.b.y - 150, lead_triangle.c.x - 400, lead_triangle.c.y - 150, 0xffea00); // triangle on the bottom left of the screen
            }
            if (elapsed_time >= 78.0f && elapsed_time <= 79.0f)
            {
                // flicker
                draw_filled_triangle(lead_triangle.a.x - 400, lead_triangle.a.y + 150, lead_triangle.b.x - 400, lead_triangle.b.y + 150, lead_triangle.c.x - 400, lead_triangle.c.y + 150, 0xfff600); // triangle on the bottom left of the screen
                draw_filled_triangle(lead_triangle.a.x, lead_triangle.a.y + 150, lead_triangle.b.x, lead_triangle.b.y + 150, lead_triangle.c.x, lead_triangle.c.y + 150, 0xffea00);                   // triangle on the bottom middle of the screen
                draw_filled_triangle(lead_triangle.a.x + 400, lead_triangle.a.y + 150, lead_triangle.b.x + 400, lead_triangle.b.y + 150, lead_triangle.c.x + 400, lead_triangle.c.y + 150, 0xffe135); // triangle on the bottom middle of the screen
                draw_filled_triangle(lead_triangle.a.x + 400, lead_triangle.a.y - 150, lead_triangle.b.x + 400, lead_triangle.b.y - 150, lead_triangle.c.x + 400, lead_triangle.c.y - 150, 0xffea00); // triangle on the bottom middle of the screen
                draw_filled_triangle(lead_triangle.a.x, lead_triangle.a.y - 150, lead_triangle.b.x, lead_triangle.b.y - 150, lead_triangle.c.x, lead_triangle.c.y - 150, 0xffea00);                   // triangle on the bottom middle of the screen
                draw_filled_triangle(lead_triangle.a.x - 400, lead_triangle.a.y - 150, lead_triangle.b.x - 400, lead_triangle.b.y - 150, lead_triangle.c.x - 400, lead_triangle.c.y - 150, 0xfffdd0); // triangle on the bottom left of the screen

                if (elapsed_time >= 78.0f && elapsed_time <= 78.5f)
                {
                    clear_color_buffer(0xff748c);
                    draw_filled_triangle(lead_triangle.a.x - 400, lead_triangle.a.y + 150, lead_triangle.b.x - 400, lead_triangle.b.y + 150, lead_triangle.c.x - 400, lead_triangle.c.y + 150, 0xffea00); // triangle on the bottom left of the screen
                    draw_filled_triangle(lead_triangle.a.x, lead_triangle.a.y + 150, lead_triangle.b.x, lead_triangle.b.y + 150, lead_triangle.c.x, lead_triangle.c.y + 150, 0xffea00);                   // triangle on the bottom middle of the screen
                    draw_filled_triangle(lead_triangle.a.x + 400, lead_triangle.a.y + 150, lead_triangle.b.x + 400, lead_triangle.b.y + 150, lead_triangle.c.x + 400, lead_triangle.c.y + 150, 0xfff600); // triangle on the bottom middle of the screen
                    draw_filled_triangle(lead_triangle.a.x + 400, lead_triangle.a.y - 150, lead_triangle.b.x + 400, lead_triangle.b.y - 150, lead_triangle.c.x + 400, lead_triangle.c.y - 150, 0xffe135); // triangle on the bottom middle of the screen
                    draw_filled_triangle(lead_triangle.a.x, lead_triangle.a.y - 150, lead_triangle.b.x, lead_triangle.b.y - 150, lead_triangle.c.x, lead_triangle.c.y - 150, 0xffea00);                   // triangle on the bottom middle of the screen
                    draw_filled_triangle(lead_triangle.a.x - 400, lead_triangle.a.y - 150, lead_triangle.b.x - 400, lead_triangle.b.y - 150, lead_triangle.c.x - 400, lead_triangle.c.y - 150, 0xffea00); // triangle on the bottom left of the screen
                }
                else if (elapsed_time >= 78.5f && elapsed_time <= 79.0f)
                {
                    clear_color_buffer(0xC3B1E1);
                    draw_filled_triangle(lead_triangle.a.x - 400, lead_triangle.a.y + 150, lead_triangle.b.x - 400, lead_triangle.b.y + 150, lead_triangle.c.x - 400, lead_triangle.c.y + 150, 0xfffdd0); // triangle on the bottom left of the screen
                    draw_filled_triangle(lead_triangle.a.x, lead_triangle.a.y + 150, lead_triangle.b.x, lead_triangle.b.y + 150, lead_triangle.c.x, lead_triangle.c.y + 150, 0xffea00);                   // triangle on the bottom middle of the screen
                    draw_filled_triangle(lead_triangle.a.x + 400, lead_triangle.a.y + 150, lead_triangle.b.x + 400, lead_triangle.b.y + 150, lead_triangle.c.x + 400, lead_triangle.c.y + 150, 0xffe135); // triangle on the bottom middle of the screen
                    draw_filled_triangle(lead_triangle.a.x + 400, lead_triangle.a.y - 150, lead_triangle.b.x + 400, lead_triangle.b.y - 150, lead_triangle.c.x + 400, lead_triangle.c.y - 150, 0xffea00); // triangle on the bottom middle of the screen
                    draw_filled_triangle(lead_triangle.a.x, lead_triangle.a.y - 150, lead_triangle.b.x, lead_triangle.b.y - 150, lead_triangle.c.x, lead_triangle.c.y - 150, 0xffea00);                   // triangle on the bottom middle of the screen
                    draw_filled_triangle(lead_triangle.a.x - 400, lead_triangle.a.y - 150, lead_triangle.b.x - 400, lead_triangle.b.y - 150, lead_triangle.c.x - 400, lead_triangle.c.y - 150, 0xfff600); // triangle on the bottom left of the screen
                }
            }
            if (elapsed_time >= 79.0f && elapsed_time <= 80.0f)
            {
                clear_color_buffer(0xff748c);
                draw_filled_triangle(lead_triangle.a.x - 400, lead_triangle.a.y + 150, lead_triangle.b.x - 400, lead_triangle.b.y + 150, lead_triangle.c.x - 400, lead_triangle.c.y + 150, 0xffea00); // triangle on the bottom left of the screen
                draw_filled_triangle(lead_triangle.a.x + 400, lead_triangle.a.y + 150, lead_triangle.b.x + 400, lead_triangle.b.y + 150, lead_triangle.c.x + 400, lead_triangle.c.y + 150, 0xffe135); // triangle on the bottom middle of the screen
                draw_filled_triangle(lead_triangle.a.x + 400, lead_triangle.a.y - 150, lead_triangle.b.x + 400, lead_triangle.b.y - 150, lead_triangle.c.x + 400, lead_triangle.c.y - 150, 0xffea00); // triangle on the bottom middle of the screen
                draw_filled_triangle(lead_triangle.a.x - 400, lead_triangle.a.y - 150, lead_triangle.b.x - 400, lead_triangle.b.y - 150, lead_triangle.c.x - 400, lead_triangle.c.y - 150, 0xffea00); // triangle on the bottom left of the screen
                if (elapsed_time >= 79.0f && elapsed_time <= 79.5f)
                {
                    clear_color_buffer(0xC3B1E1);
                    draw_filled_triangle(lead_triangle.a.x - 400, lead_triangle.a.y + 150, lead_triangle.b.x - 400, lead_triangle.b.y + 150, lead_triangle.c.x - 400, lead_triangle.c.y + 150, 0xfff600); // triangle on the bottom left of the screen
                    draw_filled_triangle(lead_triangle.a.x, lead_triangle.a.y + 150, lead_triangle.b.x, lead_triangle.b.y + 150, lead_triangle.c.x, lead_triangle.c.y + 150, 0xffea00);                   // triangle on the bottom middle of the screen
                    draw_filled_triangle(lead_triangle.a.x + 400, lead_triangle.a.y + 150, lead_triangle.b.x + 400, lead_triangle.b.y + 150, lead_triangle.c.x + 400, lead_triangle.c.y + 150, 0xffea00); // triangle on the bottom middle of the screen
                    draw_filled_triangle(lead_triangle.a.x + 400, lead_triangle.a.y - 150, lead_triangle.b.x + 400, lead_triangle.b.y - 150, lead_triangle.c.x + 400, lead_triangle.c.y - 150, 0xffe135); // triangle on the bottom middle of the screen
                    draw_filled_triangle(lead_triangle.a.x, lead_triangle.a.y - 150, lead_triangle.b.x, lead_triangle.b.y - 150, lead_triangle.c.x, lead_triangle.c.y - 150, 0xffea00);                   // triangle on the bottom middle of the screen
                    draw_filled_triangle(lead_triangle.a.x - 400, lead_triangle.a.y - 150, lead_triangle.b.x - 400, lead_triangle.b.y - 150, lead_triangle.c.x - 400, lead_triangle.c.y - 150, 0xffea00); // triangle on the bottom left of the screen
                }
                else if (elapsed_time >= 79.5f && elapsed_time <= 80.0f)
                {
                    clear_color_buffer(0x0000b3);
                    draw_filled_triangle(lead_triangle.a.x - 400, lead_triangle.a.y + 150, lead_triangle.b.x - 400, lead_triangle.b.y + 150, lead_triangle.c.x - 400, lead_triangle.c.y + 150, 0xffea00); // triangle on the bottom left of the screen
                    draw_filled_triangle(lead_triangle.a.x + 400, lead_triangle.a.y + 150, lead_triangle.b.x + 400, lead_triangle.b.y + 150, lead_triangle.c.x + 400, lead_triangle.c.y + 150, 0xfff600); // triangle on the bottom middle of the screen
                    draw_filled_triangle(lead_triangle.a.x + 400, lead_triangle.a.y - 150, lead_triangle.b.x + 400, lead_triangle.b.y - 150, lead_triangle.c.x + 400, lead_triangle.c.y - 150, 0xfffdd0); // triangle on the bottom middle of the screen
                    draw_filled_triangle(lead_triangle.a.x - 400, lead_triangle.a.y - 150, lead_triangle.b.x - 400, lead_triangle.b.y - 150, lead_triangle.c.x - 400, lead_triangle.c.y - 150, 0xffe135); // triangle on the bottom left of the screen
                                                                                                                                                                                                          // flicker
                }
            }
        }
        else if (elapsed_time >= 80.0f && elapsed_time <= 85.0f)
        {
            draw_filled_triangle(lead_triangle.a.x, lead_triangle.a.y, lead_triangle.b.x, lead_triangle.b.y, lead_triangle.c.x, lead_triangle.c.y, 0xffea00); // triangle on the bottom left of the screen
            if (elapsed_time >= 75.0f && elapsed_time <= 80)
            {
                draw_filled_triangle(lead_triangle.a.x, lead_triangle.a.y, lead_triangle.b.x, lead_triangle.b.y, lead_triangle.c.x, lead_triangle.c.y, 0xfffdd0); // triangle on the bottom left of the screen
            }
        }
        if (elapsed_time >= 80.0f && elapsed_time <= 85.0f)
        {
            clear_color_buffer(0x0000b3);
            // matrix incorporation
            scale_matrix = mat4_make_scale(p_scale.x, p_scale.y, p_scale.z);
            rotation_matrix_x = mat4_make_rotation_x(p_rotation.x); // pass the angle as float
            rotation_matrix_y = mat4_make_rotation_y(p_rotation.y);
            rotation_matrix_z = mat4_make_rotation_z(p_rotation.z);
            translate_matrix = mat4_make_translate(p_translate.x, p_translate.y, p_translate.z);

            p_rotation.x += .01;
            p_rotation.y += .01;
            p_rotation.z += .01;

            p_scale.x = 3;
            p_scale.y = 3;
            p_scale.z = 3;

            project_pyramid();
            for (int i = 0; i < t_cnt; i++)
            {
                triangle_t triangle = triangles_to_render[i];
                for (int j = 0; j < 3; j++)
                {
                    // loop through every triangle then draw every vertex of every triangle
                    draw_line(triangle.points[0].x + originX, triangle.points[0].y + originY, triangle.points[1].x + originX, triangle.points[1].y + originY, 0xFFEA00);
                    draw_line(triangle.points[1].x + originX, triangle.points[1].y + originY, triangle.points[2].x + originX, triangle.points[2].y + originY, 0xFFEA00);
                    draw_line(triangle.points[2].x + originX, triangle.points[2].y + originY, triangle.points[0].x + originX, triangle.points[0].y + originY, 0xFFEA00);
                }
            }
            t_cnt = 0;
        }
    }

    // ---------------------------------ACT III: SCENE 2 - Triangle in Cube Society--------------------------------- //

    if (elapsed_time >= 85.0f && elapsed_time <= 112.0f)
    { // 32 sec
        // PYRAMID :(

        // matrix incorporation
        scale_matrix = mat4_make_scale(p_scale.x, p_scale.y, p_scale.z);
        rotation_matrix_x = mat4_make_rotation_x(p_rotation.x); // pass the angle as float
        rotation_matrix_y = mat4_make_rotation_y(p_rotation.y);
        rotation_matrix_z = mat4_make_rotation_z(p_rotation.z);
        translate_matrix = mat4_make_translate(p_translate.x, p_translate.y, p_translate.z);

        if (elapsed_time >= 85.0f && elapsed_time <= 112.0f)
        {
            p_rotation.x += .01;
            p_scale.x += .001;
            p_scale.y += .001;
            p_scale.z += .001;
        }

        project_pyramid();
        for (int i = 0; i < t_cnt; i++)
        {
            triangle_t triangle = triangles_to_render[i];
            for (int j = 0; j < 3; j++)
            {
                draw_line(triangle.points[0].x + originX - 25, triangle.points[0].y + originY, triangle.points[1].x + originX - 25, triangle.points[1].y + originY, 0xFFFFFF);
                draw_line(triangle.points[1].x + originX - 25, triangle.points[1].y + originY, triangle.points[2].x + originX - 25, triangle.points[2].y + originY, 0xFFFFFF);
                draw_line(triangle.points[2].x + originX - 25, triangle.points[2].y + originY, triangle.points[0].x + originX - 25, triangle.points[0].y + originY, 0xFFFFFF);
            }
        }
        t_cnt = 0;

        // LEFT CUBES

        // matrix incorporation
        scale_matrix = mat4_make_scale(cube_scale.x, cube_scale.y, cube_scale.z);
        rotation_matrix_x = mat4_make_rotation_x(cube_rotation.x); // pass the angle as float
        rotation_matrix_y = mat4_make_rotation_y(cube_rotation.y);
        rotation_matrix_z = mat4_make_rotation_z(cube_rotation.z);
        translate_matrix = mat4_make_translate(cube_translate.x, cube_translate.y, cube_translate.z);

        cube_rotation.x += .01;
        cube_rotation.y += .01;

        if ((elapsed_time >= 92.0f && elapsed_time < 94.0f) || (elapsed_time >= 97.0f && elapsed_time < 99.0f) || (elapsed_time >= 113.0f && elapsed_time < 115.0f))
        {
            cube_scale.x += .006;
            cube_scale.y += .006;
            cube_scale.z += .006;
        }
        else if ((elapsed_time >= 94.0f && elapsed_time < 97.0f) || (elapsed_time >= 99.0f && elapsed_time < 102.0f))
        {
            cube_scale.x -= .0065;
            cube_scale.y -= .0065;
            cube_scale.z -= .0065;
        }

        if ((elapsed_time >= 102.0f && elapsed_time < 105.0f))
        {
            cube_translate.x += .009;
        }
        if ((elapsed_time >= 105.0f && elapsed_time < 108.0f))
        {
            cube_translate.y += .009;
        }
        if ((elapsed_time >= 108.0f && elapsed_time < 110.0f))
        {
            cube_translate.x -= .009;
        }
        if ((elapsed_time >= 110.0f && elapsed_time < 112.0f))
        {
            cube_translate.y -= .009;
        }

        project_cube();

        for (int i = 0; i < t_cnt; i++)
        {
            triangle_t triangle = triangles_to_render[i];
            for (int j = 0; j < 3; j++)
            {
                // Cube top
                draw_line(triangle.points[0].x + originX - 220, triangle.points[0].y + originY - 200, triangle.points[1].x + originX - 220, triangle.points[1].y + originY - 200, 0x000000);
                draw_line(triangle.points[1].x + originX - 220, triangle.points[1].y + originY - 200, triangle.points[2].x + originX - 220, triangle.points[2].y + originY - 200, 0x000000);
                draw_line(triangle.points[2].x + originX - 220, triangle.points[2].y + originY - 200, triangle.points[0].x + originX - 220, triangle.points[0].y + originY - 200, 0x000000);

                // Cube bottom
                draw_line(triangle.points[0].x + originX - 220, triangle.points[0].y + originY + 110, triangle.points[1].x + originX - 220, triangle.points[1].y + originY + 110, 0x000000);
                draw_line(triangle.points[1].x + originX - 220, triangle.points[1].y + originY + 110, triangle.points[2].x + originX - 220, triangle.points[2].y + originY + 110, 0x000000);
                draw_line(triangle.points[2].x + originX - 220, triangle.points[2].y + originY + 110, triangle.points[0].x + originX - 220, triangle.points[0].y + originY + 110, 0x000000);
            }
        }
        t_cnt = 0;

        // RIGHT CUBES

        // matrix incorporation
        scale_matrix = mat4_make_scale(cube_scale.x, cube_scale.y, cube_scale.z);
        rotation_matrix_x = mat4_make_rotation_x(cube_rotation.x); // pass the angle as float
        rotation_matrix_y = mat4_make_rotation_y(cube_rotation.y);
        rotation_matrix_z = mat4_make_rotation_z(cube_rotation.z);
        translate_matrix = mat4_make_translate(cube_translate.x, cube_translate.y, cube_translate.z);

        cube_rotation.x += .01;
        cube_rotation.y += .01;

        if ((elapsed_time >= 92.0f && elapsed_time < 94.0f) || (elapsed_time >= 97.0f && elapsed_time < 99.0f) || (elapsed_time >= 113.0f && elapsed_time < 115.0f))
        {
            cube_scale.x += .006;
            cube_scale.y += .006;
            cube_scale.z += .006;
        }
        else if ((elapsed_time >= 94.0f && elapsed_time < 97.0f) || (elapsed_time >= 99.0f && elapsed_time < 102.0f))
        {
            cube_scale.x -= .0065;
            cube_scale.y -= .0065;
            cube_scale.z -= .0065;
        }

        if ((elapsed_time >= 102.0f && elapsed_time < 105.0f))
        {
            cube_translate.x += .009;
        }
        if ((elapsed_time >= 105.0f && elapsed_time < 108.0f))
        {
            cube_translate.y += .009;
        }
        if ((elapsed_time >= 108.0f && elapsed_time < 110.0f))
        {
            cube_translate.x -= .009;
        }
        if ((elapsed_time >= 110.0f && elapsed_time < 112.0f))
        {
            cube_translate.y -= .009;
        }

        project_cube();

        for (int i = 0; i < t_cnt; i++)
        {
            triangle_t triangle = triangles_to_render[i];
            for (int j = 0; j < 3; j++)
            {
                // Cube top
                draw_line(triangle.points[0].x + originX + 180, triangle.points[0].y + originY - 200, triangle.points[1].x + originX + 180, triangle.points[1].y + originY - 200, 0x000000);
                draw_line(triangle.points[1].x + originX + 180, triangle.points[1].y + originY - 200, triangle.points[2].x + originX + 180, triangle.points[2].y + originY - 200, 0x000000);
                draw_line(triangle.points[2].x + originX + 180, triangle.points[2].y + originY - 200, triangle.points[0].x + originX + 180, triangle.points[0].y + originY - 200, 0x000000);

                // Cube bottom
                draw_line(triangle.points[0].x + originX + 180, triangle.points[0].y + originY + 110, triangle.points[1].x + originX + 180, triangle.points[1].y + originY + 110, 0x000000);
                draw_line(triangle.points[1].x + originX + 180, triangle.points[1].y + originY + 110, triangle.points[2].x + originX + 180, triangle.points[2].y + originY + 110, 0x000000);
                draw_line(triangle.points[2].x + originX + 180, triangle.points[2].y + originY + 110, triangle.points[0].x + originX + 180, triangle.points[0].y + originY + 110, 0x000000);
            }
        }
        t_cnt = 0;
    }

    // ---------------------------------ACT III: SCENE 3 - You take the man out of the city--------------------------------- //

    // PYRAMID
    if (elapsed_time >= 112.0f && elapsed_time < 119.0f) { 
        // matrix incorporation
        clear_color_buffer(0xC3B1E1);
        scale_matrix = mat4_make_scale(p_scale.x, p_scale.y, p_scale.z);
        rotation_matrix_x = mat4_make_rotation_x(p_rotation.x); // pass the angle as float
        rotation_matrix_y = mat4_make_rotation_y(p_rotation.y);
        rotation_matrix_z = mat4_make_rotation_z(p_rotation.z);
        translate_matrix = mat4_make_translate(p_translate.x, p_translate.y, p_translate.z);

        p_rotation.x += .025;
        p_scale.x = 3;
        p_scale.y = 3;
        p_scale.z = 3;

        project_pyramid();
        for (int i = 0; i < t_cnt; i++)
        {
            triangle_t triangle = triangles_to_render[i];
            for (int j = 0; j < 3; j++)
            {
                draw_line(triangle.points[0].x + originX - 25, triangle.points[0].y + originY, triangle.points[1].x + originX - 25, triangle.points[1].y + originY, 0xFFFFFF);
                draw_line(triangle.points[1].x + originX - 25, triangle.points[1].y + originY, triangle.points[2].x + originX - 25, triangle.points[2].y + originY, 0xFFFFFF);
                draw_line(triangle.points[2].x + originX - 25, triangle.points[2].y + originY, triangle.points[0].x + originX - 25, triangle.points[0].y + originY, 0xFFFFFF);
            }
        }
        t_cnt = 0;
    }

    // TRIANGLE
    if (elapsed_time >= 119.0f && elapsed_time < 125.0f) {
        draw_filled_triangle(lead_triangle.a.x, lead_triangle.a.y, lead_triangle.b.x, lead_triangle.b.y, lead_triangle.c.x, lead_triangle.c.y, 0xffea00);
    }

    // CUBE
    if (elapsed_time >= 125.0f && elapsed_time < 130.5f) { 

        // matrix incorporation
        scale_matrix = mat4_make_scale(cube_scale.x, cube_scale.y, cube_scale.z);
        rotation_matrix_x = mat4_make_rotation_x(cube_rotation.x); // pass the angle as float
        rotation_matrix_y = mat4_make_rotation_y(cube_rotation.y);
        rotation_matrix_z = mat4_make_rotation_z(cube_rotation.z);
        translate_matrix = mat4_make_translate(cube_translate.x, cube_translate.y, cube_translate.z);

        cube_rotation.x += .01;
        cube_rotation.y += .01;

        if (elapsed_time >= 125.0f && elapsed_time <= 128.5f)
        {
            cube_scale.x += .01;
            cube_scale.y += .01;
        }
        else if (elapsed_time >= 128.5f && elapsed_time <= 130.5f)
        {
            cube_scale.x -= .01;
            cube_scale.y -= .01;
        }
        project_cube();

        for (int i = 0; i < t_cnt; i++)
        {
            triangle_t triangle = triangles_to_render[i];
            for (int j = 0; j < 3; j++)
            {
                // Cube middle
                draw_line(triangle.points[0].x + originX - 25, triangle.points[0].y + originY - 20, triangle.points[1].x + originX - 25, triangle.points[1].y + originY - 20, 0xFFFFFF);
                draw_line(triangle.points[1].x + originX - 25, triangle.points[1].y + originY - 20, triangle.points[2].x + originX - 25, triangle.points[2].y + originY - 20, 0xFFFFFF);
                draw_line(triangle.points[2].x + originX - 25, triangle.points[2].y + originY - 20, triangle.points[0].x + originX - 25, triangle.points[0].y + originY - 20, 0xFFFFFF);
            }
        }
        t_cnt = 0;
    }

    // SQUARE
    if (elapsed_time >= 130.5f && elapsed_time < 133.0f) {
        draw_rectangle(((window_width - 300) / 4) + 200, ((window_height - 300) / 4) + 175, 150, 150, 0x0071b6);
        act2_initial_travel = 0;
        act2_vertical_movement = 0;
        act2_horizontal_movement = 0;
        act2_final_vertical_movement = 0;
        act2_triangle_leaves = 0;
    }

    //---------------------------- Finale ----------------------------//
    if (elapsed_time >= 133.0f && elapsed_time <= 165.0f)
    {
        clear_color_buffer(0xC3B1E1);
        // triangle and square meet eachother in the middle from the left and right, respectively
        if (act2_initial_travel <= 200)
        {
            act2_initial_travel++;
        }
        else
        {
            clear_color_buffer(0xffc0cb); // light pink
        }
        draw_filled_triangle(lead_triangle.a.x - 300 + (act2_initial_travel), lead_triangle.a.y, lead_triangle.b.x - 300 + (act2_initial_travel), lead_triangle.b.y, lead_triangle.c.x - 300 + (act2_initial_travel), lead_triangle.c.y, 0xffea00);
        draw_rectangle(((window_width - 300) / 4) + 500 - (act2_initial_travel), ((window_height - 300) / 4) + 175, 150, 150, 0x0071b6);

        if ((elapsed_time >= 133.0f && elapsed_time <= 133.5f) || (elapsed_time >= 135.0f && elapsed_time <= 135.5f))
        {
            draw_filled_triangle(lead_triangle.a.x - 300 + (act2_initial_travel), lead_triangle.a.y, lead_triangle.b.x - 300 + (act2_initial_travel), lead_triangle.b.y, lead_triangle.c.x - 300 + (act2_initial_travel), lead_triangle.c.y, 0xFFd500);
            draw_rectangle(((window_width - 300) / 4) + 500 - (act2_initial_travel), ((window_height - 300) / 4) + 175, 150, 150, 0x03a0ff);
        }
        else if ((elapsed_time >= 133.5f && elapsed_time <= 134.0f) || (elapsed_time >= 135.5f && elapsed_time <= 136.0f))
        {
            draw_filled_triangle(lead_triangle.a.x - 300 + (act2_initial_travel), lead_triangle.a.y, lead_triangle.b.x - 300 + (act2_initial_travel), lead_triangle.b.y, lead_triangle.c.x - 300 + (act2_initial_travel), lead_triangle.c.y, 0xFFC000);
            draw_rectangle(((window_width - 300) / 4) + 500 - (act2_initial_travel), ((window_height - 300) / 4) + 175, 150, 150, 0x50bdff);
        }
        else if ((elapsed_time >= 134.0f && elapsed_time <= 134.5f) || (elapsed_time >= 136.0f && elapsed_time <= 136.5f))
        {
            draw_filled_triangle(lead_triangle.a.x - 300 + (act2_initial_travel), lead_triangle.a.y, lead_triangle.b.x - 300 + (act2_initial_travel), lead_triangle.b.y, lead_triangle.c.x - 300 + (act2_initial_travel), lead_triangle.c.y, 0xffea00);
            draw_rectangle(((window_width - 300) / 4) + 500 - (act2_initial_travel), ((window_height - 300) / 4) + 175, 150, 150, 0x03a0ff);
        }
        else if ((elapsed_time >= 134.5f && elapsed_time <= 135.0f) || (elapsed_time >= 136.5f && elapsed_time <= 137.0f))
        {
            draw_filled_triangle(lead_triangle.a.x - 300 + (act2_initial_travel), lead_triangle.a.y, lead_triangle.b.x - 300 + (act2_initial_travel), lead_triangle.b.y, lead_triangle.c.x - 300 + (act2_initial_travel), lead_triangle.c.y, 0xffea00);
            draw_rectangle(((window_width - 300) / 4) + 500 - (act2_initial_travel), ((window_height - 300) / 4) + 175, 150, 150, 0x0071b6);
        }

        // triangle moves down, square moves up
        if (elapsed_time >= 137.0f && elapsed_time <= 140.0f)
        {
            clear_color_buffer(0xffc0cb);
            if (act2_vertical_movement <= 150)
            {
                act2_vertical_movement++;
            }

            draw_filled_triangle(lead_triangle.a.x - 300 + (act2_initial_travel), lead_triangle.a.y + (act2_vertical_movement), lead_triangle.b.x - 300 + (act2_initial_travel), lead_triangle.b.y + (act2_vertical_movement), lead_triangle.c.x - 300 + (act2_initial_travel), lead_triangle.c.y + (act2_vertical_movement), 0xffea00);
            draw_rectangle(((window_width - 300) / 4) + 500 - (act2_initial_travel), ((window_height - 300) / 4) + 175 - (act2_vertical_movement), 150, 150, 0x0071b6);
        }

        // triangle moves left, square moves right
        if (elapsed_time >= 140.0f && elapsed_time <= 143.0f)
        {
            clear_color_buffer(0xffc0cb);
            if (act2_horizontal_movement <= 200)
            {
                act2_horizontal_movement++;
            }

            draw_filled_triangle(lead_triangle.a.x - 300 + (act2_initial_travel) - (act2_horizontal_movement), lead_triangle.a.y + (act2_vertical_movement), lead_triangle.b.x - 300 + (act2_initial_travel) - (act2_horizontal_movement), lead_triangle.b.y + (act2_vertical_movement), lead_triangle.c.x - 300 + (act2_initial_travel) - (act2_horizontal_movement), lead_triangle.c.y + (act2_vertical_movement), 0xffea00);
            draw_rectangle(((window_width - 300) / 4) + 500 - (act2_initial_travel) + (act2_horizontal_movement), ((window_height - 300) / 4) + 175 - (act2_vertical_movement), 150, 150, 0x0071b6);
        }

        // triangle moves up, square moves down
        if (elapsed_time >= 143.0f && elapsed_time <= 146.0f)
        {
            clear_color_buffer(0xffc0cb);
            if (act2_final_vertical_movement <= 150)
            {
                act2_final_vertical_movement++;
            }

            draw_filled_triangle(lead_triangle.a.x - 300 + (act2_initial_travel) - (act2_horizontal_movement), lead_triangle.a.y + (act2_vertical_movement) - (act2_final_vertical_movement), lead_triangle.b.x - 300 + (act2_initial_travel) - (act2_horizontal_movement), lead_triangle.b.y + (act2_vertical_movement) - (act2_final_vertical_movement), lead_triangle.c.x - 300 + (act2_initial_travel) - (act2_horizontal_movement), lead_triangle.c.y + (act2_vertical_movement) - (act2_final_vertical_movement), 0xffea00);
            draw_rectangle(((window_width - 300) / 4) + 500 - (act2_initial_travel) + (act2_horizontal_movement), ((window_height - 300) / 4) + 175 - (act2_vertical_movement) + (act2_final_vertical_movement), 150, 150, 0x0071b6);
        }

        // triangle moves right, square moves left, they meet in the middle
        if (elapsed_time >= 146.0f && elapsed_time <= 149.0f)
        {
            clear_color_buffer(0xffc0cb);
            if (act2_final_horizontal_movement <= 300)
            {
                act2_final_horizontal_movement++;
            }

            draw_filled_triangle(lead_triangle.a.x - 300 + (act2_initial_travel) - (act2_horizontal_movement) + (act2_final_horizontal_movement), lead_triangle.a.y + (act2_vertical_movement) - (act2_final_vertical_movement), lead_triangle.b.x - 300 + (act2_initial_travel) - (act2_horizontal_movement) + (act2_final_horizontal_movement), lead_triangle.b.y + (act2_vertical_movement) - (act2_final_vertical_movement), lead_triangle.c.x - 300 + (act2_initial_travel) - (act2_horizontal_movement) + (act2_final_horizontal_movement), lead_triangle.c.y + (act2_vertical_movement) - (act2_final_vertical_movement), 0xffea00);
            draw_rectangle(((window_width - 300) / 4) + 500 - (act2_initial_travel) + (act2_horizontal_movement) - (act2_final_horizontal_movement), ((window_height - 300) / 4) + 175 - (act2_vertical_movement) + (act2_final_vertical_movement), 150, 150, 0x0071b6);
        }

        // triangle leaves
        if (elapsed_time >= 149.0f && elapsed_time <= 152.0f)
        {
            clear_color_buffer(0xffc0cb);
            if (act2_triangle_leaves <= 200)
            {
                act2_triangle_leaves++;
            }

            draw_filled_triangle(lead_triangle.a.x - 300 + (act2_initial_travel) - (act2_horizontal_movement) + (act2_final_horizontal_movement) - (act2_triangle_leaves), lead_triangle.a.y + (act2_vertical_movement) - (act2_final_vertical_movement), lead_triangle.b.x - 300 + (act2_initial_travel) - (act2_horizontal_movement) + (act2_final_horizontal_movement) - (act2_triangle_leaves), lead_triangle.b.y + (act2_vertical_movement) - (act2_final_vertical_movement), lead_triangle.c.x - 300 + (act2_initial_travel) - (act2_horizontal_movement) + (act2_final_horizontal_movement) - (act2_triangle_leaves), lead_triangle.c.y + (act2_vertical_movement) - (act2_final_vertical_movement), 0xffea00);
            draw_rectangle(((window_width - 300) / 4) + 500 - (act2_initial_travel) + (act2_horizontal_movement) - (act2_final_horizontal_movement), ((window_height - 300) / 4) + 175 - (act2_vertical_movement) + (act2_final_vertical_movement), 150, 150, 0x0071b6);
        }
        if (elapsed_time >= 152.0f && elapsed_time <= 160.0f)
        {
            clear_color_buffer(0x000067);
            if (elapsed_time >= 154.0f && elapsed_time <= 156.0f)
            {
                clear_color_buffer(0x000080);
                if (elapsed_time >= 155.0f && elapsed_time <= 156.0f)
                {
                    draw_rectangle(((window_width - 300) / 4) + 500 - (act2_initial_travel) + (act2_horizontal_movement) - (act2_final_horizontal_movement), ((window_height - 300) / 4) + 175 - (act2_vertical_movement) + (act2_final_vertical_movement), 150, 150, 0x0071b6);
                }
            }
            else if (elapsed_time >= 156.0f && elapsed_time <= 157.0f)
            {
                clear_color_buffer(0x00009a);
                if (elapsed_time >= 156.5f && elapsed_time <= 157.0f)
                {
                    draw_rectangle(((window_width - 300) / 4) + 500 - (act2_initial_travel) + (act2_horizontal_movement) - (act2_final_horizontal_movement), ((window_height - 300) / 4) + 175 - (act2_vertical_movement) + (act2_final_vertical_movement), 150, 150, 0x0081cf);
                }
            }
            else if (elapsed_time >= 157.0f && elapsed_time <= 157.0f)
            {
                clear_color_buffer(0x0000b3);
                if (elapsed_time >= 157.5f && elapsed_time <= 158.0f)
                {
                    draw_rectangle(((window_width - 300) / 4) + 500 - (act2_initial_travel) + (act2_horizontal_movement) - (act2_final_horizontal_movement), ((window_height - 300) / 4) + 175 - (act2_vertical_movement) + (act2_final_vertical_movement), 150, 150, 0x0091e9);
                }
            }
            else if (elapsed_time >= 158.0f && elapsed_time <= 159.0f)
            {
                clear_color_buffer(0x0000b3);
                if (elapsed_time >= 158.5f && elapsed_time <= 159.0f)
                {
                    draw_rectangle(((window_width - 300) / 4) + 500 - (act2_initial_travel) + (act2_horizontal_movement) - (act2_final_horizontal_movement), ((window_height - 300) / 4) + 175 - (act2_vertical_movement) + (act2_final_vertical_movement), 150, 150, 0x03a0ff);
                }
            }
            else if (elapsed_time >= 159.0f && elapsed_time <= 160.0f)
            {
                clear_color_buffer(0x0000cd);
                if (elapsed_time >= 159.5f && elapsed_time <= 160.0f)
                {
                    draw_rectangle(((window_width - 300) / 4) + 500 - (act2_initial_travel) + (act2_horizontal_movement) - (act2_final_horizontal_movement), ((window_height - 300) / 4) + 175 - (act2_vertical_movement) + (act2_final_vertical_movement), 150, 150, 0x1daaff);
                }
            }
            draw_rectangle(((window_width - 300) / 4) + 500 - (act2_initial_travel) + (act2_horizontal_movement) - (act2_final_horizontal_movement), ((window_height - 300) / 4) + 175 - (act2_vertical_movement) + (act2_final_vertical_movement), 150, 150, 0x0071b6);
        }
    }
    if(elapsed_time >= 160.0f && elapsed_time <=161.0f){
        is_running = false;
    }

}


int main(void)
{
    is_running = initialize_windowing_system();
    set_up_memory_buffers();
    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN); // set to full screen upon running

    animation_start_time = SDL_GetTicks();

    // initializing lead_triangle here
    lead_triangle.a = triangle_a;
    lead_triangle.b = triangle_b;
    lead_triangle.c = triangle_c;

    while (is_running)
    {
        process_keyboard_input();
        update_state();
        run_render_pipeline();
        SDL_Delay(frame_target_time);
    }
    clean_up_windowing_system();
    free(color_buffer);
    return 0;
}