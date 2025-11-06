#ifndef WFPERSISTANTGAMEDATA_H
#define WFPERSISTANTGAMEDATA_H

#define VECTOR(a) a*

#define WF_INVENTORY_ROW_SIZE 12
#define WF_INVENTORY_SIZE_INITIAL WF_INVENTORY_ROW_SIZE

struct WfPlayerPreferences
{
    float zoomLevel;
};

struct WfInventoryItem
{
    int itemIndex;
    int quantity;
};

struct WfInventory
{
    VECTOR(struct WfInventoryItem) pItems;
    int selectedItem;
};

void WfLoadPersistantDataFile(const char* path);

void WfSavePersistantDataFile(const char* path);

void WfNewSavePersistantData();

struct WfInventory* WfGetInventory();

struct WfPlayerPreferences* WfGetPreferences();

#endif