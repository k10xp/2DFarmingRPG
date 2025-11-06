#ifndef WFPERSISTANTGAMEDATA_H
#define WFPERSISTANTGAMEDATA_H

#define VECTOR(a) a*

#define WF_INVENTORY_ROW_SIZE 12
#define WF_INVENTORY_SIZE_INITIAL WF_INVENTORY_ROW_SIZE


struct WfInventoryItem
{
    int itemIndex;
    int quantity;
};

struct WfInventory
{
    VECTOR(struct WfInventoryItem) pItems;
};

void WfLoadPersistantDataFile(const char* path);

void WfSavePersistantDataFile(const char* path);

void WfNewSavePersistantData();

struct WfInventory* WfGetInventory();

#endif