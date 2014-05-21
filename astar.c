#include <time.h>   /* only refer: (rand)*/
#include <stdlib.h> /* only refer: (malloc, free) */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// https://stackoverflow.com/questions/822323/how-to-generate-a-random-number-in-c/14598879#14598879
int32_t randNum(int32_t minNum, int32_t maxNum) {
  int32_t result = 0, lowNum = 0, hiNum = 0;
  if (minNum < maxNum) {
    lowNum = minNum;
    hiNum = maxNum + 1;
  } else {
    lowNum = maxNum + 1;
    hiNum = minNum;
  }
  srand((unsigned)time(NULL));
  result = (rand() % (hiNum - lowNum)) + lowNum;
  return result;
}

typedef struct Point {
  int32_t x;
  int32_t y;
} Point;

typedef struct Grid {
  Point position;
  bool isBeWalkable;
} Grid;

Grid *newGrid(Point pos, bool isBeWalkable) {
  Grid *grid;
  grid = (Grid *)malloc(sizeof(Grid));
  grid->position = pos;
  grid->isBeWalkable = isBeWalkable;
  return grid;
}

typedef struct GridList {
  Grid *grid;
  struct GridList *next;
} GridList;

typedef struct PathList {
  Point pos;
  struct PathList *next;
} PathList;

typedef struct Map {
  int32_t width;
  int32_t height;
  Grid **grids;
  bool (*inMapP)(struct Map *map, Point pos);
  Grid *(*gridth)(struct Map *map, Point pos);
  GridList *(*nearbyGrids)(struct Map *map, Point pos);
  PathList *(*astarFindPath)(struct Map *map, Point srcPos, Point destPos);
} Map;

typedef struct AstarNode {
  Point pos;
  int32_t f;
  int32_t g;
  struct AstarNode *parenNode;
} AstarNode;

AstarNode *newAstarNode(Point pos, int32_t f, int32_t g, AstarNode *parenNode) {
  AstarNode *node;
  node = (AstarNode *)malloc(sizeof(AstarNode));
  node->pos = pos;
  node->f = f;
  node->g = g;
  node->parenNode = parenNode;
  return node;
}

typedef struct AstarList {
  AstarNode *node;
  struct AstarList *next;
} AstarList;

AstarList *newAstarList(AstarNode *node) {
  AstarList *alist;
  alist = (AstarList *)malloc(sizeof(AstarList));
  alist->node = node;
  alist->next = NULL;
  return alist;
}

int32_t astarG(Point srcPos, AstarNode *minNode) {
  bool isDiagonal = (srcPos.x != minNode->pos.x || srcPos.y != minNode->pos.y);
  int32_t g = isDiagonal ? 14 : 10;
  return g + minNode->g;
}

int32_t astarH(Point srcPos, Point destPos) {
  // Manhattan way
  return abs(srcPos.x - destPos.x) + abs(srcPos.y - destPos.y);
}

AstarList *appendAstarNode(AstarList *alist, AstarNode *anode) {
  AstarList *newAlist = newAstarList(anode);
  if (alist == NULL) {
    alist = newAlist;
    return alist;
  } else if (alist->node == NULL) {
    alist->node = anode;
    return alist;
  } else {
    AstarList *tmp = alist;
    while (tmp->next != NULL) {
      tmp = tmp->next;
    }
    tmp->next = newAlist;
    return alist;
  }
  free(newAlist);
  return NULL;
}

AstarNode *minFvalAstarNode(AstarList *alist) {
  if (alist == NULL || alist->node == NULL) {
    return NULL;
  }
  AstarList *tmpList;
  tmpList = alist;
  AstarNode *minNode;
  minNode = alist->node;
  int32_t minF = 0;
  int32_t tmpF = 0;
  while (tmpList && tmpList->node) {
    minF = minNode->f;
    tmpF = tmpList->node->f;
    if (minF > tmpF) {
      minF = tmpF;
      minNode = tmpList->node;
    }
    tmpList = tmpList->next;
  }
  return minNode;
}

bool equalPoint(Point a, Point b) { return (a.x == b.x && a.y == b.y); }

AstarNode *findNodeByPos(AstarList *alist, Point pos) {
  if (alist->node == NULL) {
    return NULL;
  }
  AstarList *tmp;
  tmp = alist;
  while (tmp) {
    if (equalPoint(tmp->node->pos, pos)) {
      return tmp->node;
    }
    tmp = tmp->next;
  }
  return NULL;
}

AstarList *clearAstarList(AstarList *alist) {
  AstarList *tmp, *prev;
  alist->node = NULL;
  tmp = alist->next;
  prev = NULL;
  while (tmp) {
    prev = tmp;
    tmp = tmp->next;
    free(prev->node);
    free(prev);
  }
  alist->node = NULL;
  alist->next = NULL;
  return alist;
}

PathList *reversePathList(PathList *plist) {
  PathList *prev = NULL;
  while (plist) {
    PathList *tmp = plist;
    plist = plist->next;
    tmp->next = prev;
    prev = tmp;
  }
  return prev;
}

