// To compile: gcc -Wall -g -std=c99 -I/opt/homebrew/opt/sdl2/include -L/opt/homebrew/opt/sdl2/lib /Users/hudaayaz/Desktop/3dvectors/main.c /Users/hudaayaz/Desktop/3dvectors/vector.c /Users/hudaayaz/Desktop/3dvectors/matrix.c /Users/hudaayaz/Desktop/3dvectors/mesh.c -o main -lSDL2

// gcc -Wall -g -std=c99 -I/opt/homebrew/opt/sdl2/include -L/opt/homebrew/opt/sdl2/lib /Users/hudaayaz/Desktop/3dvectors/main.c -o main -lSDL2
// To run: ./main


#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/sdl_mixer.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "vector.h"
// #include "vector.c"
#include "mesh.h"
// #include "mesh.c"
#include "triangle.h"
#include "matrix.h"

SDL_Renderer *renderer;
SDL_Window *window;
SDL_Texture *texture;
uint32_t *color_buffer;
bool is_running = false;
int window_width = 800;
int window_height = 600;
void draw_pixel(int x, int y, uint32_t color);
void draw_line(int x0, int y0, int x1, int y1, uint32_t color);
void draw_rectangle(int start_x, int start_y, int rectangle_width, int rectangle_height, uint32_t color); // void draw_rectangle(uint32_t color, int rectangle_height, int rectangle_width, int start_y, int start_x);
void draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color);
void draw_filled_triangle(int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color);
bool initialize_windowing_system();
void clean_up_windowing_system();
// void project_model();
void project_cube();
void project_pyramid();

int scaling_factor = 500;
int previous_frame_time = 0;

#define FPS 30
#define frame_target_time (scaling_factor/FPS)

vec3_t camera_position = {0, 0, -5};
// vec3_t cube_rotation = { .x = 0, .y = 0, .z = 0 };
triangle_t triangles_to_render[1000];
triangle_t p_to_render[1000];

void process_keyboard_input();
void run_render_pipeline();
void set_up_memory_buffers();
void clear_color_buffer(uint32_t color);

void build_cube_model();
// vec2_t orthographic_project_point(vec3_t point_3d);
void scale_shape();

vec2_t perspective_project_point(vec3_t point_3d);

// float rotation_angle = 0.0;

int t_cnt = 0;

//matrix incorporation
vec3_t cube_scale = {.x = 1, .y = 1, .z = 1};
vec3_t cube_rotation = {.x = 0, .y = 0, .z = 0};
vec3_t cube_translate = {.x = 1, .y = 1, .z = 7};

//pyramid incorporation
vec3_t p_scale = {.x = 1, .y = 1, .z = 1};
vec3_t p_rotation = {.x = 0, .y = 0, .z = 0};
vec3_t p_translate = {.x = 1, .y = 1, .z = 7};


mat4_t scale_matrix;
mat4_t rotation_matrix_x; //pass the angle as float
mat4_t rotation_matrix_y;
mat4_t rotation_matrix_z;
mat4_t translate_matrix;

//to keep track of animation time
// int frame_target_time = 16; // ~60 FPS
Uint32 animation_start_time = 0; // SDL time when animation starts
float animation_duration = 5.0f; // Total duration of the animation in seconds
float elapsed_time = 0.0f;

// mat4_t world_matrix;

// void initialize_matrices();

//setting up the parameters for main triangle character
    vec2_t triangle_a = {.x = 300, .y = 400}; //side a
    vec2_t triangle_b = {.x = 400, .y = 200}; //side b
    vec2_t triangle_c = {.x = 500, .y = 400}; //side c
    //basically just creating a vector 3 that can hold values of vec2_t's
    vec3_t_of_vec2_t lead_triangle = { .a = {.x=0,.y=0}, .b= {.x=0,.y=0}, .c= {.x=0,.y=0}};

void update_state();
//vvvvvvvvvvvv BASIC SETUP FILES vvvvvvvvvvvv//

void run_render_pipeline() {
    // SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_UpdateTexture(texture, NULL, color_buffer, window_width * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void process_keyboard_input() {
    SDL_Event event;
    SDL_PollEvent(&event);
    switch(event.type) {
        case SDL_QUIT:
            is_running = false;
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE)
                is_running = false;
            break;
    }
}

