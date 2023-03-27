#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <tonc.h>
#include <tonc_math.h>
#include <maxmod.h>
#include "soundbank.h"
#include "soundbank_bin.h"
#include "triangle.h"

#define NUM_TRIANGLES 32
#define DEG2RAD(deg) ((deg) * (3.14159265 / 180.0))


// Object Attribute Memory (OAM) buffers
OBJ_ATTR obj_buffer[128];
OBJ_AFFINE *obj_aff_buffer = (OBJ_AFFINE *)obj_buffer;

void init();
void input();
void logic();
void draw();
void update_positions();
void check_collisions();
int collide(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);

// Array of pointers to the OBJ_ATTR objects representing the triangles
OBJ_ATTR *triangle[NUM_TRIANGLES];

// Triangle sprite structure
struct TriangleSprite {
    u32 x;
    u32 y;
    u32 vx;
    u32 vy;
    u32 tid;
    u32 pb;
	u32 angle;
};

// Array of triangle sprite objects
struct TriangleSprite triSpr[NUM_TRIANGLES];

int main() {
    init();
	
	// Set the background color to white
    pal_bg_mem[0] = RGB15(31, 31, 31);

    // Main game loop
    while (1) {
        update_positions();
		check_collisions();
        draw();

        // Update the OAM with the new triangle positions
        oam_copy(oam_mem, obj_buffer, NUM_TRIANGLES);
    }

    return 0;
}

void init() {
    // Copy triangle sprite data to memory
    memcpy(&tile_mem[4][0], triangleTiles, triangleTilesLen);
    memcpy(pal_obj_mem, trianglePal, trianglePalLen);

    // Initialize OAM and set display control register
    oam_init(obj_buffer, 128);
    REG_DISPCNT = DCNT_OBJ | DCNT_OBJ_1D;

    // Initialize the OBJ_ATTR pointers for the triangles
    for (int i = 0; i < NUM_TRIANGLES; i++) {
        triangle[i] = &obj_buffer[i];
    }

    // Set the attributes for each triangle
    for (int i = 0; i < NUM_TRIANGLES; i++) {
        obj_set_attr(triangle[i],
                     ATTR0_SQUARE | ATTR0_AFF,     // Square, affine sprite
                     ATTR1_SIZE_32 | ATTR1_AFF_ID(i), // 32x32 pixels, affine ID
                     ATTR2_PALBANK(triSpr[i].pb) | triSpr[i].tid);  // palbank and tile index
    }

    // Initialize the triangle sprite objects with random positions and velocities
    for (int i = 0; i < NUM_TRIANGLES; i++) {
        triSpr[i].x = rand() % (240 - 32);
        triSpr[i].y = rand() % (160 - 32);
        triSpr[i].tid = 0;
        triSpr[i].pb = rand() % 15 + 13; // Divide by 2 because each color is 2 bytes
        triSpr[i].vx = (rand() % 4) + 1;
        triSpr[i].vy = (rand() % 4) + 1;
        triSpr[i].angle = float2fx(DEG2RAD(rand() % 360));

    }

    // Set the triangle palette bank and tile index
    for (int i = 0; i < NUM_TRIANGLES; i++) {
        triangle[i]->attr2 = ATTR2_BUILD(triSpr[i].tid, triSpr[i].pb, 0);
    }

    // Update the initial affine transformations for the triangles
    for (int i = 0; i < NUM_TRIANGLES; i++) {
        obj_aff_identity(&obj_aff_buffer[i]);
        obj_aff_scale(&obj_aff_buffer[i], 1, 1);
        obj_aff_rotate(&obj_aff_buffer[i], triSpr[i].angle);
        obj_aff_copy(&obj_aff_buffer[i], &obj_aff_buffer[i], 1);
    }
	
	

}

void draw() {
    vid_vsync();

    for (int i = 0; i < NUM_TRIANGLES; i++) {
        obj_set_pos(triangle[i], triSpr[i].x, triSpr[i].y);
        obj_aff_copy(&obj_aff_buffer[i], &obj_aff_buffer[i], 1);
    }
}



void input() {
    key_poll();
}

void logic() {
}

// Update the positions of the triangles and handle bouncing
void update_positions() {
    for (int i = 0; i < NUM_TRIANGLES; i++) {
        // Update the position based on the triangle's velocity
		triSpr[i].x += triSpr[i].vx;
		triSpr[i].y += triSpr[i].vy;

		// Check for collisions with the screen edges
		// Horizontal collision
		if (triSpr[i].x < 0 || triSpr[i].x > (240 - 32)) {
			// Reverse horizontal velocity
			triSpr[i].vx = -triSpr[i].vx;
		}
		// Vertical collision
		if (triSpr[i].y < 0 || triSpr[i].y > (160 - 32)) {
			// Reverse vertical velocity
			triSpr[i].vy = -triSpr[i].vy;
		}
	}
}

int collide(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
}

void check_collisions() {
    for (int i = 0; i < NUM_TRIANGLES; i++) {
        for (int j = i + 1; j < NUM_TRIANGLES; j++) {
            int dx = triSpr[i].x - triSpr[j].x;
            int dy = triSpr[i].y - triSpr[j].y;
            int distanceSquared = dx * dx + dy * dy;
            int radiusSum = 16; // Half of the triangle width (32 / 2)
            int radiusSumSquared = radiusSum * radiusSum;

            if (distanceSquared <= radiusSumSquared) {
                // Swap velocities
                int tempVx = triSpr[i].vx;
                int tempVy = triSpr[i].vy;
                triSpr[i].vx = triSpr[j].vx;
                triSpr[i].vy = triSpr[j].vy;
                triSpr[j].vx = tempVx;
                triSpr[j].vy = tempVy;
            }
        }
    }
}