PathList *astarFindPath(Map *map, Point srcPos, Point destPos) {
  if (equalPoint(srcPos, destPos) || map->gridth(map, destPos) == NULL ||
      map->gridth(map, srcPos) == NULL ||
      !map->gridth(map, srcPos)->isBeWalkable ||
      !map->gridth(map, destPos)->isBeWalkable) {
    return NULL;
  }
  AstarList *openList, *closeList;
  AstarNode *srcNode = newAstarNode(srcPos, 0, 0, NULL);
  AstarNode *minNode;
  GridList *nbyGrids, *tmpGlist;
  int32_t f, g, h;
  Point pos;
  AstarNode *foundNode;
  openList = newAstarList(srcNode);
  closeList = malloc(sizeof(AstarList));
  tmpGlist = NULL;
  minNode = NULL;
  f = 0;
  g = 0;
  h = 0;
  do {
    printf("openList %d, %d\n", openList->node->pos.x, openList->node->pos.y);
    minNode = minFvalAstarNode(openList);
    printf("minNode pos %d, %d, f: %d\n", minNode->pos.x, minNode->pos.y,
           minNode->f);
    appendAstarNode(closeList, minNode);
    /* clearAstarList(openList); */
    openList->node = NULL;
    openList->next = NULL;
    nbyGrids = map->nearbyGrids(map, minNode->pos);
    tmpGlist = nbyGrids;
    GridList *prevGlist;
    prevGlist = tmpGlist;
    while (tmpGlist != NULL) {
      if (findNodeByPos(closeList, tmpGlist->grid->position) ||
          !(tmpGlist->grid->isBeWalkable)) {
        prevGlist->next = tmpGlist->next;
      }
      prevGlist = tmpGlist;
      tmpGlist = tmpGlist->next;
    }
    /* printf("%d\n", nearbyGrids->next->next->grid->position.y); */
    tmpGlist = nbyGrids;
    while (tmpGlist != NULL) {  // each nearbyGrids
      pos = tmpGlist->grid->position;
      g = astarG(pos, minNode);
      h = astarH(pos, destPos);
      f = g + h;
      foundNode = findNodeByPos(openList, pos);
      if (foundNode) {
        if (g < foundNode->g) {
          foundNode->parenNode = minNode;
          foundNode->g = g;
          foundNode->f = g + h;
        }
      } else {
        appendAstarNode(openList, newAstarNode(pos, f, g, minNode));
      }
      // next nearbyGrid
      tmpGlist = tmpGlist->next;
    }
    printf("equalPoint? %d\n", equalPoint(minNode->pos, destPos));
    if (openList->node == NULL) {
      printf("open list is NULL\n");
    }
    printf("Keep? %d\n",
           !(equalPoint(minNode->pos, destPos) || openList->node == NULL));
  } while (!(equalPoint(minNode->pos, destPos) || openList->node == NULL));
  AstarNode *tmpNode = minNode;
  /* PathList *path = malloc(sizeof(PathList)); */
  PathList *path = malloc(sizeof(PathList));
  PathList *tmpPath;
  tmpPath = path;
  while (tmpNode != srcNode) {
    printf("collect path\n");
    tmpPath->pos = tmpNode->pos;
    tmpPath->next = (PathList *)malloc(sizeof(PathList));
    tmpPath = tmpPath->next;
    tmpNode = tmpNode->parenNode;
  };
  printf("reversing path\n");
  path = reversePathList(path);
  path->pos = srcPos;
  return path;
}

Grid *gridth(Map *map, Point pos) {
  if (map->inMapP(map, pos)) {
    return &map->grids[pos.y][pos.x];
  } else {
    return NULL;
  }
}

bool inMapP(Map *map, Point pos) {
  int32_t x, y;
  x = pos.x;
  y = pos.y;
  if (x >= map->width || y >= map->height || x < 0 || y < 0) {
    return false;
  }
  return true;
}

GridList *newGridList(Grid *grid) {
  GridList *glist;
  glist = (GridList *)malloc(sizeof(GridList));
  glist->grid = grid;
  glist->next = NULL;
  return glist;
}

GridList *appendGrid(GridList *glist, Grid *grid) {
  GridList *newGlist = newGridList(grid);
  if (glist == NULL) {
    glist = newGlist;
  } else {
    GridList *tmp = glist;
    while (tmp->next) {
      tmp = tmp->next;
    }
    tmp->next = newGlist;
  }
  return glist;
}

GridList *nearbyGrids(Map *map, Point pos) {
  int32_t directions[8][2] = {
      {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}};
  if (!map->inMapP(map, pos)) {
    return NULL;
  }
  GridList *glist, *tmp, *cur;
  glist = NULL;
  Point curPos;
  int i;
  for (i = 0; i < 8; i++) {
    curPos.x = pos.x + directions[i][0];
    curPos.y = pos.y + directions[i][1];
    if (map->inMapP(map, curPos)) {
      cur = newGridList(map->gridth(map, curPos));
      if (glist == NULL) {
        glist = cur;
        tmp = glist;
        continue;
      } else {
        tmp->next = cur;
        tmp = cur;
      }
    }
  }
  return glist;
}

