#include <stdio.h>
#include <raylib.h>
#include <raymath.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>

#define PINS_COLOR  DARKGRAY
#define BEANS_COLOR GREEN

#define BEANS_COLLIDE   0
#define BEANS_DISAPPEAR 1

typedef struct {
    Vector2 pos;
    Vector2 vel;
    float radius;
    Color color;
    int counted; // has it been counted?
} Bean;

typedef struct {
    Vector2 pos;
    float radius;
} Pin;

typedef struct {
     float x;                // Rectangle top-left corner position x
     float y;                // Rectangle top-left corner position y
     float width;            // Rectangle width
     float height;           // Rectangle height
     Color color;
     int counter;
     float proba;
} Cage;

void draw_bean(Bean bean) {
    DrawCircle( bean.pos.x, bean.pos.y, bean.radius, bean.color);
}

void draw_pin(Pin pin, Color color) {
    DrawCircle( pin.pos.x, pin.pos.y, pin.radius, color);
}

void draw_cages(Cage cage) {
    DrawRectangle( cage.x, cage.y, cage.width, cage.height, cage.color);
    // draw cage number
    DrawText(TextFormat("%d", cage.counter),
	    cage.x + cage.width / 2,
	    cage.y + 0.15 * cage.height,
	    20,
	    DARKGREEN);
}

int triangular_sum(int n) {
    if (n < 0) return -1;
    int sum = 0;
    while (n > 0) {
	sum += n;
	n--;
    }
    return sum;
}

void populate_pins(Pin *pins, int radius, int rows, int w, int h) {
    int triangle_base = 1 * w;
    int x_low_offset  = (w - triangle_base) / 2;
    int y_low_offset  = 0.1 * h;
    int y_high_offset = 0.5 * h;
    int triangle_height = y_high_offset-y_low_offset;
    int index = 0;
    for (int i = 1; i <= rows; i++) {
	float current_base =  (float)i / (float)rows * (float)triangle_base;
	for (int j = 1; j <= i; j++) {
	    int pos_x = (w / 2) - (current_base / 2) + (j * (current_base / (i+1)));
	    int pos_y = y_low_offset + (((y_high_offset - y_low_offset) / (rows-1)) * i);
	    Pin p;
	    p.pos    = (Vector2){ pos_x, pos_y };
	    p.radius = radius;
	    pins[index] = p;
	    index++;
	}
    }
}

void populate_cages(Cage *cages, int nb_cages, int w, int h) {
    int triangle_base = 1 * w;
    int wo = (w - (nb_cages * (triangle_base / nb_cages))) / 2;
    int yo = 0.1 * h + 0.5 * h;
    int index = 0;

    for (int i = 0; i < nb_cages; i++) {
	Cage c;
	c.x = wo + i * (triangle_base / nb_cages);
	c.y = yo;
	c.width = triangle_base / nb_cages;
	c.height = h - yo;
	if ( i % 2 == 0) {
	    c.color = CLITERAL(Color){ 130, 130, 130, 120 };
	} else {
	    c.color = CLITERAL(Color){ 80, 80, 80, 120 };
	}
	cages[index] = c;
	index++;
    }
}

