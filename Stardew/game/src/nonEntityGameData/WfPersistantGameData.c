#include "WfPersistantGameData.h"
#include "DynArray.h"
#include "WfItem.h"
#include "BinarySerializer.h"
#include "AssertLib.h"

struct WfPersistantData
{
    struct WfInventory inventory;
    struct WfPlayerPreferences preferences;
};

struct WfPersistantData gPersistantData;



void WfPersistantDataInit()
{
    gPersistantData.inventory.pItems = NEW_VECTOR(struct WfInventoryItem);
}

void WfLoadPersistantDataFileV1(struct BinarySerializer* pBS)
{
    BS_DeSerializeI32(&gPersistantData.inventory.selectedItem, pBS);
    u32 size = 0;
    BS_DeSerializeU32(&size, pBS);
    gPersistantData.inventory.pItems = VectorResize(gPersistantData.inventory.pItems, size);
    gPersistantData.inventory.pItems = VectorClear(gPersistantData.inventory.pItems);
    for(int i = 0; i < size; i++)
    {
        struct WfInventoryItem item;
        BS_DeSerializeI32(&item.itemIndex, pBS);
        BS_DeSerializeI32(&item.quantity, pBS);
        gPersistantData.inventory.pItems = VectorPush(gPersistantData.inventory.pItems, &item);
    }

    BS_DeSerializeFloat(&gPersistantData.preferences.zoomLevel, pBS);
}

void WfLoadPersistantDataFile(const char* path)
{
	struct BinarySerializer bs;
    BS_CreateForLoad(path, &bs);
    u32 version = 0;
    BS_DeSerializeU32(&version, &bs);
    switch (version)
    {
    case 1:
        WfLoadPersistantDataFileV1(&bs);
        break;
    default:
        EASSERT(false);
        break;
    }

    BS_Finish(&bs);
}

void WfSavePersistantDataFile(const char* path)
{
    struct BinarySerializer bs;
    BS_CreateForSave(path, &bs);

    BS_SerializeU32(1, &bs); /* version */

    /* Inventory */

    BS_SerializeI32(gPersistantData.inventory.selectedItem, &bs);

    int sz = VectorSize(gPersistantData.inventory.pItems);
    BS_SerializeU32(sz, &bs);

    for(int i=0; i<VectorSize(gPersistantData.inventory.pItems); i++)
    {
        BS_SerializeI32(gPersistantData.inventory.pItems[i].itemIndex, &bs);
        BS_SerializeI32(gPersistantData.inventory.pItems[i].quantity, &bs);
    }

    /* Preferences */

    BS_SerializeFloat(gPersistantData.preferences.zoomLevel, &bs);

    BS_Finish(&bs);
}

void WfNewSavePersistantData()
{
    /* Inventory */
    gPersistantData.inventory.pItems = VectorResize(gPersistantData.inventory.pItems, WF_INVENTORY_SIZE_INITIAL);
    gPersistantData.inventory.pItems = VectorClear(gPersistantData.inventory.pItems);
    for(int i=0; i<WF_INVENTORY_SIZE_INITIAL; i++)
    {
        struct WfInventoryItem itm;
        itm.itemIndex = -1;
        itm.quantity = 0;
        gPersistantData.inventory.pItems = VectorPush(gPersistantData.inventory.pItems, &itm);
    }

    gPersistantData.inventory.pItems[0].itemIndex = WfBasicAxe;
    gPersistantData.inventory.pItems[0].quantity = 1;

    gPersistantData.inventory.pItems[1].itemIndex = WfBasicPickAxe;
    gPersistantData.inventory.pItems[1].quantity = 1;

    gPersistantData.inventory.pItems[2].itemIndex = WfBasicHoe;
    gPersistantData.inventory.pItems[2].quantity = 1;

    gPersistantData.inventory.selectedItem = 0;

    /* Preferences */
    gPersistantData.preferences.zoomLevel = 1.0f;
    
}

struct WfInventory* WfGetInventory()
{
    return &gPersistantData.inventory;
}

struct WfPlayerPreferences* WfGetPreferences()
{
    return &gPersistantData.preferences;
}