Map newMap(int32_t width, int32_t height) {
  Grid **grids = malloc(sizeof(Grid *) * height);
  Grid *temp = malloc(sizeof(Grid) * width * height);
  int i;
  for (i = 0; i < width; i++) {
    grids[i] = temp + (i * width);
  }
  int ix, iy;
  for (ix = 0; ix < width; ix++) {
    for (iy = 0; iy < height; iy++) {
      grids[iy][ix].position = (Point) {ix, iy};
      grids[iy][ix].isBeWalkable = true;
    }
  }
  return (Map) {width,  height,      grids,        inMapP,
                gridth, nearbyGrids, astarFindPath};
};

typedef struct Guard {
  char *name;
  Map *map;
  Point position;
  bool (*move)(struct Guard *self, Point pos);
  /* Grid* (*move)(struct Guard *self, Point pos); */
  Map *(*joinMap)(struct Guard *self, Map *map);
} Guard;

Map *joinMap(Guard *self, Map *map) {
  self->map = map;
  return map;
}

bool move(Guard *guard, Point destPos) {
  Map *curMap = guard->map;
  if (guard->map == NULL) {
    return false;
  }
  PathList *path, *tmpPath;
  Point curPos;
  path = curMap->astarFindPath(curMap, guard->position, destPos);
  tmpPath = path;
  while (tmpPath) {
    curPos = tmpPath->pos;
    guard->position = curPos;
    printf("%s move to ", guard->name);
    printf("x: %d, y: %d\n", curPos.x, curPos.y);
    tmpPath = tmpPath->next;
  }
  free(path);
  return true;
}

Guard newGuard(char *name, Point position) {
  return (Guard) {name, NULL, position, move, joinMap};
}

typedef struct RectWall {
  // fixed use 4 point
  // lazy use dynamic array for dynamic size
  Point area[4];
} RectWall;

int main() {
  srand((unsigned)time(NULL));
  int i, j = 0;
  int32_t mapWidth = 20;
  int32_t mapHeight = 20;
  Map map = newMap(mapWidth, mapHeight);
  Guard guard = newGuard("wiwi", (Point) {0, 0});
  guard.joinMap(&guard, &map);

  int32_t rectWallCount = randNum(3, 5);
  printf("rectWallCount: %d\n", rectWallCount);
  RectWall *rectWalls = malloc(sizeof(RectWall) * rectWallCount);
  for (i = 0; i < rectWallCount; i++) {
    rectWalls[i].area[0].x = randNum(i, mapWidth - 1);
    rectWalls[i].area[0].y = randNum(i, mapHeight - 1);
    rectWalls[i].area[1].x = rectWalls[i].area[0].x + 1;
    rectWalls[i].area[1].y = rectWalls[i].area[0].y;
    rectWalls[i].area[2].x = rectWalls[i].area[0].x + 1;
    rectWalls[i].area[2].y = rectWalls[i].area[0].y + 1;
    rectWalls[i].area[3].x = rectWalls[i].area[0].x;
    rectWalls[i].area[3].y = rectWalls[i].area[0].y + 1;
    for (j = 0; j < 4; j++) {
      if (map.gridth(&map, rectWalls[i].area[j]) != NULL) {
        printf("rect wall: %d, %d\n",
               map.gridth(&map, rectWalls[i].area[j])->position.x,
               map.gridth(&map, rectWalls[i].area[j])->position.y);
        map.gridth(&map, rectWalls[i].area[j])->isBeWalkable = false;
      }
    }
  }
  free(rectWalls);

  PathList *plist = map.astarFindPath(&map, (Point) {0, 0}, (Point) {4, 4});

  PathList *tmpPlist = plist;
  while (tmpPlist) {
    printf("Path %d, %d\n", tmpPlist->pos.x, tmpPlist->pos.y);
    tmpPlist = tmpPlist->next;
  }

  /* int32_t pathSize = (sizeof(path) / sizeof(Point)); */
  /* for (i = 0; i < pathSize; i++) { */
  /*   guard.move(&guard, path[i]); */
  /* } */
  Point path[] = {{10, 10}, {5, 5}, {15, 15}};
  map.gridth(&map, path[1])->isBeWalkable = false;
  guard.move(&guard, path[0]);
  map.gridth(&map, path[0])->isBeWalkable = false;
  map.gridth(&map, path[1])->isBeWalkable = true;
  guard.move(&guard, path[1]);
  map.gridth(&map, path[1])->isBeWalkable = false;
  guard.move(&guard, path[2]);
  map.gridth(&map, path[2])->isBeWalkable = false;

  printf("x: %d, y: %d\n", map.grids[2][3].position.x,
         map.grids[2][3].position.y);
  GridList *grids = map.nearbyGrids(&map, (Point) {9, 9});
  GridList *cur = grids;
  int size = 0;
  while (cur != NULL) {
    printf("nearbyGrids %d, %d\n", cur->grid->position.x,
           cur->grid->position.y);
    size++;
    cur = cur->next;
  }
  printf("size %d\n", size);
  return 0;
}