bool initialize_windowing_system() {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "SDL_Init failed");
        return false;
    }
    
    window = SDL_CreateWindow(NULL, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, SDL_WINDOW_SHOWN);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow failed");
        return false;		
    }
    
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer failed");
        return false;
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, window_width, window_height);
    if (!texture) {
        fprintf(stderr, "SDL_CreateTexture failed");
        return false;
    }

    if(Mix_Init(0) != 0){
        fprintf(stderr, "Mix_Init() Failed\n");
    }
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 128);
    Mix_Music* music = Mix_LoadMUS("audio/djo.wav");
    if(!music){
        fprintf(stderr, "MUSIC NOT PLAYING!\n");
    }
    Mix_PlayMusic(music, 0);

    //the final will be 25 short answer questions
    
    return true;
}

void clean_up_windowing_system() {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_Quit();
    SDL_Quit();
}

void set_up_memory_buffers() {
    color_buffer = (uint32_t*)malloc(window_width * window_height * sizeof(uint32_t));
}

void clear_color_buffer(uint32_t color) {
    for (int y = 0; y < window_height; y++) {
        for (int x = 0; x < window_width; x++) {
            color_buffer[(y * window_width) + x] = color;
        }
    }	
}

//^^^^^^^^^^^^ BASIC SETUP FILES ^^^^^^^^^^^^//

void draw_rectangle (int start_x, int start_y, int rectangle_width, int rectangle_height, uint32_t color) {
    for (int y = start_y; y < start_y + rectangle_height; y++) {
		for (int x = start_x; x < start_x + rectangle_width; x++) {
			//color_buffer[(y*window_width)+x] = color;
            draw_pixel(x, y, color);
		}
    }
}

void draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color){
    draw_line(x1,y1,x2,y2,color);
    draw_line(x2,y3,x3,y3,color);
    draw_line(x3,y3,x1,y1,color);
}

// Note: Only works for triangles where the base is perpendicular to the height.
void draw_filled_triangle(int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color) {
    int base = x3-x1;
    // int height = y2-y1;
    // int center_of_triangle = height/2;
    for(int i = 0; i < base; i++){
        draw_triangle(x1+i,y1,x2,y2,x3-i,y3, color);
    }
}

vec2_t perspective_project_point(vec3_t point_3d) {
    point_3d.x -= camera_position.x;
    point_3d.y -= camera_position.y;
    point_3d.z -= camera_position.z;

    vec2_t projected_point = {
        .x = scaling_factor * (point_3d.x / point_3d.z), 
        .y = scaling_factor * (point_3d.y / point_3d.z)
    };
    return projected_point;
}

//vvvvvvvvvvvvvvvvvvvvvv 3D MESH vvvvvvvvvvvvvvvvvvvvvv//

void draw_pixel(int x, int y, uint32_t color) {
    // confirm pixel is in the visible window space
    if (x < window_width && y < window_height)
        color_buffer[ (y * window_width) + x ] = color;
}

void draw_line(int x0, int y0, int x1, int y1, uint32_t color) {
    int delta_x = x1 - x0;
    int delta_y = y1 - y0;

    int steps = abs(delta_x) > abs(delta_y) ? abs(delta_x) : abs(delta_y);

    float x_inc = delta_x / (float)steps;
    float y_inc = delta_y / (float)steps;

    float x = x0;
    float y = y0;

    for (int i = 0; i <= steps; i++) {
        draw_pixel(x, y, color);
        x += x_inc;
        y += y_inc;
    }
}

//vvvvvvvvvvvvvvvvvvvvvv DISPLAY vvvvvvvvvvvvvvvvvvvvvv//

