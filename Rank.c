#include "Rank.h"

#define INIT_LENGTH 1 << 6

static Element *
ElementNew(uint32_t uid, const DATA_TYPE *data, uint32_t dataNum) {
    Element *elem = (Element *)MALLOC(sizeof(Element));
    memset(elem, 0, sizeof(Element));
    elem->uid = uid;

    DATA_TYPE *temp = MALLOC(dataNum * sizeof(DATA_TYPE));
    memcpy(temp, data, dataNum * sizeof(DATA_TYPE));
    elem->data = temp;
    return elem;
}

static void 
ElementFree(Element *elem) {
    FREE(elem->data);
    FREE(elem);
}

Rank *
RankNew(const uint8_t *compareSet, uint32_t compareNum) {
    uint32_t cap = INIT_LENGTH;
    Rank *rank = (Rank *)MALLOC(sizeof(Rank));
    memset(rank, 0, sizeof(Rank));

    rank->arr = (Element **)MALLOC(cap * sizeof(Element *));
    memset(rank->arr, 0, cap * sizeof(Element *));

    rank->arrTemp = (Element **)MALLOC(cap * sizeof(Element *));
    memset(rank->arrTemp, 0, cap * sizeof(Element *));

    rank->indexArr = (Element **)MALLOC(cap * sizeof(Element *));
    memset(rank->indexArr, 0, cap * sizeof(Element *));

    rank->arrLen = 0;
    rank->cap = cap;

    rank->compareSet = (uint8_t *)MALLOC(compareNum * sizeof(uint8_t));
    memcpy(rank->compareSet, compareSet, compareNum * sizeof(uint8_t));
    rank->compareNum = compareNum;

    rank->needSort = false;
    return rank;
}

void 
RankFree(Rank *rank) {
    if (rank == NULL) {
        return;
    }
    FREE(rank->compareSet);
    FREE(rank->indexArr);
    FREE(rank->arrTemp);
    for (int i = 0; i < rank->arrLen; ++i) {
        ElementFree(rank->arr[i]);
        rank->arr[i] = NULL;
    }
    FREE(rank->arr);
    FREE(rank);
}

static void
CheckRankResize(Rank *rank) {
    if (rank->arrLen == rank->cap) {
        uint32_t newCap = rank->cap * 2;
        uint32_t elemPtrSize = sizeof(Element *);

        Element **newArr = (Element **)MALLOC(newCap * elemPtrSize);
        memcpy(newArr, rank->arr, rank->cap * elemPtrSize);
        memset(newArr + rank->cap, 0, (newCap - rank->cap) * elemPtrSize);
        FREE(rank->arr);
        rank->arr = newArr;

        Element **newIndexArr = (Element **)MALLOC(newCap * elemPtrSize);
        memcpy(newIndexArr, rank->indexArr, rank->cap * elemPtrSize);
        memset(newIndexArr + rank->cap, 0, (newCap - rank->cap) * elemPtrSize);
        FREE(rank->indexArr);
        rank->indexArr = newIndexArr;

        Element **newArrTemp = (Element **)MALLOC(newCap * elemPtrSize);
        memset(newArrTemp, 0, newCap * elemPtrSize);
        FREE(rank->arrTemp);
        rank->arrTemp = newArrTemp;

        rank->cap = newCap;
    }
}

inline static void
RankAddUidIndex(Rank *rank, uint32_t uid, Element *elem) {
    Element **indexArr = rank->indexArr;
    int i = rank->arrLen - 1;
    while (i >= 0 && uid < indexArr[i]->uid) {
        indexArr[i + 1] = indexArr[i];
        --i;
    }
    indexArr[i + 1] = elem;
}

