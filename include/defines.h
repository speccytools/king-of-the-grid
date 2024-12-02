#pragma once

#define WORLD_SIZE (32)
#define WORLD_SIZE_SQ (1024)
#define MAX_FOOD_VALUE (100)

#define MAX_PREY_POPULATION (10)
#define FOOD_DECAY (500)
#define FOOD_SPAWN_EVERY (500)
#define FOOD_SPAWN_ENERGY (500)
#define FOOD_ENERGY_MULTIPLIER (16)
#define PREY_SPAWN_EVERY (1000)
#define PREY_SPAWN_ENERGY (20000)
#define PREY_LIMIT (10)

#define SCAN_ENERGY (1000)
#define SCAN_TIME (20)
#define HIBERNATE_TIME (1000)

#define TOTAL_MEM_SIZE (65536)
#define SHARED_MEM_SIZE (8192)

#define CPU_CYCLES_PER_ITERATION (7000)

#define SCAN_SIZE_HALF (3)
#define SCAN_SIZE (7)
#define SCAN_SIZE_SQ (49)

#define SCAN_NOTHING (0)
#define SCAN_ENEMY (1)
#define SCAN_FOOD (2)
#define SCAN_FRIEND (3)

#define ITERATION_LIMIT (100000)