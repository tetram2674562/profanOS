#include <kernel/butterfly.h>
#include <minilib.h>
#include <type.h>


int fs_cnt_init_sector(vdisk_t *vdisk, sid_t sid, int type) {
    uint8_t *data;

    // check if sector unused
    if (vdisk_is_sector_used(vdisk, sid)) {
        kprintf("d%ds%d already used\n", sid.device, sid.sector);
        return 1;
    }

    vdisk_note_sector_used(vdisk, sid);

    data = calloc(SECTOR_SIZE);

    // add sector identifier
    data[0] = ST_CONT;
    data[1] = type;

    for (int i = 2; i < SECTOR_SIZE; i++) {
        data[i] = 0;
    }

    vdisk_write_sector(vdisk, sid, data);

    free(data);

    return 0;
}

#define fs_cnt_init_loca_in_sector(vdisk, sid) fs_cnt_init_sector(vdisk, sid, SF_LOCA)
#define fs_cnt_init_core_in_sector(vdisk, sid) fs_cnt_init_sector(vdisk, sid, SF_CORE)

sid_t fs_cnt_init(filesys_t *filesys, uint32_t device_id, char *meta) {
    vdisk_t *vdisk;
    sid_t main_sid;
    sid_t loca_sid;

    uint8_t *data;
    int ret_sect;

    main_sid.device = device_id;

    vdisk = fs_get_vdisk(filesys, main_sid.device);
    if (vdisk == NULL) {
        kprintf("d%d not found\n", main_sid.device);
        return NULL_SID;
    }

    // get unused sector for header
    ret_sect = vdisk_get_unused_sector(vdisk);
    if (ret_sect == -1) {
        kprintf("no more sectors in d%d\n", main_sid.device);
        return NULL_SID;
    }
    main_sid.sector = (uint32_t) ret_sect;
    vdisk_note_sector_used(vdisk, main_sid);

    // get unused sector for locator
    loca_sid.device = main_sid.device;
    ret_sect = vdisk_get_unused_sector(vdisk);
    if (ret_sect == -1) {
        kprintf("no more sectors in d%d\n", main_sid.device);
        vdisk_note_sector_unused(vdisk, main_sid);
        return NULL_SID;
    }
    loca_sid.sector = (uint32_t) ret_sect;

    // init locator
    if (fs_cnt_init_loca_in_sector(vdisk, loca_sid)) {
        kprintf("failed to init core\n");
        vdisk_note_sector_unused(vdisk, main_sid);
        vdisk_note_sector_unused(vdisk, loca_sid);
        return NULL_SID;
    }

    data = calloc(SECTOR_SIZE);

    // add sector identifier
    data[0] = ST_CONT;
    data[1] = SF_HEAD;

    for (int i = 2; i < SECTOR_SIZE; i++) {
        data[i] = 0;
    }

    // add meta and core sid
    mem_copy(data + 2, meta, min(str_len(meta), META_MAXLEN - 1));

    mem_copy(data + LAST_SID_OFFSET, &loca_sid, sizeof(sid_t));

    vdisk_write_sector(vdisk, main_sid, data);

    free(data);

    return main_sid;
}

char *fs_cnt_get_meta(filesys_t *filesys, sid_t sid) {
    vdisk_t *vdisk;
    uint8_t *data;
    char *meta;

    vdisk = fs_get_vdisk(filesys, sid.device);
    if (vdisk == NULL) {
        kprintf("d%d not found\n", sid.device);
        return NULL;
    }

    data = vdisk_load_sector(vdisk, sid);
    if (data == NULL) {
        kprintf("failed to read d%ds%d\n", sid.device, sid.sector);
        return NULL;
    }

    meta = calloc(META_MAXLEN);
    mem_copy(meta, data + 2, META_MAXLEN - 1);

    vdisk_unload_sector(vdisk, sid, data, NO_SAVE);

    return meta;
}

void fs_cnt_change_meta(filesys_t *filesys, sid_t sid, char *meta) {
    vdisk_t *vdisk;
    uint8_t *data;

    vdisk = fs_get_vdisk(filesys, sid.device);
    if (vdisk == NULL) {
        kprintf("d%d not found\n", sid.device);
        return;
    }

    data = vdisk_load_sector(vdisk, sid);
    if (data == NULL) {
        kprintf("failed to read d%ds%d\n", sid.device, sid.sector);
        return;
    }

    mem_copy(data + 2, meta, META_MAXLEN - 1);

    vdisk_unload_sector(vdisk, sid, data, SAVE);
}