void project_cube() {
    // world_matrix = mat4_identity();

    // CUBE PROJECT MODEL
    for (int i = 0; i < N_MESH_FACES; i++) { // for each triangle
        face_t mesh_face = mesh_faces[i]; 
        vec3_t face_vertices[3]; // create an array of vertices
        face_vertices[0] = mesh_vertices[mesh_face.a - 1]; // place the correct vertices into array
        face_vertices[1] = mesh_vertices[mesh_face.b - 1];
        face_vertices[2] = mesh_vertices[mesh_face.c - 1];
        
        // world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
        // world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
        // world_matrix = mat4_mul_mat4(rotation_matrix_y,world_matrix);
        // world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
        // world_matrix = mat4_mul_mat4(translate_matrix, world_matrix);

        vec3_t transformed_vertices[3]; 

        // triangle_t projected_triangle; 
        for (int j = 0; j < 3; j++) { // loop through vertices, apply transformations and projections
            vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);

            // transformed_vertices[j] = vec3_from_vec4(transformed_vertex);

            //----//

            transformed_vertex = mat4_mul_vec4(scale_matrix, transformed_vertex);
            transformed_vertex = mat4_mul_vec4(rotation_matrix_x, transformed_vertex);
            transformed_vertex = mat4_mul_vec4(rotation_matrix_y, transformed_vertex);
            transformed_vertex = mat4_mul_vec4(rotation_matrix_z, transformed_vertex);
            transformed_vertex = mat4_mul_vec4(translate_matrix, transformed_vertex);

            transformed_vertices[j] = vec3_from_vec4(transformed_vertex);

            //----//

            // vec3_t transformed_vertex = face_vertices[j];
            
            // transformed_vertex = vec3_rotate_x(transformed_vertex, cube_rotation.x);
            // transformed_vertex = vec3_rotate_y(transformed_vertex, cube_rotation.y);
            // transformed_vertex = vec3_rotate_z(transformed_vertex, cube_rotation.z);
            // transformed_vertex.z += 5;

            // transformed_vertices[j] = transformed_vertex;
            
            //----//

            //vec2_t projected_point = perspective_project_point(transformed_vertex);

            // projected_point.x += (window_width / 2);
            // projected_point.y -= (window_height / 2);

            //projected_triangle.points[j] = projected_point;
        }
        //triangles_to_render[i] = projected_triangle;


        /* BACKFACE CULLING  */
        vec3_t vertex_a = transformed_vertices[0]; /*    A    */
        vec3_t vertex_b = transformed_vertices[1]; /*   / \   */
        vec3_t vertex_c = transformed_vertices[2]; /*  C---B  */

        // Get the vector subtractiono of B-A and C-A
        vec3_t vector_ab = vec3_subtract(vertex_b, vertex_a);
        vec3_t vector_ac = vec3_subtract(vertex_c, vertex_a);

        // Compute the face normal (using corss product to find perpendiculiar vector)
        vec3_t normal = vec3_cross(vector_ab, vector_ac);

        // Find the vector between a point in the triangle and the camera origin.
        vec3_t camera_ray = vec3_subtract(camera_position, vertex_a);

        // Calculate how aligned the camera ray is with the face normal (using the dot product)
        float dot_normal_camera = vec3_dot(camera_ray, normal);

        // Bypass triangles that are looking away from the camera by continuing to next face in main loop
        if (dot_normal_camera < 0) {
            continue;
        }

        /* PROJECTION */

        triangle_t projected_triangle;

        // Sub Loop, project the vertices of curent face
        for (int j = 0; j < 3; j++) {
            vec2_t projected_point = perspective_project_point(transformed_vertices[j]); 

            /* translate projected vertex to center of screen */
            // projected_point.x += (window_width / 2);
            // projected_point.y += (window_height / 2);

            projected_triangle.points[j] = projected_point;
        }
        //save the projected triangle
        triangles_to_render[t_cnt++] = projected_triangle;
    }
}

void project_pyramid() {
    // PYRAMID PROJECT MODEL
    for (int i = 0; i < P_MESH_FACES; i++) { // for each triangle
        face_t p_mesh_face = p_mesh_faces[i]; 
        vec3_t p_face_vertices[3]; // create an array of vertices
        p_face_vertices[0] = p_mesh_vertices[p_mesh_face.a - 1]; // place the correct vertices into array
        p_face_vertices[1] = p_mesh_vertices[p_mesh_face.b - 1];
        p_face_vertices[2] = p_mesh_vertices[p_mesh_face.c - 1];

        vec3_t transformed_vertices[3]; 

        for (int j = 0; j < 3; j++) { // loop through vertices, apply transformations and projections
            vec4_t transformed_vertex = vec4_from_vec3(p_face_vertices[j]);

            transformed_vertex = mat4_mul_vec4(scale_matrix, transformed_vertex);
            transformed_vertex = mat4_mul_vec4(rotation_matrix_x, transformed_vertex);
            transformed_vertex = mat4_mul_vec4(rotation_matrix_y, transformed_vertex);
            transformed_vertex = mat4_mul_vec4(rotation_matrix_z, transformed_vertex);
            transformed_vertex = mat4_mul_vec4(translate_matrix, transformed_vertex);

            transformed_vertices[j] = vec3_from_vec4(transformed_vertex);
        }


        /* BACKFACE CULLING  */
        vec3_t vertex_a = transformed_vertices[0]; 
        vec3_t vertex_b = transformed_vertices[1]; 
        vec3_t vertex_c = transformed_vertices[2]; 

        // Get the vector subtractiono of B-A and C-A
        vec3_t vector_ab = vec3_subtract(vertex_b, vertex_a);
        vec3_t vector_ac = vec3_subtract(vertex_c, vertex_a);

        // Compute the face normal (using corss product to find perpendiculiar vector)
        vec3_t normal = vec3_cross(vector_ab, vector_ac);

        // Find the vector between a point in the triangle and the camera origin.
        vec3_t camera_ray = vec3_subtract(camera_position, vertex_a);

        // Calculate how aligned the camera ray is with the face normal (using the dot product)
        float dot_normal_camera = vec3_dot(camera_ray, normal);

        // Bypass triangles that are looking away from the camera by continuing to next face in main loop
        if (dot_normal_camera < 0) {
            continue;
        }

        /* PROJECTION */

        triangle_t projected_triangle;

        // Sub Loop, project the vertices of curent face
        for (int j = 0; j < 3; j++) {
            vec2_t projected_point = perspective_project_point(transformed_vertices[j]); 
            projected_triangle.points[j] = projected_point;
        }
        //save the projected triangle
        triangles_to_render[t_cnt++] = projected_triangle;
    }
}