static int
BinarySearch(Rank *rank, uint32_t uid){
    Element **indexArr = rank->indexArr;
    int left = 0, right = rank->arrLen - 1, mid;
    while (left <= right) {
        mid = (left + right) / 2;
        if (indexArr[mid]->uid == uid) {
            return mid;
        } else if (indexArr[mid]->uid < uid) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    return -1;

}

static Element *
RankFindElementByUid(Rank *rank, uint32_t uid) {
    int index = BinarySearch(rank, uid);
    if (index == -1){
        return NULL;
    }
    return rank->indexArr[index];
}

static void 
RankAddElement(Rank *rank, uint32_t uid, const DATA_TYPE *data) {
    CheckRankResize(rank);
    Element *elem = ElementNew(uid, data, rank->compareNum);
    rank->arr[rank->arrLen] = elem;
    RankAddUidIndex(rank, uid, elem);
    ++rank->arrLen;
    rank->needSort = true;
}

static void
RankUpdateElement(Rank *rank, Element *elem, const DATA_TYPE *newData){
    uint32_t dataSize = rank->compareNum * sizeof(DATA_TYPE);
    if (0 == memcmp(newData, elem->data, dataSize)){
        return; 
    }
    memcpy(elem->data, newData, dataSize);
    rank->needSort = true;
}

void
RankInsertOrUpdateByUid(Rank *rank, uint32_t uid, const DATA_TYPE *data) {
    Element *elem = RankFindElementByUid(rank, uid);
    if (elem) {
        RankUpdateElement(rank, elem, data);
    }
    else{
        RankAddElement(rank, uid, data);
    }
}

static int 
RankCompare(Rank *rank, Element *front, Element *back) {
    for (int i = 0; i < rank->compareNum; ++i) {
        if (front->data[i] == back->data[i]) {
            continue;
        }
        if (rank->compareSet[i] == 0) {
            return front->data[i] - back->data[i];
        } else {
            return back->data[i] - front->data[i];
        }
    }
    return 0;
}

static void 
Merge(Rank *rank, Element **tmp, int left, int mid, int right) {
    Element **arr = rank->arr;
    int i = left, j = mid + 1, k = left;
    while (i <= mid && j <= right) {
        if (RankCompare(rank, arr[i], arr[j]) <= 0) {
            tmp[k++] = arr[i++];
        } else {
            tmp[k++] = arr[j++];
        }
    }
    while (i <= mid) {
        tmp[k++] = arr[i++];
    }
    while (j <= right) {
        tmp[k++] = arr[j++];
    }
    for (i = left; i <= right; ++i) {
        arr[i] = tmp[i];
        arr[i]->rankPos = i + 1;
    }
}

#include <stdio.h>
static void 
MergeSortIteration(Rank *rank) {
    uint32_t len = rank->arrLen;
    int size, left, mid, right;
    Element **tmp = rank->arrTemp;
    for (size = 1; size < len; size *= 2) {
        for (left = 0; left < len - size; left = right + 1) {
            mid = left + size - 1;
            right = mid + size < len ? mid + size : len - 1;
            Merge(rank, tmp, left, mid, right);
        }
    }
    memset(rank->arrTemp, 0, rank->cap * sizeof(Element *));
}

void 
RankSort(Rank *rank) {
    if (!rank->needSort) {
        return;
    }
    MergeSortIteration(rank);
    rank->needSort = false;
}

uint32_t 
RankLength(Rank *rank) {
    return rank->arrLen;
}

int 
RankGetPosByUid(Rank *rank, uint32_t uid){
    RankSort(rank);
    Element *elem = RankFindElementByUid(rank, uid);
    if (!elem) {
        return -1;
    }
    return elem->rankPos;
}

const Element * 
RankGetElementByUid(Rank *rank, uint32_t uid){
    RankSort(rank);
    return RankFindElementByUid(rank, uid);
}

const Element * 
RankGetElementByPos(Rank *rank, int pos){
    if (pos < 1 || pos > rank->arrLen) {
        return NULL;
    }
    RankSort(rank);
    return rank->arr[pos - 1];
}