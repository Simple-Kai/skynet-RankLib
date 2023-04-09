#ifndef RANK_H
#define RANK_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "skynet_malloc.h"

#define DATA_TYPE int

typedef struct {
    uint32_t uid; 
    uint32_t rankPos;
    DATA_TYPE *data;
} Element;

typedef struct {
    Element **arr; 
    Element **arrTemp;
    Element **indexArr;     //索引数组  更新数据远比添加数据频繁

    uint32_t arrLen;    
    uint32_t cap;

    uint32_t compareNum;    // 排序依据数量
    uint8_t *compareSet;   // 排序依据，0：升序，1：降序
    bool needSort;
} Rank;

//不提供删除元素操作， 元素的增加和删除都是开销很大的，rank长度由业务层限制
Rank *RankNew(const uint8_t *compareSet, uint32_t compareNum);
void RankFree(Rank *rank);
void RankInsertOrUpdateByUid(Rank *rank, uint32_t uid, const DATA_TYPE *data);
void RankSort(Rank *rank);
uint32_t RankLength(Rank *rank);
int RankGetPosByUid(Rank *rank, uint32_t uid);
int RankGetElementByUid(Rank *rank, uint32_t uid, uint32_t *rankPos, DATA_TYPE *data);

#endif