//^^^^^^^^^^^^ 3D MESH ^^^^^^^^^^^^//

void update_state() {
	clear_color_buffer(0x000000); // black

    Uint32 current_time = SDL_GetTicks();
    printf("Current SDL Ticks: %u\n", current_time);

    elapsed_time = (float)(current_time - animation_start_time) / 1000.0f; // Convert to seconds
     if (elapsed_time >= 1.0f &&  elapsed_time <= 10.0f) {
        //Yellow triangle pops up on the first beat
        clear_color_buffer(0xfff3f5);
        draw_filled_triangle(lead_triangle.a.x, lead_triangle.a.y, lead_triangle.b.x, lead_triangle.b.y, lead_triangle.c.x, lead_triangle.c.y, 0xffea00);
    }
    
    if (elapsed_time >= 5.0f &&  elapsed_time <= 10.0f) {
        //makes sure progress is at 100
        //float progress = elapsed_time / animation_duration;

        // matrix incorporation
        scale_matrix = mat4_make_scale(cube_scale.x, cube_scale.y, cube_scale.z);
        rotation_matrix_x = mat4_make_rotation_x(cube_rotation.x); //pass the angle as float
        rotation_matrix_y = mat4_make_rotation_y(cube_rotation.y);
        rotation_matrix_z = mat4_make_rotation_z(cube_rotation.z);
        translate_matrix = mat4_make_translate(cube_translate.x, cube_translate.y, cube_translate.z);

        cube_rotation.x += .01;
        //cube_rotation.y += .01;
        //cube_rotation.z += .01;
        

        cube_scale.x += .01;
        cube_scale.y += .01;
        cube_scale.z += .01;
        cube_translate.y += .009;
        cube_translate.x += .03 ;
        project_cube();

        
        for (int i = 0; i < t_cnt; i++) {
            triangle_t triangle = triangles_to_render[i]; 
            for (int j = 0; j < 3; j++) {
                // loop through every triangle then draw every vertex of every triangle
                draw_rectangle(triangle.points[0].x + window_width / 2, triangle.points[0].y + window_height / 2, 5, 5, 0xFFFFFF);
                draw_rectangle(triangle.points[1].x + window_width / 2, triangle.points[1].y + window_height / 2, 5, 5, 0xFFFFFF);
                draw_rectangle(triangle.points[2].x + window_width / 2, triangle.points[2].y + window_height / 2, 5, 5, 0xFFFFFF);
                draw_line(triangle.points[0].x + window_width / 2, triangle.points[0].y + window_height / 2, triangle.points[1].x + window_width / 2, triangle.points[1].y + window_height / 2, 0xFFFFFF);
                draw_line(triangle.points[1].x + window_width / 2, triangle.points[1].y + window_height / 2, triangle.points[2].x + window_width / 2, triangle.points[2].y + window_height / 2, 0xFFFFFF);
                draw_line(triangle.points[2].x + window_width / 2, triangle.points[2].y + window_height / 2, triangle.points[0].x + window_width / 2, triangle.points[0].y + window_height / 2, 0xFFFFFF);
            }
        }
        t_cnt = 0;
    }
    if (elapsed_time >= 11.0f && elapsed_time <= 16.0f) {
        // matrix incorporation
        scale_matrix = mat4_make_scale(p_scale.x, p_scale.y, p_scale.z);
        rotation_matrix_x = mat4_make_rotation_x(p_rotation.x); //pass the angle as float
        rotation_matrix_y = mat4_make_rotation_y(p_rotation.y);
        rotation_matrix_z = mat4_make_rotation_z(p_rotation.z);
        translate_matrix = mat4_make_translate(p_translate.x, p_translate.y, p_translate.z);

        p_rotation.x += .01;
        //cube_rotation.y += .01;
        //cube_rotation.z += .01;
        

        p_scale.x += .01;
        p_scale.y += .01;
        p_scale.z += .01;
        p_translate.y += .009;
        p_translate.x += .03 ;

        project_pyramid();
        for (int i = 0; i < t_cnt; i++) {
            triangle_t triangle = triangles_to_render[i]; 
            for (int j = 0; j < 3; j++) {
                // loop through every triangle then draw every vertex of every triangle
                draw_rectangle(triangle.points[0].x, triangle.points[0].y + window_height / 2, 5, 5, 0xFFFFFF);
                draw_rectangle(triangle.points[1].x, triangle.points[1].y + window_height / 2, 5, 5, 0xFFFFFF);
                draw_rectangle(triangle.points[2].x, triangle.points[2].y + window_height / 2, 5, 5, 0xFFFFFF);
                draw_line(triangle.points[0].x, triangle.points[0].y + window_height / 2, triangle.points[1].x, triangle.points[1].y + window_height / 2, 0xFFFFFF);
                draw_line(triangle.points[1].x, triangle.points[1].y + window_height / 2, triangle.points[2].x, triangle.points[2].y + window_height / 2, 0xFFFFFF);
                draw_line(triangle.points[2].x, triangle.points[2].y + window_height / 2, triangle.points[0].x, triangle.points[0].y + window_height / 2, 0xFFFFFF);
            }
        }
        t_cnt = 0;

        
    }

        if (elapsed_time >= 17.0f && elapsed_time <= 21.0f) {
        // matrix incorporation
        scale_matrix = mat4_make_scale(p_scale.x, p_scale.y, p_scale.z);
        rotation_matrix_x = mat4_make_rotation_x(p_rotation.x); //pass the angle as float
        rotation_matrix_y = mat4_make_rotation_y(p_rotation.y);
        rotation_matrix_z = mat4_make_rotation_z(p_rotation.z);
        translate_matrix = mat4_make_translate(p_translate.x, p_translate.y, p_translate.z);

        p_rotation.x -= .01;
        //cube_rotation.y += .01;
        //cube_rotation.z += .01;
        

        p_scale.x -= .01;
        p_scale.y -= .01;
        p_scale.z -= .01;
        p_translate.y -= .009;
        p_translate.x -= .03 ;

        project_pyramid();
        for (int i = 0; i < t_cnt; i++) {
            triangle_t triangle = triangles_to_render[i]; 
            for (int j = 0; j < 3; j++) {
                // loop through every triangle then draw every vertex of every triangle
                draw_rectangle(triangle.points[0].x, triangle.points[0].y + window_height / 2, 5, 5, 0xFFFFFF);
                draw_rectangle(triangle.points[1].x, triangle.points[1].y + window_height / 2, 5, 5, 0xFFFFFF);
                draw_rectangle(triangle.points[2].x, triangle.points[2].y + window_height / 2, 5, 5, 0xFFFFFF);
                draw_line(triangle.points[0].x, triangle.points[0].y + window_height / 2, triangle.points[1].x, triangle.points[1].y + window_height / 2, 0xFFFFFF);
                draw_line(triangle.points[1].x, triangle.points[1].y + window_height / 2, triangle.points[2].x, triangle.points[2].y + window_height / 2, 0xFFFFFF);
                draw_line(triangle.points[2].x, triangle.points[2].y + window_height / 2, triangle.points[0].x, triangle.points[0].y + window_height / 2, 0xFFFFFF);
            }
        }
        t_cnt = 0;
    }
}

int main(void) { 
    is_running = initialize_windowing_system();
    set_up_memory_buffers();
    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN); // set to full screen upon running

    animation_start_time = SDL_GetTicks();

    //initializing lead_triangle here
    lead_triangle.a= triangle_a;
    lead_triangle.b= triangle_b;
    lead_triangle.c= triangle_c;

    while (is_running) {
        process_keyboard_input();
        update_state();
        run_render_pipeline();
        SDL_Delay(frame_target_time);
    }
    clean_up_windowing_system();
    free(color_buffer); 
    return 0;
}