int main(void)
{
    srand(time(0));
    int f = 100;
    int w = 8 * f;
    int h = 8 * f;

    int PIN_RADIUS  = 5 * f / 100;
    int BEAN_RADIUS = 8 * f / 100;

    /*Beans*/
    Bean beans[2000]; // allow up to XXXX beans
    int beans_index = 0;

    /*Pins*/
    int triangle_rows = 12;
    size_t p_size = triangular_sum(triangle_rows);
    Pin pins[p_size];
    populate_pins(pins, PIN_RADIUS, triangle_rows, w, h);

    /*Cages*/
    int nb_cages = triangle_rows + 1;
    Cage cages[nb_cages];
    populate_cages(cages, nb_cages, w, h);

    /*Statistics*/
    //float mu = 0.0;
    //float sd = 0.0;

    /* Rudimentary physics */
    Vector2 gravity   = { 0.0, 0.25 }; // downward pull
    float energy_loss = 0.2;          // energy loss when hitting something

    InitWindow(w, h, "Galton Bean Machine");
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
	BeginDrawing();
	    ClearBackground(BLACK);

	    /*cages*/
	    for (size_t i = 0; i < nb_cages; i++) {
	        draw_cages(cages[i]);
	    }

	    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
		Bean b = { .pos = {GetMouseX(),GetMouseY()} ,
    		           .vel = {0.0, 0.0},
    		           .radius = BEAN_RADIUS,
			   .color = BEANS_COLOR};
    		beans[beans_index] = b;
		beans_index++;
	    }
	    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
		Bean b = { .pos = {GetMouseX(),GetMouseY()} ,
    		           .vel = {0.0, 0.0},
    		           .radius = BEAN_RADIUS,
			   .color = BEANS_COLOR};
    		beans[beans_index] = b;
		beans_index++;
	    }

	    /*draw beans*/
	    for (size_t i = 0; i < beans_index; i++) {
	        draw_bean(beans[i]);
		beans[i].vel = Vector2Add(beans[i].vel, gravity);
	        beans[i].pos = Vector2Add(beans[i].pos, beans[i].vel);
		if (beans[i].pos.y + beans[i].radius >= h) {
		    if (!BEANS_DISAPPEAR) {
			beans[i].pos.y = h - beans[i].radius + 1 ;
		    }
		    beans[i].vel.x *= (1 - energy_loss);
		}
	    }

	    /*check for collisions (beans/pins)*/
	    for (size_t i = 0; i < sizeof(pins) / sizeof(pins[0]); i++) {
	       draw_pin(pins[i], PINS_COLOR); /*draw pins*/
	       for (size_t j = 0; j < sizeof(beans) / sizeof(beans[0]); j++) {
	           if (CheckCollisionCircles(pins[i].pos, pins[i].radius, beans[j].pos, beans[j].radius)) {
		       int x_random = 0; // avoid "perfect" effects
		       if (beans[j].pos.x == pins[i].pos.x) {
			   x_random = (rand() % 2) == 0 ? -1 : 1;
			   beans[j].pos.x += x_random;
		       }
	               Vector2 centers_distance = Vector2Subtract(beans[j].pos, pins[i].pos);
	               beans[j].vel = Vector2Add(beans[j].vel, centers_distance);
	               beans[j].vel = Vector2Multiply(beans[j].vel, (Vector2){ 1 * energy_loss, gravity.y});
	           }
	       }
	    }

	    /*check for collisions (beans/borders)*/
	    for (size_t i = 0; i < sizeof(beans) / sizeof(beans[0]); i++) {
	        if (beans[i].pos.x - beans[i].radius < 0 || beans[i].pos.x + beans[i].radius >= w) {
		    beans[i].vel.x *= (-1 * (1 - energy_loss));
	            beans[i].pos = Vector2Add(beans[i].pos, beans[i].vel);
		}
	        if (beans[i].pos.y - beans[i].radius < 0 || beans[i].pos.y + beans[i].radius >= h) {
		    beans[i].vel.y *= (-1 * (1 - energy_loss));
		    beans[i].pos = Vector2Add(beans[i].pos, beans[i].vel);
		}
	    }

	    /*check for collisions (beans/cages)*/
	    for (size_t i = 0; i < nb_cages; i++) {
		Rectangle r = { cages[i].x, cages[i].y, cages[i].width, cages[i].height};
		for (size_t j = 0; j < beans_index; j++) {
		    if (beans[j].counted) { continue; }
		    if (CheckCollisionCircleRec(beans[j].pos, beans[j].radius, r)) {
			beans[j].counted = 1;
		        cages[i].counter++;
		        cages[i].proba = ((float)cages[i].counter / (float)beans_index) * 100.0;
		        // snap bean to fall in line!
		        beans[j].vel.x = 0;
		    }
	       }
	    }

	    /*check for collisions (beans/beans)*/
	    if (BEANS_COLLIDE) {
		for (size_t i = 0; i < beans_index; i++) {
	    	   for (size_t j = i; j < beans_index; j++) {
	    	       if (i == j) {continue;}
	    	       if (CheckCollisionCircles(beans[i].pos, beans[i].radius, beans[j].pos, beans[j].radius)) {
	    	           int ivx = beans[i].vel.x;
	    	           int ivy = beans[i].vel.y;
	    	           int jvx = beans[j].vel.x;
	    	           int jvy = beans[j].vel.y;
	    	           if (ivx > jvx) {
	    	    	   if (jvx < 0) {
	    	    	       beans[i].vel.x += jvx * (energy_loss/3);
	    	    	       beans[j].vel.x += (ivx - jvx) * (energy_loss/3);
	    	    	   } else {
	    	    	       beans[i].vel.x -= jvx * (energy_loss/3);
	    	    	       beans[j].vel.x += (ivx - jvx) * (energy_loss/3);
	    	    	    }
	    	           }
	    	           if (jvx > ivx) {
	    	    	   if (ivx < 0) {
	    	    	       beans[j].vel.x += ivx * (energy_loss/3);
	    	    	       beans[i].vel.x += (jvx - ivx) * (energy_loss/3);
	    	    	   } else {
	    	    	       beans[j].vel.x -= ivx * (energy_loss/3);
	    	    	       beans[i].vel.x += (jvx - ivx) * (energy_loss/3);
	    	    	    }
	    	           }
	    	       }
	    	   }
	    	}
	    }

	EndDrawing();
    }
    return 0;
}
